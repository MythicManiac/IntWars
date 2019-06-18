#include "Stats.h"

using namespace std;

float Stats::getStat(uint8 blockId, uint32 stat) const {
   int block = -1;
   while(blockId) {
      blockId = blockId >> 1;
      ++block;
   }
   auto it = stats[block].find(stat);
   if(it != stats[block].end()) {
      return it->second;
   }
   
   return 0;
}

void Stats::setStat(uint8 blockId, uint32 stat, float value) {
   int block = -1;
   updatedStats[blockId].insert(stat);
   
   while(blockId) {
      blockId = blockId >> 1;
      ++block;
   }
   stats[block][stat] = value;
}

uint8 Stats::getSize(uint8 blockId, uint32 stat) {
   switch(blockId) {
   case MM_One:
      switch(stat) {
      case FM1_Spells_Enabled:
         return 2;
      case FM1_SummonerSpells_Enabled:
         return 2; // not 100% sure
      }
      
   case MM_Four:
      switch(stat) {
      case FM4_Level:
         return 1;
      
      }
   }

   return 4;
}

void Stats::update(int64 diff) {
   if(getHp5() > 0 && getCurrentHealth() < getMaxHealth()) {
      float newHealth = getCurrentHealth()+(getHp5()*diff*0.000001f);
      newHealth = std::min(getMaxHealth(), newHealth);
      setCurrentHealth(newHealth);
   }
   
   if(getMana5() > 0 && getCurrentMana() < getMaxMana()) {
      float newMana = getCurrentMana()+(getMana5()*diff*0.000001f);
      newMana = std::min(getMaxMana(), newMana);
      setCurrentMana(newMana);
   }
   if(generatingGold && getGoldPerSecond() > 0){
      float newGold = getGold()+getGoldPerSecond()*(diff*0.000001f);
      setGold(newGold);
   }
}

void Stats::levelUp() {
   setLevel(getLevel()+1);
   
   setMaxHealth(getMaxHealth()+healthPerLevel);
   setCurrentHealth((getMaxHealth() / (getMaxHealth()-healthPerLevel)) * getCurrentHealth());
   setMaxMana(getMaxMana()+manaPerLevel);
   setCurrentMana((getMaxMana() / (getMaxMana()-manaPerLevel)) * getCurrentMana());
   setBaseAd(getBaseAd()+adPerLevel);
   setArmor(getArmor()+armorPerLevel);
   setMagicArmor(getMagicArmor()+magicArmorPerLevel);
   setHp5(getHp5()+hp5RegenPerLevel);
   setMp5(getMana5()+mp5RegenPerLevel);
}

void Stats::applyStatMods(const std::vector<StatMod>& statMods) {
   for(StatMod stat : statMods) {
      if(stat.value == 0) {
         continue;
      }

      setStat(stat.blockId, stat.mask, getStat(stat.blockId, stat.mask)+stat.value);
   }
}

void Stats::unapplyStatMods(const std::vector<StatMod>& statMods) {
   for(StatMod stat : statMods) {
      if(stat.value == 0) {
         continue;
      }

      setStat(stat.blockId, stat.mask, getStat(stat.blockId, stat.mask)-stat.value);
   }
}