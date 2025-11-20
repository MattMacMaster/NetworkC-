#ifndef __HEADER_DRIVER_T_H
#define __HEADER_DRIVER_T_H

/* ***********
These are functions called by the test cases. They will not be part of your code
(but you can get inspiration from these if you want). So, you can assume that
the buffers will always have the right size. Implement them using your code.
 ************** */

// given <buffer> that holds <buf_size> bytes, set the TYPE field to the value
// <value>. <value> can be 0, 1, 2, or 3. MAke sure that no oher bits in the
// buffer are changed
void setType_t(unsigned char *buffer, int buf_size, unsigned int value);

// returns the value of field Window from <buffer> which has size <buf_size>
// bytes. This value can only be 0, 1, 2, or 3.
unsigned int getType_t(unsigned char *buffer, int buf_size);

// given <buffer> that holds <buf_size> bytes, set the Window field to the value
// <value>. <value> can be 0-31 MAke sure that no oher bits in the buffer are
// changed
void setWin_t(unsigned char *buffer, int buf_size, unsigned int value);

// returns the value of field Window from <buffer> which has size <buf_size>
// bytes. This value can only be 0-31
unsigned int getWin_t(unsigned char *buffer, int buf_size);

// given <buffer> that holds <buf_size> bytes, set the Seq num field to the
// value <value>. <value> can be 0-255 MAke sure that no oher bits in the buffer
// are changed
void setSeq_t(unsigned char *buffer, int buf_size, unsigned int value);

// returns the value of field Seq from <buffer> which has size <buf_size> bytes.
// This value can only be 0-255
unsigned int getSeq_t(unsigned char *buffer, int buf_size);

// given <buffer> that holds <buf_size> bytes, set the Length field to the value
// <value>. <value> can be 0-65535 MAke sure that no oher bits in the buffer are
// changed
void setLen_t(unsigned char *buffer, int buf_size, unsigned int value);

// returns the value of field Length from <buffer> which has size <buf_size>
// bytes. This value can only be 0-65535
unsigned int getLen_t(unsigned char *buffer, int buf_size);

// given <buffer> that holds <buf_size> bytes, set the CRC1 field to the value
// <value>. <value> can be 0 .. 2^32-1=4294967295 MAke sure that no oher bits in
// the buffer are changed
void setCRC1_t(unsigned char *buffer, int buf_size, unsigned long int value);

// returns the value of field CRC1 from <buffer> which has size <buf_size>
// bytes. This value can only be 0 .. 4294967295
unsigned long int getCRC1_t(unsigned char *buffer, int buf_size);

#endif
