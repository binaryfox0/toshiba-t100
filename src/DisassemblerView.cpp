#include "DisassemblerView.hpp"

#include <cstdint>
#include <cstdio>
#include <stack>
#include <queue>
#include <stdexcept>
#include <unordered_map>

#include "EventLog.hpp"
#include "ResourceManager.hpp"
#include "Z80Disassembler.h"
#include "DeviceEmulator.hpp"
#include "RegistersView.hpp"

#include "z80.h"
#include "imgui.h"

#define SIZEOF_ARR(arr) (sizeof(arr) / sizeof(arr[0]))
#define Z80_MAX_INSTR_LEN 4
#define HIGHLIGHT_INSTR_DUR 2.5f // 2.5s

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
    uint8_t type;
    char operands[2][12]; 
    uint8_t flags;
};
const uint8_t *memory = 0;
static std::stack<uint16_t> history;
static std::unordered_map<uint16_t, DisassembleInstr> parsed;
static std::unordered_map<uint16_t, uint16_t> parsed_jump_address;

static bool cpu_start = false;
static bool cpu_pause = true;

static uint16_t load_address = 0;
static uint32_t memory_size = 0;

static bool follow_pc = false; // if true trigger follow then set to false
// To prevent something like jr that like jr will jump to pos still in range
static uint16_t display_addr_end = 0;
static uint16_t display_instr_count = 0;

// Highlighting for instruction that jp/call/jr is pointing to
static int highlight_addr = -1;
static float highlight_start = 0.0f;
static bool highlight_addr_focus = false;

bool FindPreviousInstruction(uint16_t& address)
{
    for(int i = 1; i <= Z80_MAX_INSTR_LEN; i++)
    {
        try {
            uint16_t new_address = address - i;
            const DisassembleInstr& instr = parsed.at(new_address);
            if(new_address < load_address)
                break;
            if(instr.size != i)
                continue;
            address = new_address;
            return true;
        } catch(std::out_of_range& e) {
            continue;
        }
    }
    return false;
}

void UpdateDisplayRange(uint16_t address, bool push)
{
    uint16_t display_addr_start = address;
    display_instr_count = 0;
    while(FindPreviousInstruction(display_addr_start)) {}
    if(push) {
        if(history.empty())
            history.push(display_addr_start);
        else
            if(history.top() != display_addr_start)
                history.push(display_addr_start);
    }
    display_addr_end = display_addr_start;
    bool run = true;
    bool first = true;
    while(run) 
    {
        try {
            display_addr_end += parsed.at(display_addr_end).size;
            display_instr_count++;
            first = false;
        } catch (const std::out_of_range& e) {
            if(first)
                FindPreviousInstruction(display_addr_end);
            run = false;
        }
    }
}

