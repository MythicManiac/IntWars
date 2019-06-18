#include "Unit.h"
#include "Map.h"
#include "Game.h"
#include "Turret.h"
#include "Logger.h"
#include "Minion.h"

#include <algorithm>
#include <cstdlib>

#define EXP_RANGE 1400

using namespace std;

Unit::~Unit() {
    delete stats;
}

void Unit::update(int64 diff) {

   if (unitScript.isLoaded()) {
      try {
         unitScript.lua.get <sol::function> ("onUpdate").call <void> (diff);
      } catch (sol::error e) {
         CORE_ERROR("%s", e.what());
      }
   }

   if (isDead()) {
      if (targetUnit) {
         setTargetUnit(0);
         autoAttackTarget = 0;
         isAttacking = false;
         map->getGame()->notifySetTarget(this, 0);
         initialAttackDone = false;
      }
      return;
   }

   if(targetUnit){
      if (targetUnit->isDead() || !getMap()->teamHasVisionOn(getTeam(), targetUnit)) {
         setTargetUnit(0);
         isAttacking = false;
         map->getGame()->notifySetTarget(this, 0);
         initialAttackDone = false;
         
      } else if (isAttacking && autoAttackTarget) {
         autoAttackCurrentDelay += diff / 1000000.f;
         if (autoAttackCurrentDelay >= autoAttackDelay/stats->getAttackSpeedMultiplier()) {
            if (!isMelee()) {
               Projectile* p = new Projectile(map, autoAttackProjId, x, y, 5, this, autoAttackTarget, 0, autoAttackProjectileSpeed, 0);
               map->addObject(p);
               map->getGame()->notifyShowProjectile(p);
            } else {
               autoAttackHit(autoAttackTarget);
            }
            autoAttackCurrentCooldown = 1.f / (stats->getTotalAttackSpeed());
            isAttacking = false;
         }
         
      } else if (distanceWith(targetUnit) <= stats->getRange()) {
         refreshWaypoints();
         nextAutoIsCrit = rand() % 100 + 1 <= stats->getCritChance() * 100;
         if (autoAttackCurrentCooldown <= 0) {
            isAttacking = true;
            autoAttackCurrentDelay = 0;
            autoAttackProjId = GetNewNetID();
            autoAttackTarget = targetUnit;

            if (!initialAttackDone) {
               initialAttackDone = true;
               map->getGame()->notifyBeginAutoAttack(this, targetUnit, autoAttackProjId, nextAutoIsCrit);
            } else {
               nextAttackFlag = !nextAttackFlag; // The first auto attack frame has occurred
               map->getGame()->notifyNextAutoAttack(this, targetUnit, autoAttackProjId, nextAutoIsCrit, nextAttackFlag);
            }

            AttackType attackType = isMelee() ? ATTACK_TYPE_MELEE : ATTACK_TYPE_TARGETED;
            map->getGame()->notifyOnAttack(this, targetUnit, attackType);
         }
         
      } else {
         refreshWaypoints();
      }
      
   } else if (isAttacking) {
      if (!autoAttackTarget || 
          autoAttackTarget->isDead() || 
          !getMap()->teamHasVisionOn(getTeam(), autoAttackTarget)) {
         isAttacking = false;
         initialAttackDone = false;
         autoAttackTarget = 0;
      }
   }
   
   Object::update(diff);
   
   if (autoAttackCurrentCooldown > 0) {
      autoAttackCurrentCooldown -= diff / 1000000.f;
   }

   statUpdateTimer += diff;
   if (statUpdateTimer >= 500000) { // update stats (hpregen, manaregen) every 0.5 seconds
      stats->update(statUpdateTimer);
      statUpdateTimer = 0;
   }
}

void Unit::autoAttackHit(Unit* target) {
  float damage = (nextAutoIsCrit) ? stats->getCritDamagePct() * stats->getTotalAd() : stats->getTotalAd();
    dealDamageTo(target, damage, DAMAGE_TYPE_PHYSICAL, DAMAGE_SOURCE_ATTACK);
       if(unitScript.isLoaded()){
      try{
         unitScript.lua.get <sol::function> ("onAutoAttack").call <void> (target);
      }catch(sol::error e){
         CORE_ERROR("Error callback ondealdamage: %s", e.what());
      }
   }
}

