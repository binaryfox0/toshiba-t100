#include "DisassemblerView.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stack>
#include <imgui.h>
#include <queue>

#include "EventLog.hpp"
#include "ResourceManager.hpp"
#include "Z80Disassembler.h"
#include "DeviceEmulator.hpp"
#include "z80.h"

#define SIZEOF_ARR(arr) (sizeof(arr) / sizeof(arr[0]))

enum {
    FLAGS_NONE,
    FLAGS_JUMP_1 = 1,
    FLAGS_JUMP_2 = 2,
    FLAGS_JUMP_RELATIVE = 1 << 2
};

bool show_disassembler = false;
struct DisassembleInstr {
    uint16_t address;
    uint8_t size;
    const char* mnemonics;
    char operands[2][12]; 
    uint8_t flags;
};
const uint8_t *memory = 0;
static std::stack<uint16_t> history;
static std::unordered_map<uint16_t, DisassembleInstr> parsed;
static bool cpu_start = false;
static bool cpu_pause = true;
static uint16_t load_address = 0;
static uint32_t size = 0;

static bool follow_pc = false; // if true trigger follow then set to false

void Disassemble(uint16_t address);

void DrawInvisibleAddressButton(const char* operand, bool relative_address, uint16_t address)
{
    char label[16] = {0};
    snprintf(label, sizeof(label), "%s##%04X", operand, address);
    if(ImGui::SmallButton(label)) {
        uint16_t new_address = (relative_address ? address : 0) + strtol(operand, nullptr, 0);
        history.push(new_address);
    }
    // Fake underlined
    ImVec2 text_size = ImGui::CalcTextSize(operand);

    // Get item rect
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    float item_width = max.x - min.x;

    // Y position for underline (a bit below text)
    float underline_y = min.y + (max.y - min.y) * 0.5f + text_size.y * 0.5f + 1.0f;

    // Center the underline to match the label
    float text_start_x = min.x + (item_width - text_size.x) * 0.5f;

    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(text_start_x, underline_y),
        ImVec2(text_start_x + text_size.x, underline_y),
        ImGui::GetColorU32(ImGuiCol_Text)
    );
}

void DrawDisassembleRow(const DisassembleInstr& row)
{
    ImGui::SameLine();
    ImGui::TableSetColumnIndex(0);
    if(z80_cpu.pc == row.address) {
        ImGui::TextUnformatted(">");
        if(follow_pc) {
            ImGui::SetScrollHereY();
            follow_pc = false;
        }
    } else {
        ImGui::Dummy(ImGui::CalcTextSize(">"));
    }
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%04X", row.address);
    char tmp[12] = {0};
    for(int i = 0; i < row.size - 1; i++)
        sprintf(tmp + (i * 3), "%02X ", memory[row.address + i]);
    sprintf(tmp + ((row.size - 1) * 3), "%02X", memory[row.address + (row.size - 1)]);
    ImGui::TableSetColumnIndex(2);
    ImGui::Text("%s", tmp);
    ImGui::TableSetColumnIndex(3);
    ImGui::Text("%s", row.mnemonics);
    ImGui::TableSetColumnIndex(4);
    if(row.flags)
    {
        bool relative = row.flags & FLAGS_JUMP_RELATIVE;
        if((row.flags & 0b11) == FLAGS_JUMP_1) {
            DrawInvisibleAddressButton(row.operands[0], relative, row.address);
            ImGui::SameLine(0.0f, 0.0f);
            ImGui::TextUnformatted(row.operands[1]);
        } else {
            ImGui::TextUnformatted(row.operands[0]);
            ImGui::SameLine(0.0f, 0.0f);
            DrawInvisibleAddressButton(row.operands[1], relative, row.address);
        }
    }
    else
        ImGui::Text("%s%s", row.operands[0], row.operands[1]);    
}

bool DrawInactiveableButton(
    const std::string& id, const std::string& active_image, 
    const std::string& inactive_image, const bool active, 
    ImGuiKeyChord key_chord = ImGuiKey_None)
{
    ImageResource &image = ResourceManager::GetImage(active ? active_image : inactive_image);
    if(!active) {
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_Button));
    }
    bool b = (
        ImGui::ImageButton(id.c_str(), (ImTextureID)(intptr_t)image.texture, image.size) ||
        (key_chord == ImGuiKey_None ? 0 : ImGui::Shortcut(key_chord))
    ) && active;
    if(!active) {
        ImGui::PopStyleColor(2);
    }
    return b;
}

