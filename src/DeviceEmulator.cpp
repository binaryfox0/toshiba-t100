#include "DeviceEmulator.hpp"

#include <stdint.h>
#include <errno.h> // get error message from syscall

#include <fstream>
#include <string>
#include <cstring>
#include <map>

#include "MessageBox.hpp"
#include "EventLog.hpp"
#include "DisassemblerView.hpp"

#include "Internal.h"
#include "imgui.h"

// Reference:
// - strings is `images/TDISKBASIC.img` at offset 0x22CD6
#define TRACK_SIZE 0x1000

// Note: Based on the strings in the disk image, 
#define TRACK_TO_OFFSET(T) (T * TRACK_SIZE)
// Initial Program Loader (IPL) like bootloader on modern machine
#define IPL_OFFSET TRACK_TO_OFFSET(1)
#define IPL_LOAD_OFFSET 0xD000

// Reference:
// - references/ToshibaT100_tech_ref_Eng.pdf p10. Memory Address Space
// - strings in `images/TDISKBASIC.img` at offset 0x22D64 said:
// "tracks 1-7 hold IPL program and BASIC (if system disk)"
// Maybe this is the system disk

// p10 ToshibaT100_tech_ref_Eng.pdf
static uint8_t toshiba_ram[0x10000] = {0};
static uint8_t toshiba_rom[0x8000] = {0};
// p7  ToshibaT100_tech_reg_Eng,pdf
// static uint8_t toshiba_vram[0x4000] = {0};
static bool builtin_rom_active = true;

// Shared stuff
bool deviceemulator_show = false;


uint8_t cpu_read(void* context, uint16_t address) {
    if(address < 0x8000 && builtin_rom_active)
        return toshiba_rom[address];
    return toshiba_ram[address]; 
}

void cpu_write(void* context, uint16_t address, uint8_t value) {
    if(address < 0x8000 && builtin_rom_active) {
        AddNewEvent("CPU trying to write value: " + to_hex(value) + " to ROM at address: " + to_hex(address));
    }
    toshiba_ram[address] = value;
}

uint8_t cpu_in(z80* cpu, uint8_t port) {
    static std::map<uint8_t, std::string> in_actions = {
        {0xE4, "(uPD765 Status Register)"}
    };
    AddNewEvent("CPU trying to read data from port: " + to_hex(port) + " " + in_actions[port]);
    return 0;
}

void cpu_out(z80* cpu, uint8_t port, uint8_t value) {
    static std::map<uint8_t, std::string> out_actions = {
        {0xE6, "(uPD765 FDC Control Port)"}
    };
    AddNewEvent("CPU trying to write data to port: " + to_hex(port) + " with value: " + to_hex(value) + " " + out_actions[port]);
    switch(port)
    {
    case 0xE4:
        break;
    case 0xE6:
        break;
    }
}

z80 z80_cpu = {0};
void DeviceResetCPU()
{
    z80_init(&z80_cpu);
    z80_cpu.read_byte = cpu_read;
    z80_cpu.write_byte = cpu_write;
    z80_cpu.port_in = cpu_in;
    z80_cpu.port_out = cpu_out;
    z80_cpu.pc = IPL_LOAD_OFFSET;
}

void DeviceInit(const std::string& dskimg_path)
{
    std::ifstream dsk(dskimg_path, std::ios::binary);
    if(!dsk) {
        CreateMessageBox("Error!", ((std::string)"Failed to open specify disk image file\nerrno: " + strerror(errno)).c_str());
        return;
    }
    dsk.seekg(IPL_OFFSET, std::ios::beg);
    // Important: Simon Kessane guess that the IPL will be load to address 0xD000 maybe in ROM
    dsk.read(reinterpret_cast<char*>(toshiba_ram + IPL_LOAD_OFFSET), std::min<size_t>(TRACK_TO_OFFSET(8) - IPL_OFFSET, sizeof(toshiba_ram) - 0xD000));
    SetupDisassembler(toshiba_ram, IPL_LOAD_OFFSET, sizeof(toshiba_ram));
    DeviceResetCPU();
    deviceemulator_show = true;
    eventlog_show = true;
}

void DrawDeviceEmulator()
{
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Toshiba T100 Screen");
    ImGui::End();
}
