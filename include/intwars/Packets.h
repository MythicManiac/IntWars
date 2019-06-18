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
#ifndef _PACKETS_H
#define _PACKETS_H

#include <time.h>
#include <cmath>
#include <set>

#include <intlib/general.h>
#include <enet/enet.h>

#include "common.h"
#include "Buffer.h"
#include "Client.h"
#include "Minion.h"
#include "Turret.h"
#include "LevelProp.h"
#include "Map.h"

#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

/* New Packet Architecture */
class Packet {
protected:
   Buffer buffer;
   
public:
   const Buffer& getBuffer() const { return buffer; }
   Packet(uint8 cmd = 0) {
      buffer << cmd;
   }

};

class BasePacket : public Packet {
   
public:
   BasePacket(uint8 cmd = 0, uint32 netId = 0) : Packet(cmd) {
      buffer << netId;
   }

};

class GamePacket : public BasePacket {
   
public:
   GamePacket(uint8 cmd = 0, uint32 netId = 0) : BasePacket(cmd, netId) {
      buffer << (uint32)clock();
   }

};

class ExtendedPacket : public BasePacket {
public:
   ExtendedPacket(uint8 ecmd = 0, uint32 netId = 0) : BasePacket(PKT_S2C_Extended, netId) {
      buffer << ecmd;
      buffer << (uint8)1;
   }
};

/* Old Packet Architecture & Packets */

struct PacketHeader {
    PacketHeader() {
        netId = 0;
    }

    PacketCmd cmd;
    uint32 netId;
};

struct GameHeader {
    GameHeader() {
        netId = ticks = 0;
    }

    GameCmd cmd;
    uint32 netId;
    uint32 ticks;
};

/*typedef struct _SynchBlock {
    uint64 userId;
    uint16 unk;
    uint32 skill1;
    uint32 skill2;
    uint8 bot;
    uint32 teamId;
    uint8 name[64];
    uint8 type[64];
    uint8 rank[30];
} SynchBlock;*/

struct ClientReady {
    uint32 cmd;
    uint32 playerId;
    uint32 teamId;
};

class SynchVersionAns : public BasePacket {
public:

   SynchVersionAns(const std::vector<ClientInfo*>& players, const std::string& version, const std::string& gameMode, const uint32 map) : BasePacket(PKT_S2C_SynchVersion) {
      buffer << (uint8)9; // unk
      buffer << (uint32)map; // mapId
      for(auto p : players) {
         buffer << p->userId;
         buffer << (uint16)0x1E; // unk
         buffer << p->summonerSkills[0];
         buffer << p->summonerSkills[1];
         buffer << (uint8)0; // bot boolean
         buffer << p->getTeam();
         buffer.fill(0, 64); // name is no longer here
         buffer.fill(0, 64);
         buffer << p->getRank();
         buffer.fill(0, 24-p->getRank().length());
         buffer << p->getIcon();
         buffer << (uint16)p->getRibbon();
      }
      
      for(size_t i = 0; i < 12-players.size(); ++i) {
         buffer << (int64)-1;
         buffer.fill(0, 173);
      }
            
        buffer << version;
        buffer.fill(0, 256-version.length());
        buffer << gameMode;
        buffer.fill(0, 128-gameMode.length());
        
        buffer << "NA1";
        buffer.fill(0, 2332); // 128 - 3 + 661 + 1546
        buffer << (uint32)487826; // gameFeatures (turret range indicators, etc.)
        buffer.fill(0, 256);
        buffer << (uint32)0;
        buffer.fill(1, 19);
    }

   /* PacketHeader header;
    uint8 ok;
    uint32 mapId;
    SynchBlock players[12];
    uint8 version[256]; 
    uint8 gameMode[128];
    uint8 unk1[512];
    uint8 unk2[245];

    uint8 ekg1[256]; //ekg.riotgames.net
    uint8 msg1[256]; //"/messages"

    uint16 wUnk1; //0x50?
    uint8 ekg2[256]; //ekg.riotgames.net
    uint8 msg2[256]; //"/messages"

    uint16 wUnk2; //0x50?
    uint8 ekg3[256]; //ekg.riotgames.net
    uint8 msg3[256]; //"/messages"

    uint16 wUnk3; //0x50?
    uint32 dwUnk1;
    uint32 dwOpt; //0x377192
    uint8 bUnk1[0x100];
    uint8 bUnk2[11];*/
};

typedef struct _PingLoadInfo {
    PacketHeader header;
    uint32 unk1;
    uint64 userId;
    float loaded;
    float ping;
    uint16 unk2;
    uint16 unk3;
    uint8 unk4;
} PingLoadInfo;

uint8 *createDynamicPacket(uint8 *str, uint32 size);

class LoadScreenInfo : public Packet {
public:
    LoadScreenInfo(const std::vector<ClientInfo*>& players) : Packet(PKT_S2C_LoadScreenInfo) {
        //Zero this complete buffer
        buffer << (uint32)6; // blueMax
        buffer << (uint32)6; // redMax
        
        uint32 currentBlue = 0;
        for(ClientInfo* player : players) {
           if(player->getTeam() == TEAM_BLUE) {
              buffer << player->userId;
              currentBlue++;
           }
        }
        
        for(size_t i = 0; i < 6-currentBlue; ++i) {
           buffer << (uint64)0;
        }
        
        buffer.fill(0, 144);
        
        uint32 currentPurple = 0;
        for(ClientInfo* player : players) {
           if(player->getTeam() == TEAM_PURPLE) {
              buffer << player->userId;
              currentPurple++;
           }
        }
        
        for(int i = 0; i < 6-currentPurple; ++i) {
           buffer << (uint64)0;
        }
        
        buffer.fill(0, 144);
        buffer << currentBlue;
        buffer << currentPurple;
    }

    /*uint8 cmd;
    uint32 blueMax;
    uint32 redMax;
    uint64 bluePlayerIds[6]; //Team 1, 6 players max
    uint8 blueData[144];
    uint64 redPlayersIds[6]; //Team 2, 6 players max
    uint8 redData[144];
    uint32 bluePlayerNo;
    uint32 redPlayerNo;*/
};

