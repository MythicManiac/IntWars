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
#include "stdafx.h"
#include "Config.h"
#include "Game.h"
#include "Packets.h"
#include "ItemManager.h"
#include "ChatBox.h"
#include "LuaScript.h"
#include "Logger.h"

#include <vector>
#include <string>
#include <sstream>



using namespace std;

bool Game::handleNull(HANDLE_ARGS) {
    return true;
}

bool Game::handleKeyCheck(ENetPeer *peer, ENetPacket *packet) {
   KeyCheck *keyCheck = (KeyCheck *)packet->data;
   uint64 userId = _blowfish->Decrypt(keyCheck->checkId);

   if(userId != keyCheck->userId) {
      return false;
   }

   uint32 playerNo = 0;

   for(ClientInfo* player : players) {
      if(player->userId == userId) {

         if (player->getPeer() != NULL) {
            CORE_WARNING("Ignoring new player %d, already connected!", userId);
            return false;
         }

         peer->data = player;
         player->setPeer(peer);
         KeyCheck response;
         response.userId = keyCheck->userId;
         response.playerNo = playerNo;
         bool bRet = sendPacket(peer, reinterpret_cast<uint8 *>(&response), sizeof(KeyCheck), CHL_HANDSHAKE);
         handleGameNumber(peer, NULL);//Send 0x91 Packet?
         return true;
      }
      ++playerNo;
   }

   return false;

}

bool Game::handleGameNumber(ENetPeer *peer, ENetPacket *packet) {
    WorldSendGameNumber world;
    world.gameId = 1;
    memcpy(world.data, peerInfo(peer)->getName().c_str(), peerInfo(peer)->getName().length());
    return sendPacket(peer, reinterpret_cast<uint8 *>(&world), sizeof(WorldSendGameNumber), CHL_S2C);
}

bool Game::handleSynch(ENetPeer *peer, ENetPacket *packet) {
   SynchVersion *version = reinterpret_cast<SynchVersion *>(packet->data);
   //Logging->writeLine("Client version: %s", version->version);
   //Gets the map from the lua configuration file.
   LuaScript script(false);
   script.loadScript(Config::instance().config_File);
   sol::table config = script.getTable("game");
   int mapId = config.get<int>("map");
   CORE_INFO("Current map: %i", mapId);

   bool versionMatch = true;
   // Version might be an invalid value, currently it trusts the client
   if (strncmp(version->getVersion(), GAME_VERSION, 256) != 0) {
      versionMatch = false;
      CORE_WARNING("Client %s does not match Server %s", version->getVersion(), GAME_VERSION);
   } else {
      CORE_INFO("Accepted client version (%s)", version->getVersion());
   }

   for(ClientInfo* player : players) {
      if(player->getPeer() == peer) {
         player->setVersionMatch(versionMatch);
         break;
      }
   }

   SynchVersionAns answer(players, GAME_VERSION, "CLASSIC", (uint32)mapId);
   printPacket(reinterpret_cast<uint8 *>(&answer), sizeof(answer));
   return sendPacket(peer, answer, 3);
}

bool Game::handleMap(ENetPeer *peer, ENetPacket *packet) {
    //Builds team info
    LoadScreenInfo screenInfo(players);
    bool pInfo = sendPacket(peer, screenInfo, CHL_LOADING_SCREEN);

    //For all players send this info
    bool bOk = false;
    for (ClientInfo* player : players) {
       LoadScreenPlayerName loadName(*player);
       LoadScreenPlayerChampion loadChampion(*player);
       bool pName = sendPacket(peer, loadName, CHL_LOADING_SCREEN);
       bool pHero = sendPacket(peer, loadChampion, CHL_LOADING_SCREEN);

       bOk = (pName && pHero);

       if (!bOk)
          break;
    }

    return (pInfo && bOk);
}

