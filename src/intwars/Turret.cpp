#include "Turret.h"
#include "Map.h"
#include "TurretStats.h"
#include "Game.h"

#define TURRET_RANGE 905.f

Turret::Turret(Map* map, uint32 id, const std::string& name, float x, float y, float hp, float ad, int team) : Unit(map, id, "", new TurretStats(), 50, x, y, 1200), name(name)  {
   stats->setCurrentHealth(hp);
   stats->setMaxHealth(hp);
   stats->setBaseAd(ad);
   stats->setRange(TURRET_RANGE);
   
   autoAttackDelay = 4.35f/30.f;
   autoAttackProjectileSpeed = 1200.f;
   
   setTeam(team);
}

void Turret::update(int64 diff)
{
   // No target : try to find a new one
   if(!isAttacking) {
      const std::map<uint32, Object*>& objects = map->getObjects();
      Unit* nextTarget = 0;
      unsigned int nextTargetPriority = 10;
      for(auto& it : objects) {
         Unit* u = dynamic_cast<Unit*>(it.second);

         if(!u || u->isDead() || u->getTeam() == getTeam() || distanceWith(u) > TURRET_RANGE) {
            continue;
         }
         
         // Note: this method means that if there are two champions within turret range,
         // The player to have been added to the game first will always be targeted before the others
         if (!targetUnit) {
            auto priority = classifyTarget(u);
            if (priority < nextTargetPriority) {
               nextTarget = u;
               nextTargetPriority = priority;
            }
         } else {
            Champion* targetIsChampion = dynamic_cast<Champion*>(targetUnit);
            
            // Is the current target a champion? If it is, don't do anything
            if (!targetIsChampion) {
               // Find the next champion in range targeting an enemy champion who is also in range
               Champion* enemyChamp = dynamic_cast<Champion*>(u);
               if (enemyChamp&& enemyChamp->getTargetUnit() != 0) {
                  Champion* enemyChampTarget = dynamic_cast<Champion*>(enemyChamp->getTargetUnit());
                  if (enemyChampTarget &&                                                                   // Enemy Champion is targeting an ally
                      enemyChamp->distanceWith(enemyChampTarget) <= enemyChamp->getStats().getRange() &&    // Enemy within range of ally
                      distanceWith(enemyChampTarget) <= TURRET_RANGE) {                                     // Enemy within range of this turret
                     nextTarget = enemyChamp; // No priority required
                     break;
                  }
               }
            }
         }
      }
      if (nextTarget) {
         targetUnit = nextTarget;
         map->getGame()->notifySetTarget(this, nextTarget);
      }
   }
   
   // Lose focus of the unit target if the target is out of range
   if(targetUnit && distanceWith(targetUnit) > TURRET_RANGE) {
      setTargetUnit(0);
      map->getGame()->notifySetTarget(this, 0);
   }

   Unit::update(diff);
}
