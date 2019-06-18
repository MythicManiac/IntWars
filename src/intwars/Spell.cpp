#include "Spell.h"
#include "Map.h"
#include "Game.h"
#include "RAFManager.h"
#include "Champion.h"
#include "Inibin.h"
#include "LuaScript.h"
#include "Logger.h"
#include <intwars\Config.h>

using namespace std;

Spell::Spell(Champion* owner, const std::string& spellName, uint8 slot) : owner(owner), spellName(spellName), level(0), slot(slot), range(0), state(STATE_READY), currentCooldown(0), currentCastTime(0), castTime(0.f), castRange(1000.f), projectileSpeed(2000.f), flags(0), projectileFlags(0) {
	//If summoner
	
	std::vector<unsigned char> iniFile;
	if(slot > 3) {
		if(!RAFManager::getInstance()->readFile("DATA/Spells/"+spellName+".inibin", iniFile)) {
			return;
		}
		Inibin inibin(iniFile);
		iniFile.clear();
		cooldown.fill(1.f);
		
		return;
	}
   
   cooldown.fill(1.f);
   cost.fill(1.f);

   if(!RAFManager::getInstance()->readFile("DATA/Spells/"+spellName+".inibin", iniFile)) {
      if(!RAFManager::getInstance()->readFile("DATA/Characters/"+owner->getType()+"/Spells/"+spellName+".inibin", iniFile)) {
         if(!RAFManager::getInstance()->readFile("DATA/Characters/"+owner->getType()+"/"+spellName+".inibin", iniFile)) {
            CORE_ERROR("Couldn't find spell stats for %s", spellName.c_str());
            return;
         }
      }
   }
   
   Inibin inibin(iniFile);
   iniFile.clear();
      
   // Generate cooldown values for each level of the spell
   for(int i = 0; i < cooldown.size(); ++i) {
      char c = '0'+i+1;
      cooldown[i] = inibin.getFloatValue("SpellData", string("Cooldown")+c);
   }
   
   for(int i = 0; i < cost.size(); ++i) {
      char c = '0'+i+1;
      cost[i] = inibin.getFloatValue("SpellData", string("ManaCost")+c);
   }

   castTime = ((1.f+inibin.getFloatValue("SpellData", "DelayCastOffsetPercent")))/2.f;
   
   flags = inibin.getIntValue("SpellData", "Flags");
   castRange = inibin.getFloatValue("SpellData", "CastRange");
   projectileSpeed = inibin.getFloatValue("SpellData", "MissileSpeed");
   coefficient = inibin.getFloatValue("SpellData", "Coefficient");
   lineWidth = inibin.getFloatValue("SpellData", "LineWidth");
   
   char i = 1;
   while(true) {
      char effectNumber = '0'+i;
      string key = string("Effect")+effectNumber+"Level0Amount";
      if(!inibin.keyExists("SpellData", key)) {
         break;
      }
      
      vector<float> effectValues;
      for(char j = 0; j < 6; ++j) {
         char level = '0'+j; 
         key = string("Effect")+effectNumber+"Level"+level+"Amount";
         effectValues.push_back(inibin.getFloatValue("SpellData", key));
      }
      
      effects.push_back(effectValues);
      
      ++i;
   }
   
   targetType = floor(inibin.getFloatValue("SpellData", "TargettingType")+0.5f);
   
   // This is starting to get ugly. How many more names / paths to go ?
   if(!RAFManager::getInstance()->readFile("DATA/Spells/"+spellName+"Missile.inibin", iniFile)) {
      if(!RAFManager::getInstance()->readFile("DATA/Spells/"+spellName+"Mis.inibin", iniFile)) {
         if(!RAFManager::getInstance()->readFile("DATA/Characters/"+owner->getType()+"/Spells/"+spellName+"Missile.inibin", iniFile)) {
            if(!RAFManager::getInstance()->readFile("DATA/Characters/"+owner->getType()+"/"+spellName+"Missile.inibin", iniFile)) {
               if(!RAFManager::getInstance()->readFile("DATA/Characters/"+owner->getType()+"/Spells/"+spellName+"Mis.inibin", iniFile)) {
                  if(!RAFManager::getInstance()->readFile("DATA/Characters/"+owner->getType()+"/"+spellName+"Mis.inibin", iniFile)) {
                     return;
                  }
               }
            }
         }
      }
   }
   
   Inibin projectile(iniFile);
   iniFile.clear();
   
   castRange = projectile.getFloatValue("SpellData", "CastRange");
   projectileSpeed = projectile.getFloatValue("SpellData", "MissileSpeed");
   projectileFlags = projectile.getIntValue("SpellData", "Flags");
}

