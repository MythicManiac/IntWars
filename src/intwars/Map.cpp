#include "Map.h"
#include "Game.h"
#include "Unit.h"
#include "CollisionHandler.h"
#include "Minion.h"
#include "Logger.h"

Map::Map(Game* game, uint64 firstSpawnTime, uint64 spawnInterval, uint64 firstGoldTime, bool hasFountainHeal = false) : game(game), waveNumber(0), firstSpawnTime(firstSpawnTime), firstGoldTime(firstGoldTime), spawnInterval(spawnInterval), gameTime(0), nextSpawnTime(firstSpawnTime), nextSyncTime(10 * 1000000), firstBlood(true), killReduction(true), hasFountainHeal(hasFountainHeal)
{
   collisionHandler = new CollisionHandler(this);
   fountain = new Fountain();
}

Map::~Map()
{ 
   delete collisionHandler; 
   delete fountain;
}

void Map::update(int64 diff) {

   for(auto kv = objects.begin(); kv != objects.end();) {
	   if (kv->second->isToRemove()) 
	   {
         if (kv->second->getAttackerCount() == 0)
         {
            //collisionHandler->stackChanged(kv->second);
            collisionHandler->removeObject(kv->second);
            delete kv->second;
            kv = objects.erase(kv);
         }
         else
         {
            // do nothing for this unit while we wait for
            // attackers to finish
            ++kv;
         }

			continue;
		}

      if(kv->second->isMovementUpdated()) {
         game->notifyMovement(kv->second);
         kv->second->clearMovementUpdated();
      }
      
      Unit* u = dynamic_cast<Unit*>(kv->second);

      if(!u) {
         kv->second->update(diff);
         ++kv;
         continue;
      }
      
      for(uint32 i = 0; i < 2; ++i) {
         if(u->getTeam() == i) {
            continue;
         }
         
         if(visionUnits[u->getTeam()].find(u->getNetId()) != visionUnits[u->getTeam()].end() && teamHasVisionOn(i, u)) {
            u->setVisibleByTeam(i, true);
            game->notifySpawn(u);
            visionUnits[u->getTeam()].erase(u->getNetId());
            game->notifyUpdatedStats(u, false);
            continue;
         }

         if(!u->isVisibleByTeam(i) && teamHasVisionOn(i, u)) {
            game->notifyEnterVision(u, i);
            u->setVisibleByTeam(i, true);
            game->notifyUpdatedStats(u, false);
         } else if(u->isVisibleByTeam(i) && !teamHasVisionOn(i, u)) {
            game->notifyLeaveVision(u, i);
            u->setVisibleByTeam(i, false);
         }
      }
      
      if(u->buffs.size() != 0){
    
          for(int i = u->buffs.size(); i>0;i--){

              if(u->buffs[i-1]->needsToRemove()){
                  u->buffs.erase(u->getBuffs().begin() + (i-1));
                  //todo move this to Buff.cpp and add every stat
                  u->getStats().addMovementSpeedPercentageModifier(-u->getBuffs()[i-1]->getMovementSpeedPercentModifier());
                  continue;
              }
              u->buffs[i-1]->update(diff);
          }
      }

      if(!u->getStats().getUpdatedStats().empty()) {
         game->notifyUpdatedStats(u);
         u->getStats().clearUpdatedStats();
      }
      
      if(u->getStats().isUpdatedHealth()) {
         game->notifySetHealth(u);
         u->getStats().clearUpdatedHealth();
      }
      
      if(u->isModelUpdated()) {
         game->notifyModelUpdate(u);
         u->clearModelUpdated();
      }
      
      kv->second->update(diff);
      ++kv;
	}

	collisionHandler->update(diff);

   for (auto i = announcerEvents.begin(); i != announcerEvents.end(); i++) {
      bool isCompleted = (*i).first;

      if (!isCompleted) {
         uint64 eventTime = std::get<0>((*i).second);
         uint8 messageId = std::get<1>((*i).second);
         bool isMapSpecific = std::get<2>((*i).second);

         if (gameTime >= eventTime) {
            game->notifyAnnounceEvent(messageId, isMapSpecific);
            (*i).first = true;
         }
      }
   }
   
   gameTime += diff;
   nextSyncTime += diff;

   // By default, synchronize the game time every 10 seconds
   if (nextSyncTime >= 10 * 1000000) {
      game->notifyGameTimer();
      nextSyncTime = 0;
   }
   
   if(waveNumber) { 
      if(gameTime >= nextSpawnTime+waveNumber*8*100000) { // Spawn new wave every 0.8s
         if(spawn()) {
            waveNumber = 0;
            nextSpawnTime += spawnInterval;
         } else {
            ++waveNumber;
         }
      }  
   } else if(gameTime >= nextSpawnTime) {
      spawn();
      ++waveNumber;
   }
   
   if (hasFountainHeal)
      fountain->healChampions(this, diff);
}