typedef struct _KeyCheck {
    _KeyCheck() {
        cmd = PKT_KeyCheck;
        playerNo = 0;
        checkId = 0;
        trash = trash2 = 0;
    }

    uint8 cmd;
    uint8 partialKey[3];   //Bytes 1 to 3 from the blowfish key for that client
    uint32 playerNo;
    uint64 userId;         //uint8 testVar[8];   //User id
    uint32 trash;
    uint64 checkId;        //uint8 checkVar[8];  //Encrypted testVar
    uint32 trash2;
} KeyCheck;

struct CameraLock {
    PacketHeader header;
    uint8 isLock;
    uint32 padding;
};

/*typedef struct _ViewReq {
    uint8 cmd;
    uint32 unk1;
    float x;
    float zoom;
    float y;
    float y2;		//Unk
    uint32 width;	//Unk
    uint32 height;	//Unk
    uint32 unk2;	//Unk
    uint8 requestNo;
} ViewReq;*/

class MinionSpawn : public BasePacket {
public:
   MinionSpawn(const Minion* m) : BasePacket(PKT_S2C_ObjectSpawn, m->getNetId()) {
      buffer << (uint32)0x00150017; // unk
      buffer << (uint8)0x03; // SpawnType - 3 = minion
      buffer << m->getNetId() << m->getNetId();
      buffer << (uint32)m->getSpawnPosition();
      buffer << (uint8)0xFF; // unk
      buffer << (uint8)1; // wave number ?
      
      buffer << (uint8)m->getType();
      
      if(m->getType() == MINION_TYPE_MELEE) {
         buffer << (uint8)0; // unk
      } else {
         buffer << (uint8)1; // unk
      }
      
      buffer << (uint8)0; // unk
      
      if(m->getType() == MINION_TYPE_CASTER) {
         buffer << (uint32)0x00010007; // unk
      } else if(m->getType() == MINION_TYPE_MELEE) {
         buffer << (uint32)0x0001000A; // unk
      } else if(m->getType() == MINION_TYPE_CANNON) {
         buffer << (uint32)0x0001000D;
      } else {
         buffer << (uint32)0x00010007; // unk
      }
      buffer << (uint32)0x00000000; // unk
      buffer << (uint32)0x00000000; // unk
      buffer << (uint16)0x0000; // unk
      buffer << 1.0f; // unk
      buffer << (uint32)0x00000000; // unk
      buffer << (uint32)0x00000000; // unk
      buffer << (uint32)0x00000000; // unk
      buffer << (uint16)0x0200; // unk
      buffer << (uint32)clock(); // unk
      
      const std::vector<Vector2>& waypoints = m->getWaypoints();
      
      buffer << (uint8)((waypoints.size()-m->getCurWaypoint()+1)*2); // coordCount
      buffer << m->getNetId();
      buffer << (uint8)0; // movement mask
      buffer << MovementVector::targetXToNormalFormat(m->getX());
      buffer << MovementVector::targetYToNormalFormat(m->getY());
      for(int i = m->getCurWaypoint(); i < waypoints.size(); ++i) {
			buffer << MovementVector::targetXToNormalFormat(waypoints[i].X);
			buffer << MovementVector::targetXToNormalFormat(waypoints[i].Y);
      }
   }
   
   MinionSpawn(uint32 netId) : BasePacket(PKT_S2C_ObjectSpawn, netId) {
      buffer.fill(0, 3);
   }
};

class SpellAnimation : public BasePacket {
public:
   SpellAnimation(Unit* u, const std::string& animationName) : BasePacket(PKT_S2C_SpellAnimation, u->getNetId()) {
      buffer << (uint32)0x00000005; // unk
      buffer << (uint32)0x00000000; // unk
      buffer << (uint32)0x00000000; // unk
      buffer << 1.0f; // unk
      buffer << animationName;
      buffer << (uint8)0;
   }
};

class SetAnimation : public BasePacket {
public:
   SetAnimation(Unit* u, const std::vector<std::pair<std::string, std::string>> animationPairs) : BasePacket(PKT_S2C_SetAnimation, u->getNetId()) {
      buffer << (uint8)animationPairs.size();

      for (int i = 0; i < animationPairs.size(); i++) {
         buffer << (uint32)animationPairs[i].first.length();
         buffer << animationPairs[i].first;
         buffer << (uint32)animationPairs[i].second.length();
         buffer << animationPairs[i].second;
      }
   }
};

class FaceDirection : public BasePacket {
public:
   FaceDirection(Unit* u, float relativeX, float relativeY, float relativeZ) : BasePacket(PKT_S2C_FaceDirection, u->getNetId()) {
      buffer << relativeX << relativeZ << relativeY;
      buffer << (uint8)0;
      buffer << (float)0.0833; // Time to turn ?
   }
};

class Dash : public GamePacket {
public:
   Dash(Unit* u, float toX, float toY, float dashSpeed) : GamePacket(PKT_S2C_Dash, 0) {
      buffer << (uint16)1; // nb updates ?
      buffer << (uint8)5; // unk
      buffer << u->getNetId();
      buffer << (uint8)0; // unk
      buffer << dashSpeed; // Dash speed
      buffer << (uint32)0; // unk
      buffer << u->getX() << u->getY();
      buffer << (uint32)0; // unk
      buffer << (uint8)0;
      
      buffer << (uint32)0x4c079bb5; // unk
      buffer << (uint32)0xa30036df; // unk
      buffer << (uint32)0x200168c2; // unk
      
      buffer << (uint8)0x00; // Vector bitmask on whether they're int16 or int8
      
      MovementVector from = u->getMap()->toMovementVector(u->getX(), u->getY());
      MovementVector to = u->getMap()->toMovementVector(toX, toY);
      
      buffer << from.x << from.y;
      buffer << to.x << to.y;
   }
};

class LeaveVision : public BasePacket {
public:
   LeaveVision(Object* o) : BasePacket(PKT_S2C_LeaveVision, o->getNetId()) { }

};

class DeleteObjectFromVision : public BasePacket {
public:
   DeleteObjectFromVision(Object* o) : BasePacket(PKT_S2C_DeleteObject, o->getNetId()) { }

};

/**
 * This is basically a "Unit Spawn" packet with only the net ID and the additionnal data
 */
