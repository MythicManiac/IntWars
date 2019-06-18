#ifndef INVENTORY_H
#define INVENTORY_H

#define MAX_STACK_AMOUNT 5

#include "Item.h"


class Inventory {
private:
   std::vector<ItemInstance*> items;
   std::vector<ItemInstance*> _getAvailableRecipeParts(const ItemTemplate* recipe);

public:
    
   Inventory() {
      items = { 0, 0, 0, 0, 0, 0, 0 };
   }
   
   const ItemInstance* addItem(const ItemTemplate* itemTemplate);
   void swapItems(uint8 slotFrom, uint8 slotTo);
   const std::vector<ItemInstance*>& getItems() const { return items; }
   void removeItem(uint8 slot);
   ItemInstance* getItemSlot(uint8 slot);
   
   std::vector<ItemInstance*> getAvailableRecipeParts(const ItemTemplate* recipe);
    
};

#endif   /* INVENTORY_H */

