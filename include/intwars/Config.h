#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "RAFManager.h"

class Config {
public:
	std::string luadir;
	std::string config_File;
	std::string filearchives_Path;
	Config();

public:
  static Config& instance()
  {
      static Config instance;
      return instance;
  }

 /*void setluaconfig(std::string file);
 void setfilearchivesPath(std::string path);
 std::string getluaconfig();
 std::string getfilearchivesPath();*/
  ~Config();
};

#endif   /* CONFIG_H */