class EnterVisionAgain : public BasePacket {
public:
   EnterVisionAgain(Minion* m) : BasePacket(PKT_S2C_ObjectSpawn, m->getNetId()) {
      buffer.fill(0, 13);
      buffer << 1.0f;
      buffer.fill(0, 13);
      buffer << (uint8)0x02;
      buffer << (uint32)clock(); // unk
      
      const std::vector<Vector2>& waypoints = m->getWaypoints();
      
      buffer << (uint8)((waypoints.size()-m->getCurWaypoint()+1)*2); // coordCount
      buffer << m->getNetId();
      buffer << (uint8)0; // movement mask
      buffer << MovementVector::targetXToNormalFormat(m->getX());
      buffer << MovementVector::targetYToNormalFormat(m->getY());
      for(int i = m->getCurWaypoint(); i < waypoints.size(); ++i) {
			buffer << MovementVector::targetXToNormalFormat(waypoints[i].X);
			buffer << MovementVector::targetXToNormalFormat(waypoints[i].Y);
      }
   }

   EnterVisionAgain(Champion* c) : BasePacket(PKT_S2C_ObjectSpawn, c->getNetId()) {
      buffer << (uint16)0; // extraInfo
      buffer << (uint8)0; //c->getInventory().getItems().size(); // itemCount?
      //buffer << (uint8)7; // unknown

      /*
      for (int i = 0; i < c->getInventory().getItems().size(); i++) {
         ItemInstance* item = c->getInventory().getItems()[i];

         if (item != 0 && item->getTemplate() != 0) {
            buffer << (uint8)item->getStacks();
            buffer << (uint8)0; // unk
            buffer << (uint32)item->getTemplate()->getId();
            buffer << (uint8)item->getSlot();
         }
         else {
            buffer.fill(0, 7);
         }
      }
      */

      buffer.fill(0, 10);
      buffer << (float)1.0f;
      buffer.fill(0, 13);

      buffer << (uint8)2; // Type of data: Waypoints=2
      buffer << (uint32)clock(); // unk

      const std::vector<Vector2>& waypoints = c->getWaypoints();

      buffer << (uint8)((waypoints.size() - c->getCurWaypoint() + 1) * 2); // coordCount
      buffer << c->getNetId();
      buffer << (uint8)0; // movement mask; 1=KeepMoving?
      buffer << MovementVector::targetXToNormalFormat(c->getX());
      buffer << MovementVector::targetYToNormalFormat(c->getY());
		for (int i = c->getCurWaypoint(); i < waypoints.size(); ++i) {
			buffer << MovementVector::targetXToNormalFormat(waypoints[i].X);
			buffer << MovementVector::targetXToNormalFormat(waypoints[i].Y);
      }
   }
};

class AddGold : public BasePacket {
public:
   AddGold(Champion* richMan, Unit* died, float gold) : BasePacket(PKT_S2C_AddGold, richMan->getNetId()) {
      buffer << richMan->getNetId();
      if(died) {
         buffer << died->getNetId();
      } else {
         buffer << (uint32)0;
      }
      buffer << gold;
   }
};

struct MovementReq {
    PacketHeader header;
    MoveType type;
    float x;
    float y;
    uint32 targetNetId;
    uint8 vectorNo;
    uint32 netId;
    uint8 moveData;
};

struct MovementAns {
    MovementAns() {
        header.cmd = PKT_S2C_MoveAns;
    }

    GameHeader header;
    uint16 nbUpdates;
    uint8 vectorNo;
    uint32 netId;
    uint8 moveData; //bitMasks + Move Vectors

    MovementVector *getVector(uint32 index) {
        if(index >= (uint8)vectorNo / 2)
        { return NULL; }
        MovementVector *vPoints = (MovementVector *)(&moveData + maskCount());
        return &vPoints[index];
    }

    int maskCount() {
        float fVal = (float)vectorNo / 2;
        return (int)std::ceil((fVal - 1) / 4);
    }

    static uint32 size(uint8 vectorNo) {
        float fVectors = vectorNo;
        int maskCount = (int)std::ceil((fVectors - 1) / 4);
        return sizeof(MovementAns) + (vectorNo * sizeof(MovementVector)) + maskCount; //-1 since struct already has first moveData byte
    }

    uint32 size() {
        return size(vectorNo / 2);
    }

    static MovementAns *create(uint32 vectorNo) {
        int nSize = size(vectorNo / 2);
        MovementAns *packet = (MovementAns *)new uint8[nSize];
        memset(packet, 0, nSize);
        packet->header.cmd = PKT_S2C_MoveAns;
        packet->header.ticks = clock();
        packet->vectorNo = vectorNo;
        return packet;
    }

    static void destroy(MovementAns *packet) {
        delete [](uint8 *)packet;
    }

};

/*typedef struct _ViewAns {
    _ViewAns() {
        cmd = PKT_S2C_ViewAns;
        unk1 = 0;
    }

    uint8 cmd;
    uint32 unk1;
    uint8 requestNo;
} ViewAns;*/


typedef struct _QueryStatus {
    _QueryStatus() {
        header.cmd = PKT_S2C_QueryStatusAns;
        ok = 1;
    }
    PacketHeader header;
    uint8 ok;
} QueryStatus;

typedef struct _SynchVersion {
    PacketHeader header;
    uint32 unk1;
    int8 version[256]; // version string might be shorter?
    
    const int8 *getVersion() {
        return version;
    }
} SynchVersion;

typedef struct _WorldSendGameNumber {
    _WorldSendGameNumber() {
        header.cmd = PKT_World_SendGameNumber;
        memset(data, 0, sizeof(data));
        gameId = 0;
    }

    PacketHeader header;
    uint64 gameId;
    uint8 data[128];
} WorldSendGameNumber;


struct CharacterStats {

   CharacterStats(uint8 masterMask, uint32 netId, uint32 mask, float value) : updateNo(1), masterMask(masterMask), netId(netId), mask(mask), size(4) {
      header.cmd = (GameCmd)PKT_S2C_CharStats;
      header.ticks = clock();
      this->value.fValue = value;
   }
   
