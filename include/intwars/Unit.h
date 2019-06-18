#ifndef _UNIT_H
#define _UNIT_H

#include "Object.h"
#include "Stats.h"
#include <string>
#include "Buff.h"
#include "LuaScript.h"

#define DETECT_RANGE 475.f

enum DamageType : uint8 {
   DAMAGE_TYPE_PHYSICAL = 0,
   DAMAGE_TYPE_MAGICAL = 1,
   DAMAGE_TYPE_TRUE = 2
};

enum DamageSource {
   DAMAGE_SOURCE_ATTACK,
   DAMAGE_SOURCE_SPELL,
   DAMAGE_SOURCE_SUMMONER_SPELL, //Ignite shouldn't destroy Banshee's
   DAMAGE_SOURCE_PASSIVE //Red/Thornmail shouldn't as well
};

enum AttackType : uint8 {
   ATTACK_TYPE_RADIAL,
   ATTACK_TYPE_MELEE,
   ATTACK_TYPE_TARGETED
};

enum MoveOrder {
   MOVE_ORDER_MOVE,
   MOVE_ORDER_ATTACKMOVE
};

class Unit : public Object 
{
public:
	Unit(Map* map, uint32 id, std::string model, Stats* stats, uint32 collisionRadius = 40, float x = 0, float y = 0, uint32 visionRadius = 0) : Object(map, id, x, y, collisionRadius, visionRadius), stats(stats),
		statUpdateTimer(0), model(model), autoAttackDelay(0), autoAttackProjectileSpeed(0), isAttacking(false),
		autoAttackCurrentCooldown(0), autoAttackCurrentDelay(0), modelUpdated(false), moveOrder(MOVE_ORDER_MOVE), deathFlag(false),
		targetUnit(0), autoAttackTarget(0), distressCause(0), melee(false), nextAutoIsCrit(false), initialAttackDone(false), nextAttackFlag(false), killDeathCounter(0)

	{ }
	virtual ~Unit();
	Stats& getStats() { return *stats; }
	virtual void update(int64 diff) override;
	virtual float getMoveSpeed() const { return stats->getMovementSpeed(); }
	int getKillDeathCounter() { return killDeathCounter; }

	std::vector<Buff*> buffs;

	std::vector<Buff*>& getBuffs() { return buffs; }

	/**
	* This is called by the AA projectile when it hits its target
	*/
	virtual void autoAttackHit(Unit* target);

	virtual void dealDamageTo(Unit* target, float damage, DamageType type, DamageSource source);

	bool isDead() const;
	virtual void die(Unit* killer);

	void setAutoAttackDelay(float newDelay) { autoAttackDelay = newDelay; }
	void setAutoAttackProjectileSpeed(float newSpeed) { autoAttackProjectileSpeed = newSpeed; }
	void setModel(const std::string& newModel);
	const std::string& getModel();
	bool isModelUpdated() { return modelUpdated; }
	void clearModelUpdated() { modelUpdated = false; }
	void addBuff(Buff* b){
		if (getBuff(b->getName()) == 0) {
			buffs.push_back(b);
			getStats().addMovementSpeedPercentageModifier(b->getMovementSpeedPercentModifier());
		}
		else {
			getBuff(b->getName())->setTimeElapsed(0); // if buff already exists, just restart its timer
		}
	}

	void setDistressCall(Unit* distress) { distressCause = distress; }
	Unit* getDistressCall() { return distressCause; }
	virtual bool isInDistress() const { return false; /*return distressCause;*/ }

	//todo: use statmods
	Buff* getBuff(std::string name);
	void setMoveOrder(MoveOrder moveOrder) { this->moveOrder = moveOrder; }
	void setTargetUnit(Unit* target);
	void setAutoAttackTarget(Unit* target);
	Unit* getTargetUnit() const { return targetUnit; }
	virtual void refreshWaypoints();
	bool isMelee() const { return melee; }
	void setMelee(bool melee) { this->melee = melee; }
	unsigned int classifyTarget(Unit* target);
protected:
   Stats* stats;

   float autoAttackDelay, autoAttackProjectileSpeed;
   float autoAttackCurrentCooldown, autoAttackCurrentDelay;
	bool isAttacking, modelUpdated, melee, initialAttackDone, nextAttackFlag;
	Unit *distressCause;
   uint64 statUpdateTimer;
   uint32 autoAttackProjId;
   MoveOrder moveOrder;
   
   /**
    * Unit we want to attack as soon as in range
    */
   Unit* targetUnit;
   Unit* autoAttackTarget;
   
   bool deathFlag;
   
   std::string model;
   
   bool targetable;
   bool nextAutoIsCrit;
   LuaScript unitScript = LuaScript(true);
   
   int killDeathCounter;
};

#endif