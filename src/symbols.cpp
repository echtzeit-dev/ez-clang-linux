#include "ez/symbols.h"

#include "ez/abi.h"
#include "ez/assert.h"
#include "ez/support.h"

#include <cstring>
#include <string>

#include <dlfcn.h>

#define STRINGIFY(NAME) #NAME
#define X(NAME) { STRINGIFY(NAME), ptr2addr((void *)&NAME) }

uint32_t getBootstrapSymbols(const Symbol *BootstrapSyms[]) {
  static const Symbol BootstrapSym = X(__ez_clang_rpc_lookup);
  *BootstrapSyms = &BootstrapSym;
  return 1;
}

uint32_t lookupBuiltinSymbol(const char *Data, uint32_t Length) {
  if (uint32_t Addr = lookupSymbol(Data, Length))
    return Addr;

  // Verbose error output: lookup failures for reserved names are fatal
  static constexpr const char *PrefixRPC = "__ez_clang_rpc_";
  if (memcmp(PrefixRPC, Data, strlen(PrefixRPC)) == 0)
    fail("Builtin RPC endpoint not found");

  static constexpr const char *PrefixRuntime = "__ez_clang_";
  if (memcmp(PrefixRuntime, Data, strlen(PrefixRuntime)) == 0)
    fail("Builtin runtime function not found");
  
  return 0;
}

//
// Linking with -rdynamic allows us to locate symbols in our own process by name
// with dlsym() as if it was a shared library.
// 
static bool DidDlOpenOwnProcess = false;

uint32_t lookupSymbol(const char *Data, uint32_t Length) {
  if (!DidDlOpenOwnProcess) {
    // We never dlclose() it, but that should be ok
    void *Handle = ::dlopen(nullptr, RTLD_LAZY | RTLD_GLOBAL);
    assert(*static_cast<int *>(Handle) == 0, "Unexpected dlsym() handle");
    DidDlOpenOwnProcess = true;
  }

  std::string NullTerminated(Data, Length);
  return ptr2addr(::dlsym(nullptr, NullTerminated.c_str()));
}
