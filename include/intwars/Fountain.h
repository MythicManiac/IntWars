#ifndef _FOUNTAIN_H
#define _FOUNTAIN_H

#define NUM_SIDES 2 // Change implementation when more game-modes with more teams are available
#define PERCENT_MAX_HEALTH_HEAL .15
#define PERCENT_MAX_MANA_HEAL .15

#include <vector>

#include "Map.h"
#include "Target.h"

using std::vector;

class Fountain {
public:
   Fountain() : fountainSize(1000.0f), healTickTimer(0) {}
   Fountain(float fountainSize) : fountainSize(fountainSize), healTickTimer(0) {}

   void setHealLocations(Map* map);
   void healChampions(Map* map, long long diff);

protected:
   float fountainSize;
   uint64 healTickTimer;
   vector<Target*> healLocations;
};

#endif // _FOUNTAIN_H
