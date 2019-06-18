#ifndef __H_FILEREADER
#define __H_FILEREADER

#include <fstream>
#include <string>
#include <vector>

#include "stdafx.h"

class FileReader {
public:
   FileReader(const std::string& filename);
   
   template<typename U>
   FileReader& operator>>(U& data)
   {
      file.read(reinterpret_cast<char*>(&data), sizeof(U));
      return *this;
   }

   void seek(uint32 offset);
   uint32 pos();
   void read(std::vector<unsigned char>&, uint32 size);

private:
   std::ifstream file;

};


#endif