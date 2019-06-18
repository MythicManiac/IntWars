#ifndef _MINION_STATS_H
#define _MINION_STATS_H

#include "Stats.h"


enum MinionFieldMaskOne : uint32
{
};

enum MinionFieldMaskTwo : uint32
{
   Minion_FM2_CurrentHp =  0x00000001,
   Minion_FM2_MaxHp   =    0x00000002,
   Minion_FM2_Ad         = 0x00001000,
   Minion_FM2_Atks       = 0x00004000
};

enum MinionFieldMaskThree : uint32
{
};

enum MinionFieldMaskFour : uint32
{
   Minion_FM4_MoveSpeed = 0x0000004
};

enum MinionFieldMaskFive : uint32
{
};

class MinionStats : public Stats {
protected:
   float range;
public:

   MinionStats() : range(0) {}

   float getMaxHealth() const override {
      return getStat(MM_Two, Minion_FM2_MaxHp);
   }
   
   float getCurrentHealth() const override {
      return getStat(MM_Two, Minion_FM2_CurrentHp);
   }
   
   float getBaseAd() const override {
      return getStat(MM_Two, Minion_FM2_Ad);
   }
   
   float getRange() const override {
      return range;
   }
   
   float getBaseAttackSpeed() const override {
      return getStat(MM_Two, Minion_FM2_Atks);
   }
   
   float getMovementSpeed() const override {
      return getStat(MM_Four, Minion_FM4_MoveSpeed);
   }

   float getAttackSpeedMultiplier() const override {
      return 1.f;
   }
   

   void setMaxHealth(float health) override {
      setStat(MM_Two, Minion_FM2_MaxHp, health);
      updatedHealth = true;
   }
   
   void setCurrentHealth(float health) override {
      setStat(MM_Two, Minion_FM2_CurrentHp, health);
      updatedHealth = true;
   }
   
   void setBaseAd(float ad) override {
      setStat(MM_Two, Minion_FM2_Ad, ad);
   }
   
   void setRange(float range) override {
      this->range = range;
   }
   
   void setBaseAttackSpeed(float speed) override {
      setStat(MM_Two, Minion_FM2_Atks, speed);
   }
   
   void setMovementSpeed(float speed) override {
      setStat(MM_Four, Minion_FM4_MoveSpeed, speed);
   }
   
   void setAttackSpeedMultiplier(float multiplier) override {
   }
   
   uint8 getSize(uint8 blockId, uint32 stat) override {
      return 4;
   }

};

#endif