   CharacterStats(uint8 masterMask, uint32 netId, uint32 mask, unsigned short value) : updateNo(1), masterMask(masterMask), netId(netId), mask(mask), size(2) {
      header.cmd = (GameCmd)PKT_S2C_CharStats;
      header.ticks = clock();
      this->value.sValue = value;
   }

   GameHeader header;
   uint8 updateNo;
   uint8 masterMask;
   uint32 netId;
   uint32 mask;
   uint8 size;
   union {
      unsigned short sValue;
      float fValue;
   } value;
};

struct ChatMessage {
    uint8 cmd;
    uint32 playerId;
    uint32 botNetId;
    uint8 isBotMessage;

    ChatType type;
    uint32 unk1; // playerNo?
    uint32 length;
    uint8 unk2[32];
    int8 msg;

    int8 *getMessage() {
        return &msg;
    }
    uint32 *getLength() {
       return &length;
    }
};

typedef struct _UpdateModel {
    _UpdateModel(uint32 netID, const char *szModel) {
        memset(this, 0, sizeof(_UpdateModel));
        header.cmd = (PacketCmd)0x97;
        header.netId = netID;
        id = netID & ~0x40000000;
        bOk = 1;
        unk1 = -1;
        strncpy((char *)szName, szModel, 32);
    }
    PacketHeader header;
    uint32 id;
    uint8 bOk;
    uint32 unk1;
    uint8 szName[32];
} UpdateModel;
typedef struct _StatePacket {
    _StatePacket(PacketCmd state) {
        header.cmd = state;
    }
    PacketHeader header;
} StatePacket;
typedef struct _StatePacket2 {
    _StatePacket2(PacketCmd state) {
        header.cmd = state;
        nUnk = 0;
    }
    PacketHeader header;
    int16 nUnk;
} StatePacket2;

struct FogUpdate2 {
    FogUpdate2() {
        header.cmd = PKT_S2C_FogUpdate2;
        header.netId = 0x40000019;
    }
    PacketHeader header;
    float x;
    float y;
    uint32 radius;
    uint8 unk1;
};

struct Click {

   PacketHeader header;
   uint32 zero;
   uint32 targetNetId; // netId on which the player clicked

};

class HeroSpawn : public Packet {
public:
	HeroSpawn(ClientInfo* player, int playerId) : Packet(PKT_S2C_HeroSpawn) {
		buffer << (uint32)0; // ???
		buffer << (uint32)player->getChampion()->getNetId();
		buffer << (uint32)playerId; // player Id
		buffer << (uint8)40; // netNodeID ?
		buffer << (uint8)0; // botSkillLevel Beginner=0 Intermediate=1
      if(player->getTeam() == TEAM_BLUE) {
         buffer << (uint8)1; // teamNumber BotTeam=2,3 Blue=Order=1 Purple=Chaos=0
      } else {
         buffer << (uint8)0; // teamNumber BotTeam=2,3 Blue=Order=1 Purple=Chaos=0
      }
		buffer << (uint8)0; // isBot
		//buffer << (uint8)0; // botRank (deprecated as of 4.18)
		buffer << (uint8)0; // spawnPosIndex
		buffer << (uint32)player->getSkinNo();
		buffer << player->getName();
		buffer.fill(0, 128-player->getName().length());
		buffer << player->getChampion()->getType();
		buffer.fill(0, 40-player->getChampion()->getType().length());
		buffer << (float)0.f; // deathDurationRemaining
		buffer << (float)0.f; // timeSinceDeath
      buffer << (uint32)0; // UNK (4.18)
		buffer << (uint8)0; // bitField
	}
};

struct HeroSpawn2 : public BasePacket {
    HeroSpawn2(Champion* p) : BasePacket(PKT_S2C_ObjectSpawn, p->getNetId()) {
        buffer.fill(0, 15);
        buffer << (uint8)0x80; // unk
        buffer << (uint8)0x3F; // unk
        buffer.fill(0, 13);
        buffer << (uint8)3; // unk
        buffer << (uint32)1; // unk
        buffer << p->getX();
        buffer << p->getY();
        buffer << (float)0x3F441B7D; // z ?
        buffer << (float)0x3F248DBB; // Rotation ?
    }
};

class TurretSpawn : public BasePacket {
public:
   TurretSpawn(Turret* t) : BasePacket(PKT_S2C_TurretSpawn) {
      buffer << t->getNetId();
      buffer << t->getName();
      buffer.fill(0, 64 - t->getName().length());
      buffer << "\x00\x22\x00\x00\x80\x01";
   }

   /*PacketHeader header;
   uint32 tID;
   uint8 name[28];
   uint8 type[42];*/
};

struct GameTimer {
    GameTimer(float fTime) {
        header.cmd = PKT_S2C_GameTimer;
        header.netId = 0;
        this->fTime = fTime;
    }
    PacketHeader header;
    float fTime;
};
struct GameTimerUpdate {
    GameTimerUpdate(float fTime) {
        header.cmd = PKT_S2C_GameTimerUpdate;
        header.netId = 0;
        this->fTime = fTime;
    }
    PacketHeader header;
    float fTime;
};

struct HeartBeat {
   PacketHeader header;
   float receiveTime;
   float ackTime;
};

struct SpellSet {
    SpellSet(uint32 netID, uint32 _spellID, uint32 _level) {
        memset(this, 0, sizeof(SpellSet));
        header.cmd = PacketCmd(0x5A);
        header.netId = netID;
        spellID = _spellID;
        level = _level;
    }
    PacketHeader header;
    uint32 spellID;
    uint32 level;
};

typedef struct _SkillUpPacket {
    PacketHeader header;
    uint8 skill;
} SkillUpPacket;

typedef struct _BuyItemReq {
    PacketHeader header;
    uint32 id;
} BuyItemReq;

typedef struct _BuyItemAns {
    _BuyItemAns() {
        header.cmd = (PacketCmd)PKT_S2C_BuyItemAns;
        unk2 = 0;
        unk3 = 0x40;
    }
    PacketHeader header;
    uint32 itemId;
    uint8 slotId;
    uint8 stack;
    uint8 unk2;
    uint8 unk3;
} BuyItemAns;

typedef struct _SellItem {
    PacketHeader header;
    uint8 slotId;
} SellItem;

