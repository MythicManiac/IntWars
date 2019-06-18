#include "RAFFile.h"
#include <stdio.h>
#include "miniz.h"
#include "Logger.h"

#define BUF_SIZE 1024*1024

using namespace std;

RAFFile::RAFFile(const std::string& filename) : filename(filename) {
   FileReader rafHeaderFile(filename);
   rafHeaderFile >> header;
   
   if(header.magicNumber != 0x18be0ef0) {
      return;
   }

   vector<string> pathList;
   uint32 pathListSize, pathCount;

   rafHeaderFile.seek(header.pathListOffset);
   rafHeaderFile >> pathListSize >> pathCount;

   for(uint32 i = 0; i < pathCount; i++) {
      uint32 offset, len;

      rafHeaderFile >> offset >> len;

      uint32 prevPos = rafHeaderFile.pos();

      rafHeaderFile.seek(header.pathListOffset + offset);
      vector<unsigned char> pathBuf;
      rafHeaderFile.read(pathBuf, len);
      string path(reinterpret_cast<char*>(pathBuf.data()), len);
      pathList.push_back(path);

      rafHeaderFile.seek(prevPos);
   }
   
   uint32 fileCount;
   
   rafHeaderFile.seek(header.fileListOffset);
   rafHeaderFile >> fileCount;
   
   for(uint32 i = 0; i < fileCount; ++i) {
      File entry;
      uint32 pathId;

      rafHeaderFile >> entry.hash >> entry.offset >> entry.size >> pathId;
      entry.path = pathList[pathId];

      fileEntries[entry.hash] = entry;
   }
}

uint32 RAFFile::getHash(const std::string& path) {
	unsigned int hash = 0;
	unsigned int mask = 0xF0000000;
	for(unsigned int i = 0; i < path.length(); i++)
	{
		hash = tolower(path[i]) + (0x10 * hash);
		if(hash & mask)
			hash ^= hash & mask ^ ((hash & mask) >> 24);
	}
	return hash;
}

bool RAFFile::readFile(const std::string& path, vector<unsigned char>& toFill) {
   uint32 hash = getHash(path);
   vector<unsigned char> compressedFile;
   
   auto entry = fileEntries.find(hash);
   
   if(entry == fileEntries.end()) {
      return false;
   }
   
   FileReader rafDataFile(filename+".dat");
   
   rafDataFile.seek(entry->second.offset);
   rafDataFile.read(compressedFile, entry->second.size);
   
   z_stream stream;
   uint8 s_outbuf[BUF_SIZE]; // 1MB
   
   // Init the z_stream
   memset(&stream, 0, sizeof(stream));
   stream.next_in = &compressedFile[0];
   stream.avail_in = compressedFile.size();
   stream.next_out = s_outbuf;
   stream.avail_out = BUF_SIZE;
   
   if (inflateInit(&stream))
   {
      CORE_ERROR("inflateInit() failed!\n");
      return false;
   }

   for ( ; ; )
   {
      int status;

      status = inflate(&stream, Z_SYNC_FLUSH);

      if ((status == Z_STREAM_END) || (!stream.avail_out))
      {
         toFill.insert(toFill.end(), s_outbuf, s_outbuf + BUF_SIZE - stream.avail_out);
         stream.avail_out = BUF_SIZE;
         stream.next_out = s_outbuf;
      }

      if (status == Z_STREAM_END)
         break;  
      
      if (status != Z_OK)
      {
         CORE_ERROR("inflate() failed with status %i!\n", status);
         return false;
      }
   }

    if (inflateEnd(&stream) != Z_OK)
    {
      CORE_ERROR("inflateEnd() failed!\n");
      return false;
    }
   
   return true;
}