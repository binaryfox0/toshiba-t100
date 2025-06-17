#include "DeviceEmulator.hpp"

#include <cstring>
#include <stdint.h>
#include <errno.h> // get error message from syscall

#include <fstream>
#include <string>
#include <sstream>
#include <type_traits>
#include <iomanip>

#include "MessageBox.hpp"
#include "EventLog.hpp"
#include "DisassemblerView.hpp"

#include "Z80Disassembler.h"
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

static uint8_t toshiba_ram[0x10000] = {0};
static uint8_t toshiba_rom[0x8000] = {0};
static bool builtin_rom_active = true;

// Shared stuff
bool deviceemulator_show = false;

template<typename T>
inline typename std::enable_if<std::is_integral<T>::value, std::string>::type
to_hex(T val) {
    std::stringstream ss;
    ss << "0x"
       << std::hex << std::setw(sizeof(T) * 2)
       << std::uppercase << std::setfill('0')
       << static_cast<uint64_t>(val);  // cast avoids char printing weirdness
    return ss.str();
}


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
    AddNewEvent("CPU trying to read data from port: " + to_hex(port));
    return 0;
}

void cpu_out(z80* cpu, uint8_t port, uint8_t value) {
    AddNewEvent("CPU trying to write data to port: " + to_hex(port) + " with value: " + to_hex(value));
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
