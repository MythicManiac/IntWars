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
#ifndef _NETWORK_LISTENER_H
#define _NETWORK_LISTENER_H

#include <enet/enet.h>
#include <intlib/base64.h>
#include <intlib/blowfish.h>

#include "Map.h"
#include "common.h"
#include "Client.h"

class Minion;
class Packet;

#define HANDLE_ARGS ENetPeer *peer, ENetPacket *packet
#define PEER_MTU 996
#define RELIABLE ENET_PACKET_FLAG_RELIABLE
#define UNRELIABLE 0
#define GAME_VERSION "Version 4.20.0.315 [PUBLIC]"

#define peerInfo(p) ((ClientInfo*)p->data)

class Game
{
	public:
		Game();
		~Game();
                
      uint32 strToId(std::string str);

		bool initialize(ENetAddress *address, const char *baseKey);
		void netLoop();
      
   
		bool handlePacket(ENetPeer *peer, ENetPacket *packet, uint8 channelID);
      bool handleDisconnect(ENetPeer* peer);

		// Handlers
		bool handleNull(HANDLE_ARGS);
		bool handleKeyCheck(HANDLE_ARGS);
		bool handleLoadPing(HANDLE_ARGS);
		bool handleSpawn(HANDLE_ARGS);
		bool handleMap(HANDLE_ARGS);
		bool handleSynch(HANDLE_ARGS);
		bool handleGameNumber(HANDLE_ARGS);
		bool handleQueryStatus(HANDLE_ARGS);
		bool handleStartGame(HANDLE_ARGS);
		bool handleView(HANDLE_ARGS);
		bool handleMove(HANDLE_ARGS);
		bool affirmMove(HANDLE_ARGS);
		bool handleAttentionPing(HANDLE_ARGS);
		bool handleChatBoxMessage(HANDLE_ARGS);
		bool handleSkillUp(HANDLE_ARGS);
		bool handleEmotion(HANDLE_ARGS);
		bool handleBuyItem(HANDLE_ARGS);
		bool handleSellItem(HANDLE_ARGS);
      bool handleCastSpell(HANDLE_ARGS);
      bool handleClick(HANDLE_ARGS);
      bool handleSwapItems(HANDLE_ARGS);
      bool handleHeartBeat(HANDLE_ARGS);
      
      // Notifiers
      void notifyMinionSpawned(Minion* m, int team);
      void notifySetHealth(Unit* u);
      void notifyUpdatedStats(Unit* u, bool partial = true);
      void notifyMovement(Object* o);
      void notifyDamageDone(Unit* source, Unit* target, float amount, DamageType type = DAMAGE_TYPE_PHYSICAL);
      void notifyBeginAutoAttack(Unit* attacker, Unit* victim, uint32 futureProjNetId, bool isCritical);
      void notifyTeleport(Unit* u, float _x, float _y);
      void notifyProjectileSpawn(Projectile* p);
      void notifyProjectileDestroy(Projectile* p);
      void notifyParticleSpawn(Champion* source, Target* target, const std::string& particleName);
      void notifyModelUpdate(Unit* object);
      void notifyLevelUp(Champion* c);
      void notifyItemBought(Champion* c, const ItemInstance* i);
      void notifyItemsSwapped(Champion* c, uint8 fromSlot, uint8 toSlot);
      void notifyRemoveItem(Champion* c, uint8 slot, uint8 remaining);
      void notifySetTarget(Unit* attacker, Unit* target);
      void notifyChampionDie(Champion* die, Unit* killer, uint32 goldFromKill);
      void notifyChampionDeathTimer(Champion* die);
      void notifyChampionRespawn(Champion* c);
      void notifyShowProjectile(Projectile* p);
      void notifyNpcDie(Unit* die, Unit* killer);
      void notifyNextAutoAttack(Unit* attacker, Unit* target, uint32 futureProjNetId, bool isCritical, bool nextAttackFlag);
      void notifyOnAttack(Unit* attacker, Unit* attacked, AttackType attackType);
      void notifyAddBuff(Unit* u, Unit* source, std::string buffName);
      void notifyRemoveBuff(Unit* u, std::string buffName);
      void notifyAddGold(Champion* c, Unit* died, float gold);
      void notifyStopAutoAttack(Unit* attacker);
      void notifyDebugMessage(std::string htmlDebugMessage);
      void notifySpawn(Unit* u);
      void notifyLeaveVision(Object* o, uint32 team);
      void notifyEnterVision(Object* o, uint32 team);
      void notifyChampionSpawned(Champion* c, uint32 team);
      void notifySetCooldown(Champion* c, uint8 slotId, float currentCd, float totalCd = 0.0f);
      void notifyGameTimer();
      void notifyAnnounceEvent(uint8 messageId, bool isMapSpecific);
      void notifySpellAnimation(Unit* u, const std::string& animation);
      void notifySetAnimation(Unit* u, const std::vector<std::pair<std::string, std::string>>& animationPairs);
      void notifyDash(Unit* u, float _x, float _y, float dashSpeed);

		// Tools
		static void printPacket(const uint8 *buf, uint32 len);
		void printLine(uint8 *buf, uint32 len);
      
   protected:
		bool sendPacket(ENetPeer *peer, const uint8 *data, uint32 length, uint8 channelNo, uint32 flag = RELIABLE);
      bool sendPacket(ENetPeer *peer, const Packet& packet, uint8 channelNo, uint32 flag = RELIABLE);
		bool broadcastPacket(uint8 *data, uint32 length, uint8 channelNo, uint32 flag = RELIABLE);
      bool broadcastPacket(const Packet& packet, uint8 channelNo, uint32 flag = RELIABLE);
      bool broadcastPacketTeam(uint8 team, const uint8 *data, uint32 length, uint8 channelNo, uint32 flag = RELIABLE);
      bool broadcastPacketTeam(uint8 team, const Packet& packet, uint8 channelNo, uint32 flag = RELIABLE);
      bool broadcastPacketVision(Object* o, const Packet& packet, uint8 channelNo, uint32 flag = RELIABLE);
      bool broadcastPacketVision(Object* o, const uint8 *data, uint32 length, uint8 channelNo, uint32 flag = RELIABLE);

	private:
		bool _isAlive, _started;
      int playersReady;
		ENetHost *_server;
		BlowFish *_blowfish;
      ENetPeer* currentPeer;
      std::vector<ClientInfo*> players;
      
      void registerHandler(bool (Game::*handler)(HANDLE_ARGS), PacketCmd pktcmd,Channel c);
      bool (Game::*_handlerTable[0x100][0x7])(HANDLE_ARGS);
      void initHandlers();
      
      Map* map;
};

extern uint32 GetNewNetID();

#endif

