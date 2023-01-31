#ifndef EZ_ABI_H
#define EZ_ABI_H

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define EZ_CLANG_RPC_ENDPOINT(Name) char *Name(const char *Data, size_t Size)

// RPC endpoints:
EZ_CLANG_RPC_ENDPOINT(__ez_clang_rpc_lookup);
EZ_CLANG_RPC_ENDPOINT(__ez_clang_rpc_commit);
EZ_CLANG_RPC_ENDPOINT(__ez_clang_rpc_execute);
EZ_CLANG_RPC_ENDPOINT(__ez_clang_rpc_mem_read_cstring);

#undef EZ_CLANG_RPC_ENDPOINT

char *__ez_clang_inline_heap_acquire(size_t Bytes);
void __ez_clang_report_string(const char *Data, size_t Size);
void __ez_clang_report_value(uint32_t SeqID, const char *Blob, size_t Size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // EZ_ABI_H