//building the map
bool Game::handleSpawn(ENetPeer *peer, ENetPacket *packet) {
   StatePacket2 start(PKT_S2C_StartSpawn);
   bool p1 = sendPacket(peer, reinterpret_cast<uint8 *>(&start), sizeof(StatePacket2), CHL_S2C);
   CORE_INFO("Spawning map");

   int playerId = 0;
   ClientInfo* playerInfo = 0;

   for(auto p : players) {
      if (p->getPeer() == peer) {
         playerInfo = p;
      }

      HeroSpawn spawn(p, playerId++);
      sendPacket(peer, spawn, CHL_S2C);

      PlayerInfo info(p);
      sendPacket(peer, info, CHL_S2C);

      p->getChampion()->getStats().setSummonerSpellEnabled(0, true);
      p->getChampion()->getStats().setSummonerSpellEnabled(1, false);

      // TODO: Recall slot
   }

   const std::map<uint32, Object*>& objects = map->getObjects();

   for(auto kv : objects) {
      Turret* t = dynamic_cast<Turret*>(kv.second);
      if(t) {
         TurretSpawn turretSpawn(t);
         sendPacket(peer, turretSpawn, CHL_S2C);

         // To suppress game HP-related errors for enemy turrets out of vision
         SetHealth sh(t);
         sendPacket(peer, sh, CHL_S2C);
         continue;
      }

      LevelProp* lp = dynamic_cast<LevelProp*>(kv.second);
      if(lp) {
         LevelPropSpawn lpsPacket(lp);
         sendPacket(peer, lpsPacket, CHL_S2C);
      }
   }

   // Level props are just models, we need button-object minions to allow the client to interact with it
   if (playerInfo != 0 && playerInfo->getTeam() == TEAM_BLUE) {
      // Shop (blue team)
      MinionSpawn ms1(0xff10c6db);
      sendPacket(peer, ms1, CHL_S2C);
      SetHealth sh1(0xff10c6db);
      sendPacket(peer, sh1, CHL_S2C);

      // Vision for hardcoded objects
      // Top inhib
      MinionSpawn ms2(0xffd23c3e);
      sendPacket(peer, ms2, CHL_S2C);
      SetHealth sh2(0xffd23c3e);
      sendPacket(peer, sh2, CHL_S2C);

      // Mid inhib
      MinionSpawn ms3(0xff4a20f1);
      sendPacket(peer, ms3, CHL_S2C);
      SetHealth sh3(0xff4a20f1);
      sendPacket(peer, sh3, CHL_S2C);

      // Bottom inhib
      MinionSpawn ms4(0xff9303e1);
      sendPacket(peer, ms4, CHL_S2C);
      SetHealth sh4(0xff9303e1);
      sendPacket(peer, sh4, CHL_S2C);

      // Nexus
      MinionSpawn ms5(0xfff97db5);
      sendPacket(peer, ms5, CHL_S2C);
      SetHealth sh5(0xfff97db5);
      sendPacket(peer, sh5, CHL_S2C);

   } else if (playerInfo != 0 && playerInfo->getTeam() == TEAM_PURPLE) {
      // Shop (purple team)
      MinionSpawn ms1(0xffa6170e);
      sendPacket(peer, ms1, CHL_S2C);
      SetHealth sh1(0xffa6170e);
      sendPacket(peer, sh1, CHL_S2C);

      // Vision for hardcoded objects
      // Top inhib
      MinionSpawn ms2(0xff6793d0);
      sendPacket(peer, ms2, CHL_S2C);
      SetHealth sh2(0xff6793d0);
      sendPacket(peer, sh2, CHL_S2C);

      // Mid inhib
      MinionSpawn ms3(0xffff8f1f);
      sendPacket(peer, ms3, CHL_S2C);
      SetHealth sh3(0xffff8f1f);
      sendPacket(peer, sh3, CHL_S2C);

      // Bottom inhib
      MinionSpawn ms4(0xff26ac0f);
      sendPacket(peer, ms4, CHL_S2C);
      SetHealth sh4(0xff26ac0f);
      sendPacket(peer, sh4, CHL_S2C);

      // Nexus
      MinionSpawn ms5(0xfff02c0f);
      sendPacket(peer, ms5, CHL_S2C);
      SetHealth sh5(0xfff02c0f);
      sendPacket(peer, sh5, CHL_S2C);
   }

   StatePacket end(PKT_S2C_EndSpawn);
   bool p3 = sendPacket(peer, reinterpret_cast<uint8 *>(&end), sizeof(StatePacket), CHL_S2C);

   return true;
}

