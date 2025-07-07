#include "DisassemblerView/InstructionTable.hpp"

#include <vector>
#include <algorithm>

#include "imgui.h"

#include "DisassemblerView/Core.hpp"
#include "DisassemblerView/InstructionTableMultiselect.hpp"
#include "DisassemblerView/Main.hpp"
#include "Z80Disassembler.h"

#include "Internal.h"
#include "UIHelpers.hpp"
#include "DeviceResources.hpp"

#define HIGHLIGHT_DUR 2.5f //2.5s

static const auto& parsed              = DisassemblerView::GetDisassemblerParsedGeneral();
static const auto& parsed_jump_address = DisassemblerView::GetDisassemblerParsedJumpAddress();
static const auto& mem                 = DisassemblerView::GetDisassemblerMemory();

static const auto& bytes_range = DisassemblerView::GetDisassemblerBytesRange();

// Highlighting for instruction that jp/call/jr is pointing to
static int highlight_addr = -1;
static float highlight_start = 0.0f;
static int focus_addr = -1;

// just for row break
static std::vector<display_instr> display_ranges;
static std::vector<uint16_t> history;
static uint16_t display_addr_end = 0;

INLINE void GenerateHexPairString(char* str, const size_t maxlen, const uint8_t *bytes, const size_t bytes_len)
{
    size_t remaining_len = maxlen;
    for(size_t i = 0; i < bytes_len - 1; i++, remaining_len -= 3)
        snprintf(str + (i * 3), remaining_len, "%02X ", bytes[i]);
    snprintf(str + (bytes_len - 1) * 3, remaining_len, "%02X", bytes[bytes_len - 1]);
}

bool FindPreviousInstruction(uint16_t& address)
{
    for (int i = 1; i <= Z80_MAX_INSTR_LEN; i++)
    {
        if (address < bytes_range.x + i) break;
        uint16_t test_addr = address - i;
        try {
            const DisassembleInstr& instr = parsed.at(test_addr);
            if (instr.size == i) {
                address = test_addr;
                return true;
            }
        } catch (...) {
            continue;
        }
    }
    return false;
}


INLINE void AddressButtonPressed(const uint16_t address)
{
    uint16_t new_address = parsed_jump_address.at(address);
    DisassemblerView::UpdateDisplayRange(new_address, true);
    focus_addr = highlight_addr = new_address;
    highlight_start = ImGui::GetTime();
}

void DrawDisassembleRow(const DisassembleInstr& row, bool is_pc)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::PushID(row.address);
    if(ImGui::Selectable("##row_select", highlight_addr == row.address || is_pc, 
        ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, 0)))
        ;
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::TableSetColumnIndex(0);


    ImGui::Text("%04X:", row.address);
    ImGui::TableNextColumn();

    char tmp[12] = {0};
    GenerateHexPairString(tmp, sizeof(tmp), mem + row.address, row.size);
    ImGui::TextUnformatted(tmp);
    ImGui::TableNextColumn();

    ImGui::TextUnformatted(z80_get_mnemonic_from_index(row.type));
    ImGui::TableNextColumn();
    if(row.flags)
    {
        if((row.flags & 0b1) == FLAGS_JUMP_1) {
            if(DrawHyperlinkButton((std::string(row.operands[0]) + "##" + to_hex(row.address)).c_str()))
                AddressButtonPressed(row.address);
            ImGui::SameLine(0.0f, 0.0f);
            ImGui::TextUnformatted(row.operands[1]);
        } else {
            ImGui::TextUnformatted(row.operands[0]);
            ImGui::SameLine(0.0f, 0.0f);
            if(DrawHyperlinkButton((std::string(row.operands[1]) + "##" + to_hex(row.address)).c_str()))
                AddressButtonPressed(row.address);
        }
    }
    else
        ImGui::Text("%s%s", row.operands[0], row.operands[1]);    
}

static auto& scroll_synced = DisassemblerView::GetScrollSynced();
INLINE void ScrollSync()
{
    static float last_scroll = 0.0f;
    float current_scroll = ImGui::GetScrollY();
    if(std::abs(current_scroll - last_scroll) > 0.0f) {
        scroll_synced = current_scroll;
    } else {
        ImGui::SetScrollY(scroll_synced);
    }
    last_scroll = ImGui::GetScrollY();
}

