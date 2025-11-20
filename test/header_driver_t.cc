// add include files for your classes and components

// the test driver function prototypes
#include "header_driver_t.h"
#include "../include/PacketHeader.h"

// Implement the test driver functions so that all tests pass.

// given <buffer> that holds <buf_size> bytes, set the TYPE field to the value
// <value>. <value> can be 0, 1, 2, or 3. MAke sure that no oher bits in the
// buffer are changed
void setType_t(unsigned char *buffer, int buf_size, unsigned int value) {
  PacketHeader *packet_header = new PacketHeader();
  packet_header->setType(buffer, value);
}

// returns the value of field Window from <buffer> which has size <buf_size>
// bytes. This value can only be 0, 1, 2, or 3.
unsigned int getType_t(unsigned char *buffer, int buf_size) {
  PacketHeader *packet_header = new PacketHeader();
  return packet_header->getType(buffer);
}

// given <buffer> that holds <buf_size> bytes, set the Window field to the value
// <value>. <value> can be 0-31 MAke sure that no oher bits in the buffer are
// changed
void setWin_t(unsigned char *buffer, int buf_size, unsigned int value) {
  PacketHeader *packet_header = new PacketHeader();
  packet_header->setWin(buffer, value);
}

// returns the value of field Window from <buffer> which has size <buf_size>
// bytes. This value can only be 0-31
unsigned int getWin_t(unsigned char *buffer, int buf_size) {
  PacketHeader *packet_header = new PacketHeader();
  return packet_header->getWin(buffer);
}

// given <buffer> that holds <buf_size> bytes, set the Seq num field to the
// value <value>. <value> can be 0-255 MAke sure that no oher bits in the buffer
// are changed
void setSeq_t(unsigned char *buffer, int buf_size, unsigned int value) {
  PacketHeader *packet_header = new PacketHeader();
  packet_header->setSeq(buffer, value);
}

// returns the value of field Seq from <buffer> which has size <buf_size> bytes.
// This value can only be 0-255
unsigned int getSeq_t(unsigned char *buffer, int buf_size) {
  PacketHeader *packet_header = new PacketHeader();
  return packet_header->getSeq(buffer);
}

// given <buffer> that holds <buf_size> bytes, set the Length field to the value
// <value>. <value> can be 0-65535 MAke sure that no oher bits in the buffer are
// changed
void setLen_t(unsigned char *buffer, int buf_size, unsigned int value) {
  PacketHeader *packet_header = new PacketHeader();

  packet_header->setLen(buffer, value);
}

// returns the value of field Length from <buffer> which has size <buf_size>
// bytes. This value can only be 0-65535
unsigned int getLen_t(unsigned char *buffer, int buf_size) {
  PacketHeader *packet_header = new PacketHeader();
  return packet_header->getLength(buffer);
}

// given <buffer> that holds <buf_size> bytes, set the CRC1 field to the value
// <value>. <value> can be 0 .. 2^32-1=4294967295 MAke sure that no oher bits in
// the buffer are changed
void setCRC1_t(unsigned char *buffer, int buf_size, unsigned long int value) {
  PacketHeader *packet_header = new PacketHeader();
  packet_header->setCRC(buffer, value);
}

// returns the value of field CRC1 from <buffer> which has size <buf_size>
// bytes. This value can only be 0 .. 4294967295
unsigned long int getCRC1_t(unsigned char *buffer, int buf_size) {
  PacketHeader *packet_header = new PacketHeader();
  return packet_header->getCRC(buffer);
}
