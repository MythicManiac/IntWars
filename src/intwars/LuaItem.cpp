#include "LuaScript.h"
#include "sol/state.hpp"
#include "Item.h"
#include "Inventory.h"

void LuaScript::addItem() {
    sol::constructors <sol::types < ItemTemplate*, uint8, uint8>> itemInstanceCtr;
    sol::userdata <ItemInstance> itemInstanceUserData(
            "ItemInstance", itemInstanceCtr,
            "getTemplate", &ItemInstance::getTemplate,
            "getStacks", &ItemInstance::getStacks,
            "getSlot", &ItemInstance::getSlot,
            "incrementStacks", &ItemInstance::incrementStacks,
            "getRecipeSearchFlag", &ItemInstance::getRecipeSearchFlag,
            "setRecipeSearchFlag", &ItemInstance::setRecipeSearchFlag);
    lua.set_userdata(itemInstanceUserData);

    sol::constructors < sol::types<uint32, uint32, uint32, float, bool, std::vector<StatMod>, std::vector < uint32>>> itemTemplateCtr;
    sol::userdata <ItemTemplate> itemTemplateUserData(
            "ItemTemplate", itemTemplateCtr,
            "getId", &ItemTemplate::getId,
            "getMaxStack", &ItemTemplate::getMaxStack,
            "getPrice", &ItemTemplate::getPrice,
            "getTotalPrice", &ItemTemplate::getTotalPrice,
            "getSellBackModifier", &ItemTemplate::getSellBackModifier,
            "isTrinket", &ItemTemplate::isTrinket,
            //"getStatMods", &ItemTemplate::getStatMods, //StatMod not defined
            "isRecipe", &ItemTemplate::isRecipe,
            "getRecipeParts", &ItemTemplate::getRecipeParts);
    lua.set_userdata(itemTemplateUserData);

    sol::constructors <sol::types<>> inventoryCtr;
    sol::userdata <Inventory> inventoryUserData(
            "Inventory", inventoryCtr,
            "getItems", &Inventory::getItems,
            "addItem", &Inventory::addItem,
            "swapItems", &Inventory::swapItems,
            "removeItem", &Inventory::removeItem,
            "getAvailableRecipeParts", &Inventory::getAvailableRecipeParts);
    lua.set_userdata(inventoryUserData);
}