/**
 * TODO : handle armor, magic resistance [...]
 */
void Unit::dealDamageTo(Unit* target, float damage, DamageType type, DamageSource source) {
    //CORE_INFO("0x%08X deals %f damage to 0x%08X !", getNetId(), damage, target->getNetId());
    
   if(unitScript.isLoaded()){
      try{
         /*damage = */ unitScript.lua.get <sol::function> ("onDealDamage").call <void> (target, damage, type, source);
      }catch(sol::error e){
         CORE_ERROR("Error callback ondealdamage: %s", e.what());
      }
   }
    
    
    float defense = 0;
    float regain = 0;
    switch (type) {
        case DAMAGE_TYPE_PHYSICAL:
            defense = target->getStats().getArmor();
            defense = ((1 - stats->getArmorPenPct()) * defense) - stats->getArmorPenFlat();
            
            break;
        case DAMAGE_TYPE_MAGICAL:
            defense = target->getStats().getMagicArmor();
            defense = ((1 - stats->getMagicPenPct()) * defense) - stats->getMagicPenFlat();
            break;
    }
    
    switch(source) {
        case DAMAGE_SOURCE_SPELL:
            regain = stats->getSpellVamp();
            break;
        case DAMAGE_SOURCE_ATTACK:
            regain = stats->getLifeSteal();
            break;
    }
    
    //Damage dealing. (based on leagueoflegends' wikia)
    damage = defense >= 0 ? (100 / (100 + defense)) * damage : (2 - (100 / (100 - defense))) * damage;

    target->getStats().setCurrentHealth(max(0.f, target->getStats().getCurrentHealth() - damage));
    if (!target->deathFlag && target->getStats().getCurrentHealth() <= 0) {
        target->deathFlag = true;
        target->die(this);
    }
    map->getGame()->notifyDamageDone(this, target, damage, type);
    
    //Get health from lifesteal/spellvamp
    if (regain != 0) {
        stats->setCurrentHealth(min(stats->getMaxHealth(), stats->getCurrentHealth() + (regain * damage)));
        map->getGame()->notifyUpdatedStats(this);
    }
}

bool Unit::isDead() const {
    return deathFlag;
}

void Unit::setModel(const std::string& newModel) {
    model = newModel;
    modelUpdated = true;
}

const std::string& Unit::getModel() {
    return model;
}

void Unit::die(Unit* killer) {
   setToRemove();
   map->stopTargeting(this);

   map->getGame()->notifyNpcDie(this, killer);

	float exp = map->getExperienceFor(this);
	auto champs = map->getChampionsInRange(this, EXP_RANGE, true);
	//Cull allied champions
	champs.erase(std::remove_if(champs.begin(), 
								champs.end(), 
								[this](Champion * l) { return l->getTeam() == getTeam(); }),
				champs.end());
	if (champs.size() > 0) {
		float expPerChamp = exp / champs.size();
		for (auto c = champs.begin(); c != champs.end(); ++c) {
			(*c)->getStats().setExp((*c)->getStats().getExperience() + expPerChamp);
		}
	}

   if (killer)
   {
      Champion* cKiller = dynamic_cast<Champion*>(killer);

      if (!cKiller) {
         return;
      }

      float gold = map->getGoldFor(this);

      if (!gold) {
         return;
      }

      cKiller->getStats().setGold(cKiller->getStats().getGold() + gold);
      map->getGame()->notifyAddGold(cKiller, this, gold);

      if (cKiller->killDeathCounter < 0){
         cKiller->setChampionGoldFromMinions(cKiller->getChampionGoldFromMinions() + gold);
         CORE_INFO("Adding gold form minions to reduce death spree: %f", cKiller->getChampionGoldFromMinions());
      }

      if (cKiller->getChampionGoldFromMinions() >= 50 && cKiller->killDeathCounter < 0){
         cKiller->setChampionGoldFromMinions(0);
         cKiller->killDeathCounter += 1;
      }
   }
}

void Unit::setTargetUnit(Unit* target) 
{
	if (target == 0) // If we are unsetting the target (moving around)
	{
		if (targetUnit != 0) // and we had a target
			targetUnit->setDistressCall(0);	// Unset the distress call	
		// TODO: Replace this with a delay?
	}
	else target->setDistressCall(this); // Otherwise set the distress call
	targetUnit = target;
   refreshWaypoints();
}

