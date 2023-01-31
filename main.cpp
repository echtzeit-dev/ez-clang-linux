#include "ez/assert.h"
#include "ez/device.h"
#include "ez/response.h"
#include "ez/protocol.h"
#include "ez/serialize.h"
#include "ez/support.h"
#include "ez/symbols.h"

#include <csetjmp>
#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <unistd.h>

#define EZ_CLANG_PROTOCOL_VERSION_STR "0.0.6"

//
// Static buffer for RPC requests and responses
//
char MessageBuffer[0x2000] __attribute__((aligned(0x100)));

//
// Buffer for JITed code
//
extern char *CodeBuffer;
extern size_t CodeBufferSize;

void ez_clang_setup() {
  device_setupSendReceive();

  SetupInfo Info;
  Info.Version = EZ_CLANG_PROTOCOL_VERSION_STR;
  Info.CodeBufferSize = 8 * ::getpagesize(); // 8 pages are a lot
  Info.CodeBuffer = device_allocCodeBuffer(Info.CodeBufferSize);
  Info.NumSymbols = getBootstrapSymbols(&Info.Symbols);
  sendSetupMessage(MessageBuffer, Info);
}

bool ez_clang_tick(uint8_t &ErrCode) {
  // Reserve the entire MessageBuffer for input.
  responseClearBuffer();

  // Explicit errors during message processing can be recoverable. We send
  // back an error response and wait for the next message.
  HeaderInfo Msg;
  if (!receiveMessage(MessageBuffer, c_array_size(MessageBuffer), Msg)) {
    uint32_t Size;
    const char *ErrResp = errorGetBuffer(Size);
    sendMessage(Result, Msg.SeqID, ErrResp, Size);
    return true;
  }

  // Shutdown and confirm with Hangup message
  if (Msg.OpCode == Hangup) {
    ErrCode = 0;
    return false;
  }

  // Define the ResponseBuffer in direct succession to the input message.
  char *InputEnd = MessageBuffer + Msg.PayloadBytes;
  size_t RemainingCapacity = c_array_size(MessageBuffer) - Msg.PayloadBytes;
  const char *RespBegin = responseSetBuffer(InputEnd, RemainingCapacity);

  // Invoke the handler for the requested endpoint. Handlers can use the error()
  // function to write error responses.
  const char *RespEnd = Msg.Handler(MessageBuffer, Msg.PayloadBytes);

  // Send the response back to the host and finish this tick.
  sendMessage(Result, Msg.SeqID, RespBegin, RespEnd - RespBegin);
  return true;
}

extern bool device_aborting;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  }

  device_notifyBoot();

  GlobalAssertionFailureBuffer = MessageBuffer;
  GlobalAssertionFailureBufferSize = c_array_size(MessageBuffer);

  // If an assertion fails, we longjmp back here and get a non-zero error code
  int EC = setjmp(GlobalAssertionFailureReturnPoint);
  uint8_t ErrCode = EC; // EC >= 0 && EC <= 0x7f

  // If we didn't get here by longjmp, then this is a restart
  if (ErrCode == 0) {
    device_listenOnPort(argv[1]);
    device_notifyReady();
    ez_clang_setup();
    while (ez_clang_tick(ErrCode))
      device_notifyTick();
  }

  // After fatal errors, don't wait for host to acknowlegde disconnect
  device_aborting = (ErrCode != 0);

  sendHangupMessage(ErrCode);
  device_notifyShutdown();
  return EC;
}
