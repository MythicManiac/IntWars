#include <sol.hpp>
#include <stdafx.h>

#ifndef LUASCRIPT_H
#define	LUASCRIPT_H

class LuaScript{
private:
bool loaded = false;
public:
    LuaScript(bool defineClasses);
    bool isLoaded() { return loaded; }
    void loadScript(std::string location);
    sol::table getTable(std::string name);
    void setFunction();
    void setLoaded(bool load){ loaded = load;}
    sol::state lua;
    
    void addChampion();
    void addUnit ();
    void addItem ();
    void addGame ();
};


#endif	/* LUASCRIPT_H */

