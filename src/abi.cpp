#include "ez/abi.h"

#include "ez/assert.h"
#include "ez/response.h"
#include "ez/protocol.h"
#include "ez/serialize.h"
#include "ez/support.h"
#include "ez/symbols.h"

#include <cinttypes>
#include <cstdint>
#include <cstring>

extern "C" {

char *__ez_clang_rpc_lookup(const char *Data, size_t Size) {
  const char *DataBegin = Data;

  uint32_t SymbolsRemaining;
  Data += readSize(Data, SymbolsRemaining);

  char *Response = responseAcquire(1 + 8 + SymbolsRemaining * 8);
  Response += writeBool(Response, false); // HasError
  Response += writeUInt64(Response, SymbolsRemaining);

  while (SymbolsRemaining > 0) {
    uint32_t Length;
    Data += readSize(Data, Length);
    if (uint32_t Addr = lookupBuiltinSymbol(Data, Length)) {
      Response += writeUInt64(Response, Addr);
    } else if (uint32_t Addr = lookupSymbol(Data, Length)) {
      Response += writeUInt64(Response, Addr);
    } else {
      Response += writeUInt64(Response, 0);
    }
    Data += Length;
    SymbolsRemaining -= 1;
  }

  assert(Data == DataBegin + Size, "Invalid input length");
  return responseFinalize(Response);
}

char *__ez_clang_rpc_commit(const char *Data, size_t Size) {
  const char *DataBegin = Data;

  uint32_t SegmentsRemaining;
  Data += readSize(Data, SegmentsRemaining);
  while (SegmentsRemaining > 0) {
    uint32_t TargetAddr;
    Data += readAddr(Data, TargetAddr);
    uint32_t SegmentSize;
    Data += readSize(Data, SegmentSize);
    uint32_t ContentSize;
    Data += readSize(Data, ContentSize);
    memcpy(addr2ptr(TargetAddr), Data, ContentSize);
    memset(addr2ptr(TargetAddr + ContentSize), 0,
           SegmentSize - ContentSize);
    Data += ContentSize;
    SegmentsRemaining -= 1;
  }

  assert(Data == DataBegin + Size, "Invalid input length");

  char *Resp = responseAcquire(1);
  Resp += writeBool(Resp, false); // HasError
  return responseFinalize(Resp);
}

const char *InlineHeapPtr = nullptr;
const char *InlineHeapEnd = nullptr;

char *__ez_clang_rpc_execute(const char *Data, size_t Size) {
  assert(Size == 8, "Invalid input length");

  uint32_t FnAddr;
  Data += readAddr(Data, FnAddr);
  if ((FnAddr & 0x1) != 0x1)
    return error("Attempted to call non-Thumb function @ 0x%08" PRIx32, FnAddr);

  typedef void ClingFn_t(void *);
  ClingFn_t *Fn = (ClingFn_t *)((uintptr_t)FnAddr);

  // Acquire response memory so we can provide the remaining MessageBuffer space
  // as an inline-heap for the function; it's accessible from JITed code via
  // __ez_clang_inline_heap_acquire()
  constexpr uint32_t ResponseSize = 1;
  char *Resp = responseAcquire(ResponseSize);
  InlineHeapPtr = align_ptr<32>(Resp + ResponseSize);
  InlineHeapEnd = align_ptr<32>(responseGetLimit());

  // FIXME: Drop the unused parameter in generated wrappers
  uint64_t Unused = 0;
  Fn((void *)&Unused);

  InlineHeapPtr = nullptr;
  InlineHeapEnd = nullptr;
  Resp += writeBool(Resp, false); // HasError
  return responseFinalize(Resp);
}

char *__ez_clang_rpc_mem_read_cstring(const char *Data, size_t Size) {
  assert(Size == 8, "Invalid input length");

  uint32_t Addr;
  readAddr(Data, Addr);
  const char *Str = addr2ptr(Addr);
  char *Resp = responseAcquire(8 + strlen(Str));
  Resp += writeString(Resp, Str);
  return responseFinalize(Resp);
}

void __ez_clang_report_value(uint32_t SeqID, const char *Blob, size_t Size) {
  // The host uses this function to print expression values. It knows the type
  // of the data in this blob.
  sendMessage(ReportValue, SeqID, Blob, Size);
}

void __ez_clang_report_string(const char *Data, size_t Size) {
  constexpr uint32_t NoSequenceNumber = 0;
  sendMessage(ReportString, NoSequenceNumber, Data, Size);
}

char *__ez_clang_inline_heap_acquire(size_t Bytes) {
  char *Block = const_cast<char *>(InlineHeapPtr);
  InlineHeapPtr = align_ptr<32>(InlineHeapPtr + Bytes);
  return Block;
}

} // extern "C"
