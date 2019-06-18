#ifndef __H_BUFFER
#define __H_BUFFER

#include <vector>

class Buffer {

private:
   std::vector<uint8> buffer;

public:
   const std::vector<uint8>& getBytes() const { return buffer; }
   void clear() { buffer.clear(); }
   
   Buffer& operator<<(const std::string& data)
   {
      buffer.insert(buffer.end(), (uint8*)data.c_str(), (uint8*)(data.c_str())+data.length());
      return *this;
   }
   
   template<typename U>
   Buffer& operator<<(const U& data)
   {
      buffer.insert(buffer.end(), (uint8*)&data, (uint8*)(&data)+sizeof(U));
      return *this;
   }
   
   void fill(uint8 data, uint32 length) {
      for(uint32 i = 0; i < length; ++i) {
         buffer.push_back(data);
      }
   }
   
   uint32 size() const {
      return buffer.size();
   }
};

#endif