#ifndef __H_INIBIN
#define __H_INIBIN

#include <map>
#include <string>
#include <stdio.h>

#include "MemoryReader.h"

using namespace std;

struct Value {
   Value() : floatV(0), boolV(0), intV(0) {
   
   }

   float floatV;
   bool boolV;
   uint32 intV;
   std::string stringV;
};

class Inibin {
public:
   Inibin(const std::vector<unsigned char>& file) : buffer(file) {
      
      size = buffer.size();
      buffer >> version >> oldLength >> bitmask;
      
      if(version != 2) {
         return;
      }
      
      uint16 nbKeys;
      
      if(bitmask & 0x0001) {
         buffer >> nbKeys;
         readUint32Values(readKeys(nbKeys));
      }
      
      if(bitmask & 0x0002) {
         buffer >> nbKeys;
         readFloatValues(readKeys(nbKeys));
      }
      
      if(bitmask & 0x0004) {
         buffer >> nbKeys;
         readUint10Values(readKeys(nbKeys));
      }
      
      if(bitmask & 0x0008) {
         buffer >> nbKeys;
         readUint16Values(readKeys(nbKeys));
      }
      
      if(bitmask & 0x0010) {
         buffer >> nbKeys;
         readUint8Values(readKeys(nbKeys));
      }
      
      if(bitmask & 0x0020) {
         buffer >> nbKeys;
         readBooleanValues(readKeys(nbKeys));
      }
      
      // 3
      if(bitmask & 0x0040) {
         buffer >> nbKeys;
         buffer.skip(nbKeys*(4+3));
      }
      
      // 12
      if(bitmask & 0x0080) {
         buffer >> nbKeys;
         buffer.skip(nbKeys*(4+12));
      }
      
      // 2
      if(bitmask & 0x0100) {
         buffer >> nbKeys;
         readUint16Values(readKeys(nbKeys));
      }
      
      // 8
      if(bitmask & 0x0200) {
         buffer >> nbKeys;
         buffer.skip(nbKeys*(4+8));
      }
      
      // 4
      if(bitmask & 0x0400) {
         buffer >> nbKeys;
         readUint32Values(readKeys(nbKeys));
      }
      
      // 16
      if(bitmask & 0x0800) {
         buffer >> nbKeys;
         buffer.skip(nbKeys*(4+16));
      }
      
      if(bitmask & 0x1000) {
         buffer >> nbKeys;
         readStringValues(readKeys(nbKeys));
      }
   }
   
   std::vector<uint32> readKeys(uint16 nbKeys) {
      std::vector<uint32> toReturn;

      for(uint16 i = 0; i < nbKeys; ++i) {
         uint32 key;
         buffer >> key;
         toReturn.push_back(key);
      }

      return toReturn;
   }
   
   static uint32 getKeyHash(const std::string& sectionName, const std::string& varName) {
      uint32 hash = 0;
      
      for(char c : sectionName) {
         hash = tolower(c) + 65599 * hash;
      }

      hash = (65599 * hash + 42);
      for(char c : varName) {
         hash = tolower(c) + 65599 * hash;
      }

      return hash;
   }
   
   void readUint32Values(const std::vector<uint32>& keys) {
      for(uint32 key : keys) {
         uint32 v;
         buffer >> v;
      
         Value value;
         value.floatV = (float)v;
         value.intV = v;
         
         values[key] = value;
      }
   }
   
   void readUint16Values(const std::vector<uint32>& keys) {
      for(uint32 key : keys) {
         uint16 v;
         buffer >> v;
         
         Value value;
         value.floatV = (float)v;
         value.intV = v;
         values[key] = value;
      }
   }
   
   void readUint8Values(const std::vector<uint32>& keys) {
      for(uint32 key : keys) {
         uint8 v;
         buffer >> v;
                  
         Value value;
         value.floatV = v;
         value.intV = v;
         values[key] = value;
      }
   }
   
   void readUint10Values(const std::vector<uint32>& keys) {
      for(uint32 key : keys) {
         uint8 v;
         buffer >> v;
         Value value;
         value.floatV = v*0.1f;
         values[key] = value;
      }
   }
   
   void readFloatValues(const std::vector<uint32>& keys) {
      for(uint32 key : keys) {
         Value value;
         buffer >> value.floatV;
         
         values[key] = value;
      }
   }
   
   void readBooleanValues(const std::vector<uint32>& keys) {      
      int index = 0;
      uint8 boolean;
      
      for(uint32 key : keys) {
         Value value;
      
         if(index % 8 == 0) {
            buffer >> boolean;
         } else {
            boolean >>= 1;
         }
         
         value.boolV = boolean & 0x01;
         value.intV = boolean & 0x01;
         values[key] = value;
         ++index;
      }
   }
   
   void readStringValues(const std::vector<uint32>& keys) {
      uint32 i = 1;
      for(uint32 key : keys) {
         Value value;
  
         uint16 offset;
         
         buffer >> offset;
         value.stringV = reinterpret_cast<const char*>(&buffer.getBytes()[offset+keys.size()*2-i*2]);
         
         values[key] = value;
         ++i;
      }
   }
         
   float getFloatValue(const std::string& sectionName, const std::string& varName) {
      if(values[getKeyHash(sectionName, varName)].floatV == 0 && values[getKeyHash(sectionName, varName)].stringV.length() > 0) {
         return (float)atof(values[getKeyHash(sectionName, varName)].stringV.c_str());
      }
      return values[getKeyHash(sectionName, varName)].floatV;
   }
   
   bool getBoolValue(const std::string& sectionName, const std::string& varName) {
      if(values[getKeyHash(sectionName, varName)].stringV.length() > 0) {
         return values[getKeyHash(sectionName, varName)].stringV == "Yes";
      }
      return values[getKeyHash(sectionName, varName)].boolV;
   }
   
   uint32 getIntValue(const std::string& sectionName, const std::string& varName) {
      return getIntValue(getKeyHash(sectionName, varName));
   }
   
   uint32 getIntValue(uint32 keyHash) {
      if(values[keyHash].intV == 0 && values[keyHash].stringV.length() > 0) {
         return atoi(values[keyHash].stringV.c_str());
      }
      return values[keyHash].intV;
   }
   
   const std::string& getStringValue(const std::string& sectionName, const std::string& varName) {
      return values[getKeyHash(sectionName, varName)].stringV;
   }
   
   bool keyExists(const std::string& sectionName, const std::string& varName) {
      return values.find(getKeyHash(sectionName, varName)) != values.end();
   }
   
   void printValues() {
      for(auto& v : values) {
         printf("%08X : ", v.first);
         if(!v.second.stringV.empty()) {
            printf("\"%s\"\n", v.second.stringV.c_str());
            continue;
         }
         
         if(!v.second.intV && !v.second.floatV) {
            printf("%d\n", v.second.boolV);
            
            continue;
         }
         
         if(!v.second.intV) {
            printf("%f\n", v.second.floatV);
            continue;
         }
         
         printf("%d\n", v.second.intV);
      }
   }

private:
   
   std::map<uint32, Value> values;
   MemoryReader buffer;
   uint8 version;
   uint16 oldLength;
   uint16 bitmask;
   uint32 size;


};




#endif