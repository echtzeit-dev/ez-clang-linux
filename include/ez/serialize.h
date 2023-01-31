#ifndef EZ_SERIALIZE_H
#define EZ_SERIALIZE_H

#include <cstddef>
#include <cstdint>

uint32_t writeUInt64(char *Buffer, uint32_t Value);
uint32_t writeSInt64(char Buffer[], int32_t Value);
uint32_t writeBytes(char Buffer[], const void *Bytes, size_t Count);
uint32_t writeString(char Buffer[], const char *Value);
uint32_t writeBool(char Buffer[], bool Value);
uint32_t readUInt8(const char Buffer[], uint8_t &Value);
uint32_t readUInt16(const char Buffer[], uint16_t &Value);
uint32_t readUInt32(const char Buffer[], uint32_t &Value);
uint32_t readUInt64(const char Buffer[], uint64_t &Value);
uint32_t readUInt64as32(const char Buffer[], uint32_t &Value);
uint32_t readAddr(const char Buffer[], uint32_t &Value);
uint32_t readSize(const char Buffer[], uint32_t &Value);
uint32_t readString(const char Buffer[], char *const Value, uint32_t &Size);

#endif // EZ_SERIALIZE_H
