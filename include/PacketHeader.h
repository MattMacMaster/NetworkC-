

#ifndef PACKETHEADER_H
#define PACKETHEADER_H
#include <cstdint>

using uchar = unsigned char;
using uint = unsigned int;

/*
class of helper functions:
set functions:
take in a buffer and a value for a specific field in the packet header,
values are checked to be within valid range.
values are placed into correct bit positions in the buffer using bitwise
operations. get functions: take in a buffer and extract the value of a specific
field in the packet header using bitwise operations to isolate the required
bits.
*/
class PacketHeader {

public:
  /*
  takes in buffer and value to set the type field in the packet header.
  value is checked to be between 0 and 3.
  value is shifted 6 bits to the left into bit position 0 and 1 of the byte.
  Bits 0,1 in the buffer are cleared using a bitwise AND with 00111111 (0x3F)
  The value is placed in the 0,1 bits in the buffer using bitwise OR
  */
  void setType(uchar *buffer, uint value);

  /*
  takes in buffer and value to set the widow field in the packet header.
  value is checked to be between 0 and 31.
  bits 3-7 are cleared using a bitwise AND with 11100000 (0xE0)
  The value is placed in bits 3-7 in the buffer using bitwise OR
  */
  void setWin(uchar *buffer, uint value);

  /*
  takes in buffer and value to set the sequence field in the packet header.
  value is checked to be between 0 and 255.
  The value is placed in in buffer[1]
  */
  void setSeq(uchar *buffer, uint value);

  /*
  takes in buffer and value to set the sequence field in the packet header.
  value is checked to be between 0 and 65535.
  value is shifted 8 bits right, placing the 8 most significant bits into the 8
  least significant bits. shufted value is placed in buffer[2] value is placed
  in buffer[3], since buffer[3] is of type char: 8 bits, only the 8 least
  significant bits are stored.
  */
  void setLen(uchar *buffer, uint value);

  //Added and testing
  void setTR(uchar *buffer, bool tr);


  /*
  takes in buffer and value to set the sequence field in the packet header.
  value is checked to be between 0 and 4294967295.
  value is shifted 24 bits, placing the 8 most significant bits into the 8 least
  significant bits. shufted value is placed in buffer[8] value is shifted 16
  bits, placing the 8-15 bits into the 8 least significant bits. shufted value
  is placed in buffer[9] value is shifted 8 bits, placing the 16-23 bits into
  the 8 least significant bits shufted value is placed in buffer[10] value is
  placed in buffer[11], since buffer[11] is of type char: 8 bits, only the 8
  least significant (24-31) bits are stored.
  */
  void setCRC(uchar *buffer, unsigned long int value);

  /*
  the value from buffer[0] os shifted 6 bits to the right, placing bits 0 and 1
  into the least significant bits a bitwise AND with 00000011 (0x03) is
  performed to clear any pissible bits in 0-6 - unncessary I wanted to make sure
  resulting value is returned

  */
  uint getType(const uchar *buffer);

  /*
  the value from buffer[0] is ANDed with 00011111 (0x1F) to clear bits 0-2 to
  only leave the window bits 3-7 resulting value is returned
  */
  uint getWin(const uchar *buffer);

  /*
  value of buffer[1] is returned unchanged, Sequence is an 8 bit value, taking
  up the whole byte, no manipulation is needed.
  */
  uint getSeq(const uchar *buffer);

  /*
  an usngined int variable length to store the 'length' value is innitialized to
  0. Length variable is set to the value of buffer[2] shifted 8 bits to the
  left, placing it into the 8 most significant bits(0-7) using OR VVariable
  Length is ANDed with 0xFF00(1111111100000000) to clear any possible bits in
  8-15 - again unncessary Value from buffer[3] is ORed with Length to place it
  into the 8 least significant bits (8-15) the resulting Length value is
  returned
  */
  uint getLength(const uchar *buffer);

  /*
  an unsigned long int variable CRC is initialized to 0, will be used to store
  the CRC value. value from buffer[8] is cast to unsigned long int and shifted
  24 bits to the left, placing it into the 8 most significant bits (0-7) using
  oR CRC variable is ANDed with 0xFF000000 to clear any possible bits in 8-31
  value from buffer[9] is cast to unsigned long int and shifted 16 bits to the
  left, placing it into bits 8-15 uing OR CRC variable is ANDed with 0xFFFF0000
  to clear any possible bits in 16-31 value from buffer[10] is cast to unsigned
  long int and shifted 8 bits to the left, placing it into bits 16-23 using OR
  CRC variable is ANDed with 0xFFFFFF00 to clear any possible bits in 24-31
  value from buffer[11] is cast to unsigned long int and placed into bits 24-31
  using OR the resulting CRC value is returned
  */
  unsigned long int getCRC(const uchar *buffer);
};

#endif // PACKETHEADER_H