void Unit::setAutoAttackTarget(Unit* target) {
   autoAttackTarget = target;
}

void Unit::refreshWaypoints() {
   if (!targetUnit || (distanceWith(targetUnit) <= stats->getRange() && waypoints.size() == 1)) {
      return;
   }

   if (distanceWith(targetUnit) <= stats->getRange()-2.f) {
      setWaypoints({Vector2(x, y)});
   } else {
      Target* t = new Target(waypoints[waypoints.size()-1]);
      if(t->distanceWith(targetUnit) >= 25.f) {
			setWaypoints({ Vector2(x, y), Vector2(targetUnit->getX(), targetUnit->getY()) });
      }
      delete t;
   }
}

Buff* Unit::getBuff(std::string name){
   for(auto& buff : buffs){
      if(buff->getName() == name){
         return buff;
       }
   }
   return 0;
}

//Prioritize targets
unsigned int Unit::classifyTarget(Unit* target) 
{
	/*
	Under normal circumstances, a minion’s behavior is simple. Minions follow their attack route until they reach an enemy to engage. 
	Every few seconds, they will scan the area around them for the highest priority target. When a minion receives a call for help 
	from an ally, it will evaluate its current target in relation to the target designated by the call. It will switch its attack 
	to the new target if and only if the new target is of a higher priority than their current target. Minions prioritize targets 
	in the following order:
	
		1. An enemy champion designated by a call for help from an allied champion. (Enemy champion attacking an Allied champion)
		2. An enemy minion designated by a call for help from an allied champion. (Enemy minion attacking an Allied champion)
		3. An enemy minion designated by a call for help from an allied minion. (Enemy minion attacking an Allied minion)
		4. An enemy turret designated by a call for help from an allied minion. (Enemy turret attacking an Allied minion)
		5. An enemy champion designated by a call for help from an allied minion. (Enemy champion attacking an Allied minion)
		6. The closest enemy minion.
		7. The closest enemy champion.
	*/

	if (target->targetUnit && target->targetUnit->isInDistress()) // If an ally is in distress, target this unit. (Priority 1~5)
	{
		if (dynamic_cast<Champion*>(target) != 0 && dynamic_cast<Champion*>(target->targetUnit) != 0) // If it's a champion attacking a friendly champion
			return 1;
		else if (dynamic_cast<Minion*>(target) != 0 && dynamic_cast<Champion*>(target->targetUnit) != 0) // If it's a minion attacking a friendly champion.
			return 2;
		else if (dynamic_cast<Minion*>(target) != 0 && dynamic_cast<Minion*>(target->targetUnit) != 0) // Minion attacking minion
			return 3;
		else if (dynamic_cast<Turret*>(target) != 0 && dynamic_cast<Minion*>(target->targetUnit) != 0) // Turret attacking minion
			return 4;
		else if (dynamic_cast<Champion*>(target) != 0 && dynamic_cast<Minion*>(target->targetUnit) != 0) // Champion attacking minion
			return 5;
	}
	
	Minion* m = dynamic_cast<Minion*>(target);
	if (m) 
	{
		switch (m->getType()) 
		{
		case MINION_TYPE_MELEE:
			return 6;
		case MINION_TYPE_CASTER:
			return 7;
		case MINION_TYPE_CANNON:
		case MINION_TYPE_SUPER:
			return 8;
		}
	}

	if (dynamic_cast<Champion*>(target)) 
		return 9;

	return 10;

   /*Turret* t = dynamic_cast<Turret*>(target);
   
   // Turrets before champions
   if (t) {
      return 6;
   }
   
   Minion* m = dynamic_cast<Minion*>(target);

   if (m) {
      switch (m->getType()) {
         case MINION_TYPE_MELEE:
            return 4;
         case MINION_TYPE_CASTER:
            return 5;
         case MINION_TYPE_CANNON:
         case MINION_TYPE_SUPER:
            return 3;
      }
   }

   Champion* c = dynamic_cast<Champion*>(target);
   if (c) {
      return 7;
   }

   //Trap (Shaco box) return 1
   //Pet (Tibbers) return 2

   return 10;*/
}