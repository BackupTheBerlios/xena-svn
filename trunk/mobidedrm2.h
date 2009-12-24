#ifndef MOBIDEDRM_H
#define MOBIDEDRM_H
#include <iostream>
#include <fstream>
#include <string>
#include <boost/crc.hpp>

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef boost::crc_optimal<32, 0x04C11DB7, 0, 0xFFFFFFFF, true, true>   mycrc_32_type;

std::string PC1(std::string key, std::string src, bool decryption = true);
uint getSizeOfTrailingDataEntries(std::string ptr, uint size, ulong flags);

class section{
 public:
  ulong offset;
  uchar flags, val;
  section(ulong off, uchar fl, uchar v):
    flags = fl, offset = off, val = v{}
    uchar& getFlags(){return flags;}
    uchar& getVal(){return val;}
    ulong& getOffset(){return offset;}
};

class MobiDeDrm{
  uint num_sections;
  std::ifstream *mobiBookIn;
  std::ofstream *mobiBookOut;
  std::string pid;
  ulong extraDataFlags;
  std::ostringstream *data;
  std::vector<section> sections;
 public:
  MobiDeDrm(std::string ifname, std::string ofname, std::string kindlePid);
  ~MobiDeDrm(){}
  uint getSectionsCount();
  bool isMobiFileFormat();
  void loadSection();
  bool isValidCryptoType();
  void getSectionExtraDataFlags();
  void patch(uint offset, uint newOff);
  void patchSection(uint section, uint newOff, uint in_off = 0);
  std::string parseDRM(uint count);
  std::ostringstream getCleanBook();
  uint checksum(std::string s);
};

#endif
