#include "kindlepid.h"
#include "mobidedrm2.h"
using namespace std;

string PC1(string key, string src, bool decryption){
  int sum1, sum2, keyXorVal;
  sum1 = sum2 = keyXorVal = 0;
  string dst;
  if (key.length()!=16){
    cout << "Bad key length!" << endl;
    return dst;
  }
  vector<uchar> wkey;
  for (int i = 0; i < 8; i++)
    wkey.push_back(key[i*2]<<8 | key[i*2+1]);

  for (int i=0; i < src.length(); i++){
    uint temp1 = 0;
    uint byteXorVal = 0;
    for (int j = 0; j < 8; j++){
      temp1 ^= wkey[j];
      sum2  = (sum2+j)*20021 + sum1;
      sum1  = (temp1*346) & 0xFFFF;
      sum2  = (sum2+sum1) & 0xFFFF;
      temp1 = (temp1*20021+1) & 0xFFFF;
      byteXorVal ^= temp1 ^ sum2;
    }
    uchar curByte = src[i];
    if(!decryption)
      keyXorVal = curByte * 257;
    curByte = ((curByte ^ (byteXorVal >> 8)) ^ byteXorVal) & 0xFF;
    if (decryption)
      keyXorVal = curByte * 257;
    for (int j = 0; j < 8; j++)
      wkey[j] ^= keyXorVal;
    dst += curByte;
  }
  return dst;
}

//checksumpid(std::string s); // included from kindlepid.h?
uint getSizeOfTrailingDataEntry(string ptr, uint size){
  uchar bitpos = 0;
  uchar result = 0;
  while(true){
    uchar v = ptr[size-1];
    result |= (v & 0x7F) << bitpos;
    bitpos += 7;
    size -= 1;
    if ((v & 0x80) != 0 or (bitpos >= 28) or (size == 0))
      return result;
  }
  return 0; //Can't get here
}

uint getSizeOfTrailingDataEntries(string ptr, uint size, ulong flags){
  uchar num = 0;
  flags >>= 1;
  while (flags){
    if (flags & 1)
      num += getSizeOfTrailingDataEntry(ptr, size - num);
    flags >>= 1;
  }
  return num;
}

uint MobiDeDrm::checksum(string s){
  mycrc_32_type result;
  result.process_bytes(s.c_str(), s.length());
  return result.checksum();
}

MobiDeDrm::checksumPid(string s){
  string letters = "ABCDEFGHIJKLMNPQRSTUVWXYZ123456789";
  uint crc = checksum(s);
  crc = crc ^ (crc >> 16);
  string res(s);
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


MobiDeDrm::MobiDeDrm(string ifname, string ofname, string kindlePid){
  mobiBookIn = new ifstream(ifname.c_str());
  if (checksumPid(kindlePid.substr(0, kindlePid.length() - 2)) != kindlePid)
    raise DrmException("invalid PID checksum");
  kindlePid = kindlePid.substr(0, kindlePid.length() - 2);
		
  mobiBookIn = new ifstream(ifname); // data_file
  mobiBookIn->seekg (0, ios::end);
  uint iflen = mobiBookIn->tellg();
  mobiBookIn->seekg(0,ios::beg);
  char * buffer = new char[iflen];
  mobiBookIn->read(buffer, iflen);
  string data_file(buffer);
  string header = data_file.substr(0, 72);
  if (header.substr(0x3C, 0x3C+8) != 'BOOKMOBI')
    raise DrmException("invalid file format");
  self.num_sections, = struct.unpack('>H', data_file[76:78]) //Unsigned short

		self.sections = []
		for i in xrange(self.num_sections):
  offset, a1,a2,a3,a4 = struct.unpack('>LBBBB', data_file[78+i*8:78+i*8+8]) //unsigned long, uchar, uchar, uchar, uchar
			flags, val = a1, a2<<16|a3<<8|a4
			self.sections.append( (offset, flags, val) )

		sect = self.loadSection(0)
			records, = struct.unpack('>H', sect[0x8:0x8+2]) //unsigned short
			extra_data_flags, = struct.unpack('>L', sect[0xF0:0xF4]) //unsigned long

			crypto_type, = struct.unpack('>H', sect[0xC:0xC+2]) //unsigned short
		if crypto_type != 2:
			raise DrmException("invalid encryption type: %d" % crypto_type)

			  // calculate the keys
		drm_ptr, drm_count, drm_size, drm_flags = struct.unpack('>LLLL', sect[0xA8:0xA8+16])
		found_key = self.parseDRM(sect[drm_ptr:drm_ptr+drm_size], drm_count, kindlePid)
		if not found_key:
			raise DrmException("no key found. maybe the PID is incorrect")

			  // kill the drm keys
		self.patchSection(0, "\0" * drm_size, drm_ptr)
			  // kill the drm pointers
		self.patchSection(0, "\xff" * 4 + "\0" * 12, 0xA8)
			  // clear the crypto type
		self.patchSection(0, "\0" * 2, 0xC)

			  // decrypt sections
		print "Decrypting. Please wait...",
		for i in xrange(1, records+1):
			data = self.loadSection(i)
			extra_size = getSizeOfTrailingDataEntries(data, len(data), extra_data_flags)
			self.patchSection(i, PC1(found_key, data[0:len(data) - extra_size]))
		print "done"
}
