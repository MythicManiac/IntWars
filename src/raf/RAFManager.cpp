#include "RAFManager.h"
#include "tinydir.h"
#include "stdafx.h"
#include "Logger.h"


#ifdef _WIN32
#include <windows.h>
#include<Winreg.h>
#endif
using namespace std;

RAFManager* RAFManager::instance = 0;

bool RAFManager::init(const string& rootDirectory) {
   tinydir_dir dir;

   if(tinydir_open_sorted(&dir, rootDirectory.c_str()) == -1) {
      return false;
   }

   for (size_t i = 0; i < dir.n_files; i++)
   {
      tinydir_file file;
      tinydir_readfile_n(&dir, &file, i);

      if (!file.is_dir || strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0) {
         continue;
      }

      tinydir_dir subDir;
      tinydir_open_sorted(&subDir, (rootDirectory + '/' + file.name).c_str());
      
      for (size_t j = 0; j < subDir.n_files; j++)
      {
         tinydir_file subFile;
         tinydir_readfile_n(&subDir, &subFile, j);
      
         string filename(subFile.name);
         string check = ".raf";
         
         if(filename.length() >= 5 && filename.find(check) && filename[filename.length()-1] == 'f') {
            string rafPath = rootDirectory + '/' + file.name + '/' + filename;
            RAFFile* raf = new RAFFile(rafPath);
            files.push_back(raf);
         }
      }
      
      tinydir_close(&subDir);
   }
   
   CORE_INFO("Loaded %lu RAF files", files.size());

   tinydir_close(&dir);
   
   return true;
}

bool RAFManager::readFile(const std::string& path, vector<unsigned char>& toFill) {
	Sleep(2); // until we don't have everything correct, do not let it lag our pc

	std::ifstream t_FileStream(path, std::ios::binary);
	t_FileStream.seekg(0, std::ios::end);
	std::streamsize size = t_FileStream.tellg();
	t_FileStream.seekg(0, std::ios::beg);
	toFill.resize((unsigned)size);
	if (t_FileStream.read((char*)toFill.data(), size)) {
		return true;
	}

	// TODO
	for(vector<RAFFile*>::iterator it = files.begin(); it != files.end(); ++it) {
		RAFFile* f = *it;
		if(f->readFile(path, toFill)) {
			return true;
		}
	}

	return false;
}
/*// why should we want this.. lets just insert path in source..
std::string RAFManager::findGameBasePath()
{
	//CORE_INFO("Searching for LoL base path...");
#ifdef _WIN32
	HKEY hKey;
	std::vector<string> strKeyPathCU, strKeyPathLM;
	strKeyPathCU.push_back("SOFTWARE\\RIOT GAMES\\RADS"); 
	strKeyPathCU.push_back("VirtualStore\\MACHINE\\SOFTWARE\\Wow6432Node\\RIOT GAMES\\RADS");
	strKeyPathCU.push_back("SOFTWARE\\Classes\\VirtualStore\\MACHINE\\SOFTWARE\\Wow6432Node\\RIOT GAMES\\RADS");
	strKeyPathCU.push_back("SOFTWARE\\Classes\\VirtualStore\\MACHINE\\SOFTWARE\\RIOT GAMES\\RADS");

	strKeyPathLM.push_back("Software\\Wow6432Node\\Riot Games\\RADS");
	strKeyPathLM.push_back("SOFTWARE\\RIOT GAMES\\RADS");

	string strKeyName = "LOCALROOTFOLDER";
	DWORD dwValueType;
	TCHAR byteValue[100];
	DWORD dwValueSize;


   //Check CLASSES_ROOT keys
   for (size_t i = 0; i < strKeyPathCU.size(); i++)
   {
      if (RegOpenKeyExA(HKEY_CLASSES_ROOT, strKeyPathCU[i].c_str(), 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
      {
         //CORE_WARNING("Cannot open key HKEY_CLASSES_ROOT '%s'", strKeyPathCU[i].c_str());
         continue;
      }

      if (RegQueryValueExA(hKey, strKeyName.c_str(), NULL, &dwValueType, (LPBYTE)byteValue, &dwValueSize) != ERROR_SUCCESS)
      {
         //CORE_WARNING("Cannot read key HKEY_CLASSES_ROOT '%s'", strKeyPathCU[i].c_str());
         continue;
      }

      string sValue(byteValue);
      sValue += "/projects/lol_game_client/";
      CORE_INFO("Found base path in %s", sValue.c_str());

      return sValue;
   }

   //Check CURRENT_USER keys
   for (size_t i = 0; i < strKeyPathCU.size(); i++)
   {
      if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, strKeyPathCU[i].c_str(), 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
      {
         //CORE_WARNING("Cannot open key HKEY_CURRENT_USER '%s'", strKeyPathCU[i].c_str());
         continue;
      }

      if (RegQueryValueExA(hKey, strKeyName.c_str(), NULL, &dwValueType, (LPBYTE)byteValue, &dwValueSize) != ERROR_SUCCESS)
      {
         //CORE_WARNING("Cannot read key HKEY_CURRENT_USER '%s'", strKeyPathCU[i].c_str());
         continue;
      }

      string sValue(byteValue);
      sValue += "/projects/lol_game_client/";
      CORE_INFO("Found base path in %s", sValue.c_str());

      return sValue;
   }

	//Check LOCAL_MACHINE keys

	for(size_t i=0; i< strKeyPathLM.size();i++)
   {
      if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, strKeyPathLM[i].c_str(), 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
      {
         //CORE_WARNING("Cannot open key HKEY_LOCAL_MACHINE '%s'", strKeyPathCU[i].c_str());
         continue;
      }

		if( RegQueryValueExA(hKey, strKeyName.c_str(), NULL, &dwValueType, (LPBYTE)byteValue, &dwValueSize) != ERROR_SUCCESS )
      {
         //CORE_WARNING("Cannot read key HKEY_LOCAL_MACHINE '%s'", strKeyPathCU[i].c_str());
			continue;
		}

		std::string sValue(byteValue);
		sValue += "/projects/lol_game_client/";

		CORE_INFO("Found base path in %s",sValue.c_str());
		return sValue;
	}

	CORE_ERROR("Couldnt find League of Legends game path or unable to read Registry keys");
	return "";

#else
	return "";
#endif
}*/