bool Game::handleStartGame(HANDLE_ARGS) {

   if(++playersReady == players.size()) {
      StatePacket start(PKT_S2C_StartGame);
      broadcastPacket(reinterpret_cast<uint8 *>(&start), sizeof(StatePacket), CHL_S2C);

      for(ClientInfo* player : players) {
         if(player->getPeer() == peer && !player->isVersionMatch()) {
            DebugMessage dm("Your client version does not match the server. Check the server log for more information.");
            sendPacket(peer, dm, CHL_S2C);
         }
      }

      _started = true;
   }

   if(_started) {
      for(auto p : players) {
         map->addObject(p->getChampion());

         // Send the initial game time sync packets, then let the map send another
         float gameTime = map->getGameTime() / 1000000.f;

         GameTimer timer(gameTime); // 0xC1
         sendPacket(p->getPeer(), reinterpret_cast<uint8 *>(&timer), sizeof(GameTimer), CHL_S2C);

         GameTimerUpdate timer2(gameTime); // 0xC2
         sendPacket(p->getPeer(), reinterpret_cast<uint8 *>(&timer2), sizeof(GameTimerUpdate), CHL_S2C);
      }
   }

   return true;
}

bool Game::handleAttentionPing(ENetPeer *peer, ENetPacket *packet) {
   AttentionPing *ping = reinterpret_cast<AttentionPing *>(packet->data);
   AttentionPingAns response(peerInfo(peer), ping);
   return broadcastPacketTeam(peerInfo(peer)->getTeam(), response, CHL_S2C);
}

bool Game::handleView(ENetPeer *peer, ENetPacket *packet) {
   ViewRequest *request = reinterpret_cast<ViewRequest *>(packet->data);
   ViewAnswer answer(request);
   if (request->requestNo == 0xFE)
   {
      answer.setRequestNo(0xFF);
   }
   else
   {
      answer.setRequestNo(request->requestNo);
   }
   sendPacket(peer, answer, CHL_S2C, UNRELIABLE);
   return true;
}

inline void SetBitmaskValue(uint8 mask[], int pos, bool val) {
    if(pos < 0)
    { return; }
    if(val)
    { mask[pos / 8] |= 1 << (pos % 8); }
    else
    { mask[pos / 8] &= ~(1 << (pos % 8)); }
}

inline bool GetBitmaskValue(uint8 mask[], int pos) {
    return pos >= 0 && ((1 << (pos % 8)) & mask[pos / 8]) != 0;
}

#include <vector>

std::vector<Vector2> readWaypoints(uint8 *buffer, int coordCount) {
    unsigned int nPos = (coordCount + 5) / 8;
    if(coordCount % 2)
    { nPos++; }
    int vectorCount = coordCount / 2;
    std::vector<Vector2> vMoves;
    MovementVector lastCoord;
    for(int i = 0; i < vectorCount; i++) {
        if(GetBitmaskValue(buffer, (i - 1) * 2)) {
            lastCoord.x += *(char *)&buffer[nPos++];
        } else {
            lastCoord.x = *(short *)&buffer[nPos];
            nPos += 2;
        }
        if(GetBitmaskValue(buffer, (i - 1) * 2 + 1)) {
            lastCoord.y += *(char *)&buffer[nPos++];
        } else {
            lastCoord.y = *(short *)&buffer[nPos];
            nPos += 2;
        }
        vMoves.push_back(lastCoord);
    }
    return vMoves;
}

bool Game::handleMove(ENetPeer *peer, ENetPacket *packet) {

   if(peerInfo(peer)->getChampion()->isDashing() || peerInfo(peer)->getChampion()->isDead()) {
      return true;
   }

   MovementReq *request = reinterpret_cast<MovementReq *>(packet->data);
   std::vector<Vector2> vMoves = readWaypoints(&request->moveData, request->vectorNo);

   switch(request->type)
   {
   case STOP:
   {
       //TODO anticheat, currently it trusts client 100%

      peerInfo(peer)->getChampion()->setPosition(request->x, request->y);
      float x = ((request->x) - MAP_WIDTH)/2;
      float y = ((request->y) - MAP_HEIGHT)/2;

      for(int i=0;i<vMoves.size(); i++)
		{
			vMoves.at(i).X = request->x;
			vMoves.at(i).Y = request->y;
      }

		CORE_INFO("Stopped at x: %f, y: %f", request->x, request->y);
      break;
   }
   case EMOTE:
      //Logging->writeLine("Emotion");
      return true;
   case ATTACKMOVE:
      peerInfo(peer)->getChampion()->setMoveOrder(MOVE_ORDER_ATTACKMOVE);
      break;
   case MOVE:
      peerInfo(peer)->getChampion()->setMoveOrder(MOVE_ORDER_MOVE);
      break;
   }

   // Sometimes the client will send a wrong position as the first one, override it with server data
   vMoves[0] = Vector2(peerInfo(peer)->getChampion()->getX(), peerInfo(peer)->getChampion()->getY());
   peerInfo(peer)->getChampion()->setWaypoints(vMoves);
   Unit* u = dynamic_cast<Unit*>(map->getObjectById(request->targetNetId));
   if(!u) {
      peerInfo(peer)->getChampion()->setTargetUnit(0);
      return true;
   }

   peerInfo(peer)->getChampion()->setTargetUnit(u);

   return true;
}