Object* Map::getObjectById(uint32 id) {
   if(objects.find(id) == objects.end()) {
      return 0;
   }
   
   return objects[id];
}

void Map::addObject(Object* o) {
   objects[o->getNetId()] = o;
   
   Unit* u = dynamic_cast<Unit*>(o);
   if(!u) {
      return;
   }

	collisionHandler->addObject(o);

   visionUnits[o->getTeam()][o->getNetId()] = u;
   
   Minion* m = dynamic_cast<Minion*>(u);
   
   if(m) {
      game->notifyMinionSpawned(m, m->getTeam());
   }
   
   Champion* c = dynamic_cast<Champion*>(o);
   
   if(c) {
      champions[c->getNetId()] = c;
      game->notifyChampionSpawned(c, c->getTeam());
   }
}

void Map::removeObject(Object* o) {
   Champion * c = dynamic_cast<Champion*>(o);
   
   if (c) {
      champions.erase(c->getNetId());
   }
   
   objects.erase(o->getNetId());
   visionUnits[o->getTeam()].erase(o->getNetId());
}

void Map::stopTargeting(Unit* target) {
   for(auto kv = objects.begin(); kv != objects.end(); ++kv) {
      Unit* u = dynamic_cast<Unit*>(kv->second);

      if(!u) {
         continue;
      }
      
      if(u->getTargetUnit() == target) {
         u->setTargetUnit(0);
         u->setAutoAttackTarget(0);
         game->notifySetTarget(u, 0);
      }
   }
}

std::vector<Champion*> Map::getChampionsInRange(Target* t, float range, bool isAlive) {
	std::vector<Champion*> champs;
	for (auto kv = champions.begin(); kv != champions.end(); ++kv) {
		Champion* c = kv->second;
		if (t->distanceWith(c)<=range) {
			if(isAlive && !c->isDead() || !isAlive) {
				champs.push_back(c);
			}
		}
	}
	return champs;
}

std::vector<Unit*> Map::getUnitsInRange(Target* t, float range, bool isAlive) {
   std::vector<Unit*> units;
   for (auto kv = objects.begin(); kv != objects.end(); ++kv) {
      Unit* u = dynamic_cast<Unit*>(kv->second);
      if (u && t->distanceWith(u)<=range) {
         if(isAlive && !u->isDead() || !isAlive) {
            units.push_back(u);
         }
      }
   }
   return units;
}

bool Map::teamHasVisionOn(int team, Object* o) {
   
   if (o == 0){
      return false;
   }
   
   if(o->getTeam() == team) {
      return true;
   }

   for(auto kv : objects) 
   {
      if (kv.second->getTeam() == team && kv.second->distanceWith(o) < kv.second->getVisionRadius() && !mesh.isAnythingBetween(kv.second, o))
      {
         Unit * unit = dynamic_cast<Unit*>(kv.second);
         if ((unit) && unit->isDead()) continue;
         return true;
      }
   }
   
   return false;
}

MovementVector Map::toMovementVector(float x, float y) {
   return MovementVector((int16)((x-mesh.getWidth()/2)/2), (int16)((y-mesh.getHeight()/2)/2));
}
