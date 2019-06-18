#include "Item.h"
#include "ItemManager.h"

uint32 ItemTemplate::getTotalPrice() const {
   uint32 toReturn = price;
   
   for(uint32 itemId : recipes) {
      const ItemTemplate* item = ItemManager::getInstance()->getItemTemplateById(itemId);
      if(!item) {
         continue;
      }
      toReturn += item->getTotalPrice();
   }
   
   return toReturn;
}