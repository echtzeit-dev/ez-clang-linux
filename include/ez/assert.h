#ifndef EZ_ASSERT_H
#define EZ_ASSERT_H

#include "support.h"

#include <cstdint>
#include <csetjmp>

#if defined(assert)
  static_assert(false, "Internal assert() clashes with existing definition");
#endif

#if !defined(NDEBUG) // && defined(DEBUG)

void assert(bool Premise, const char *Description);
EZ_NORETURN void fail(const char *Description);

#else
# define assert(Premise, Description)
# define fail(Description)
#endif

extern jmp_buf GlobalAssertionFailureReturnPoint;
extern char *GlobalAssertionFailureBuffer;
extern uint32_t GlobalAssertionFailureBufferSize;

#endif // EZ_ASSERT_H
