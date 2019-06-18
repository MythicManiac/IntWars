#include "LuaScript.h"
#include <sol.hpp>
#include <sol/state.hpp>

#include "Unit.h"
#include "Object.h"
#include "Target.h"


void LuaScript::addUnit() {
    sol::constructors <sol::types<float, float>> targetCtr;
    sol::userdata <Target> targetUserData(
            "Target", targetCtr,
            //"distanceWith", &Target::distanceWith, //will not work... multiple implementations?
            "getX", &Target::getX,
            "getY", &Target::getY,
            "setPosition", &Target::setPosition,
            "isSimpleTarget", &Target::isSimpleTarget);
    lua.set_userdata(targetUserData);
    
    //Because Object is abstract:
    /*sol::constructors <sol::types < Map*, uint32, float, float, uint32>> objCtr;
    sol::userdata <Object> objUserData(
            "Object", objCtr,
            "setTeam", &Object::setTeam,
            "setTeam", &Object::setTeam,
            "getTarget", &Object::getTarget,
            "setTarget", &Object::setTarget,
            //"setTargetUnit", &Object::setTargetUnit, //not in Object anymore
            "isToRemove", &Object::isToRemove,
            "setToRemove", &Object::setToRemove,
            "getNetId", &Object::getNetId,
            "getMap", &Object::getMap,
            "setPosition", &Object::setPosition,
            "getCollisionRadius", &Object::getCollisionRadius,
            //"distanceWith", &Object::distanceWith, //Will not work
            "getX", &Object::getX,
            "getY", &Object::getY,
            "isTargetable", &Object::isTargetable,
            "setTargetable", &Object::setTargetable);
    lua.set_userdata(objUserData);*/

    //Basic documentation in 3... 2... 1...
    //constructor types here ------------\ are needed, even though we won't use it in lua
    sol::constructors <sol::types < Map*, uint32, std::string, Stats*, uint32, float, float, uint32>> unitCtr;
    sol::userdata <Unit> unitUserData(// this is the actual user data.
            "Unit", unitCtr, //Unit's constructor, not really useful, but necessary.
            "getStats", &Unit::getStats, //"methodName", &Class::method
            "getMoveSpeed", &Unit::getMoveSpeed,
            "getModel", &Unit::getModel,
            "setModel", &Unit::setModel,
            "getBuffs", &Unit::getBuffs,
            "addBuff", &Unit::addBuff,
            "dealDamageTo", &Unit::dealDamageTo,
            "getTarget", &Unit::getTarget, //some Object methods
            "setTarget", &Unit::setTarget,
            "getTeam", &Unit::getTeam,
            "setTeam", &Unit::setTeam,
            "setTargetUnit", &Unit::setTargetUnit,
            "isToRemove", &Unit::isToRemove,
            "setToRemove", &Unit::setToRemove,
            "getNetId", &Unit::getNetId,
            "getMap", &Unit::getMap,
            "setPosition", &Unit::setPosition,
            "getCollisionRadius", &Unit::getCollisionRadius,
            //"distanceWith", &Unit::distanceWith, //Will not work
            "getX", &Unit::getX, // some Target methods
            "getY", &Unit::getY,
            "getBuff", &Unit::getBuff);
            
    lua.set_userdata(unitUserData); //Add the userData to lua

            
    sol::constructors <sol::types < std::string, float, BuffType, Unit*>, sol::types<std::string, float, BuffType, Unit*, Unit*> > buffCtr;
    sol::userdata <Buff> buffUserData(// this is the actual user data.
            "Buff", buffCtr, //Unit's constructor, not really useful, but necessary.
            "getName", &Buff::getName,
            "setMovementSpeedPercentModifier", &Buff::setMovementSpeedPercentModifier
            );//"methodName", &Class::method);
    lua.set_userdata(buffUserData); //Add the userData to lua
    
    //Setting lua values:
    lua.set("DAMAGE_TYPE_PHYSICAL", DAMAGE_TYPE_PHYSICAL);
    lua.set("DAMAGE_TYPE_MAGICAL", DAMAGE_TYPE_MAGICAL);
    lua.set("DAMAGE_TYPE_TRUE", DAMAGE_TYPE_TRUE);
    lua.set("DAMAGE_SOURCE_ATTACK", DAMAGE_SOURCE_ATTACK);
    lua.set("DAMAGE_SOURCE_SPELL", DAMAGE_SOURCE_SPELL);
    lua.set("DAMAGE_SOURCE_SUMMONER_SPELL", DAMAGE_SOURCE_SUMMONER_SPELL);
    lua.set("DAMAGE_SOURCE_PASSIVE", DAMAGE_SOURCE_PASSIVE);
    lua.set("BUFFTYPE_ETERNAL", BUFFTYPE_ETERNAL);
    lua.set("BUFFTYPE_TEMPORARY", BUFFTYPE_TEMPORARY);

}