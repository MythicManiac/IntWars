#ifndef __TURRET_H
#define __TURRET_H

#include "Unit.h"

class Turret : public Unit {
private:
   std::string name;

public:
   Turret(Map* map, uint32 id, const std::string& name, float x = 0, float y = 0, float hp = 0, float ad = 0, int team = 0);

   const std::string& getName() const { return name; }
   virtual void update(int64 diff) override;
   void refreshWaypoints() override {}
   

};

#endif