class RemoveItem : public BasePacket {
public:
   RemoveItem(Unit* u, uint8 slot, uint8 remaining) : BasePacket(PKT_S2C_RemoveItem, u->getNetId()) {
      buffer << slot;
      buffer << remaining;
   }
};

typedef struct _EmotionPacket {
    PacketHeader header;
    uint8 id;
} EmotionPacket;

struct SwapItemsReq {
    PacketHeader header;
    uint8 slotFrom;
    uint8 slotTo;
};

class SwapItemsAns : public BasePacket {
public:
   SwapItemsAns(Champion* c, uint8 slotFrom, uint8 slotTo) : BasePacket(PKT_S2C_SwapItems, c->getNetId()) {
      buffer << slotFrom << slotTo;
   }
};

typedef struct _EmotionResponse {
    _EmotionResponse() {
        header.cmd = PKT_S2C_Emotion;
    }
    PacketHeader header;
    uint8 id;
} EmotionResponse;

/* New Style Packets */

class Announce : public BasePacket {
public:
   Announce(uint8 messageId, uint32 mapId = 0) : BasePacket(PKT_S2C_Announce) {
      buffer << messageId;
      buffer << (uint64)0;
      
      if (mapId > 0) {
         buffer << mapId;
      }
   }
};

class AddBuff : public Packet {
public:
   AddBuff(Unit* u, Unit* source, int stacks, std::string name) : Packet(PKT_S2C_AddBuff) {
      buffer << u->getNetId();//target
      
      buffer << (uint8) 0x05; //maybe type?
      buffer << (uint8) 0x02;
      buffer << (uint8) 0x01; // stacks
      buffer << (uint8) 0x00; // bool value
      buffer << RAFFile::getHash(name);
      buffer << (uint8) 0xde;
      buffer << (uint8) 0x88;
      buffer << (uint8) 0xc6;
      buffer << (uint8) 0xee;
      buffer << (uint8) 0x00;
      buffer << (uint8) 0x00;
      buffer << (uint8) 0x00;
      buffer << (uint8) 0x00;
      buffer << (uint8) 0x00;
      buffer << (uint8) 0x50;
      buffer << (uint8) 0xc3;
      buffer << (uint8) 0x46;
     
      if (source != 0) {
         buffer << source->getNetId(); //source
      } else {
         buffer << (uint32)0;
      }
   }
};

class RemoveBuff : public BasePacket {
public:
   RemoveBuff(Unit* u, std::string name) : BasePacket(PKT_S2C_RemoveBuff, u->getNetId()) {
      buffer << (uint8)0x05;
      buffer << RAFFile::getHash(name);
      buffer << (uint32) 0x0;
      //buffer << u->getNetId();//source?
   }
};

class DamageDone : public BasePacket {
public:
   DamageDone(Unit* source, Unit* target, float amount, DamageType type) : BasePacket(PKT_S2C_DamageDone, target->getNetId()) {
      buffer << (uint8)((type << 4) | 0x04);
      buffer << (uint16)0x4B; // 4.18
      buffer << amount; // 4.18
      buffer << target->getNetId();
      buffer << source->getNetId();
   }
};

class NpcDie : public ExtendedPacket {
public:
   NpcDie(Unit* die, Unit* killer) : ExtendedPacket(EPKT_S2C_NPC_Die, die->getNetId()) {
      buffer << (uint32)0;
      buffer << (uint8)0;
      buffer << killer->getNetId();
      buffer << (uint8)0; // unk
      buffer << (uint8)7; // unk
      buffer << (uint32)0; // Flags?
   }
};

class LoadScreenPlayerName : public Packet {
public:
   LoadScreenPlayerName(const ClientInfo& player) : Packet(PKT_S2C_LoadName) {
      buffer << (uint32)player.userId;
	  buffer << (uint16)clock();
	  buffer << 0x8E00; //sometimes 0x8E02
      buffer << (uint32)0;
      buffer << (uint32)player.getName().length()+1;
      buffer << player.getName();
      buffer << (uint8)0;
   }

    /*uint8 cmd;
    uint64 userId;
    uint32 skinId;
    uint32 length;
    uint8* description;*/
};

class LoadScreenPlayerChampion : public Packet {
public:
   LoadScreenPlayerChampion(const ClientInfo& player) : Packet(PKT_S2C_LoadHero) {
      buffer << player.userId;
      buffer << player.skinNo;
      buffer << (uint32)player.getChampion()->getType().length()+1;
      buffer << player.getChampion()->getType();
      buffer << (uint8)0;
   }

    /*uint8 cmd;
    uint64 userId;
    uint32 skinId;
    uint32 length;
    uint8* description;*/
};

struct AttentionPing {
    AttentionPing() {
    }
    AttentionPing(AttentionPing *ping) {
        cmd = ping->cmd;
        unk1 = ping->unk1;
        x = ping->x;
        y = ping->y;
        targetNetId = ping->targetNetId;
        type = ping->type;
    }

    uint8 cmd;
    uint32 unk1;
    float x;
    float y;
    uint32 targetNetId;
    uint8 type;
};

class AttentionPingAns : public Packet {
public:
   AttentionPingAns(ClientInfo *player, AttentionPing *ping) : Packet(PKT_S2C_AttentionPing){
      buffer << (uint32)0; //unk1
      buffer << ping->x;
      buffer << ping->y;
      buffer << ping->targetNetId;
      buffer << (uint32)player->getChampion()->getNetId();
      buffer << ping->type;
      buffer << (uint8)0xFB; // 4.18
      /*
      switch (ping->type)
      {
         case 0:
            buffer << (uint8)0xb0;
            break;
         case 1:
            buffer << (uint8)0xb1;
            break;
         case 2:
            buffer << (uint8)0xb2; // Danger
            break;
         case 3:
            buffer << (uint8)0xb3; // Enemy Missing
            break;
         case 4:
            buffer << (uint8)0xb4; // On My Way
            break;
         case 5:
            buffer << (uint8)0xb5; // Retreat / Fall Back
            break;
         case 6:
            buffer << (uint8)0xb6; // Assistance Needed
            break;            
      }
      */
   }
};

