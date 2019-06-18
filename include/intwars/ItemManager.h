#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include <map>

#include "Item.h"

class ItemManager {
private:
   std::map<uint32, ItemTemplate*> itemTemplates;
   ItemManager() { }
   static ItemManager* instance;
   
public:
   static ItemManager* getInstance() {
      if(!instance) {
         instance = new ItemManager();
      }
      
      return instance;
   }
   
   void init();
   const ItemTemplate* getItemTemplateById(uint32 id);

};

#endif