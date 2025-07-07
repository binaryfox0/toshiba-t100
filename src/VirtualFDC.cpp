#include "VirtualFDC.hpp"

#include "Internal.h"
#include "EventLog.hpp"

// In references/UPD765-NEC.pdf, page 7
typedef enum msr_mask {
    // FDD stands for Floppy Disk Drive
    MSR_FDD0_BUSY   = (1 << 0),
    MSR_FDD1_BUSY   = (1 << 1),
    MSR_FDD2_BUSY   = (1 << 2),
    MSR_FDD3_BUSY   = (1 << 3),
    // FDC stands for Floppy Disk Controller
    MSR_FDC_BUSY    = (1 << 4),
    // Execution mode.
    // Only set if during executing phase in non-DMA mode
    // Goes low if execution phase has ended and result phase was started
    // Only operated in non-DMA mode
    MSR_EXEC_MODE   = (1 << 5),
    // Represent IO direction.
    // If equals 1, transfer is from Data Register to Processor
    // Otherwise, transfer is from Processor to Data Register
    MSR_IO_DIR       = (1 << 6),
    // Request for Master
    // Indicate Data Register is ready to send/recieve data
    // Should use with DIO (SR_IO_DIR) to perform "ready" and "direction"
    MSR_RQM         = (1 << 7)
} msr_mask;

typedef enum sr0_mask {
    SR0_UNIT_SEL        = 3,
    SR0_HEAD_ADDR       = (1 << 2),
    SR0_NOT_READY       = (1 << 3),
    SR0_EQUIPMENT_CHK   = (1 << 4),
    SR0_SEEK_END        = (1 << 5),
    SR0_IRQ_CODE        = (3 << 6)
} sr0_mask;

typedef enum sr0_irqs {
    SR0_IRQ_NORMAL              = (0 << 6),
    SR0_IRQ_CMD_FAILED          = (1 << 6),
    SR0_IRQ_INVALID_CMD         = (2 << 6),
    SR0_IRQ_FDD_STATE_CHANGED   = (3 << 6)
} sr0_irqs;

typedef struct fdc_cmd_entry {
    const char* action_str; // only debug/logging
    uint8_t expected, mask, opr_bytes;
} fdc_cmd_entry;
static const fdc_cmd_entry cmd_entries[] =
{
    {"Read data",             0b00000110, 0b00011111, 8}, // Read data
    {"Read a track",          0b00000010, 0b10011111, 8}, // Read a track
    {"Read deleted data",     0b00001100, 0b00011111, 8}, // Read deleted data
    {"Read ID",               0b00001010, 0b10111111, 1}, // Read ID
    {"Write data",            0b00000101, 0b00111111, 8}, // Write data
    {"Format a track",        0b00001101, 0b10111111, 5}, // Format a track
    {"Scan equal",            0b00010001, 0b00011111, 8}, // Scan equal
    {"Write deleted data",    0b00001001, 0b00111111, 8}, // Write deleted data
    {"Scan low or equal",     0b00011001, 0b00011111, 8}, // Scan low or equal
    {"Recalibrate",           0b00000111, 0b11111111, 1}, // Recalibrate
    {"Sense interrupt state", 0b00001000, 0b11111111, 0}, // Sense interrupt status
    {"Specify",               0b00000011, 0b11111111, 2}, // Specify
    {"Sense drive status",    0b00000100, 0b11111111, 1}, // Sense drive status
    {"Scan high or equal",    0b00011101, 0b00011111, 8}, // Scan high or equal
    {"Seek",                  0b00001111, 0b11111111, 2}, // Seek
};

static uint8_t msr = MSR_RQM;
uint8_t ReadStatusRegister() {
    return msr;
}

static uint8_t out_queue[16] = {0};
static int out_index = 0;
static int out_available_bytes = 0;
uint8_t ReadDataRegister()
{
    if(out_available_bytes) {
        out_available_bytes--;
        AddNewEvent("FDC: " + std::to_string(out_available_bytes) + " bytes to read");
        if(out_available_bytes == 0) {
            msr = MSR_RQM;
        }
        return out_queue[out_index++];
    }

    return 0;
}

static uint8_t command_index = 0;
static uint8_t in_queue[9] = {0};
static int in_index = 0;

static uint8_t sr0 = SR0_IRQ_INVALID_CMD;
static uint8_t pcn = 0; // position of head at present time

INLINE void cmd_sense_interrupt_status() {
    out_index = 0;
    out_available_bytes = 2;
    out_queue[0] = sr0;
    out_queue[1] = pcn;
}

// Remove non-inlined function overhead
INLINE void dispatch_command() {
    switch(command_index) {
    case 10: cmd_sense_interrupt_status();
    }
    command_index = 0;
    msr = MSR_IO_DIR | MSR_RQM; // Available to read
}

static bool command_phase = false;
static int opr_bytes = 0;

void WriteToDataRegister(uint8_t byte)
{
    if(command_phase) {
        if(opr_bytes > 0) {
            in_queue[in_index++] = byte;
            if(opr_bytes)
                return;
        }
        dispatch_command();
        command_phase = 0;
    }
    // Find command
    for(int i = 0; i < ARRSZ(cmd_entries); i++) {
        const fdc_cmd_entry& entry = cmd_entries[i];
        if((byte & entry.mask) == entry.expected) {
            AddNewEvent(std::string("FDC: Recieved the following command : ") + entry.action_str);
            command_index = i;
            if(entry.opr_bytes)
                opr_bytes = entry.opr_bytes;
            else
                dispatch_command();
            break;
        }
    }
    // Assuming I addded all support for all commands
}