class BeginAutoAttack : public BasePacket {
public:
   BeginAutoAttack(Unit* attacker, Unit* attacked, uint32 futureProjNetId, bool isCritical) : BasePacket(PKT_S2C_BeginAutoAttack, attacker->getNetId()) {
      buffer << attacked->getNetId();
      buffer << (uint8)0x80; // unk
      buffer << futureProjNetId; // Basic attack projectile ID, to be spawned later
      if (isCritical)
        buffer << (uint8)0x49;
      else
        buffer << (uint8)0x40; // unk -- seems to be flags related to things like critical strike (0x49)
      // not sure what this is, but it should be correct (or maybe attacked x z y?) - 4.18
      buffer << (uint8)0x80;
      buffer << (uint8)0x01;
      buffer << MovementVector::targetXToNormalFormat(attacked->getX());
      buffer << (uint8)0x80;
      buffer << (uint8)0x01;
      buffer << MovementVector::targetYToNormalFormat(attacked->getY());
      buffer << (uint8)0xCC;
      buffer << (uint8)0x35;
      buffer << (uint8)0xC4;
      buffer << (uint8)0xD1;
      buffer << attacker->getX() << attacker->getY();
   }
};

class NextAutoAttack : public BasePacket {
public:
   NextAutoAttack(Unit* attacker, Unit* attacked, uint32 futureProjNetId, bool isCritical, bool initial) : BasePacket(PKT_S2C_NextAutoAttack, attacker->getNetId()) {
      buffer << attacked->getNetId();
      if (initial)
         buffer << (uint8)0x80; // These flags appear to change only to 0x80 and 0x7F after the first autoattack.
      else
         buffer << (uint8)0x7F;
         
      buffer << futureProjNetId;
      if (isCritical)
        buffer << (uint8)0x49;
      else
        buffer << (uint8)0x40; // unk -- seems to be flags related to things like critical strike (0x49)

      // not sure what this is, but it should be correct (or maybe attacked x z y?) - 4.18
      buffer << "\x40\x01\x7B\xEF\xEF\x01\x2E\x55\x55\x35\x94\xD3";
   }
};

class StopAutoAttack : public BasePacket {
public:
   StopAutoAttack(Unit* attacker) : BasePacket(PKT_S2C_StopAutoAttack, attacker->getNetId()) {
      buffer << (uint32)0; // Unk. Rarely, this is a net ID. Dunno what for.
      buffer << (uint8)3; // Unk. Sometimes "2", sometimes "11" when the above netId is not 0.
   }
};

class OnAttack : public ExtendedPacket {
public:
   OnAttack(Unit* attacker, Unit* attacked, AttackType attackType) : ExtendedPacket(EPKT_S2C_OnAttack, attacker->getNetId()) {
      buffer << (uint8)attackType;
      buffer << attacked->getX();
      buffer << attacked->getZ();
      buffer << attacked->getY();
      buffer << attacked->getNetId();
   }
};

class SetTarget : public BasePacket {
public:
   SetTarget(Unit* attacker, Unit* attacked) : BasePacket(PKT_S2C_SetTarget, attacker->getNetId()) {
      if (attacked != 0) {
         buffer << attacked->getNetId();
      } else {
         buffer << (uint32)0;
      }
   }

};

class SetTarget2 : public BasePacket {
public:
   SetTarget2(Unit* attacker, Unit* attacked) : BasePacket(PKT_S2C_SetTarget2, attacker->getNetId()) {
      if (attacked != 0) {
         buffer << attacked->getNetId();
      }
      else {
         buffer << (uint32)0;
      }
   }

};

class ChampionDie : public BasePacket {
public:
   ChampionDie(Champion* die, Unit* killer, uint32 goldFromKill) : BasePacket(PKT_S2C_ChampionDie, die->getNetId()) {
      buffer << goldFromKill; // Gold from kill?
      buffer << (uint8)0;
      if (killer != 0)
         buffer << killer->getNetId();
      else
         buffer << (uint32)0;

      buffer << (uint8)0;
      buffer << (uint8)7;
      buffer << die->getRespawnTimer() / 1000000.f; // Respawn timer, float
   }
};

class ChampionDeathTimer : public ExtendedPacket {
public:
   ChampionDeathTimer(Champion* die) : ExtendedPacket(EPKT_S2C_ChampionDeathTimer, die->getNetId()) {
      buffer << die->getRespawnTimer() / 1000000.f; // Respawn timer, float
   }
};

class ChampionRespawn : public BasePacket {
public:
   ChampionRespawn(Champion* c) : BasePacket(PKT_S2C_ChampionRespawn, c->getNetId()) {
      buffer << c->getX() << c->getY() << c->getZ();
   } 
};

class ShowProjectile : public BasePacket {
public:
   ShowProjectile(Projectile* p) : BasePacket(PKT_S2C_ShowProjectile, p->getOwner()->getNetId()) {
      buffer << p->getNetId();
   }
};

class SetHealth : public BasePacket {
public:
   SetHealth(Unit* u) : BasePacket(PKT_S2C_SetHealth, u->getNetId()) {
      buffer << (uint16)0x0000; // unk,maybe flags for physical/magical/true dmg
      buffer << u->getStats().getMaxHealth();
      buffer << u->getStats().getCurrentHealth();
   }
   
   SetHealth(uint32 itemHash) : BasePacket(PKT_S2C_SetHealth, itemHash) {
      buffer << (uint16)0;
   }
};

class SkillUpResponse : public BasePacket {
public:
    SkillUpResponse(uint32 netId, uint8 skill, uint8 level, uint8 pointsLeft) : BasePacket(PKT_S2C_SkillUp, netId) {
        buffer << skill << level << pointsLeft;
    }
};

class TeleportRequest : public BasePacket {
    
public:
    TeleportRequest(int netId,float x, float y, bool first) : BasePacket(PKT_S2C_MoveAns, (uint32) 0x0){
      buffer << (uint32) clock();//not 100% sure
      buffer << (uint8) 0x01;
      buffer << (uint8) 0x00;
      if(first == true){
      buffer << (uint8) 0x02;
      }else{
          buffer << (uint8) 0x03;
      }///      }//seems to be id, 02 = before teleporting, 03 = do teleport
      buffer << (uint32)netId;
      if(first == false){
          static uint8 a = 0x01;
          
          buffer << (uint8) a; // if it is the second part, send 0x01 before coords
          a++;
      }
      buffer << (uint16)x;
      buffer << (uint16)y;
    }
    
};




