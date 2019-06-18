#include "Inventory.h"
#include "stdafx.h"
#include "ItemManager.h"
#include "Logger.h"

using namespace std;

const ItemInstance* Inventory::addItem(const ItemTemplate* itemTemplate) {
   int slot = -1;
   
   if(itemTemplate->isTrinket()) {
      if(items[6] == 0) {
         items[6] = new ItemInstance(itemTemplate, 6, 1);
         return items[6];
      }
      
      return 0;
   }

   if(itemTemplate->getMaxStack() > 1) {
      for(slot = 0; slot < 6; ++slot) {
         if(items[slot] == 0) {
            continue;
         }
      
         if(items[slot]->getTemplate() == itemTemplate) {
            if(items[slot]->getStacks() < itemTemplate->getMaxStack()) {
               items[slot]->incrementStacks();
               return items[slot];
            } else if(items[slot]->getStacks() == itemTemplate->getMaxStack()) {
               return 0;
            }
         }
      }
   }
   
   for(slot = 0; slot < 6; ++slot) {
      if(items[slot] == 0) {
         break;
      }
   }
   
   if(slot == 6) { // Inventory full
      return 0;
   }
   
   CORE_INFO("Adding item %d to slot %d", itemTemplate->getId(), slot);
   items[slot] = new ItemInstance(itemTemplate, slot, 1);
   
   return items[slot];
}

vector<ItemInstance*> Inventory::getAvailableRecipeParts(const ItemTemplate* recipe) {
   vector<ItemInstance*> toReturn;
   
   for(uint32 itemId : recipe->getRecipeParts()) {
      const ItemTemplate* item = ItemManager::getInstance()->getItemTemplateById(itemId);
      if(!item) {
         continue;
      }
      vector<ItemInstance*> parts = _getAvailableRecipeParts(item);
      toReturn.insert(toReturn.begin(), parts.begin(), parts.end());
   }
   
   for(ItemInstance* i : items) {
      if(i) {
         i->setRecipeSearchFlag(false);
      }
   }
   
   return toReturn;
}

vector<ItemInstance*> Inventory::_getAvailableRecipeParts(const ItemTemplate* recipe) {
   vector<ItemInstance*> toReturn;
   
   for(ItemInstance* i : items) {
      if(!i) {
         continue;
      }
      
      if(i->getTemplate()->getId() == recipe->getId() && !i->getRecipeSearchFlag()) {
         toReturn.push_back(i);
         i->setRecipeSearchFlag(true);
         return toReturn;
      }
   }
   
   for(uint32 itemId : recipe->getRecipeParts()) {
      vector<ItemInstance*> parts = _getAvailableRecipeParts(ItemManager::getInstance()->getItemTemplateById(itemId));
      toReturn.insert(toReturn.begin(), parts.begin(), parts.end());
   }
   
   return toReturn;
}

void Inventory::swapItems(uint8 slotFrom, uint8 slotTo) {
   ItemInstance* to = items[slotTo];
   items[slotTo] = items[slotFrom];
   items[slotFrom] = to;
}

void Inventory::removeItem(uint8 slot) {
   if(items[slot] == 0) {
      return;
   }
   
   delete items[slot];
   items[slot] = 0;
}

ItemInstance* Inventory::getItemSlot(uint8 slot) {
   if(items[slot] == 0) {
      return NULL;
   }
   
   return items[slot];
}