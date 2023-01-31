#ifndef EZ_DEVICE_H
#define EZ_DEVICE_H

#include <cstddef>
#include <cstdint>
#include <string>

void device_notifyBoot();
void device_notifyReady();
void device_notifyTick();
void device_notifyShutdown();

void device_setupSendReceive();
void device_sendBytes(const char *Buffer, size_t Size);
bool device_receiveBytes(char Buffer[], uint32_t Count);

void device_flushReceiveBuffer();

// Ad-hoc device interface extensions for Linux/sockets support
char *device_allocCodeBuffer(size_t Size);
void device_listenOnPort(std::string PortStr);

#endif // EZ_DEVICE_H
