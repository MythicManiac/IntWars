#include <map>

#include "Map.h"
#include "Projectile.h"
#include "Spell.h"
#include "Unit.h"
#include "Minion.h"
#include "Turret.h"
#include "Champion.h"

using namespace std;

void Projectile::update(int64 diff) {

   if(!target) {
      setToRemove();
      return;
   }
   
   if(target->isSimpleTarget()) { // Skillshot
      const std::map<uint32, Object*>& objects = map->getObjects();
      for(auto& it : objects) {
         if(isToRemove()) {
            return;
         }
         
         if(collide(it.second)) {
         
            if(find(objectsHit.begin(), objectsHit.end(), it.second) != objectsHit.end()) {
               continue;
            }
         
            Unit* u = dynamic_cast<Unit*>(it.second);
            if(!u) {
               continue;
            }
            
            if(u->getTeam() == owner->getTeam() && !(flags & SPELL_FLAG_AffectFriends)) {
               continue;
            }
            
            if(u->getTeam() != owner->getTeam() && !(flags & SPELL_FLAG_AffectEnemies)) {
               continue;
            }
            
            if(u->isDead() && !(flags & SPELL_FLAG_AffectDead)) {
               continue;
            }
            
            Minion* m = dynamic_cast<Minion*>(u);
            if(m && !(flags & SPELL_FLAG_AffectMinions)) {
               continue;
            }
            
            Turret* t = dynamic_cast<Turret*>(u);
            if(t && !(flags & SPELL_FLAG_AffectTurrets)) {
               continue;
            }
            
            Champion* c = dynamic_cast<Champion*>(u);
            if(c && !(flags & SPELL_FLAG_AffectHeroes)) {
               continue;
            }
            
            objectsHit.push_back(u);
            originSpell->applyEffects(u, this);
         }
      }
   } else {
      Unit* u = static_cast<Unit*>(target);
      if(collide(u)) { // Autoguided spell
         if(originSpell) {
            originSpell->applyEffects(u, this);
         } else { // auto attack
            owner->autoAttackHit(u);
            setToRemove();
         }
      }
   }
   
   Object::update(diff);
   
}