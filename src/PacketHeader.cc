#include "../include/PacketHeader.h"

#include <cstdint>
#include <stdexcept>

void PacketHeader::setType(uchar *buffer, uint type) {
  if (type > 3) {
    throw std::out_of_range("type value must be between 0 and 3");
  } else {
    type = static_cast<uchar>(type);
    type = type << 6;
    buffer[0] = (buffer[0] & 0x3F);
    buffer[0] = buffer[0] | type;
  }
}

void PacketHeader::setTR(uchar *buffer, bool tr) {
    if(tr)
        buffer[0] |= (1 << 5);   // Set bit 5
    else
        buffer[0] &= ~(1 << 5);  // Clear bit 5
}


void PacketHeader::setWin(uchar *buffer, uint window) {
  if (window > 31) {
    throw std::out_of_range("window value must be between 0 and 31");
  } else {
    window = static_cast<uchar>(window);
    buffer[0] = (buffer[0] & 0xE0);
    buffer[0] = buffer[0] | window;
  }
}

void PacketHeader::setSeq(uchar *buffer, uint sequence_number) {
  if (sequence_number > 255) {
    throw std::out_of_range("Sequence value must be between 0 and 31");
  } else {
    sequence_number = static_cast<uchar>(sequence_number);
    buffer[1] = sequence_number;
  }
}

void PacketHeader::setLen(uchar *buffer, uint length) {
  if (length > 65535) {
    throw std::out_of_range("Length value must be between 0 and 65535");
  }
  length = static_cast<uint16_t>(length);
  buffer[2] = (length >> 8);
  length = length & 0xFF;
  buffer[3] = length;
}

void PacketHeader::setCRC(uchar *buffer, unsigned long int crc) {
  if (crc > 4294967295) {
    throw std::out_of_range("CRC value must be between 0 and 4294967295");
  }
  buffer[11] = crc;
  buffer[10] = ((crc >> 8) & 0xFF);
  buffer[9] = ((crc >> 16) & 0xFF);
  buffer[8] = ((crc >> 24) & 0xFF);
}

uint PacketHeader::getType(const uchar *buffer) {
  return ((buffer[0] >> 6) & 0x03);
}

uint PacketHeader::getWin(const uchar *buffer) { return (buffer[0] & 0x1F); }

uint PacketHeader::getSeq(const uchar *buffer) { return buffer[1]; }

uint PacketHeader::getLength(const uchar *buffer) {
  uint length = 0;
  length = (buffer[2] << 8);
  length = length & 0xFF00;
  length = length | buffer[3];
  return length;
}

unsigned long int PacketHeader::getCRC(const uchar *buffer) {
  unsigned long int CRC = 0;
  CRC = (static_cast<unsigned long int>(buffer[8]) << 24);
  CRC = CRC & 0xFF000000;
  CRC = CRC | (static_cast<unsigned long int>(buffer[9]) << 16);
  CRC = CRC & 0xFFFF0000;
  CRC = CRC | (static_cast<unsigned long int>(buffer[10]) << 8);
  CRC = CRC & 0xFFFFFF00;
  CRC = CRC | static_cast<unsigned long int>(buffer[11]);
  return CRC;
}
