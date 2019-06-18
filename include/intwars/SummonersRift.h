#ifndef __SUMMONERSRIFT_H
#define __SUMMONERSRIFT_H

#include "Map.h"

class SummonersRift : public Map 
{
public:
   SummonersRift(Game* game);
   
   virtual ~SummonersRift() { }
   virtual void update(long long diff) override;
   float getGoldPerSecond() override { return 1.9f; }
   
   const Target getRespawnLocation(int team) const override;
   float getGoldFor(Unit* u) const override;
   float getExperienceFor(Unit* u) const override;

	virtual std::pair<int, Vector2> getMinionSpawnPosition(uint32 spawnPosition) const override;
	virtual void setMinionStats(Minion* minion) const override;

   bool spawn() override;

   const int getMapId() const override { return 1; }
};

#endif