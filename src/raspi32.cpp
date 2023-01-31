#include "ez/device.h"

#include "ez/assert.h"
#include "ez/protocol.h"
#include "ez/response.h"
#include "ez/support.h"

#include <errno.h>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <iomanip>
#include <sstream>

char *device_allocCodeBuffer(size_t Size) {
  int Align = ::getpagesize();
  char *Buffer = static_cast<char *>(::memalign(Align, Size));
  assert(Buffer, "Failed to allocate memory for JITed code");

  int Success = ::mprotect(Buffer, Size, PROT_READ | PROT_WRITE | PROT_EXEC);
  assert(Success == 0, "Failed to make JITed code memory executable");

  return Buffer;
}

void device_setupSendReceive() {
  // Sockets have a defined and reliable handshake procedure built-in. We could
  // do something like authentication here, but right now this is just unused.
}

static unsigned now() {
  return (unsigned)time(nullptr);
}

static int SockFD = 0;
static unsigned timestampStartup;

void device_notifyBoot() {
  timestampStartup = now();
}

void device_notifyReady() {
  printf("Connected\n");
}

void device_notifyTick() {
  // TODO: Dump milliseconds or exact time
  printf("%u Tick\n", now() - timestampStartup);
}

bool device_aborting = false;

void device_notifyShutdown() {
  if (device_aborting) {
    // Exit after fatal error
    printf("Aborting\n");
  } else {
    // Regular exit
    char AcknowledgeDisconnect[32];
    device_receiveBytes(AcknowledgeDisconnect, 32);
    printf("Exiting\n");
  }
  close(SockFD);
}

void device_flushReceiveBuffer() {}

void device_listenOnPort(std::string PortStr) {
  if (SockFD) {
    printf("Fatal: Cannot connect more than once\n");
    exit(1);
  }

  addrinfo Hints{};
  Hints.ai_family = AF_INET;
  Hints.ai_socktype = SOCK_STREAM;
  Hints.ai_flags = AI_PASSIVE;

  addrinfo *AI;
  if (int EC = getaddrinfo(nullptr, PortStr.c_str(), &Hints, &AI)) {
    printf("Error setting up bind address: %s\n", gai_strerror(EC));
    exit(1);
  }

  // Create a socket from first addrinfo structure returned by getaddrinfo.
  int ListenFD = 0;
  if ((ListenFD = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol)) < 0) {
    printf("Error creating socket: %s\n", strerror(errno));
    exit(1);
  }

  // Avoid "Address already in use" errors.
  const int Yes = 1;
  if (setsockopt(ListenFD, SOL_SOCKET, SO_REUSEADDR, &Yes, sizeof(int)) == -1) {
    printf("Error calling setsockopt: %s\n", strerror(errno));
    exit(1);
  }

  // Bind the socket to the desired port.
  if (bind(ListenFD, AI->ai_addr, AI->ai_addrlen) < 0) {
    printf("Error on binding: %s\n", strerror(errno));
    exit(1);
  }

  // Listen for incomming connections.
  static constexpr int ConnectionQueueLen = 1;
  printf("Listening on port %s\n", PortStr.c_str());
  listen(ListenFD, ConnectionQueueLen);
  SockFD = accept(ListenFD, AI->ai_addr, &AI->ai_addrlen);
  if (SockFD == -1) {
    printf("Error on accepting: %s\n", strerror(errno));
    exit(1);
  }

  // Don't accept any further connections.
  close(ListenFD);
}

bool device_receiveBytes(char *Dst, size_t Size) {
  std::stringstream OS;
  ssize_t Completed = 0;
  while (Completed < static_cast<ssize_t>(Size)) {
    ssize_t Read = ::read(SockFD, Dst + Completed, Size - Completed);
    if (Read < 0) {
      auto ErrCode = errno;
      if (ErrCode == EAGAIN || ErrCode == EINTR)
        continue;
      // TODO: What else here?
      return false;
    }
    for (unsigned i = 0; i < Read; i += 1) {
      char Byte = Dst[Completed + i];
      OS << " " << std::hex << std::setw(2) << std::setfill('0') << (int)Byte;
    }
    Completed += Read;
  }
  if (OS.rdbuf()->in_avail())
    printf("Receive <-\n %s\n", OS.str().c_str());
  return true;
}

void device_sendBytes(const char *Src, size_t Size) {
  std::stringstream OS;
  ssize_t Completed = 0;
  while (Completed < static_cast<ssize_t>(Size)) {
    ssize_t Written = ::write(SockFD, Src + Completed, Size - Completed);
    if (Written < 0) {
      auto ErrCode = errno;
      assert(ErrCode == EAGAIN || ErrCode == EINTR, "Failed to send bytes");
      continue;
    }
    for (unsigned i = 0; i < Written; i += 1) {
      char Byte = Src[Completed + i];
      OS << " " << std::hex << std::setw(2) << std::setfill('0') << (int)Byte;
    }
    Completed += Written;
  }
  if (OS.rdbuf()->in_avail())
    printf("Send ->\n %s\n", OS.str().c_str());
}