/**
 * Called when the character casts the spell
 */
bool Spell::cast(float x, float y, Unit* u, uint32 futureProjNetId, uint32 spellNetId) {

   this->x = x;
   this->y = y;
   this->target = u;
   this->futureProjNetId = futureProjNetId;
   this->spellNetId = spellNetId;

   if(castTime > 0 && !(flags & SPELL_FLAG_InstantCast)) {
      owner->setPosition(owner->getX(), owner->getY());//stop moving serverside too. TODO: check for each spell if they stop movement or not
      state = STATE_CASTING;
      currentCastTime = castTime;
   } else {
      finishCasting();
   }
   
   return true;
}

/**
 * Called when the spell is finished casting and we're supposed to do things
 * such as projectile spawning, etc.
 */
void Spell::finishCasting() {
	doLua();
	   
	state = STATE_COOLDOWN;
	currentCooldown = getCooldown();
	
	if(getSlot() < 4)
	{
		owner->getStats().setCurrentMana(owner->getStats().getCurrentMana()-cost.at(getLevel()-1));
		owner->getMap()->getGame()->notifySetCooldown(owner, getSlot(), cooldown.at(getLevel()-1), cooldown.at(getLevel()-1));
	}
}

std::string Spell::getStringForSlot(){
    
    switch(getSlot()){
        case 0:
            return "Q";
        case 1:
            return "W";
        case 2:
            return "E";
        case 3:
            return "R";
		case 4:
			return "D";
		case 5:
			return "F";
    }
    
    return "undefined";
}



