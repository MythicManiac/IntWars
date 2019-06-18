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
#ifndef _CLIENT_H
#define _CLIENT_H

#include "common.h"
#include "ChampionFactory.h"
#include <string>

enum TeamId : uint32 {
   TEAM_BLUE = 0x64,
   TEAM_PURPLE = 0xC8
};

struct ClientInfo
{
	ClientInfo(const std::string& rank, TeamId team, uint16 ribbon, uint32 icon) : keyChecked(false), versionMatch(true), ticks(0), skinNo(0), rank(rank), team(team), ribbon(ribbon), icon(icon), peer(0)
	{
	}

	~ClientInfo()
	{
	}

	void setName(const std::string& name)
	{
		this->name = name;
	}

	void setChampion(Champion* champion)
	{
		this->champion = champion; //ChampionFactory::getChampionFromType(type);
      champion->setSkin(skinNo);
	}
	
	void setSkinNo(uint32 skinNo)
	{
		this->skinNo = skinNo;
	}
   
   const std::string& getRank() const { return rank; }
   uint32 getTeam() const { return team; }
   
   Champion* getChampion() const { return champion; }
   
   const std::string& getName() const { return name; }
   
   uint32 getSkinNo() const { return skinNo; }
   
   void setSummoners(uint32 sum1, uint32 sum2) {
      summonerSkills[0] = sum1;
      summonerSkills[1] = sum2;
   }
   
   void setTicks(int a){
       ticks = (uint32) a;
   }
   
   uint32 getRibbon(){
       return ribbon;
   }
   
   uint32 getIcon(){
       return icon;
   }

   uint32 getTicks()
   {
      ticks++;
      return ticks;
   }
   
   void setPeer(ENetPeer* peer) { this->peer = peer; }
   ENetPeer* getPeer() { return peer; }
   
   void setVersionMatch(bool versionMatch) { this->versionMatch = versionMatch; }
   bool isVersionMatch() { return versionMatch; }
   
   bool keyChecked, versionMatch;
   uint64 userId;
   uint32 ticks;
   uint32 skinNo;
   uint32 summonerSkills[2];
   std::string name, rank;
   uint16 ribbon;
   uint32 icon;
   uint32 team;
   
   Champion* champion;
   ENetPeer* peer;

};

#endif
