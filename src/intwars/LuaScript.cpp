#include "LuaScript.h"
#include <sol/state.hpp>

LuaScript::LuaScript(bool defineClasses){
    lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::package, sol::lib::math, sol::lib::debug);
    
    if (defineClasses) { //Done this way because of number of sections
        addGame(); 
        addChampion();
        addItem();
        addUnit();
    }
}

sol::table LuaScript::getTable(std::string name){
    return lua.get<sol::table>(name);
}

void LuaScript::loadScript(std::string location){
        loaded = true;
        lua.open_file(location);
}