namespace DisassemblerView
{
static bool stop_access = false; // to prevent out-of-bound element/SIGSEGV when the address doesn't in current display ranges
void UpdateDisplayRange(uint16_t address, bool push)
{
    while (parsed.count(address) == 0 && address > bytes_range.x)
        address--;
    if(!history.empty())
        if(in_range(history.back(), display_addr_end, address))
            return;
    uint16_t display_addr_start = address;
    while (FindPreviousInstruction(display_addr_start)) {}

    if (push)
        history.push_back(display_addr_start);

    display_addr_end = display_addr_start;
    display_ranges.clear();
    while (true)
    {
        auto it = parsed.find(display_addr_end);
        if (it == parsed.end())
            break;
        display_ranges.push_back({false, display_addr_end});
        if(mem[it->second.address] == 0xC9)
            display_ranges.push_back({true});
        display_addr_end += it->second.size;
    }
    stop_access = true;
}

void InitInstructionTable() {
    UpdateDisplayRange(bytes_range.x, true);
}

void DrawInstructionTable()
{
    if (highlight_addr != -1) {
        if (ImGui::GetTime() - highlight_start > HIGHLIGHT_DUR)
            highlight_addr = -1;
    }

    ImGui::BeginChild("##instruction_view");
    ScrollSync();
    if(parsed.empty())
    {
        const char* text = "Please load a disk by open menu File->Open.";
        ImVec2 avail = ImGui::GetWindowSize();
        ImVec2 text_size = ImGui::CalcTextSize(text);

        ImGui::SetCursorPos(ImVec2(
            (avail.x - text_size.x) * 0.5f,
            (avail.y - text_size.y) * 0.5f
        ));
        ImGui::TextUnformatted(text);
    } else {
        if (ImGui::BeginTable("##instr_view", 4))
        {
            ImGui::TableSetupColumn("##address_col", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("##bytes_col");
            ImGui::TableSetupColumn("##instrs_col");
            ImGui::TableSetupColumn("##operands_col");

            static InstructionTableMultiselect selection;
            ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(
                ImGuiMultiSelectFlags_ClearOnClickVoid | ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect2d ,
                selection.Size, display_ranges.size()
            );
            selection.UserData = (void*)&display_ranges;
            selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* self, int index) -> ImGuiID {
                // return index;
                return (*(std::vector<display_instr>*)self->UserData)[index].address;
            };
            selection.ApplyRequests(ms_io);
            // info("%d", selection.Size);
            if(ImGui::IsKeyChordPressed(ImGuiKey_C | ImGuiKey_ModCtrl) && selection.Size > 0)
                selection.WriteSelectionToClipboard(display_ranges);

            ImGuiListClipper clipper;
            clipper.Begin(display_ranges.size());

            if (focus_addr != -1) {
                uint16_t tmp = history.back();
                auto it = std::find_if(display_ranges.begin(), display_ranges.end(), [](const display_instr& i){ return i.address == focus_addr; });
                if(it != display_ranges.end())
                    ImGui::SetScrollY(ImGui::GetTextLineHeightWithSpacing() * std::distance(display_ranges.begin(), it));
                focus_addr = -1;
            }

            while (clipper.Step())
            {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                    const display_instr& _instr = display_ranges[i];
                    ImGui::SetNextItemSelectionUserData(i);
                    if(_instr.is_separator) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(" ");
                        continue;
                    }
                    const DisassembleInstr& instr = parsed.at(_instr.address);
                    bool selected = selection.Contains(instr.address);
                    bool is_highlight = _instr.address == DeviceResources::CPU.pc || selected;

                    DrawDisassembleRow(instr, is_highlight);

                    if(stop_access) {
                        stop_access = false;
                        break;
                    }
                }
            }
            clipper.End();

            ms_io = ImGui::EndMultiSelect();
            selection.ApplyRequests(ms_io);

            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

void FocusAddress(uint16_t address, bool highlight)
{
    focus_addr = address;
    if(highlight) {
        highlight_addr = address;
        highlight_start = ImGui::GetTime();
    }
}

const std::vector<display_instr>& GetInstructionDisplayRange() {
    return display_ranges;
}

std::vector<uint16_t>& GetDisassemblerViewHistory() {
    return history;
}
};
