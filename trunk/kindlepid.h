#ifndef KINDLEPID_H
#define KINDLEPID_H
#include <boost/crc.hpp>
#include <string>
#include <vector>
#include <ios>
#include <iostream>
#include <ostream>
#include <cmath>

typedef boost::crc_optimal<32, 0x04C11DB7, 0, 0xFFFFFFFF, true, true>   mycrc_32_type;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef std::vector< uchar > SerialChars;

//Class for implementing code originally from "kindlepid.py"
//By Igor Skochinsky <skochinsky@mail.ru> in 2007
//Proted by Evan Carew <carew@pobox.com> in 2009
class KindlePidFinder{
  std::string letters, buffer, pid, chksum;
  unsigned int PidLen;
public:
  //Setup the class' globals
  KindlePidFinder(std::string serial);
  //Perform a checksum on the pid
  std::string checksumPid();
  //Calculate the pid from the serial number
  std::string pidFromSerial();
  //Perform something like the standard crc32 calculation
  uint checksum(std::string s);
};



#endif
