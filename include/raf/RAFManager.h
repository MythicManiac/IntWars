#ifndef __H_RAFMANAGER
#define __H_RAFMANAGER

#include "RAFFile.h"
#include <string>
#include <vector>

class RAFManager {
public:
   static RAFManager* getInstance() {
      if(!instance) {
         instance = new RAFManager();
      }
      
      return instance;
   }
   
   bool init(const std::string& rootDirectory);
   //std::string findGameBasePath();
   bool readFile(const std::string& path, std::vector<unsigned char>& toFill);

private:
   RAFManager() { }
   std::vector<RAFFile*> files;
   static RAFManager* instance;


};

#endif