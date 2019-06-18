#ifndef _CHAMPION_FACTORY_H
#define _CHAMPION_FACTORY_H

#include "Champion.h"

class ChampionFactory {

public:

   /**
    * Todo : Code every champion and add em here..
    */
   static Champion* getChampionFromType(const std::string& type, Map* map, uint32 id, uint32 playerId) {
      return new Champion(type, map, id, playerId);
   }


};

#endif