void DrawDisassemblerToolbar()
{
    if(DrawInactiveableButton(
        "##arror_back", "arrow_back_normal.png", "arrow_back_grayed.png", 
        history.size() > 1, ImGuiKey_Escape))
    {
            history.pop();
            uint16_t new_address = history.top();
            Disassemble(new_address);
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("Go to previous location (Esc)");
    ImGui::SameLine();
    if(DrawInactiveableButton("##play_arrow", "play_arrow_normal.png", "play_arrow_grayed.png", !cpu_start)) {
        cpu_start = true;
        AddNewEvent("Machine started!");
    }
    ImGui::SameLine();
    if(DrawInactiveableButton("##shutdown", "shutdown_normal.png", "shutdown_grayed.png", cpu_start)) {
        DeviceResetCPU();
        cpu_start = false;
        cpu_pause = true;
        AddNewEvent("Machine reset!");
    }
    ImGui::SameLine();
    if(DrawInactiveableButton("##pause", "pause_normal.png", "pause_grayed.png", !cpu_pause && cpu_start))
        cpu_pause = true;
    ImGui::SameLine();
    if(DrawInactiveableButton("##resume", "resume_normal.png", "resume_grayed.png", cpu_pause && cpu_start))
        cpu_pause = false;
    ImGui::SameLine();
    if(DrawInactiveableButton("##step_over", "step_over_normal.png", "step_over_grayed.png", cpu_start && cpu_pause)) {
        uint16_t old_pc = z80_cpu.pc;
        DisassembleInstr& p = parsed[old_pc];
        z80_step(&z80_cpu);
        follow_pc = true;
        if(z80_cpu.pc != old_pc && z80_cpu.pc != old_pc + p.size)
        {
            if(z80_get_type(memory + old_pc) == RET)
                history.pop();
            else {
                uint16_t new_address = (p.flags & FLAGS_JUMP_RELATIVE ? p.address : 0) + strtol((p.flags & 0b11) == FLAGS_JUMP_1 ? p.operands[0] : p.operands[1], nullptr, 0);
                history.push(new_address);
            }
        }
    }
}

void DrawDisassemblerView()
{
    if(!cpu_pause)
        z80_step(&z80_cpu);
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Disassembler");
    DrawDisassemblerToolbar();
    ImGui::BeginChild("ScrollReigon", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    if (ImGui::BeginTable("MyTable", 5,
        ImGuiTableFlags_SizingStretchProp     // Optional: make columns stretch
    )) {
        // Hidden button for address
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, ImGui::GetStyle().FramePadding.y));
        // Optional: Set column widths or behavior
        ImGui::TableSetupColumn("##col1"); // Hidden name, no header
        ImGui::TableSetupColumn("##col2");
        ImGui::TableSetupColumn("##col3");

        // Temporarily override style for hover + selection
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
        ImGui::PushStyleColor(ImGuiCol_Header,       ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));

        uint16_t tmp = history.top();
        while(parsed.count(tmp) > 0)
        {
            DisassembleInstr& instr = parsed[tmp];
            tmp += instr.size;
            ImGui::TableNextRow();

            // Span the row by beginning at column 0
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(tmp); // Unique ID per row
            if (ImGui::Selectable("##row_select", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 0))) {
                // Row clicked
            }
            ImGui::PopID();

            DrawDisassembleRow(instr);
        }

        ImGui::PopStyleColor(5); // Restore hover/active/header colors
        // ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::End();
}

std::stack<uint16_t> ret_addr;
void Disassemble(uint16_t address)
{
    std::queue<uint16_t> more_address;
    while(parsed.count(address) == 0)
    {
        if(address > size)
            break;
        const uint8_t* instr = memory + address;
        uint8_t type = z80_get_type(instr);
        uint8_t len = z80_get_instr_len(instr);
        DisassembleInstr& out = parsed[address];
        out.address = address;
        out.size = len;
        out.mnemonics = inst_mnemonics[type];
        z80_get_operands(instr, out.operands[0], sizeof(out.operands[0]), out.operands[1], sizeof(out.operands[1]));
        switch(type)
        {
        case JP: {
            if(instr[0] == 0xC3) {
                out.flags |= FLAGS_JUMP_1;
                address = *(uint16_t*)(instr + 1);
                continue;
            }
            out.flags |= FLAGS_JUMP_2;
            more_address.push(*(uint16_t*)(instr + 1));
            break;
        }
        case JR: {
            out.flags |= FLAGS_JUMP_RELATIVE;
            if(instr[0] == 0x18) {
                out.flags |= FLAGS_JUMP_1;
                address = *(uint8_t*)(instr + 1) + out.address;
                continue;
            }
            out.flags |= FLAGS_JUMP_2;
            more_address.push(*(uint8_t*)(instr + 1) + out.address);
            break;
        }
        case CALL: {
            if(instr[0] == 0xCD) { // unconditional
                out.flags |= FLAGS_JUMP_1;
                ret_addr.push(address + len);
                address = *(uint16_t*)(instr + 1);
                continue;
            }
            out.flags |= FLAGS_JUMP_2;
            more_address.push(*(uint16_t*)(instr + 1));
            break;
        }
        case RET:
        {
            if(ret_addr.empty()) {
                AddNewEvent("Disassembler: Empty return address stack. Maybe this code is faulty");
                break;
            }
            address = ret_addr.top();
            ret_addr.pop();
            continue;
        }
        }
        address += len;
    }
    while(!more_address.empty()) {
        Disassemble(more_address.front());
        more_address.pop();
    }
}

void SetupDisassembler(const uint8_t* memory_in, uint16_t address, uint32_t size_in)
{
    history.push(address);
    memory = memory_in;
    load_address = address;
    size = size_in;

    Disassemble(address);
    show_disassembler = true;
}