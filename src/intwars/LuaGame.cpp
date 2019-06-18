#include "LuaScript.h"
#include "sol/state.hpp"
#include "Game.h"
#include "Map.h"

void LuaScript::addGame() {
    sol::constructors <sol::types<>> gameCtr;
    sol::userdata <Game> gameUserData(
            "Game", gameCtr,
            "notifyMinionSpawned", &Game::notifyMinionSpawned,
            "notifySetHealth", &Game::notifySetHealth,
            "notifyUpdatedStats", &Game::notifyUpdatedStats,
            "notifyMovement", &Game::notifyMovement,
            "notifyDamageDone", &Game::notifyDamageDone,
            "notifyBeginAutoAttack", &Game::notifyBeginAutoAttack,
            "notifyNextAutoAttack", &Game::notifyNextAutoAttack,
            "notifyTeleport", &Game::notifyTeleport,
            "notifyProjectileSpawn", &Game::notifyProjectileSpawn,
            "notifyProjectileDestroy", &Game::notifyProjectileDestroy,
            "notifyParticleSpawn", &Game::notifyParticleSpawn,
            "notifyModelUpdate", &Game::notifyModelUpdate,
            "notifyLevelUp", &Game::notifyLevelUp,
            "notifyItemBought", &Game::notifyItemBought,
            "notifyItemsSwapped", &Game::notifyItemsSwapped,
            "notifyRemoveItem", &Game::notifyRemoveItem,
            "notifySetTarget", &Game::notifySetTarget);
    lua.set_userdata(gameUserData);

    //Because Map is abstract...
    /*sol::constructors <sol::types < Game*>> mapCtr;
    sol::userdata <Map> mapUserData(
            "Map", mapCtr,
            "getGoldPerSecond", &Map::getGoldPerSecond,
            "getObjectById", &Map::getObjectById,
            "addObject", &Map::addObject,
            "getExperienceToLevelUp", &Map::getExperienceToLevelUp,
            "getGame", &Map::getGame,
            "getObjects", &Map::getObjects);
    lua.set_userdata(mapUserData);*/
}
