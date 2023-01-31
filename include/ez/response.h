#ifndef EZ_RESPONSE_H
#define EZ_RESPONSE_H

#include <cstddef>
#include <cstdint>

extern char *GlobalResponseBuffer;
extern size_t GlobalResponseCapacity;
extern const char *GlobalResponseBufferEnd;

char *responseAcquire(uint32_t ExpectedBytes);
char *responseFinalize(char *ResponseEnd);
const char *responseSetBuffer(char *InputEnd, size_t Capacity);
void responseClearBuffer();

const char *responseGetBuffer();
const char *responseGetLimit();

char *error(const char *Fmt, ...);
char *errorEx(char *Buffer, uint32_t Size, const char *Fmt, ...);

const char *errorGetBuffer(uint32_t &Size);
char *errorFinalize(uint32_t Length);

uint32_t formatlnBytesHex(char Buffer[], const char *Bytes,
                          size_t BytesRemaining, size_t LineMax = 0);

#endif // EZ_RESPONSE_H
