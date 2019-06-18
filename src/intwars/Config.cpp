#include "Config.h"
#include <iostream>

Config::Config()
{
	this->luadir = "../lua/";
	this->config_File = this->luadir + "config.lua";
	this->filearchives_Path = "filearchives/";//RAFManager::getInstance()->findGameBasePath() + "filearchives";
}


Config::~Config()
{
}
/*
void Config::setluaconfig(std::string file)
{
  this->config_File = file;
}

std::string Config::getluaconfig()
{
    return this->config_File;
}

void Config::setfilearchivesPath(std::string path)
{
  this->filearchives_Path = path;
}
std::string Config::getfilearchivesPath()
{
  return this->filearchives_Path;
}
*/