#ifndef DEVICE_EMULATOR_HPP
#define DEVICE_EMULATOR_HPP

#include <string>
#include <stdint.h>

#include "z80_cpp.h"

extern bool deviceemulator_show;
extern z80 z80_cpu;
void DeviceResetCPU();
void DeviceInit(const std::string& dskimg_path);
void DrawDeviceEmulator();

#endif