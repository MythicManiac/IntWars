#ifndef BUFF_H
#define BUFF_H

#include "stdafx.h"
#include <string>

class Unit;
class LuaScript;

enum BuffType{
BUFFTYPE_ETERNAL,
BUFFTYPE_TEMPORARY
};



class Buff{
protected:
   float duration;
   float movementSpeedPercentModifier;
   float timeElapsed;
   bool remove;
   Unit* attachedTo;
   Unit* attacker; // who added this buff to the unit it's attached to
   BuffType buffType;
   LuaScript* buffScript;
   
   void init();
   std::string name;

public:

   BuffType const getBuffType(){return buffType;}
   Unit* const getUnit(){
      return attachedTo;
   }
   
   void setName(const std::string& name){
      this->name = name;
   }
   
    
   bool needsToRemove() { return remove; }
   Buff(std::string buffName, float dur, BuffType type, Unit* u, Unit* attacker);
   Buff(std::string buffName, float dur, BuffType type, Unit* u) : Buff(buffName, dur, type, u, u) { } //no attacker specified = selfbuff, attacker aka source is same as attachedto

   float getMovementSpeedPercentModifier() { return movementSpeedPercentModifier;}

   void setMovementSpeedPercentModifier(float modifier){ movementSpeedPercentModifier = modifier; }
   
   std::string getName() const {
      return name;
   }
   
   void setTimeElapsed(float time){
   timeElapsed = time;
   }
   

   void update(int64 diff);
   
};


#endif	/* BUFF_H */