struct CastSpell {
    PacketHeader header;
    uint8 spellSlotType; // 4.18 [deprecated? -> 2 first(highest) bits: 10 - ability or item, 01 - summoner spell]
    uint8 spellSlot; // 0-3 [0-1 if spellSlotType has summoner spell bits set]
    float x, y;
    float x2, y2;
    uint32 targetNetId; // If 0, use coordinates, else use target net id
};

class CastSpellAns : public GamePacket {
public:
   CastSpellAns(Spell* s, float x, float y, uint32 futureProjNetId, uint32 spellNetId) : GamePacket(PKT_S2C_CastSpellAns, s->getOwner()->getNetId()) {
      Map* m = s->getOwner()->getMap();
   
      buffer << (uint8)0 << (uint8)0x66 << (uint8)0x00; // unk
      buffer << s->getId(); // Spell hash, for example hash("EzrealMysticShot")
      buffer << (uint32)spellNetId; // Spell net ID
      buffer << (uint8)0; // unk
      buffer << 1.0f; // unk
      buffer << s->getOwner()->getNetId() << s->getOwner()->getNetId();
      buffer << (uint32)s->getOwner()->getChampionHash();
      buffer << (uint32)futureProjNetId; // The projectile ID that will be spawned
      buffer << x << m->getHeightAtLocation(x, y) << y;
      buffer << x << m->getHeightAtLocation(x, y) << y;
      buffer << (uint8)0; // unk
      buffer << s->getCastTime();
      buffer << (float)0.f; // unk
      buffer << (float)1.0f; // unk
      buffer << s->getCooldown();
      buffer << (float)0.f; // unk
      buffer << (uint8)0; // unk
      buffer << s->getSlot(); 
      buffer << s->getCost();
      buffer << s->getOwner()->getX() << s->getOwner()->getZ() << s->getOwner()->getY();
      buffer << (uint64)1; // unk
   }
};

class PlayerInfo : public BasePacket{

public:

   PlayerInfo(ClientInfo* player) : BasePacket(PKT_S2C_PlayerInfo, player->getChampion()->getNetId()){
   
   buffer << (uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x7D  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x83  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xA9  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xC5  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xD7  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xD7  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0xD7  <<(uint8) 0x14  <<(uint8) 0x00  <<(uint8) 0x00;

   buffer << player->summonerSkills[0];
   buffer << player->summonerSkills[1];
 
   buffer <<(uint8) 0x41  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x42  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x04  <<(uint8) 0x52  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x03  <<(uint8) 0x61  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x62  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x64  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x03  <<(uint8) 0x71  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x72  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x03  <<(uint8) 0x82  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x03  <<(uint8) 0x92  <<(uint8) 0x74  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x41  <<(uint8) 0x75  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x42  <<(uint8) 0x75  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x02  <<(uint8) 0x43  <<(uint8) 0x75  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x02  <<(uint8) 0x52  <<(uint8) 0x75  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x03  <<(uint8) 0x62  <<(uint8) 0x75  <<(uint8) 0x03  <<(uint8) 0x00  <<(uint8) 0x01  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x00  <<(uint8) 0x1E  <<(uint8) 0x00;
   
   }


};

class SpawnProjectile : public BasePacket {
public:
   SpawnProjectile(Projectile* p) : BasePacket(PKT_S2C_SpawnProjectile, p->getNetId()) {
      float targetZ = p->getMap()->getHeightAtLocation(p->getTarget()->getX(), p->getTarget()->getY());
   
      buffer << p->getX() << p->getZ() << p->getY();
      buffer << p->getX() << p->getZ() << p->getY();
      buffer << (uint64)0x000000003f510fe2; // unk
      buffer << (float)0.577f; // unk
      buffer << p->getTarget()->getX() << targetZ << p->getTarget()->getY();
      buffer << p->getX() << p->getZ() << p->getY();
      buffer << p->getTarget()->getX() << targetZ << p->getTarget()->getY();
      buffer << p->getX() << p->getZ() << p->getY();
      buffer << uint32(0); // unk
      buffer << p->getMoveSpeed(); // Projectile speed
      buffer << (uint64)0x00000000d5002fce; // unk
      buffer << (uint32)0x7f7fffff; // unk
      buffer << (uint8)0 << (uint8)0x66 << (uint8)0;
      buffer << (uint32)p->getProjectileId(); // unk (projectile ID)
      buffer << (uint32)0; // Second net ID
      buffer << (uint8)0; // unk
      buffer << 1.0f;
      buffer << p->getOwner()->getNetId() << p->getOwner()->getNetId();
      
      Champion* c = dynamic_cast<Champion*>(p->getOwner());
      if(c) {
         buffer << (uint32)c->getChampionHash();
      } else {
         buffer << (uint32)0;
      }
      
      buffer << p->getNetId();
      buffer << p->getTarget()->getX() << targetZ << p->getTarget()->getY();
      buffer << p->getTarget()->getX() << targetZ << p->getTarget()->getY();
      buffer << (uint32)0x80000000; // unk
      buffer << (uint32)0x000000bf; // unk
      buffer << (uint32)0x80000000; // unk
      buffer << (uint32)0x2fd5843f; // unk
      buffer << (uint32)0x00000000; // unk
      buffer << (uint16)0x0000; // unk
      buffer << (uint8)0x2f; // unk
      buffer << (uint32)0x00000000; // unk
      buffer << p->getX() << p->getZ() << p->getY();
      buffer << (uint64)0x0000000000000000; // unk
   }

};

class SpawnParticle : public BasePacket {
public:
   SpawnParticle(Champion* owner, Target* t, const std::string& particle, uint32 netId) : BasePacket(PKT_S2C_SpawnParticle, owner->getNetId()) {
      buffer << (uint8)1; // number of particles
      buffer << owner->getChampionHash();
      buffer << RAFFile::getHash(particle);
      buffer << (uint32)0x00000020; // flags ?
      buffer << (uint32)0; // unk
      buffer << (uint16)0; // unk
      buffer << (uint8)1; // number of targets ?
      buffer << owner->getNetId();
      buffer << netId; // Particle net id ?
      buffer << owner->getNetId();
      
      if(t->isSimpleTarget()) {
         buffer << (uint32)0;
      } else {
         buffer << static_cast<Object*>(t)->getNetId();
      }
      
      buffer << (uint32)0; // unk
      
      for(int i = 0; i < 3; ++i) {
         buffer << static_cast<int16>((t->getX() - MAP_WIDTH)/2);
         buffer << 50.f;
         buffer << static_cast<int16>((t->getY() - MAP_HEIGHT)/2);
      }
      
      buffer << (uint32)0; // unk
      buffer << (uint32)0; // unk
      buffer << (uint32)0; // unk
      buffer << (uint32)0; // unk
      buffer << 1.f; // unk
      
   }
};

class DestroyProjectile : public BasePacket {
public:
   DestroyProjectile(Projectile* p) : BasePacket(PKT_S2C_DestroyProjectile, p->getNetId()) { }
};

class UpdateStats : public GamePacket {
public:
   UpdateStats(Unit* u, bool partial = true) : GamePacket(PKT_S2C_CharStats, 0) {
      std::map<uint8, std::set<uint32> > stats;
   
      if(partial) {
         stats = u->getStats().getUpdatedStats();
      } else {
         stats = u->getStats().getAllStats();
      }
      
      std::set<uint8> masks;
      uint8 masterMask = 0;
      
      for(auto& p : stats) {
         masterMask |= p.first;
         masks.insert(p.first);
      }
      
      buffer << (uint8)1;
      buffer << masterMask;
      buffer << u->getNetId();
      
      for(uint8 m : masks) {
         uint32 mask = 0;
         uint8 size = 0;
         
         const std::set<uint32>& updatedStats = stats.find(m)->second;
         
         for(auto it = updatedStats.begin(); it != updatedStats.end(); ++it) {      
            size += u->getStats().getSize(m, *it);
            mask |= *it;
         }
         
         buffer << mask;
         buffer << size;
         
         for(int i = 0; i < 32; ++i) {
            uint32 tmpMask = (1 << i);
            if(tmpMask & mask) {
               if(u->getStats().getSize(m, tmpMask) == 4) {
                  float f = u->getStats().getStat(m, tmpMask);
                  unsigned char *c = reinterpret_cast<unsigned char *>(&f);
                  if(c[0] >= 0xFE) {
                     c[0] = 0xFD;
                  }
                  buffer << f;
               } else if(u->getStats().getSize(m, tmpMask) == 2) {
                  uint16 stat = (uint16)floor(u->getStats().getStat(m, tmpMask) + 0.5);
                  buffer << stat;
               } else {
                  uint8 stat = (uint8)floor(u->getStats().getStat(m, tmpMask) + 0.5);
                  buffer << stat;
               }
            }
         }
      }
   }
};

class LevelPropSpawn : public BasePacket {
    public:
        LevelPropSpawn(LevelProp* lp) : BasePacket(PKT_S2C_LevelPropSpawn) {
            buffer << lp->getNetId();
            buffer << (uint32)0x00000040; // unk
            buffer << (uint8)0; // unk
            buffer << lp->getX() << lp->getZ() << lp->getY();
            buffer << 0.f; // Rotation Y
            
            buffer << lp->getDirectionX() << lp->getDirectionZ() << lp->getDirectionY();
            buffer << lp->getUnk1() << lp->getUnk2();

            buffer << 1.0f << 1.0f << 1.0f; // Scaling
            buffer << (uint32)300; // unk
            buffer << (uint32)2; // nPropType [size 1 -> 4] (4.18) -- if is a prop, become unselectable and use direction params
            
            buffer << lp->getName();
            buffer.fill(0, 64-lp->getName().length());
            buffer << lp->getType();
            buffer.fill(0, 64-lp->getType().length());
        }
        