void Spell::loadLua(LuaScript& script){
   std::string scriptloc;
   
   if (getSlot() > 3) {
      scriptloc = Config::instance().luadir + "summonerSpells/" + spellName + ".lua";
   } else {
      scriptloc = Config::instance().luadir + "champions/" + owner->getType() + "/" + getStringForSlot() + ".lua"; //lua/championname/(q/w/e/r), example: /lua/Ezreal/q, also for stuff like nidalee cougar they will have diff folders!
   }

   CORE_INFO("Spell script loc is: %s" , scriptloc.c_str());
   
   script.lua.script("package.path = '" + Config::instance().luadir + "lib/?.lua;' .. package.path"); //automatically load vector lib so scripters dont have to worry about path
   script.lua.set_function("getOwnerX", [this]() { return owner->getX(); });
   script.lua.set_function("getOwnerY", [this]() { return owner->getY(); });
   script.lua.set_function("getSpellLevel", [this]() { return getLevel(); });
   script.lua.set_function("getOwnerLevel", [this]() { return owner->getStats().getLevel(); });
   script.lua.set_function("getChampionModel", [this]() { return owner->getModel(); });
   script.lua.set_function("getCastTarget", [this]() { return this->target; });
   
   script.lua.set_function("setChampionModel", [this](const std::string& newModel) {
      owner->setModel(newModel); 
      return;
   });
   script.lua.set_function("getSpellToX", [this]() { return x; });
   script.lua.set_function("getSpellToY", [this]() { return y; });
   script.lua.set_function("getRange", [this]() { return castRange; });
   script.lua.set_function("teleportTo", [this](float _x, float _y) { // expose teleport to lua
      owner->getMap()->getGame()->notifyTeleport(owner, _x, _y);
      return;
   });
   script.lua.set_function("isWalkable", [this](float _x, float _y) {
      return owner->getMap()->isWalkable(_x, _y);
   });
   script.lua.set_function("getClosestTerrainExit", [this](Unit* u, float x, float y, bool noForward) {
	  Vector2 location = Vector2(x, y);
      return std::make_tuple(owner->getMap()->getAIMesh()->getClosestTerrainExit(u, location, noForward).X, owner->getMap()->getAIMesh()->getClosestTerrainExit(u, location, noForward).Y);
   });
   script.lua.set_function("addMovementSpeedBuff", [this](Unit* u, float amount, float duration) { // expose teleport to lua
       Buff* b = new Buff("", duration, BUFFTYPE_TEMPORARY, u);
       b->setMovementSpeedPercentModifier(amount);
       u->addBuff(b);
       u->getStats().addMovementSpeedPercentageModifier(b->getMovementSpeedPercentModifier());
      return;
   });
   
	script.lua.set_function("addBuff", [this](Buff b, Unit* u){
		u->addBuff(new Buff(b));
		return;
	});
   
	script.lua.set_function("getEffectValue", [this](uint32 effectNo) {
		if(effectNo >= effects.size() || level >= effects[effectNo].size()) {
			return 0.f;
		}
		return effects[effectNo][level];
	});
   
	script.lua.set_function("getOwner", [this]() { return owner; });
   
	script.lua.set_function("getTeam", [this](Object* o) { return o->getTeam(); });
	script.lua.set_function("isDead", [this](Unit* u) { return u->isDead(); });
   
	script.lua.set_function("getProjectileSpeed", [this]() { return projectileSpeed; });
	script.lua.set_function("getCoefficient", [this]() { return coefficient; });
   
	script.lua.set_function("addProjectile", [this](float toX, float toY) { 
		Projectile* p = new Projectile(owner->getMap(), GetNewNetID(), owner->getX(), owner->getY(), lineWidth, owner, new Target(toX, toY), this, projectileSpeed, RAFFile::getHash(spellName +"Missile"), projectileFlags ? projectileFlags : flags);
		owner->getMap()->addObject(p);
		owner->getMap()->getGame()->notifyProjectileSpawn(p);

		return;
	});
   
	script.lua.set_function("addProjectileTarget", [this](Target *t) { 
		Projectile* p = new Projectile(owner->getMap(), GetNewNetID(), owner->getX(), owner->getY(), lineWidth, owner, t, this, projectileSpeed, RAFFile::getHash(spellName +"Missile"), projectileFlags ? projectileFlags : flags);
		owner->getMap()->addObject(p);
		owner->getMap()->getGame()->notifyProjectileSpawn(p);

		return;
	});
   
	script.lua.set_function("addProjectileCustom", [this](const std::string& name, float projSpeed, float toX, float toY) { 
		Projectile* p = new Projectile(owner->getMap(), GetNewNetID(), owner->getX(), owner->getY(), lineWidth, owner, new Target(toX, toY), this, projectileSpeed, RAFFile::getHash(name), projectileFlags ? projectileFlags : flags);
		owner->getMap()->addObject(p);
		owner->getMap()->getGame()->notifyProjectileSpawn(p);

		return;
	});
   
	script.lua.set_function("addProjectileTargetCustom", [this](const std::string& name, float projSpeed, Target *t) { 
		Projectile* p = new Projectile(owner->getMap(), GetNewNetID(), owner->getX(), owner->getY(), lineWidth, owner, t, this, projectileSpeed, RAFFile::getHash(name), projectileFlags ? projectileFlags : flags);
		owner->getMap()->addObject(p);
		owner->getMap()->getGame()->notifyProjectileSpawn(p);

		return;
	});
   
   /**
    * For spells that don't require SpawnProjectile, but for which we still need to track the projectile server-side
    */
	script.lua.set_function("addServerProjectile", [this](float toX, float toY) { 
		Projectile* p = new Projectile(owner->getMap(), futureProjNetId, owner->getX(), owner->getY(), lineWidth, owner, new Target(toX, toY), this, projectileSpeed, 0, projectileFlags ? projectileFlags : flags);
		owner->getMap()->addObject(p);

		return;
	});
   
	script.lua.set_function("addParticle", [this](const std::string& particle, float toX, float toY) { 
		Target* t = new Target(toX, toY);
		owner->getMap()->getGame()->notifyParticleSpawn(owner, t, particle);
		delete t;
		return;
	});
   
   script.lua.set_function("addParticleTarget", [this](const std::string& particle, Target* u) { 
      owner->getMap()->getGame()->notifyParticleSpawn(owner, u, particle);
      return;
   });
   
	script.lua.set_function("spellAnimation", [this](const std::string& animation, Unit* u) { 
		owner->getMap()->getGame()->notifySpellAnimation(u, animation);
		return;
	});

	// TODO: Set multiple animations
	script.lua.set_function("setAnimation", [this](const std::string& animation1, const std::string& animation2, Unit* u) {
		std::vector<std::pair<std::string, std::string>> animationPairs;
		animationPairs.push_back(std::make_pair(animation1, animation2));

		owner->getMap()->getGame()->notifySetAnimation(u, animationPairs);
		return;
	});

	script.lua.set_function("resetAnimations", [this](Unit* u) {
		std::vector<std::pair<std::string, std::string>> animationPairs;
		owner->getMap()->getGame()->notifySetAnimation(u, animationPairs);
		return;
	});

	script.lua.set_function("dashTo", [this](Unit* u, float x, float y, float dashSpeed) {
		u->dashTo(x, y, dashSpeed);
		u->setTargetUnit(0);
		owner->getMap()->getGame()->notifyDash(u, x, y, dashSpeed);
		return;
	});

	script.lua.set_function("getUnitsInRange", [this](Target* t, float range, bool isAlive) {
		return owner->getMap()->getUnitsInRange(t, range, isAlive);
	});

	script.lua.set_function("getChampionsInRange", [this](Target* t, float range, bool isAlive) {
		return owner->getMap()->getChampionsInRange(t, range, isAlive);
	});

	try{
		script.loadScript(scriptloc); //todo: abstract class that loads a lua file for any lua
		}catch(sol::error e){//lua error? don't crash the whole server
		CORE_ERROR("Error in spell script: %s", e.what());
	}
}

