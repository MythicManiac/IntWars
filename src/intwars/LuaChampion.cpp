#include "LuaScript.h"
#include <sol.hpp>
#include <sol/state.hpp>
#include <stdafx.h>

#include "Stats.h"
#include "Champion.h"
#include "Spell.h"

void LuaScript::addChampion() {
    sol::constructors <sol::types < std::string, Map*, uint32, uint32>> championCtr;
    sol::userdata <Champion> championUserData(
            "Champion", championCtr,
            "getSpell", &Champion::getSpell,
            "getStats", &Champion::getStats,
            "getMoveSpeed", &Champion::getMoveSpeed,
            "getModel", &Champion::getModel,
            "setModel", &Champion::setModel,
            "addBuff", &Champion::addBuff,
            "getBuffs", &Champion::getBuffs,
            "dealDamageTo", &Champion::dealDamageTo,
            "getType", &Champion::getType, // not inherited from Unit
            //"getLevel", &Champion::getLevel,
            "getTarget", &Champion::getTarget,
            "getNetId", &Champion::getNetId,
            "getMap", &Champion::getMap,
            "getX", &Champion::getX,
            "getY", &Champion::getY,
            "setTarget", &Champion::setTarget,
            "setTargetUnit", &Champion::setTargetUnit,
            "getTeam", &Champion::getTeam,
            "setTeam", &Champion::setTeam,
            "getBuff", &Champion::getBuff); //Also inherited from unit
            //"distanceWith", &Champion::distanceWith); //Will simply not work
    lua.set_userdata(championUserData);

    sol::constructors <sol::types<>> statsCtr;
    sol::userdata <Stats> statsUserData(//and now the part where I want to kill myself
            "Stats", statsCtr,
            "getTotalAp", &Stats::getTotalAp,
            "getTotalAd", &Stats::getTotalAd,
            "getBonusAdFlat", &Stats::getBonusAdFlat,
            "getArmot", &Stats::getArmor,
            "getMagicResist", &Stats::getMagicArmor,
            "getRange", &Stats::getRange,
            "getCurrentHealth", &Stats::getCurrentHealth,
            "getCurrentMana", &Stats::getCurrentMana,
            "getMaxHealth", &Stats::getMaxHealth,
            "getMaxMana", &Stats::getMaxMana,
            "getBaseAttackSpeed", &Stats::getBaseAttackSpeed,
            "getAttackSpeedMultiplier", &Stats::getAttackSpeedMultiplier,
            "getGold", &Stats::getGold,
            "setGold", &Stats::setGold,
            "getGoldPerSecond", &Stats::getGoldPerSecond,
            "getLevel", &Stats::getLevel,
            "setCritChance", &Stats::setCritChance,
            "setBonusApFlat", &Stats::setBonusApFlat,
            "setBonusAdFlat", &Stats::setBonusAdFlat,
            "setArmor", &Stats::setArmor,
            "setMagicResist", &Stats::setMagicArmor,
            "setCurrentHealth", &Stats::setCurrentHealth,
            "setCurrentMana", &Stats::setCurrentMana,
            "setMaxMana", &Stats::setMaxMana,
            "setMaxHealth", &Stats::setMaxHealth,
            "setMovementSpeed", &Stats::setMovementSpeed,
            "setSize", &Stats::setSize,
            "setAttackSpeedMultiplier", &Stats::setAttackSpeedMultiplier,
            "getSpellCostReduction", &Stats::getSpellCostReduction,
            "setSpellCostReduction", &Stats::setSpellCostReduction,
            "getArmorPenFlat", &Stats::getArmorPenFlat,
            "getArmorPenPct", &Stats::getArmorPenPct,
            "getMagicPenFlat", &Stats::getMagicPenFlat,
            "getMagicPenPct", &Stats::getMagicPenPct,
            "setArmorPenFlat", &Stats::setArmorPenFlat,
            "setArmorPenPct", &Stats::setArmorPenPct,
            "setMagicPenFlat", &Stats::setMagicPenFlat,
            "setMagicPenPct", &Stats::setMagicPenPct);
    lua.set_userdata(statsUserData);

    sol::constructors <sol::types <Champion*, std::string, uint8>> spellCtr;
    sol::userdata <Spell> spellUserData(
            "Spell", spellCtr,
            "getOwner", &Spell::getOwner,
            "getId", &Spell::getId,
            "getCastTime", &Spell::getCastTime,
            "getStringForSlot", &Spell::getStringForSlot,
            "getCost", &Spell::getCost,
            "getFlags", &Spell::getFlags,
            "getLevel", &Spell::getLevel,
            "levelUp", &Spell::levelUp,
            "getState", &Spell::getState,
            "getSlot", &Spell::getSlot);
    lua.set_userdata(spellUserData);
}