bool Game::handleLoadPing(ENetPeer *peer, ENetPacket *packet) {
   PingLoadInfo *loadInfo = reinterpret_cast<PingLoadInfo *>(packet->data);
   PingLoadInfo response;
   memcpy(&response, packet->data, sizeof(PingLoadInfo));
   response.header.cmd = PKT_S2C_Ping_Load_Info;
   response.userId = peerInfo(peer)->userId;
   //Logging->writeLine("loaded: %f, ping: %f, %f", loadInfo->loaded, loadInfo->ping, loadInfo->f3);
   return broadcastPacket(reinterpret_cast<uint8 *>(&response), sizeof(PingLoadInfo), CHL_LOW_PRIORITY, UNRELIABLE);
}

bool Game::handleQueryStatus(HANDLE_ARGS) {
   QueryStatus response;
   return sendPacket(peer, reinterpret_cast<uint8 *>(&response), sizeof(QueryStatus), CHL_S2C);
}

bool Game::handleClick(HANDLE_ARGS) {
   Click *click = reinterpret_cast<Click *>(packet->data);
   CORE_INFO("Object %X clicked on %X", peerInfo(peer)->getChampion()->getNetId(),click->targetNetId);

   return true;
}

bool Game::handleCastSpell(HANDLE_ARGS) {
   CastSpell *spell = reinterpret_cast<CastSpell *>(packet->data);

   // There are some bits triggering this
   /*if (spell->spellSlotType & 0x0F > 0) {
      CORE_INFO("Summoner Spell Cast");
      CORE_INFO("Type: 0x%X, Slot %d, coord %f ; %f, coord2 %f, %f, target NetId %08X", spell->spellSlotType, spell->spellSlot, spell->x, spell->y, spell->x2, spell->y2, spell->targetNetId);
      return true;
   }*/

   CORE_INFO("Spell Cast : Type: 0x%X, Slot %d, coord %f ; %f, coord2 %f, %f, target NetId %08X", spell->spellSlotType, spell->spellSlot, spell->x, spell->y, spell->x2, spell->y2, spell->targetNetId);

   uint32 futureProjNetId = GetNewNetID();
   uint32 spellNetId = GetNewNetID();
   Object *targetObj = map->getObjectById( spell->targetNetId );
   Unit *targetUnit = dynamic_cast<Unit*>(targetObj);

   Spell* s = peerInfo(peer)->getChampion()->castSpell(spell->spellSlot, spell->x, spell->y, targetUnit, futureProjNetId, spellNetId);

   if(!s) {
      return false;
   }

   CastSpellAns response(s, spell->x, spell->y, futureProjNetId, spellNetId);
   broadcastPacket(response, CHL_S2C);

   return true;
}

