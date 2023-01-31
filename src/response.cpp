#include "ez/response.h"

#include "ez/assert.h"
#include "ez/serialize.h"
#include "ez/support.h"

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>

char *ResponsePtr = nullptr;
char *ResponseBuffer = nullptr;
const char *ResponseLimit = nullptr;

void responseClearBuffer() {
  ResponsePtr = nullptr;
  ResponseBuffer = nullptr;
  ResponseLimit = nullptr;
}

const char *responseSetBuffer(char *Buffer, size_t Capacity) {
  assert(ResponseBuffer == nullptr && ResponsePtr == nullptr &&
         ResponseLimit == nullptr, "Response not cleared?");
  ResponseBuffer = align_ptr<64>(Buffer);
  uint32_t Padding = ResponseBuffer - Buffer;
  memset(Buffer, 0, Padding);
  ResponsePtr = ResponseBuffer;
  ResponseLimit = Buffer + Capacity;
  return ResponseBuffer;
}

char *responseAcquire(uint32_t ExpectedBytes) {
  assert(ResponsePtr != nullptr, "Response buffer not set?");
  char *Trange = ResponsePtr;
  ResponsePtr += ExpectedBytes;  // Advance to next tranche
  assert(ResponsePtr <= ResponseLimit, "Response buffer out-of-memory");
  return Trange;
}

char *responseFinalize(char *ResponseEnd) {
  assert(ResponseEnd <= ResponsePtr, "Missed to acquire response memory?");
  return ResponseEnd;
}

const char *responseGetBuffer() {
  assert(ResponseBuffer != nullptr, "Response buffer not set?");
  return ResponseBuffer;
}

const char *responseGetLimit() {
  assert(ResponseLimit != nullptr, "Response Limit not set?");
  return ResponseLimit;
}

// Error format: error code, message length, message
static char *errorAllocate(char *Buffer) {
  ResponsePtr = Buffer;
  Buffer += writeBool(Buffer, true); // HasError
  Buffer += writeUInt64(Buffer, 0); // Fill in length on finalize
  return Buffer;
}

char *errorFinalize(uint32_t Length) {
  char *Err = ResponsePtr + 1;
  Err += writeUInt64(Err, Length);
  return Err + Length; // Not including terminating null
}

const char *errorGetBuffer(uint32_t &Size) {
  readSize(ResponsePtr + 1, Size);
  Size += 1 + 8; // Error status + message length field
  return ResponsePtr;
}

char *error(const char *Fmt, ...) {
  assert(ResponseBuffer != nullptr, "Set to end of input buffer first");
  va_list Args;
  va_start(Args, Fmt);
  uint32_t Size = ResponseLimit - ResponseBuffer;
  uint32_t Length = vsnprintf(errorAllocate(ResponseBuffer), Size, Fmt, Args);
  va_end(Args);
  return errorFinalize(Length);
}

char *errorEx(char *Buffer, uint32_t Size, const char *Fmt, ...) {
  va_list Args;
  va_start(Args, Fmt);
  uint32_t Length = vsnprintf(errorAllocate(Buffer), Size, Fmt, Args);
  va_end(Args);
  return errorFinalize(Length);
}

uint32_t formatlnBytesHex(char Buffer[], const char *Bytes,
                          size_t BytesRemaining, size_t LineMax) {
  if (BytesRemaining <= 0)
    return 0;

  char *Out = Buffer;
  constexpr const char *HexDigits = "0123456789ABCDEF";
  size_t iMax = BytesRemaining;
  if (LineMax > 0 && LineMax < BytesRemaining)
    iMax = LineMax;

  for (size_t i = 0; i < iMax; i += 1) {
    Out[0] = HexDigits[Bytes[i] >> 4];    // lo hex
    Out[1] = HexDigits[Bytes[i] & 0x0F];  // hi hex
    Out[2] = ' ';
    Out += 3;
  }

  Out[0] = '\n';
  Out[1] = '\0';
  return Out - Buffer + 1; // Not including terminating zero
}
