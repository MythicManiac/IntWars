#ifndef _SPELL_H
#define _SPELL_H

#include "stdafx.h"
#include "Projectile.h"
#include "RAFFile.h"
#include <vector>
#include "LuaScript.h"

#include <array>


class Unit;
class Champion;

enum SpellFlag : uint32 {
   SPELL_FLAG_AutoCast                   = 0x00000002,
   SPELL_FLAG_InstantCast                = 0x00000004,
   SPELL_FLAG_PersistThroughDeath        = 0x00000008,
   SPELL_FLAG_NonDispellable             = 0x00000010,
   SPELL_FLAG_NoClick                    = 0x00000020,
   SPELL_FLAG_AffectImportantBotTargets  = 0x00000040,
   SPELL_FLAG_AllowWhileTaunted          = 0x00000080,
   SPELL_FLAG_NotAffectZombie            = 0x00000100,
   SPELL_FLAG_AffectUntargetable         = 0x00000200,
   SPELL_FLAG_AffectEnemies              = 0x00000400,
   SPELL_FLAG_AffectFriends              = 0x00000800,
   SPELL_FLAG_AffectBuildings            = 0x00001000,
   SPELL_FLAG_NotAffectSelf              = 0x00002000,
   SPELL_FLAG_AffectNeutral              = 0x00004000,
   SPELL_FLAG_AffectAllSides             = 0x00004C00,
   SPELL_FLAG_AffectMinions              = 0x00008000,
   SPELL_FLAG_AffectHeroes               = 0x00010000,
   SPELL_FLAG_AffectTurrets              = 0x00020000,
   SPELL_FLAG_AffectAllUnitTypes         = 0x00038000,
   SPELL_FLAG_AlwaysSelf                 = 0x00040000,
   SPELL_FLAG_AffectDead                 = 0x00080000,
   SPELL_FLAG_AffectNotPet               = 0x00100000,
   SPELL_FLAG_AffectBarracksOnly         = 0x00200000,
   SPELL_FLAG_IgnoreVisibilityCheck      = 0x00400000,
   SPELL_FLAG_NonTargetableAlly          = 0x00800000,
   SPELL_FLAG_NonTargetableEnemy         = 0x01000000,
   SPELL_FLAG_NonTargetableAll           = 0x01800000,
   SPELL_FLAG_TargetableToAll            = 0x02000000,
   SPELL_FLAG_AffectWards                = 0x04000000,
   SPELL_FLAG_AffectUseable              = 0x08000000,
   SPELL_FLAG_IgnoreAllyMinion           = 0x10000000,
   SPELL_FLAG_IgnoreEnemyMinion          = 0x20000000,
   SPELL_FLAG_IgnoreLaneMinion           = 0x40000000,
   SPELL_FLAG_IgnoreClones               = 0x80000000,
};

enum SpellState {
   STATE_READY,
   STATE_CASTING,
   STATE_COOLDOWN
};

enum SpellTargetType : uint8 {
   TARGET_SELF     = 0, // teemo W ; xin Q
   TARGET_UNIT     = 1, // Taric E ; Annie Q ; teemo Q ; xin E
   TARGET_LOC_AOE  = 2, // Lux E, Ziggs R
   TARGET_CONE     = 3, // Annie W, Kass E
   TARGET_SELF_AOE = 4, // sivir R, Gangplanck E
   TARGET_LOC      = 6, // Ez Q, W, E, R ; Mundo Q
   TARGET_LOC2     = 7  // Morg Q, Cait's Q -- These don't seem to have Missile inibins, and SpawnProjectile doesn't seem necessary to show the projectiles
};

class Spell {
protected:
   Champion* owner;
   uint8 level;
   uint8 slot;
   std::string spellName;
   uint8 targetType;
   uint32 flags, projectileFlags;
   
   float castTime;
   float castRange;
   float projectileSpeed;
   float lineWidth;
   std::array<float, 5> cooldown;
   std::array<float, 5> cost;
   
   // Warning : this value usually contains one of the "ad/ap" bonus coefficient, as seen in "deals 50 (+{coefficient}%) damage"
   // However, it may not be accurate and there's no way to tell whether it's the ad or ap bonus for hybrid spells
   // Sometimes, it is also stored as an effect value instead of the coefficient
   float coefficient;
   std::vector< std::vector<float> > effects;
   
   float range;
   
   SpellState state;
   float currentCooldown;
   float currentCastTime;
   uint32 futureProjNetId;
   uint32 spellNetId;
   
   Unit* target;
   float x, y;
   
public:
   Spell(Champion* owner, const std::string& spellName, uint8 slot);
   
   /**
    * Called when the character casts the spell
    */
   virtual bool cast(float x, float y, Unit* u = 0, uint32 futureProjNetId = 0, uint32 spellNetId = 0);
   
   /**
    * Called when the spell is finished casting and we're supposed to do things
    * such as projectile spawning, etc.
    */
   virtual void finishCasting();
   
   /**
    * Called every diff milliseconds to update the spell
    */
   virtual void update(int64 diff);
   
   /**
    * Called by projectiles when they land / hit
    * In here we apply the effects : damage, buffs, debuffs...
    */
   virtual void applyEffects(Unit* t, Projectile* p = 0);
   
   Champion* getOwner() const { return owner; }
   
   /**
    * @return Spell's unique ID
    */
   uint32 getId() const;
   float getCastTime() const { return castTime; }
   
   std::string getStringForSlot();
   
   /*
    * does spell effects in lua if defined.
    */
   void doLua();
   void loadLua(LuaScript& script);
   void reloadLua();
   
   void setSlot(int _slot){
       slot=_slot;
   }
   
   /**
    * TODO : Add in CDR % from champion's stat
    */
   float getCooldown() const { 
      return 0; // TODO: Remove this
      if(!level) {
         return 0;
      }
      return cooldown[level-1];
   }
   
   /**
    * @return the mana/energy/health cost
    */
   float getCost() const {
      return 0; // TODO: Remove this
      if(!level) {
         return 0;
      }
      return cost[level-1];
   }
   
   uint32 getFlags() const { return flags; }
   
   uint8 getLevel() const {
      return level;
   }
   
   virtual void levelUp() {
      ++level;
   }
   
   SpellState getState() const {
      return state;
   }
   
   uint8 getSlot() const {
      return slot;
   }

};

#endif