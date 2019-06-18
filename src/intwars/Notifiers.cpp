#include "stdafx.h"
#include "Game.h"
#include "Packets.h"
#include "Logger.h"

#include <iostream>

using namespace std;

void Game::notifyMinionSpawned(Minion* m, int team) {
   MinionSpawn ms(m);
   
   broadcastPacketTeam(team == 0 ? TEAM_BLUE : TEAM_PURPLE, ms, CHL_S2C);
   
   notifySetHealth(m);
}

void Game::notifySetHealth(Unit* u) {
   SetHealth sh(u);
   broadcastPacketVision(u, sh, CHL_S2C);
}

void Game::notifyUpdatedStats(Unit* u, bool partial) {
   UpdateStats us(u, partial);
   
   Turret* t = dynamic_cast<Turret*>(u);
   if(t) {
      broadcastPacket(us, CHL_LOW_PRIORITY, 2);
      return;
   }
   
   if(!partial) {
      broadcastPacketTeam((1-u->getTeam()) == 0 ? TEAM_BLUE : TEAM_PURPLE, us, CHL_LOW_PRIORITY, 2);
   } else {
      broadcastPacketVision(u, us, CHL_LOW_PRIORITY, 2);
   }
}

void Game::notifyAddBuff(Unit* u, Unit* source, std::string buffName) {
   AddBuff add(u, source, 1, buffName);
   broadcastPacket(add, CHL_S2C);
}

void Game::notifyRemoveBuff(Unit* u, std::string buffName) {
   RemoveBuff remove(u, buffName);
   broadcastPacket(remove, CHL_S2C);
}

void Game::notifyTeleport(Unit* u, float _x, float _y) {
   // Can't teleport to this point of the map
   if (!map->isWalkable(_x, _y)) {
      _x = MovementVector::targetXToNormalFormat(u->getPosition().X);
      _y = MovementVector::targetYToNormalFormat(u->getPosition().Y);
   } else {
      u->setPosition(_x, _y);

      //TeleportRequest first(u->getNetId(), u->teleportToX, u->teleportToY, true);
      //sendPacket(currentPeer, first, CHL_S2C);

      _x = MovementVector::targetXToNormalFormat(_x);
      _y = MovementVector::targetYToNormalFormat(_y);
   }

   TeleportRequest second(u->getNetId(), _x, _y, false);
   broadcastPacketVision(u, second, CHL_S2C);
}

void Game::notifyMovement(Object* o) {
   const std::vector<Vector2>& waypoints = o->getWaypoints();
   MovementAns *answer = MovementAns::create(waypoints.size()*2);
   
   answer->nbUpdates = 1;
   answer->netId = o->getNetId();
   for(size_t i = 0; i < waypoints.size(); i++) {
		answer->getVector(i)->x = MovementVector::targetXToNormalFormat(waypoints[i].X);
		answer->getVector(i)->y = MovementVector::targetYToNormalFormat(waypoints[i].Y);
   }
   
   broadcastPacketVision(o, reinterpret_cast<uint8 *>(answer), answer->size(), 4);
   MovementAns::destroy(answer);
}

void Game::notifyDamageDone(Unit* source, Unit* target, float amount, DamageType type) {
   DamageDone dd(source, target, amount, type);
   broadcastPacket(dd, CHL_S2C);
}

void Game::notifyBeginAutoAttack(Unit* attacker, Unit* victim, uint32 futureProjNetId, bool isCritical) {
   BeginAutoAttack aa(attacker, victim, futureProjNetId, isCritical);
   broadcastPacket(aa, CHL_S2C);
}

void Game::notifyNextAutoAttack(Unit* attacker, Unit* target, uint32 futureProjNetId, bool isCritical, bool nextAttackFlag) {
   NextAutoAttack aa(attacker, target, futureProjNetId, isCritical, nextAttackFlag);
   broadcastPacket(aa, CHL_S2C);
}

void Game::notifyOnAttack(Unit* attacker, Unit* attacked, AttackType attackType) {
   OnAttack oa(attacker, attacked, attackType);
   broadcastPacket(oa, CHL_S2C);
}

void Game::notifyProjectileSpawn(Projectile* p) {
   SpawnProjectile sp(p);
   broadcastPacket(sp, CHL_S2C);
}

void Game::notifyProjectileDestroy(Projectile* p) {
   DestroyProjectile dp(p);
   broadcastPacket(dp, CHL_S2C);
}

void Game::notifyParticleSpawn(Champion* source, Target* target, const std::string& particleName) {
   SpawnParticle sp(source, target, particleName, GetNewNetID());
   broadcastPacket(sp, CHL_S2C);
}

void Game::notifyModelUpdate(Unit* object) {
    UpdateModel mp(object->getNetId(), object->getModel().c_str());
    broadcastPacket(reinterpret_cast<uint8 *>(&mp), sizeof(UpdateModel), CHL_S2C);
}

void Game::notifyItemBought(Champion* c, const ItemInstance* i) {
   BuyItemAns response;
   response.header.netId = c->getNetId();
   response.itemId = i->getTemplate()->getId();
   response.slotId = i->getSlot();
   response.stack = i->getStacks();
   
   broadcastPacketVision(c, reinterpret_cast<uint8 *>(&response), sizeof(response), CHL_S2C);
}

void Game::notifyItemsSwapped(Champion* c, uint8 fromSlot, uint8 toSlot) {
   SwapItemsAns sia(c, fromSlot, toSlot);
   broadcastPacketVision(c, sia, CHL_S2C);
}

void Game::notifyLevelUp(Champion* c) {
   LevelUp lu(c);
   broadcastPacket(lu, CHL_S2C);
}

