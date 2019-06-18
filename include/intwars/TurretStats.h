#ifndef __H_TURRET_STATS
#define __H_TURRET_STATS

#include "Stats.h"

class TurretStats : public Stats {
protected:
   float range;
   
public:
   virtual void setAttackSpeedMultiplier(float multiplier) override {
   }

   virtual float getAttackSpeedMultiplier() const override {
      return 1.f;
   }
   
   virtual void setRange(float range) override {
      this->range = range;
   }
   
   virtual float getRange() const override {
      return range;
   }

};

#endif