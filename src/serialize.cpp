#include "ez/serialize.h"

#include "ez/assert.h"

#include <cstring>

// Write 32-bit unsigned integer as 64-bit little endian.
uint32_t writeUInt64(char *Buffer, uint32_t Value) {
  constexpr uint32_t Mask = 0xFF;
  Buffer[0] = static_cast<uint8_t>((Value & (Mask << 0)) >> 0);
  Buffer[1] = static_cast<uint8_t>((Value & (Mask << 8)) >> 8);
  Buffer[2] = static_cast<uint8_t>((Value & (Mask << 16)) >> 16);
  Buffer[3] = static_cast<uint8_t>((Value & (Mask << 24)) >> 24);
  Buffer[4] = 0;
  Buffer[5] = 0;
  Buffer[6] = 0;
  Buffer[7] = 0;
  return 8;
}

// Write 32-bit signed integer as 64-bit little endian.
uint32_t writeSInt64(char Buffer[], int32_t Value) {
  constexpr uint32_t Mask = 0xFF;
  Buffer[0] = static_cast<uint8_t>((Value & (Mask << 0)) >> 0);
  Buffer[1] = static_cast<uint8_t>((Value & (Mask << 8)) >> 8);
  Buffer[2] = static_cast<uint8_t>((Value & (Mask << 16)) >> 16);
  Buffer[3] = static_cast<uint8_t>((Value & (Mask << 24)) >> 24);
  Buffer[4] = 0;
  Buffer[5] = 0;
  Buffer[6] = 0;
  Buffer[7] = static_cast<uint8_t>((Value & 0x80000000ul) >> 24);
  return 8;
}

uint32_t writeBytes(char Buffer[], const void *Bytes, size_t Count) {
  memcpy(Buffer, Bytes, Count);
  return Count;
}

uint32_t writeString(char Buffer[], const char *Value) {
  uint32_t Chars = strlen(Value);
  uint32_t Written = writeUInt64(Buffer, Chars);
  memcpy(Buffer + Written, Value, Chars);
  return Written + Chars;
}

uint32_t writeBool(char Buffer[], bool Value) {
  Buffer[0] = (Value == true);
  return 1;
}

uint32_t readUInt8(const char Buffer[], uint8_t &Value) {
  Value = Buffer[0];
  return 1;
}

uint32_t readUInt16(const char Buffer[], uint16_t &Value) {
  Value = Buffer[0];
  Value |= static_cast<uint16_t>(Buffer[1]) << 8;
  return 2;
}

uint32_t readUInt32(const char Buffer[], uint32_t &Value) {
  Value = Buffer[0];
  Value |= static_cast<uint32_t>(Buffer[1]) << 8;
  Value |= static_cast<uint32_t>(Buffer[2]) << 16;
  Value |= static_cast<uint32_t>(Buffer[3]) << 24;
  return 4;
}

uint32_t readUInt64(const char Buffer[], uint64_t &Value) {
  Value = Buffer[0];
  Value |= static_cast<uint64_t>(Buffer[1]) << 8;
  Value |= static_cast<uint64_t>(Buffer[2]) << 16;
  Value |= static_cast<uint64_t>(Buffer[3]) << 24;
  Value |= static_cast<uint64_t>(Buffer[4]) << 32;
  Value |= static_cast<uint64_t>(Buffer[5]) << 40;
  Value |= static_cast<uint64_t>(Buffer[6]) << 48;
  Value |= static_cast<uint64_t>(Buffer[7]) << 56;
  return 8;
}

uint32_t readUInt64as32(const char Buffer[], uint32_t &Value) {
  Value = Buffer[0];
  Value |= static_cast<uint32_t>(Buffer[1]) << 8;
  Value |= static_cast<uint32_t>(Buffer[2]) << 16;
  Value |= static_cast<uint32_t>(Buffer[3]) << 24;
  if (Buffer[4] != 0 || Buffer[5] != 0 || Buffer[6] != 0 || Buffer[7] != 0) {
    assert(Buffer[4] == 0, "Out of bounds: expected 32-bit value");
    assert(Buffer[5] == 0, "Out of bounds: expected 32-bit value");
    assert(Buffer[6] == 0, "Out of bounds: expected 32-bit value");
    assert(Buffer[7] == 0, "Out of bounds: expected 32-bit value");
  }
  return 8;
}

uint32_t readAddr(const char Buffer[], uint32_t &Value) {
  return readUInt64as32(Buffer, Value);
}

uint32_t readSize(const char Buffer[], uint32_t &Value) {
  return readUInt64as32(Buffer, Value);
}

uint32_t readString(const char Buffer[], char *const Value, uint32_t &Size) {
  uint32_t SizeLen = readSize(Buffer, Size);
  memcpy(Value, Buffer + SizeLen, Size);
  Value[Size] = '\0';
  return SizeLen + Size;
}