        // TODO : remove this once we find a better solution for jungle camp spawning command
        LevelPropSpawn(uint32 netId, const std::string& name, const std::string& type, float x, float y, float z, float dirX, float dirY, float dirZ, float unk1, float unk2) : BasePacket(PKT_S2C_LevelPropSpawn) {
            buffer << netId;
            buffer << (uint32)0x00000040; // unk
            buffer << (uint8)0; // unk
            buffer << x << z << y;
            buffer << 0.f; // Rotation Y
            buffer << dirX << dirZ << dirY; // Direction
            buffer << unk1 << unk2;
            buffer << 1.0f << 1.0f << 1.0f; // Scaling
            buffer << (uint32)300; // unk
            buffer << (uint8)1; // bIsProp -- if is a prop, become unselectable and use direction params
            buffer << name;
            buffer.fill(0, 64-name.length());
            buffer << type;
            buffer.fill(0, 64-type.length());
        }

};

struct ViewRequest {
    uint8 cmd;
    uint32 unk1;
    float x;
    float zoom;
    float y;
    float y2;		//Unk
    uint32 width;	//Unk
    uint32 height;	//Unk
    uint32 unk2;	//Unk
    uint8 requestNo;
};

class LevelUp : public BasePacket {
public:
   LevelUp(Champion* c) : BasePacket(PKT_S2C_LevelUp, c->getNetId()) {
      buffer << c->getStats().getLevel();
      buffer << c->getSkillPoints();
   }
};

class ViewAnswer : public Packet {
public:
   ViewAnswer(ViewRequest *request) : Packet(PKT_S2C_ViewAns) {
      buffer << request->unk1;
   }
   void setRequestNo(uint8 requestNo){
      buffer << requestNo;
   }
};

class DebugMessage : public BasePacket {
public:
   DebugMessage(const std::string& message) : BasePacket(PKT_S2C_DebugMessage) {
      buffer << (uint32)0;
      buffer << message;
      buffer.fill(0, 512-message.length());
   }
};


class SetCooldown : public BasePacket {
public:
   SetCooldown(uint32 netId, uint8 slotId, float currentCd, float totalCd = 0.0f)
         : BasePacket(PKT_S2C_SetCooldown, netId) {
      buffer << slotId;
      buffer << (uint8)0xF8; // 4.18
      buffer << totalCd;
      buffer << currentCd;
   }
};

/* End New Packets */

#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

#endif