bool Game::handleChatBoxMessage(HANDLE_ARGS) {
   ChatMessage *message = reinterpret_cast<ChatMessage *>(packet->data);
   //Lets do commands
   if(message->msg == '.') {
      const char *cmd[] = { ".set", ".gold", ".speed", ".health", ".xp", ".ap", ".ad", ".mana", ".model", ".help", ".spawn", ".size", ".junglespawn", ".skillpoints", ".level", ".tp", ".coords", ".ch"};
      std::ostringstream debugMsg;

      // help command
      if (strncmp(message->getMessage(), cmd[9], strlen(cmd[9])) == 0) {
         debugMsg << "List of available commands: ";

         for (auto c : cmd) {
            debugMsg << c << " ";
         }
         notifyDebugMessage(debugMsg.str());
         return true;
      }

      // set
      if(strncmp(message->getMessage(), cmd[0], strlen(cmd[0])) == 0) {
         uint32 blockNo, fieldNo;
         float value;
         sscanf(&message->getMessage()[strlen(cmd[0])+1], "%u %u %f", &blockNo, &fieldNo, &value);
         blockNo = 1 << (blockNo - 1);
         uint32 mask = 1 << (fieldNo - 1);
         peerInfo(peer)->getChampion()->getStats().setStat(blockNo, mask, value);
         return true;
      }

      // gold
      if(strncmp(message->getMessage(), cmd[1], strlen(cmd[1])) == 0) {
         float gold = (float)atoi(&message->getMessage()[strlen(cmd[1]) + 1]);
         peerInfo(peer)->getChampion()->getStats().setGold(gold);
         return true;
      }

      // speed
      if(strncmp(message->getMessage(), cmd[2], strlen(cmd[2])) == 0)
      {
         float data = (float)atoi(&message->getMessage()[strlen(cmd[2])+1]);

         CORE_INFO("Setting speed to %f", data);

         peerInfo(peer)->getChampion()->getStats().setMovementSpeed(data);
         return true;
      }

      //health
      if(strncmp(message->getMessage(), cmd[3], strlen(cmd[3])) == 0)
      {
         float data = (float)atoi(&message->getMessage()[strlen(cmd[3])+1]);

         peerInfo(peer)->getChampion()->getStats().setCurrentHealth(data);
         peerInfo(peer)->getChampion()->getStats().setMaxHealth(data);

         notifySetHealth(peerInfo(peer)->getChampion());

         return true;
      }

      // xp
      if(strncmp(message->getMessage(), cmd[4], strlen(cmd[4])) == 0)
      {
         float data = (float)atoi(&message->getMessage()[strlen(cmd[5])+1]);

         CORE_INFO("Setting experience to %f", data);

         peerInfo(peer)->getChampion()->getStats().setExp(data);
         return true;
      }

      // ap
      if(strncmp(message->getMessage(), cmd[5], strlen(cmd[5])) == 0)
      {
         float data = (float)atoi(&message->getMessage()[strlen(cmd[5])+1]);

         CORE_INFO("Setting AP to %f", data);

         peerInfo(peer)->getChampion()->getStats().setBonusApFlat(data);
         return true;
      }

      // ad
      if(strncmp(message->getMessage(), cmd[6], strlen(cmd[6])) == 0)
      {
         float data = (float)atoi(&message->getMessage()[strlen(cmd[5])+1]);

         CORE_INFO("Setting AD to %f", data);

         peerInfo(peer)->getChampion()->getStats().setBonusAdFlat(data);
         return true;
      }

      // Mana
      if(strncmp(message->getMessage(), cmd[7], strlen(cmd[7])) == 0)
      {
         float data = (float)atoi(&message->getMessage()[strlen(cmd[7])+1]);

         CORE_INFO("Setting Mana to %f", data);

         peerInfo(peer)->getChampion()->getStats().setCurrentMana(data);
         peerInfo(peer)->getChampion()->getStats().setMaxMana(data);
         return true;
      }

      // Model
      if(strncmp(message->getMessage(), cmd[8], strlen(cmd[8])) == 0) {
         std::string sModel = (char *)&message->getMessage()[strlen(cmd[8]) + 1];
         peerInfo(peer)->getChampion()->setModel(sModel);
         return true;
      }

      // Size
      if(strncmp(message->getMessage(), cmd[11], strlen(cmd[11])) == 0) {
         float data = (float)atoi(&message->getMessage()[strlen(cmd[11])+1]);

         CORE_INFO("Setting size to %f", data);

         peerInfo(peer)->getChampion()->getStats().setSize(data);
         return true;
      }

      // Mob Spawning-Creating, updated
      if(strncmp(message->getMessage(), cmd[12], strlen(cmd[12])) == 0) {
         const char *cmd[] = { "c baron" , "c wolves", "c red", "c blue", "c dragon", "c wraiths", "c golems"};
         return true;
      }

      // Skillpoints
      if(strncmp(message->getMessage(), cmd[13], strlen(cmd[13])) == 0) {
         peerInfo(peer)->getChampion()->setSkillPoints(17);

         SkillUpResponse skillUpResponse(peerInfo(peer)->getChampion()->getNetId(), 0, 0, 17);
         sendPacket(peer, skillUpResponse, CHL_GAMEPLAY);
         return true;
      }

      // Level
      if(strncmp(message->getMessage(), cmd[14], strlen(cmd[14])) == 0) {
         float data = (float)atoi(&message->getMessage()[strlen(cmd[14])+1]);

         peerInfo(peer)->getChampion()->getStats().setLevel(data);
         return true;
      }

      // tp
      if(strncmp(message->getMessage(), cmd[15], strlen(cmd[15])) == 0) {
         float x, y;
         sscanf(&message->getMessage()[strlen(cmd[15])+1], "%f %f", &x, &y);

         notifyTeleport(peerInfo(peer)->getChampion(), x, y);
         return true;
      }

       // coords
      if(strncmp(message->getMessage(), cmd[16], strlen(cmd[16])) == 0) {
         CORE_INFO("At %f;%f", peerInfo(peer)->getChampion()->getX(), peerInfo(peer)->getChampion()->getY());
         debugMsg << "At Coords - X:" << peerInfo(peer)->getChampion()->getX() << " Y:" << peerInfo(peer)->getChampion()->getY() << " Z:" << peerInfo(peer)->getChampion()->getZ();
         notifyDebugMessage(debugMsg.str());
         return true;
      }

      // Ch(ampion)
      if(strncmp(message->getMessage(), cmd[17], strlen(cmd[17])) == 0) {
         std::string champ = (char *)&message->getMessage()[strlen(cmd[17]) + 1];
         Champion* c = new Champion(champ, map, peerInfo(peer)->getChampion()->getNetId(), peerInfo(peer)->userId);

         c->setPosition(peerInfo(peer)->getChampion()->getX(), peerInfo(peer)->getChampion()->getY());
         c->setModel(champ); // trigger the "modelUpdate" proc

         map->removeObject(peerInfo(peer)->getChampion());
         delete peerInfo(peer)->getChampion();
         map->addObject(c);

         peerInfo(peer)->setChampion(c);

         return true;
      }

   }

   switch(message->type) {
   case CMT_ALL:
      return broadcastPacket(packet->data, packet->dataLength, CHL_COMMUNICATION);
   case CMT_TEAM:
      return broadcastPacketTeam(peerInfo(peer)->getTeam(), packet->data, packet->dataLength, CHL_COMMUNICATION);
   default:
      //Logging->errorLine("Unknown ChatMessageType");
      return sendPacket(peer, packet->data, packet->dataLength, CHL_COMMUNICATION);
   }

   return false;
}

