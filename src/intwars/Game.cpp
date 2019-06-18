/*
IntWars playground server for League of Legends protocol testing
Copyright (C) 2012  Intline9 <Intline9@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <chrono>
#include <thread>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include "stdafx.h"
#include "Config.h"
#include "Game.h"
#include "LuaScript.h"
#include "SummonersRift.h"
#include "Logger.h"


#define REFRESH_RATE 16666 // 60 fps

uint32 GetNewNetID() {
	static uint32 dwStart = 0x40000001;
	return dwStart++;
}

Game::Game() : _started(false), playersReady(0)
{

}

Game::~Game()
{
	_isAlive = false;

	delete _blowfish;

   if (_server != NULL)
	   enet_host_destroy(_server);
}

uint32 Game::strToId(std::string str){
    if(str == "FLASH"){
        return SPL_Flash;
    }else if(str == "IGNITE"){
        return SPL_Ignite;
    }else if(str == "HEAL"){
        return SPL_Heal;
    }else if(str == "BARRIER"){
        return SPL_Barrier;
    }else if(str == "SMITE"){
        return SPL_Smite;
    }else if(str == "GHOST"){
        return SPL_Ghost;
    }else if(str == "REVIVE"){
        return SPL_Revive;
    }else if(str == "CLEANSE"){
        return SPL_Cleanse;
    }else if(str == "TELEPORT"){
        return SPL_Teleport;
    }

    return 0;
}

template<typename T>
std::string toString(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

bool Game::initialize(ENetAddress *address, const char *baseKey){
    if (enet_initialize () != 0)
        return false;
    atexit(enet_deinitialize);

    _server = enet_host_create(address, 32, 0, 0);
    if(_server == NULL)
        return false;

    std::string key = base64_decode(baseKey);
    if(key.length() <= 0)
        return false;

    _blowfish = new BlowFish((uint8*)key.c_str(), 16);
    initHandlers();

   map = new SummonersRift(this);

   //TODO: better lua implementation

   LuaScript script(false);

   //script.loadScript(Config::instance().luadir + "config.lua");
	script.loadScript(Config::instance().config_File);


  //  sol::state lua;
  //  lua.open_libraries(sol::lib::base, sol::lib::table);

  //  lua.open_file(Config::instance().luadir + "config.lua");
    sol::table playerList = script.getTable("players");
    for (int i=1;i<12;i++) {
        try {
            std::string playerIndex = "player"+toString(i);

            sol::table playerData = playerList.get<sol::table>(playerIndex);

            std::string rank = playerData.get<std::string>("rank");
            std::string name = playerData.get<std::string>("name");
            std::string champion = playerData.get<std::string>("champion");
            std::string team = playerData.get<std::string>("team");
            int skin = playerData.get<int>("skin");
            int ribbon = playerData.get<int>("ribbon");
            int icon = playerData.get<int>("icon");
            std::string summoner1 = playerData.get<std::string>("summoner1");
            std::string summoner2 = playerData.get<std::string>("summoner2");

            ClientInfo* player = new ClientInfo(rank, ((team == "BLUE") ? TEAM_BLUE : TEAM_PURPLE), ribbon, icon);

           player->setName(name);


		   player->setSkinNo(skin);
		   static int id = 1;
		   player->userId = id; // same as StartClient.bat
		   id++;
		   player->setSummoners(strToId(summoner1), strToId(summoner2));
           Champion* c = ChampionFactory::getChampionFromType(champion, map, GetNewNetID(), player->userId);
		   float respawnX, respawnY;
		   std::tie(respawnX, respawnY) = c->getRespawnPosition();
           c->setPosition(respawnX, respawnY);
           c->setTeam((team == "BLUE") ? 0 : 1);
           c->levelUp();


           player->setChampion(c);




           players.push_back(player);

        } catch(sol::error e) {
            //CORE_ERROR("Error loading champion: %s", e.what());
            break;
        }
    }

   // Uncomment the following to get 2-players
   /*ClientInfo* player2 = new ClientInfo("GOLD", TEAM_PURPLE);
   player2->setName("tseT");
   Champion* c2 = ChampionFactory::getChampionFromType("Ezreal", map, GetNewNetID());
   c2->setPosition(100.f, 273.55f);
   c2->setTeam(1);
   map->addObject(c2);
   player2->setChampion(c2);
   player2->setSkinNo(4);
   player2->userId = 2; // same as StartClient.bat
   player2->setSummoners(SPL_Ignite, SPL_Flash);

   players.push_back(player2);*/

	return _isAlive = true;
}

void Game::netLoop()
{
	ENetEvent event;
	std::chrono::time_point<std::chrono::high_resolution_clock> tStart, tEnd;
	tStart = std::chrono::high_resolution_clock::now();
	long long tDiff;

	while(true)
	{
      while(enet_host_service(_server, & event, 0) > 0) {
         switch (event.type)
         {
         case ENET_EVENT_TYPE_CONNECT:
            //Logging->writeLine("A new client connected: %i.%i.%i.%i:%i", event.peer->address.host & 0xFF, (event.peer->address.host >> 8) & 0xFF, (event.peer->address.host >> 16) & 0xFF, (event.peer->address.host >> 24) & 0xFF, event.peer->address.port);

            /* Set some defaults */
            event.peer->mtu = PEER_MTU;
            event.data = 0;

            break;

         case ENET_EVENT_TYPE_RECEIVE:
            currentPeer = event.peer;
            if(!handlePacket(event.peer, event.packet,event.channelID))
            {
               //enet_peer_disconnect(event.peer, 0);
            }

            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy (event.packet);
            break;

         case ENET_EVENT_TYPE_DISCONNECT:
            handleDisconnect(event.peer);
            delete (ClientInfo*)event.peer->data;
            break;
         }
      }
      tEnd = tStart;
	   tStart = std::chrono::high_resolution_clock::now();
	   tDiff = std::chrono::duration_cast<std::chrono::microseconds>(tStart - tEnd).count();

      if(_started) {
         map->update(tDiff);
      }

      tEnd = std::chrono::high_resolution_clock::now();
      if(tEnd-(std::chrono::microseconds(REFRESH_RATE)) < tStart) {
         std::this_thread::sleep_for(std::chrono::microseconds(REFRESH_RATE)-(tEnd-tStart));
      }
   }
}
