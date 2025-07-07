#ifndef VIRTUAL_FLOPPY_DISK_CONTROLLER_HPP
#define VIRTUAL_FLOPPY_DISK_CONTROLLER_HPP

#include <stdint.h>

uint8_t ReadStatusRegister();
uint8_t ReadDataRegister();
void WriteToDataRegister(uint8_t byte);

#endif