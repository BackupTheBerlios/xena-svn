#include <boost/crc.hpp>
#include <string>
#include <vector>
#include <ios>
#include <iostream>
#include <ostream>
#include <cmath>

using namespace std;

typedef boost::crc_optimal<32, 0x04C11DB7, 0, 0xFFFFFFFF, true, true>   mycrc_32_type;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef vector< uchar > SerialChars;

//Class for implementing code originally from "kindlepid.py"
//By Igor Skochinsky <skochinsky@mail.ru> in 2007
//Proted by Evan Carew <carew@pobox.com> in 2009
class KindlePidFinder{
  string letters, buffer, pid, chksum;
  unsigned int PidLen;
public:
  //Setup the class' globals
  KindlePidFinder(string serial);
  //Perform a checksum on the pid
  string checksumPid();
  //Calculate the pid from the serial number
  string pidFromSerial();
  //Perform something like the standard crc32 calculation
  uint checksum(string s);
};

KindlePidFinder::KindlePidFinder(string serial){
  letters = "ABCDEFGHIJKLMNPQRSTUVWXYZ123456789";
  buffer = serial;
  PidLen = 7;
}

string KindlePidFinder::checksumPid(){
  mycrc_32_type result;
  uint crc = checksum(pid);
  crc = crc ^ (crc >> 16);
  string res(pid);
  uchar len = letters.length();
  for(int i = 0; i < 2; i++){
    uchar b = crc & 0xff;
    float bdiv = b / len;
    uint pos = (uchar)(floor(bdiv)) ^ (b % len);
    res += letters[pos%len];
    crc >>= 8;
  }
  return res;
}

uint KindlePidFinder::checksum(string s){
  mycrc_32_type result;
  result.process_bytes(s.c_str(), s.length());
  return result.checksum();
}

string KindlePidFinder::pidFromSerial(){
  SerialChars crc_chars(4), kindleNum(PidLen);
  uint res = checksum(buffer);
  res =~ res;
  for (int i = 0; i < buffer.length(); ++i)
    kindleNum[i%PidLen] ^= buffer[i];

  crc_chars[0] = res >> 24 & 0xff;
  crc_chars[1] = res >> 16 & 0xff;
  crc_chars[2] = res >> 8 & 0xff;
  crc_chars[3] = res & 0xff;

  for (int i = 0; i < PidLen; ++i)
    kindleNum[i] ^= crc_chars[i & 3];

  for (int i = 0; i < PidLen; i++){
    uchar b = kindleNum[i] & 0xff;
    pid+=letters[(b >> 7) + ((b >> 5 & 3) ^ (b & 0x1f))];
  }
  pid += "*";
  return pid;
}

/***************************************************
 *  main(int, char **)
 *  Basic driver function for scripting the KindlePidFinder class
 *  Requires: single argument representing the Kindle's
 *            serial number. This serial number is either 
 *            printed on the back, or can be accessed 
 *            via the 411 keyboard command.
 ***************************************************/
int main(int argc, char **argv){
  if (argc != 2){
    cout << "Usage: getKindlePid \"YOURSERIALNUMHERE\"" << endl;
    exit(0);
  }

  string buffer = argv[1];
  KindlePidFinder kpf(buffer);
  string pid = kpf.pidFromSerial();
  pid = kpf.checksumPid();

  cout << pid << endl;
}

