#include "ez/protocol.h"

#include "ez/assert.h"
#include "ez/response.h"
#include "ez/serialize.h"
#include "ez/support.h"
#include "ez/device.h"

#include <cstring>

constexpr uint32_t MessageHeaderSize = 32;

void sendMessage(EPCOpCode OpC, uint32_t SeqNo, const char Payload[],
                 uint32_t PayloadSize) {
  char HeaderBuffer[MessageHeaderSize];
  char *Data = HeaderBuffer;
  Data += writeUInt64(Data, PayloadSize + MessageHeaderSize);
  Data += writeUInt64(Data, OpC);
  Data += writeUInt64(Data, SeqNo);
  Data += writeUInt64(Data, 0);
  device_sendBytes(HeaderBuffer, MessageHeaderSize);
  device_sendBytes(Payload, PayloadSize);
}

void sendSetupMessage(char Buffer[], SetupInfo Info) {
  char *Data = Buffer;
  Data += writeString(Data, Info.Version);
  Data += writeUInt64(Data, ptr2addr(Info.CodeBuffer));
  Data += writeUInt64(Data, Info.CodeBufferSize);
  Data += writeUInt64(Data, Info.NumSymbols);

  for (size_t i = 0; i < Info.NumSymbols; i++) {
    Data += writeString(Data, Info.Symbols[i].Name);
    Data += writeUInt64(Data, Info.Symbols[i].Addr);
  }

  sendMessage(Setup, 0, Buffer, Data - Buffer);
}

void sendHangupMessage(uint8_t ErrCode) {
  if (ErrCode == 0) {
    char SuccessByte = 0;
    sendMessage(Hangup, 0, &SuccessByte, sizeof(SuccessByte));
  } else {
    uint32_t Size;
    const char *Buffer = errorGetBuffer(Size);
    sendMessage(Hangup, 0, Buffer, Size);
  }
}

bool receiveMessage(char Buffer[], uint32_t BufferSize, HeaderInfo &Msg) {
  if (!device_receiveBytes(Buffer, MessageHeaderSize))
    fail("Error receiving message header. Shutting down.");

  // All header fields are 64-bit
  uint64_t Bytes;
  uint64_t OpCode;
  uint64_t SeqID;
  uint64_t TagAddr;

  // Parse them all, so we can re-use the buffer for the payload
  const char *Data = Buffer;
  Data += readUInt64(Data, Bytes);
  Data += readUInt64(Data, OpCode);
  Data += readUInt64(Data, SeqID);
  Data += readUInt64(Data, TagAddr);

  // If any of these values exceeds the 32-bit range, we got a malformed header
  if (Bytes >= 0x100000000 || OpCode >= 0x100000000 ||
      SeqID >= 0x100000000 || TagAddr >= 0x100000000) {
    // Each hex line shows 8 Bytes (16 chars) + 8 spaces + newline + zero
    char BytesHex[26];
    char OpCodeHex[26];
    char SeqIDHex[26];
    char TagAddrHex[26];
    formatlnBytesHex(BytesHex, Buffer, 8);
    formatlnBytesHex(OpCodeHex, Buffer + 8, 8);
    formatlnBytesHex(SeqIDHex, Buffer + 16, 8);
    formatlnBytesHex(TagAddrHex, Buffer + 24, 8);
    errorEx(Buffer, BufferSize,
            "Message header invalid: All values must be in 32-bit range. "
            "Received bytes interpretation:\n"
            "  Length:  %s"
            "  Opcode:  %s"
            "  SeqID:   %s"
            "  TagAddr: %s"
            , BytesHex, OpCodeHex, SeqIDHex, TagAddrHex);
    device_flushReceiveBuffer();
    return false;
  }

  // Check that payload contents fits the buffer
  Bytes -= MessageHeaderSize;
  if (Bytes > BufferSize) {
    errorEx(Buffer, BufferSize,
            "Message payload (%lu bytes) exceeds buffer size (%lu bytes)",
            Bytes, BufferSize);
    device_flushReceiveBuffer();
    return false;
  }

  // Validate Opcode
  if (OpCode != Call && OpCode != Hangup) {
    device_receiveBytes(Buffer, Bytes);
    errorEx(Buffer, BufferSize, "Received unexpected message op-code: %lu",
            OpCode);
    return false;
  }

  // TODO: At some point, allow to validate endpoint and function addresses!
  if (OpCode == Call)
    Msg.Handler = reinterpret_cast<RPCEndpoint *>(addr2ptr(TagAddr));

  Msg.PayloadBytes = Bytes;
  Msg.OpCode = OpCode;
  Msg.SeqID = SeqID;

  return device_receiveBytes(Buffer, Bytes);
}
