#ifndef ITEM_H
#define ITEM_H

#include "stdafx.h"
#include "Stats.h"
#include <vector>

class ItemTemplate
{
private:
   uint32 id;
   uint32 maxStack;
   uint32 price;
   bool trinket;
   float sellBackModifier;
   std::vector<StatMod> statMods;
   
   /**
    * Necessary item ids to create this one.
    */
   std::vector<uint32> recipes;
   
public:
   ItemTemplate(uint32 id, uint32 maxStack, uint32 price, float sellBackModifier, bool trinket, const std::vector<StatMod>& statMods, const std::vector<uint32>& recipes)
   : id(id), maxStack(maxStack), price(price), sellBackModifier(sellBackModifier), trinket(trinket), statMods(statMods), recipes(recipes) { }
   
   uint32 getId() const { return id; }
   uint32 getMaxStack() const { return maxStack; }
   uint32 getPrice() const { return price; }
   
   /**
    * Returns the total price of an item. This will change for recipes, as they include their parts fees
    */
   uint32 getTotalPrice() const;
   
   bool isTrinket() const { return trinket; }
   float getSellBackModifier() const { return sellBackModifier; }
   const std::vector<StatMod>& getStatMods() const { return statMods; }
   
   bool isRecipe() const { return recipes.size() > 0; }
   const std::vector<uint32> getRecipeParts() const { return recipes; }

};

class ItemInstance {
private:
   const ItemTemplate* itemTemplate;
   uint8 slot, stacks;
   float cooldown;
   bool recipeSearchFlag;

public:
   ItemInstance(const ItemTemplate* itemTemplate, uint8 slot = 0, uint8 stacks = 1) : itemTemplate(itemTemplate), cooldown(0), slot(slot), stacks(stacks), recipeSearchFlag(false) { }
   const ItemTemplate* getTemplate() const { return itemTemplate; }
   uint8 getSlot() const { return slot; }
   uint8 getStacks() const { return stacks; }
   void incrementStacks() { ++stacks; }
   void decrementStacks() { --stacks; }
   
   bool getRecipeSearchFlag() const { return recipeSearchFlag; }
   void setRecipeSearchFlag(bool flag) { recipeSearchFlag = flag; }

};

#endif   /* ITEM_H */

