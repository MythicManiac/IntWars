#ifndef __H_MEMORYREADER
#define __H_MEMORYREADER

#include <fstream>
#include <string>
#include <vector>

#include "stdafx.h"

class MemoryReader {
public:
   MemoryReader(const std::vector<unsigned char>& buffer) : buffer(buffer) { }
   
   template<typename U>
   MemoryReader& operator>>(U& data)
   {
      memcpy(&data, &buffer[0], sizeof(U));
      buffer.erase(buffer.begin(), buffer.begin()+sizeof(U));
      
      return *this;
   }
   
   MemoryReader& operator>>(std::string& data)
   {
      int i = 0;
      for(; buffer[i] != 0; ++i);
      ++i;
      
      data.assign(reinterpret_cast<char*>(&buffer[0]), i);
      buffer.erase(buffer.begin(), buffer.begin()+i);
      
      return *this;
   }
   
   void skip(uint32 toSkip) {
      if(toSkip > buffer.size()) {
         return;
      }
      buffer.erase(buffer.begin(), buffer.begin()+toSkip);
   }
   
   uint32 size() {
      return buffer.size();
   }
   
   const std::vector<unsigned char>& getBytes() {
      return buffer;
   }

private:
   std::vector<unsigned char> buffer;

};


#endif