#ifndef _CHAMPION_H
#define _CHAMPION_H

#include "Inventory.h"
#include "Unit.h"
#include "Spell.h"
#include <vector>

class Champion : public Unit {


protected:
   std::string type;
   std::vector<Spell*> spells;
   uint8 skillPoints;
   uint8 skin;
   int64 respawnTimer;
   Inventory inventory;
   float championGoldFromMinions;
   int64 championHitFlagTimer;
   
public:
   Spell* getSpell(int index){ return spells[index]; }
   Champion(const std::string& type, Map* map, uint32 id, uint32 playerId);
   const std::string& getType() { return type; }
   uint32 playerId;
   uint32 playerHitId;
   int getTeamSize();
   std::pair<float, float> getRespawnPosition();
   Spell* castSpell(uint8 slot, float x, float y, Unit* target, uint32 futureProjNetId, uint32 spellNetId);
   Spell* levelUpSpell(uint8 slot);
   
   virtual void update(int64 diff);

   void setSkillPoints(int _skillPoints){
       skillPoints = (uint8)_skillPoints;
   }
   
   void setSkin(uint8 skin) { this->skin = skin; }
   uint32 getChampionHash();

	virtual bool isInDistress() const override { return distressCause!=0; }

   uint8 getSkillPoints() const { return skillPoints; }
   void levelUp();
   
   Inventory& getInventory() { return inventory; }
   
   virtual void die(Unit* killer) override;
   int64 getRespawnTimer() const { return respawnTimer; }

   void onCollision(Object *collider);
   
   float getChampionGoldFromMinions() { return championGoldFromMinions; }
   void setChampionGoldFromMinions(float gold) { this->championGoldFromMinions = gold; }
   
   void setChampionHitFlagTimer(int64 time) { this->championHitFlagTimer = time; }

   virtual void dealDamageTo(Unit* target, float damage, DamageType type, DamageSource source) override;
};

#endif