bool Game::handleSkillUp(HANDLE_ARGS) {
    SkillUpPacket *skillUpPacket = reinterpret_cast<SkillUpPacket *>(packet->data);
    //!TODO Check if can up skill? :)

    Spell*s = peerInfo(peer)->getChampion()->levelUpSpell(skillUpPacket->skill);

    if(!s) {
      return false;
    }

    SkillUpResponse skillUpResponse(peerInfo(peer)->getChampion()->getNetId(), skillUpPacket->skill, s->getLevel(), peerInfo(peer)->getChampion()->getSkillPoints());
    sendPacket(peer, skillUpResponse, CHL_GAMEPLAY);

    peerInfo(peer)->getChampion()->getStats().setSpellEnabled(skillUpPacket->skill, true);

    return true;
}

bool Game::handleBuyItem(HANDLE_ARGS) {

   BuyItemReq *request = reinterpret_cast<BuyItemReq *>(packet->data);

   const ItemTemplate* itemTemplate = ItemManager::getInstance()->getItemTemplateById(request->id);
   if(!itemTemplate) {
      return false;
   }

   std::vector<ItemInstance*> recipeParts = peerInfo(peer)->getChampion()->getInventory().getAvailableRecipeParts(itemTemplate);
   uint32 price = itemTemplate->getTotalPrice();
   const ItemInstance* i;

   if(recipeParts.empty()) {
      if(peerInfo(peer)->getChampion()->getStats().getGold() < price) {
         return true;
      }

      i = peerInfo(peer)->getChampion()->getInventory().addItem(itemTemplate);

      if(i == 0) { // Slots full
         return false;
      }
   } else {
      for(ItemInstance* instance : recipeParts) {
         price -= instance->getTemplate()->getTotalPrice();
      }

      if(peerInfo(peer)->getChampion()->getStats().getGold() < price) {
         return false;
      }

      for(ItemInstance* instance : recipeParts) {
         peerInfo(peer)->getChampion()->getStats().unapplyStatMods(instance->getTemplate()->getStatMods());
         notifyRemoveItem(peerInfo(peer)->getChampion(), instance->getSlot(), 0);
         peerInfo(peer)->getChampion()->getInventory().removeItem(instance->getSlot());
      }

      i = peerInfo(peer)->getChampion()->getInventory().addItem(itemTemplate);
   }

   peerInfo(peer)->getChampion()->getStats().setGold(peerInfo(peer)->getChampion()->getStats().getGold()-price);
   peerInfo(peer)->getChampion()->getStats().applyStatMods(itemTemplate->getStatMods());
   notifyItemBought(peerInfo(peer)->getChampion(), i);

   return true;
}