void Game::notifyRemoveItem(Champion* c, uint8 slot, uint8 remaining) {
   RemoveItem ri(c, slot, remaining);
   broadcastPacketVision(c, ri, CHL_S2C);
}

void Game::notifySetTarget(Unit* attacker, Unit* target) {
   SetTarget st(attacker, target);
   broadcastPacket(st, CHL_S2C);

   SetTarget2 st2(attacker, target);
   broadcastPacket(st2, CHL_S2C);
}

void Game::notifyChampionDie(Champion* die, Unit* killer, uint32 goldFromKill) {
   ChampionDie cd(die, killer, goldFromKill);
   broadcastPacket(cd, CHL_S2C);

   notifyChampionDeathTimer(die);
}

void Game::notifyChampionDeathTimer(Champion* die) {
   ChampionDeathTimer cdt(die);
   broadcastPacket(cdt, CHL_S2C);
}

void Game::notifyChampionRespawn(Champion* c) {
   ChampionRespawn cr(c);
   broadcastPacket(cr, CHL_S2C);
}

void Game::notifyShowProjectile(Projectile* p) {
   ShowProjectile sp(p);
   broadcastPacket(sp, CHL_S2C);
}

void Game::notifyNpcDie(Unit* die, Unit* killer) {
   NpcDie nd(die, killer);
   broadcastPacket(nd, CHL_S2C);
}

void Game::notifyAddGold(Champion* c, Unit* died, float gold) {
   AddGold ag(c, died, gold);
   broadcastPacket(ag, CHL_S2C);
}

void Game::notifyStopAutoAttack(Unit* attacker) {
   StopAutoAttack saa(attacker);
   broadcastPacket(saa, CHL_S2C);
}

void Game::notifyDebugMessage(std::string htmlDebugMessage) {
   DebugMessage dm(htmlDebugMessage);
   broadcastPacket(dm, CHL_S2C);
}

void Game::notifySpawn(Unit* u) {

   Minion* m = dynamic_cast<Minion*>(u);
   
   if(m) {
      notifyMinionSpawned(m, 1-m->getTeam());
   }
   
   Champion* c = dynamic_cast<Champion*>(u);
   
   if(c) {
      notifyChampionSpawned(c, 1-c->getTeam());
   }
   
   notifySetHealth(u);
}

void Game::notifyLeaveVision(Object* o, uint32 team) {
   LeaveVision lv(o);
   broadcastPacketTeam(team == 0 ? TEAM_BLUE : TEAM_PURPLE, lv, CHL_S2C);

   // Not exactly sure what this is yet
   Champion* c = dynamic_cast<Champion*>(o);
   if (!o) {
      DeleteObjectFromVision deleteObj(o);
      broadcastPacketTeam(team == 0 ? TEAM_BLUE : TEAM_PURPLE, deleteObj, CHL_S2C);
   }
}

void Game::notifyEnterVision(Object* o, uint32 team) {
   Minion* m = dynamic_cast<Minion*>(o);
   
   if(m) {
      EnterVisionAgain eva(m);
      broadcastPacketTeam(team == 0 ? TEAM_BLUE : TEAM_PURPLE, eva, CHL_S2C);
      notifySetHealth(m);
      return;
   }
   
   Champion* c = dynamic_cast<Champion*>(o);
   
   // TODO: Fix bug where enemy champion is not visible to user when vision is acquired until the enemy champion moves
   if(c) {
      EnterVisionAgain eva(c);
      broadcastPacketTeam(team == 0 ? TEAM_BLUE : TEAM_PURPLE, eva, CHL_S2C);
      notifySetHealth(c);
      return;
   }
}

void Game::notifyChampionSpawned(Champion* c, uint32 team) {
   HeroSpawn2 hs(c);
   broadcastPacketTeam(team == 0 ? TEAM_BLUE : TEAM_PURPLE, hs, CHL_S2C);
}

void Game::notifySetCooldown(Champion* c, uint8 slotId, float currentCd, float totalCd) {
   SetCooldown cd(c->getNetId(), slotId, currentCd, totalCd);
   broadcastPacket(cd, CHL_S2C);
}

void Game::notifyGameTimer() {
   float gameTime = map->getGameTime() / 1000000.f;
   GameTimer gameTimer(gameTime);
   broadcastPacket(reinterpret_cast<uint8 *>(&gameTimer), sizeof(GameTimer), CHL_S2C);
}

void Game::notifyAnnounceEvent(uint8 messageId, bool isMapSpecific) {
   Announce announce(messageId, isMapSpecific ? map->getMapId() : 0);
   broadcastPacket(announce, CHL_S2C);
}

void Game::notifySpellAnimation(Unit* u, const std::string& animation) {
   SpellAnimation sa(u, animation);
   broadcastPacketVision(u, sa, CHL_S2C);
}

void Game::notifySetAnimation(Unit* u, const std::vector<std::pair<std::string, std::string>>& animationPairs) {
   SetAnimation setAnimation(u, animationPairs);
   broadcastPacketVision(u, setAnimation, CHL_S2C);
}

void Game::notifyDash(Unit* u, float _x, float _y, float dashSpeed) {
   // TODO: Fix dash: it stays in the current location and doesn't hit a wall if the target location can't be reached
   float _z = u->getZ();

   /*if (!map->isWalkable(_x, _y)) {
      _x = u->getPosition().X;
      _y = u->getPosition().Y;
   }
   else {
      // Relative coordinates to dash towards
      float newX = _x;
      float newY = _y;
      _z -= map->getHeightAtLocation(_x, _y);
      _x = u->getPosition().X - _x;
      _y = u->getPosition().Y - _y;

      u->setPosition(newX, newY);
   }*/

   Dash dash(u, _x, _y, dashSpeed);
   broadcastPacketVision(u, dash, CHL_S2C);
}