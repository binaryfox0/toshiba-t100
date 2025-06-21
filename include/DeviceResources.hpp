#ifndef DEVICE_RESOURCES_HPP
#define DEVICE_RESOURCES_HPP

#include <stdint.h>

#include <unordered_map>
#include <atomic>
#include <thread>

#include "z80_cpp.h"

class DeviceResources
{
public:
    DeviceResources() = delete;

    static uint8_t RAM[0x10000];
    static uint8_t ROM[0x8000];
    static bool    ROMActive;
    static z80     CPU;
    static std::atomic<bool> CPUPause;
    static std::atomic<bool> CPUExit;
    static std::thread CPUThread;

    static std::unordered_map<uint16_t, bool> CPUBreak;
    static void (*BreakHandle)(uint16_t);

    static void LoadDiskBasic(const char* disk_path);

    static void FreeResources();

    static void ResetCPU();

private:
    static void     CPUExecutionLoop();
    static uint8_t  CPUReadByte(void* context, uint16_t address);
    static void    CPUWriteByte(void* context, uint16_t address, uint8_t value);
    static uint8_t        CPUIn(z80* cpu, uint8_t port);
    static void          CPUOut(z80* cpu, uint8_t port, uint8_t value);
};

#endif