void DrawInvisibleAddressButton(const char* operand, uint16_t address)
{
    char label[16] = {0};
    snprintf(label, sizeof(label), "%s##%04X", operand, address);
    if(ImGui::SmallButton(label)) {
        uint16_t new_address = parsed_jump_address[address];
        UpdateDisplayRange(new_address, true);
        highlight_addr = new_address;
        highlight_addr_focus = true;
        highlight_start = ImGui::GetTime();
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

void DrawDisassembleRow(const DisassembleInstr& row, bool is_pc)
{
    ImGui::SameLine();
    ImGui::TableSetColumnIndex(0);
    if(is_pc) {
        ImGui::TextUnformatted(">");
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
    ImGui::TextUnformatted(z80_get_mnemonic_from_index(row.type));
    ImGui::TableSetColumnIndex(4);
    if(row.flags)
    {
        if((row.flags & 0b1) == FLAGS_JUMP_1) {
            DrawInvisibleAddressButton(row.operands[0], row.address);
            ImGui::SameLine(0.0f, 0.0f);
            ImGui::TextUnformatted(row.operands[1]);
        } else {
            ImGui::TextUnformatted(row.operands[0]);
            ImGui::SameLine(0.0f, 0.0f);
            DrawInvisibleAddressButton(row.operands[1], row.address);
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

static const char* pauseresume_active_img = "resume_normal.png";
void UpdatePauseResumeButtonImage() {
    if(cpu_pause) pauseresume_active_img = "resume_normal.png";
    else pauseresume_active_img = "pause_normal.png";
}

void DrawDisassemblerToolbar()
{
    if(DrawInactiveableButton(
        "##arror_back", "arrow_back_normal.png", "arrow_back_grayed.png", 
        history.size() > 1, ImGuiKey_Escape)
    ) {
        history.pop();
        UpdateDisplayRange(history.top(), false);
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("Go to previous location (Esc)");
    ImGui::SameLine();
    static const char* onoff_active_img = "play_arrow_normal.png";
    if(DrawInactiveableButton("##onoff", onoff_active_img, "", true, ImGuiKey_F5)) {
        if((cpu_start = !cpu_start)) {
            onoff_active_img = "stop_normal.png";
            AddNewEvent("Machine started!");
        } else {
            cpu_pause = true;
            UpdatePauseResumeButtonImage();
            DeviceResetCPU();
            onoff_active_img = "play_arrow_normal.png";
            AddNewEvent("Machine reset!");
        }
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip(cpu_start ? "Stop machine (F5)" : "Start machine (F5)");
    ImGui::SameLine();

    if(DrawInactiveableButton("##pausecontinue", pauseresume_active_img, "resume_grayed.png", cpu_start, ImGuiKey_F8)) {
        cpu_pause = !cpu_pause;
        UpdatePauseResumeButtonImage();
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip(cpu_pause ? "Resume machine (F8)" : "Pause machine (F8)");
    ImGui::SameLine();

    if(DrawInactiveableButton("##step_into", "step_into_normal.png", "step_into_grayed.png", cpu_start && cpu_pause, ImGuiKey_F11)) {
        uint16_t old_pc = z80_cpu.pc;
        z80_step(&z80_cpu);
        follow_pc = true;
        if(z80_cpu.pc != old_pc && z80_cpu.pc != old_pc + parsed[old_pc].size)
        {
            if(z80_get_type(memory + old_pc) == RET && history.size() > 1) {
                history.pop();
                UpdateDisplayRange(history.top(), false);
            }
            else {
                DisassembleInstr& start_row = parsed[old_pc];
                DisassembleInstr& end_row   = parsed[z80_cpu.pc];
                printf("Old: %04X. New: %04X\n", old_pc, z80_cpu.pc);
                printf("Start: %04X: %s %s%s\n", start_row.address, z80_get_mnemonic_from_index(start_row.type), start_row.operands[0], start_row.operands[1]);
                printf("End:   %04X: %s %s%s\n", end_row.address, z80_get_mnemonic_from_index(end_row.type), end_row.operands[0], end_row.operands[1]);
                UpdateDisplayRange(z80_cpu.pc, true);
                highlight_addr_focus = true;
            }
        }
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("Step Into (F11)");
    ImGui::SameLine();

    static char str[5] = "";
    ImageResource& img = ResourceManager::GetImage("jump_to_element.png");
    if(ImGui::ImageButton("##jumptoelement", img.texture, img.size))
        UpdateDisplayRange(strtol(str, 0, 16), true);
    ImGui::SameLine();
    if(ImGui::InputTextWithHint("##address", "<hex address>", str, sizeof(str), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal))
        UpdateDisplayRange(strtol(str, 0, 16), true);
}
void DrawDisassemblerView()
{
    if(!cpu_pause)
        z80_step(&z80_cpu);
    if(highlight_addr != -1) {
        if(ImGui::GetTime() - highlight_start > HIGHLIGHT_INSTR_DUR)
            highlight_addr = -1;
    }
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Disassembler");
    DrawDisassemblerToolbar();
    ImGui::BeginChild("ScrollReigon", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    if (ImGui::BeginTable("##instr_view", 5, ImGuiTableFlags_SizingStretchProp))
    {
        // Hidden button for address
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, ImGui::GetStyle().FramePadding.y));
        // ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));

        ImGuiListClipper clipper;
        clipper.Begin(display_instr_count);

        static int scroll_index = 0;
        if(highlight_addr_focus) {
            uint16_t tmp = history.top();
            int index = 0;
            bool found = false;
            while(parsed.count(tmp) > 0) {
                if(tmp == highlight_addr) {
                    found = true;
                    break;
                }
                tmp += parsed[tmp].size;
                index++;
            }
            highlight_addr_focus = false;
            if(found)
                ImGui::SetScrollY(ImGui::GetTextLineHeightWithSpacing() * index);
            else
                UpdateDisplayRange(z80_cpu.pc, true);
        }

        static int clipper_start_prev = -1; // to find change
        while(clipper.Step())
        {
            static int clipper_addr_start = -1;
            if(clipper.DisplayStart != clipper_start_prev) {
                clipper_start_prev = clipper.DisplayStart;
                clipper_addr_start = history.top();
                for(int i = 0; 
                    i < clipper.DisplayStart && parsed.count(clipper_addr_start) > 0;
                    i++, clipper_addr_start += parsed[clipper_addr_start].size
                ) {}
            }
            uint16_t tmp = clipper_addr_start;
            bool need2break = false;
            for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                bool is_pc = tmp == z80_cpu.pc;
                DisassembleInstr& instr = parsed[tmp];
                tmp += instr.size;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if(need2break) {
                    ImGui::TableNextRow();
                    need2break = false;
                }
                ImGui::PushID(tmp); // Unique ID per row
                bool highlight = highlight_addr == instr.address || is_pc;
                if (ImGui::Selectable("##row_select", highlight, 
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 0))) {
                    // Row clicked
                }
                ImGui::PopID();

                DrawDisassembleRow(instr, is_pc);
                if(instr.type == RET)
                    need2break = true;
            }
        }
        clipper.End();

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar();
        ImGui::EndTable();
    }
    ImGui::EndChild();
    ImGui::End();

    registersview_show = true;
}


// MARK: Disassembler section
std::stack<uint16_t> ret_addr;
void Disassemble(uint16_t address)
{
    std::queue<uint16_t> more_address;
    while(parsed.count(address) == 0)
    {
        if(address < load_address || address > memory_size)
            break;
        const uint8_t* instr = memory + address;
        uint8_t len = z80_get_instr_len(instr);
        DisassembleInstr& out = parsed[address];
        out.address = address;
        out.size = len;
        out.type = z80_get_type(instr);
        z80_get_operands(instr, out.operands[0], sizeof(out.operands[0]), out.operands[1], sizeof(out.operands[1]));
        switch(out.type)
        {
        case JP: {
            uint16_t jump_address = *(uint16_t*)(instr + 1);
            AddNewEvent("Disassembler: Jump to address " + to_hex(jump_address));
            parsed_jump_address[address] = jump_address;
            if(instr[0] == 0xC3) {
                out.flags |= FLAGS_JUMP_1;
                address = jump_address;
                continue;
            }
            out.flags |= FLAGS_JUMP_2;
            more_address.push(jump_address);
            break;
        }
        case JR: {
            uint16_t jump_address = *(int8_t *)(instr + 1) + out.address + 2;
            AddNewEvent("Disassembler: Jump relatively to address " + to_hex(jump_address));
            parsed_jump_address[address] = jump_address;
            out.flags |= FLAGS_JUMP_RELATIVE;
            if (instr[0] == 0x18) {
              out.flags |= FLAGS_JUMP_1;
              address = jump_address;
              continue;
            }
            out.flags |= FLAGS_JUMP_2;
            more_address.push(jump_address);
            break;
        }
        case CALL: {
            uint16_t jump_address = *(uint16_t*)(instr + 1);
            AddNewEvent("Disassembler: Call function at address " + to_hex(jump_address));
            parsed_jump_address[address] = jump_address;
            if(parsed.count(jump_address) > 0)
                break;
            if(instr[0] == 0xCD) { // unconditional
                out.flags |= FLAGS_JUMP_1;
                ret_addr.push(address + len);
                address = jump_address;
                continue;
            }
            out.flags |= FLAGS_JUMP_2;
            more_address.push(jump_address);
            break;
        }
        case RET:
        {
            if(ret_addr.empty()) {
                AddNewEvent("Disassembler: Empty return address stack. Maybe this code is faulty");
                break;
            }
            address = ret_addr.top();
            AddNewEvent("Disassembler: Return to address " + to_hex(address));
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
    memory = memory_in;
    load_address = address;
    memory_size = size_in;

    Disassemble(address);
    UpdateDisplayRange(address, true);
    show_disassembler = true;
}