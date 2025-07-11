#include "DeviceResources.hpp"

#include <cstdint>
#include <string.h>

#include <fstream>
#include <thread>
#include <unordered_map>
#include <chrono>


#include "Internal.h"
#include "EventLog.hpp"

#include "DisassemblerView/Main.hpp"
#include "MessageBox.hpp"

// Devices
#include "VirtualFDC.hpp"

#include "z80.h"

#define DISK_TRACK_SIZE  0x1000

#define IPL_FILE_OFFSET  DISK_TRACK_SIZE
#define IPL_LOAD_ADDRESS 0xD000
#define IPL_SIZE         DISK_TRACK_SIZE * 8 - DISK_TRACK_SIZE

bool DeviceResources::MachineStarted = false;

uint8_t DeviceResources::RAM[0x10000] = {0};
uint8_t DeviceResources::ROM[0x8000]  = {0};
bool    DeviceResources::ROMActive    = true;

z80     DeviceResources::CPU          = {};
std::thread DeviceResources::CPUThread;
std::unordered_map<uint16_t, bool> DeviceResources::CPUBreak = {};
void (*DeviceResources::BreakHandle)(uint16_t) = nullptr;
std::atomic<bool> DeviceResources::CPUPause     = true;
std::atomic<bool> DeviceResources::CPUExit      = false;

char* DeviceResources::PrevDskPath = 0;

void DeviceResources::LoadDiskBasic(const char* disk_path)
{
    // Load IPL
    std::ifstream file(disk_path, std::ios::binary);
    if(!file.is_open()) {
        return;
    }
    if(PrevDskPath) {
        if(strcmp(PrevDskPath, disk_path)) {
            free(PrevDskPath);
            PrevDskPath = strdup(disk_path);
        }
    } else {
        PrevDskPath = strdup(disk_path);
    }
    for(int i = 0; i < DISK_TRACK_SIZE; i++) {
        uint8_t byte = 0;
        file.read((char*)&byte, 1);
        if(byte != 0) {
            CreateMessageBox("Error!", "Invalid TDISKBASIC disk");
            return;
        }
    }
    // Reset
    memset(RAM, 0, sizeof(RAM));
    memset(ROM, 0, sizeof(ROM));
    ROMActive = true;
    // file.seekg(IPL_FILE_OFFSET, std::ios::beg);
    file.read(reinterpret_cast<char*>(RAM + IPL_LOAD_ADDRESS), IPL_SIZE);
    file.close();
    
    StopCPUThread();
    CPUExit = false;
    ResetCPU();
    CPUThread = std::thread(CPUExecutionLoop);

    // Processing
    DisassemblerView::Init(RAM, sizeof(RAM), IPL_LOAD_ADDRESS);
    eventlog_show = true;
}

void DeviceResources::StopCPUThread() {
    CPUPause = true;
    CPUExit = true;
    if(CPUThread.joinable())
        CPUThread.join();    
}

void DeviceResources::FreeResources() {
    StopCPUThread();
}

void DeviceResources::ResetCPU() {
    CPUPause       = true;
    CPUExit        = false;
    z80_init(&CPU);
    CPU.read_byte  = CPUReadByte;
    CPU.write_byte = CPUWriteByte;
    CPU.port_in    = CPUIn;
    CPU.port_out   = CPUOut;
    CPU.pc         = IPL_LOAD_ADDRESS;

}

void DeviceResources::CPUExecutionLoop()
{
    static constexpr unsigned long TARGET_Z80_HZ    = 4'000'000; // 4MHz
    static constexpr unsigned long TARGET_FPS       = 60;
    static constexpr unsigned long CYCLES_PER_FRAME = TARGET_Z80_HZ / TARGET_FPS;
    using namespace std::chrono_literals;
    using Clock = std::chrono::high_resolution_clock;
    while(true)
    {
        if(CPUPause)
        {
            if(CPUExit)
                break;
            std::this_thread::sleep_for(1s);
        } else {
            auto frame_start = Clock::now();
            unsigned long frame_start_cycle = CPU.cyc;
            while(CPU.cyc - frame_start_cycle < CYCLES_PER_FRAME)
            {
                if(CPUBreak.count(CPU.pc)) {
                    if(CPUBreak[CPU.pc])
                    {
                        static bool breakbefore = false;
                        if(breakbefore) {
                            breakbefore = false;
                            z80_step(&CPU);
                            continue;
                        }
                        breakbefore = true;
                        CPUPause = true;
                        if(BreakHandle)
                            BreakHandle(CPU.pc);
                        break;
                    }
                }
                z80_step(&CPU);
            }
            auto frame_end = Clock::now();
            auto frame_dur = std::chrono::duration_cast<std::chrono::microseconds>(frame_end - frame_start);
            static constexpr auto target_duration = std::chrono::microseconds(1'000'000 / TARGET_FPS);
            if(frame_dur < target_duration)
                std::this_thread::sleep_for(target_duration - frame_dur);
        }
    }
}

uint8_t DeviceResources::CPUReadByte(void* context, uint16_t address)
{
    if(address < 0x8000 && ROMActive)
        return ROM[address];
    return RAM[address];
}

void DeviceResources::CPUWriteByte(void* context, uint16_t address, uint8_t value)
{
    if(address < 0x8000 && ROMActive) {
        AddNewEvent("CPU trying to write value: " + to_hex(value) + " to ROM at address: " + to_hex(address));
    }
    RAM[address] = value;
}

uint8_t DeviceResources::CPUIn(z80* cpu, uint8_t port)
{
    static std::unordered_map<uint8_t, std::string> in_actions = {
        // p17
        {0x3C, "(Memory Bank Selection)"},
        // p57
        {0xE4, "(uPD765 Main Status Register)"},
        {0xE5, "(uPD765 Data Register)"},
    };
    AddNewEvent("CPU trying to read data from port: " + to_hex(port) + " " + in_actions[port]);
    uint8_t out = 0;
    switch(port) {
    case 0xE4: out = ReadStatusRegister(); break;
    case 0xE5: out = ReadDataRegister(); break;
    }
    AddNewEvent("Responded with value: " + to_bin(out) + " (" + to_hex(out) + ")");
    return out;
}

void DeviceResources::CPUOut(z80* cpu, uint8_t port, uint8_t value)
{
    static std::unordered_map<uint8_t, std::string> out_actions = {
        {0xE0, "(uPD765 TL Signal On)"},
        {0xE2, "(uPD765 TL Signal Off)"},
        {0xE5, "(uPD765 Data Register)"},
        {0xE6, "(uPD765 Control Signal)"}
    };
    AddNewEvent("CPU trying to write data to port: " + to_hex(port) + " with value: " + to_hex(value) + " " + out_actions[port]);
    switch(port)
    {
    case 0xE5:
        WriteToDataRegister(value);
        break;
    }
}