bool Game::handleSellItem(HANDLE_ARGS) {
   SellItem *sell = reinterpret_cast<SellItem *>(packet->data);

   ItemInstance* i;
   i = peerInfo(peer)->getChampion()->getInventory().getItemSlot(sell->slotId);
   if(!i)
   {
      return false;
   }

   float sellPrice = i->getTemplate()->getTotalPrice() * i->getTemplate()->getSellBackModifier();
   peerInfo(peer)->getChampion()->getStats().setGold(peerInfo(peer)->getChampion()->getStats().getGold()+sellPrice);

   if(i->getTemplate()->getMaxStack() > 1) {
      i->decrementStacks();
      notifyRemoveItem(peerInfo(peer)->getChampion(), sell->slotId, i->getStacks());

      if(i->getStacks() == 0) {
         peerInfo(peer)->getChampion()->getInventory().removeItem(sell->slotId);
      }
   }
   else {
      notifyRemoveItem(peerInfo(peer)->getChampion(), sell->slotId, 0);
      peerInfo(peer)->getChampion()->getInventory().removeItem(sell->slotId);
   }

   return true;
}

bool Game::handleSwapItems(HANDLE_ARGS) {
   SwapItemsReq* request = reinterpret_cast<SwapItemsReq*>(packet->data);

   if(request->slotFrom > 6 || request->slotTo > 6) {
      return false;
   }

   peerInfo(peer)->getChampion()->getInventory().swapItems(request->slotFrom, request->slotTo);
   notifyItemsSwapped(peerInfo(peer)->getChampion(), request->slotFrom, request->slotTo);

   return true;
}

bool Game::handleEmotion(HANDLE_ARGS) {
    EmotionPacket *emotion = reinterpret_cast<EmotionPacket *>(packet->data);
    //for later use -> tracking, etc.
    switch(emotion->id) {
        case 0:
            //dance
            //Logging->writeLine("dance");
            break;
        case 1:
            //taunt
            //Logging->writeLine("taunt");
            break;
        case 2:
            //laugh
            //Logging->writeLine("laugh");
            break;
        case 3:
            //joke
            //Logging->writeLine("joke");
            break;
    }
    EmotionResponse response;
    response.header.netId = emotion->header.netId;
    response.id = emotion->id;
    return broadcastPacket(reinterpret_cast<uint8 *>(&response), sizeof(response), CHL_S2C);
}

bool Game::handleDisconnect(ENetPeer* peer) {
   if (peerInfo(peer)) {
      // TODO: Handle disconnect
      ClientInfo* clientInfo = peerInfo(peer);

      CORE_INFO("Player %d disconnected", clientInfo->userId);
   }
   return true;
}

bool Game::handleHeartBeat(HANDLE_ARGS) {
   HeartBeat *heartbeat = reinterpret_cast<HeartBeat *>(packet->data);

   float diff = heartbeat->ackTime - heartbeat->receiveTime;
   if (heartbeat->receiveTime > heartbeat->ackTime) {
      CORE_WARNING("Player %d sent an invalid heartbeat - Timestamp error (diff: %.f)", peerInfo(peer)->userId, diff);
   } else {
      CORE_INFO("Player %d sent heartbeat (diff: %.f)", peerInfo(peer)->userId, diff);
   }

   return true;
}
