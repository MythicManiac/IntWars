#ifndef _LEVELPROP_H
#define _LEVELPROP_H

#include "Object.h"

class LevelProp : public Object {

private:
   std::string name;
   std::string type;
   float z, dirX, dirY, dirZ, unk1, unk2;

public:
   LevelProp(Map* map, uint32 id, float x, float y, float z, float dirX, float dirY, float dirZ, float unk1, float unk2, const std::string& name, const std::string& type) : Object(map, id, x, y, 0), z(z), dirX(dirX), dirY(dirY), dirZ(dirZ), unk1(unk1), unk2(unk2), name(name), type(type) { }
   float getZ() const { return z; }
   float getDirectionX() const { return dirX; }
   float getDirectionY() const { return dirY; }
   float getDirectionZ() const { return dirZ; }
   float getUnk1() const { return unk1; }
   float getUnk2() const { return unk2; }

   float getMoveSpeed() const override { return 0.f; }
   
   const std::string& getName() const { return name; }
   const std::string& getType() const { return type; }

};


#endif