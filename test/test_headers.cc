#include "header_driver_t.h"
#include <gtest/gtest.h>

// Define a test fixture class
class HeadersTest : public ::testing::Test {
protected:
  // we only test the header, and that is 12 bytes long
  static const int SIZE = 12;
  unsigned char buffer[SIZE];

  void SetUp() override {
    // Initialize buffer with zeros before each test
    for (int i = 0; i < SIZE; ++i) {
      buffer[i] = 0;
    }
  }

  void TearDown() override {
    // no cleanup needed
  }
};

// The tests using the fixture
// sets type correctly
TEST_F(HeadersTest, SetsType) {
  setType_t(buffer, SIZE, 1);
  EXPECT_EQ(buffer[0], 0x40);
}

// make sure other bits in the buffer are not affected
TEST_F(HeadersTest, SetsTypeOnly) {
  buffer[0] = 0xeb;
  setType_t(buffer, SIZE, 2);
  EXPECT_EQ(buffer[0], 0xab);
}

// tests reading the type
TEST_F(HeadersTest, GetsType) {
  buffer[0] = 0xd9;
  EXPECT_EQ(getType_t(buffer, SIZE), 3);
}

// test setting window
TEST_F(HeadersTest, SetsWindow) {
  buffer[0] = 0xc2;
  setWin_t(buffer, SIZE, 28);
  EXPECT_EQ(buffer[0], 0xdc);
}

// test getting the window
TEST_F(HeadersTest, GetsWindow) {
  buffer[0] = 0xd3;
  EXPECT_EQ(getWin_t(buffer, SIZE), 19);
}

// test setting seq
TEST_F(HeadersTest, SetsSeq) {
  buffer[0] = 0xc2;
  buffer[2] = 0xfc;
  setSeq_t(buffer, SIZE, 113);
  EXPECT_EQ(buffer[0], 0xc2);
  EXPECT_EQ(buffer[2], 0xfc);
  EXPECT_EQ(buffer[1], 113);
}

// test getting the seq
TEST_F(HeadersTest, GetsSeq) {
  buffer[1] = 99;
  EXPECT_EQ(getSeq_t(buffer, SIZE), 99);
}

// test setting length (16 bit)
TEST_F(HeadersTest, SetsLen) {
  buffer[1] = buffer[2] = buffer[3] = 0xff;
  setLen_t(buffer, SIZE, 0x43a9);
  EXPECT_EQ(buffer[1], 0xff);
  EXPECT_EQ(buffer[2], 0x43);
  EXPECT_EQ(buffer[3], 0xa9);
  EXPECT_EQ(buffer[4], 0);
}

// test getting the length (16 bit)
TEST_F(HeadersTest, GetsLen) {
  buffer[1] = 99;
  buffer[4] = 77;
  buffer[2] = 0x7b;
  buffer[3] = 0xd6;
  EXPECT_EQ(getLen_t(buffer, SIZE), 0x7bd6);
}

// test setting CRC1 (32 bit)
TEST_F(HeadersTest, SetsCRC1) {
  buffer[8] = buffer[7] = buffer[6] = 0xaa;
  setCRC1_t(buffer, SIZE, 0x72ab3df8);
  EXPECT_EQ(buffer[8], 0x72);
  EXPECT_EQ(buffer[9], 0xab);
  EXPECT_EQ(buffer[10], 0x3d);
  EXPECT_EQ(buffer[11], 0xf8);
}

// test getting CRC1 (32 bit)
TEST_F(HeadersTest, GetsCRC1) {
  buffer[8] = 0x5c;
  buffer[9] = 0xa7;
  buffer[10] = 0x61;
  buffer[11] = 0x9b;
  EXPECT_EQ(getCRC1_t(buffer, SIZE), 0x5ca7619b);
}

// #############################
// Custom Tests
// #############################

// Checking if type setter will fail if the buffer input is too big
TEST_F(HeadersTest, SetTypeError) {
  EXPECT_ANY_THROW(setType_t(buffer, SIZE, 4));
}

// Checking if type setter will fail if the buffer input is too big
TEST_F(HeadersTest, SetWinError) {
  EXPECT_ANY_THROW(setWin_t(buffer, SIZE, 0x4a));
}

// Checking if program will die if invalid param is passed:
TEST_F(HeadersTest, SetSeqError) {
  EXPECT_ANY_THROW(setSeq_t(buffer, SIZE, 0xFFF));
}

// Checking that program does not accept negative numbers:
// Checking if program will die if invalid param is passed:
TEST_F(HeadersTest, SetLenError) {
  EXPECT_ANY_THROW(setLen_t(buffer, SIZE, 0xFFFFFF));
}

// Checking if program will die if invalid param is passed:
TEST_F(HeadersTest, SetCRC1Error) {
  EXPECT_ANY_THROW(setCRC1_t(buffer, SIZE, 0xFFFFFFFFF));
}