void Spell::doLua(){
   
   LuaScript script(true);
    
   loadLua(script); //comment this line for no reload on the fly, better performance

   CORE_INFO("Spell from slot %i", getSlot());

   try{
      script.lua.script("finishCasting()");
   }catch(sol::error e){//lua error? don't crash the whole server
      CORE_ERROR("%s", e.what());
   }
}

/**
 * Called every diff milliseconds to update the spell
 */
void Spell::update(int64 diff) {
   switch(state) {
   case STATE_READY:
      return;
   case STATE_CASTING:
      currentCastTime -= diff/1000000.f;
      if(currentCastTime <= 0) {
         finishCasting();
      }
      break;
   case STATE_COOLDOWN:
      currentCooldown -= diff/1000000.f;
      if(currentCooldown < 0) {
         state = STATE_READY;
      }
      break;
   }
}

uint32 Spell::getId() const {
   return RAFFile::getHash(spellName);
}

void Spell::applyEffects(Unit* u, Projectile* p) {

   LuaScript script(true);
   
   script.lua.set_function("getTarget", [&u]() { return u; });
   
   script.lua.set_function("dealPhysicalDamage", [this, &u](float amount) {
      owner->dealDamageTo(u, amount, DAMAGE_TYPE_PHYSICAL, DAMAGE_SOURCE_SPELL);
      return;
   });
   
   script.lua.set_function("dealMagicalDamage", [this, &u](float amount) {
      owner->dealDamageTo(u, amount, DAMAGE_TYPE_MAGICAL, DAMAGE_SOURCE_SPELL);
      return;
   });
   
   script.lua.set_function("destroyProjectile", [this, &p]() { 
      p->setToRemove();
      p->getMap()->getGame()->notifyProjectileDestroy(p);
      return;
   });
   
   script.lua.set_function("getNumberObjectsHit", [this, &p]() { 
      return p->getObjectsHit().size();
   });

   script.lua.set_function("addBuff", [this](Buff b, Unit* u){
      u->addBuff(new Buff(b));
      return;
   });
   
   loadLua(script); //comment this line for no reload on the fly, better performance
   
   try{
      script.lua.script("applyEffects()");
   }catch(sol::error e){//lua error? don't crash the whole server
      CORE_ERROR("%s", e.what());
   }
}