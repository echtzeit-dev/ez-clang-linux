#ifndef EZ_SYMBOLS_H
#define EZ_SYMBOLS_H

#include <cstdint>

struct Symbol {
  const char *Name;
  uint32_t Addr;
  operator bool() const { return Name != nullptr && Addr != 0; }
};

uint32_t getBootstrapSymbols(const Symbol *BootstrapSyms[]);

uint32_t lookupBuiltinSymbol(const char *Data, uint32_t Length);
uint32_t lookupSymbol(const char *Data, uint32_t Length);

#endif // EZ_SYMBOLS_H
