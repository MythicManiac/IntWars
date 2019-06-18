#include "FileReader.h"

FileReader::FileReader(const std::string& filename) : file(filename.c_str(), std::ifstream::binary) {

}

void FileReader::seek(uint32 offset) {
   file.seekg(offset);
}

uint32 FileReader::pos() {
   return file.tellg();
}

void FileReader::read(std::vector<unsigned char>& data, uint32 size) {
   data.resize(size);
   file.read(reinterpret_cast<char*>(&data[0]), size);
}