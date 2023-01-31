#include "ez/assert.h"

#include "ez/response.h"

jmp_buf GlobalAssertionFailureReturnPoint;
char *GlobalAssertionFailureBuffer;
uint32_t GlobalAssertionFailureBufferSize;

#if !defined(NDEBUG)

// We will send back a hangup message and pass this in the payload
EZ_NORETURN void fail(const char *Description) {
  errorEx(GlobalAssertionFailureBuffer, GlobalAssertionFailureBufferSize,
          "Assertion failed: %s", Description);
  longjmp(GlobalAssertionFailureReturnPoint, 1); // TODO: Allow custom error codes
  EZ_BUILTIN_UNREACHABLE;
}

void assert(bool Premise, const char *Description) {
  if (!Premise)
    fail(Description);
}

#endif
