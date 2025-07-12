#include "DisassemblerView/InstructionTable.hpp"

#include <vector>
#include <algorithm>

#include "imgui.h"

#include "DisassemblerView/Core.hpp"
#include "DisassemblerView/InstructionTableMultiselect.hpp"
#include "DisassemblerView/Main.hpp"
#include "DisassemblerView/ImGuiModified.hpp"
#include "DisassemblerView/Breakpoint.hpp"

#include "Z80Disassembler.h"
#include "Internal.h"
#include "UIHelpers.hpp"
#include "DeviceResources.hpp"
#include "imgui_internal.h"

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
        uint16_t test_addr = address - i;
        if(test_addr < bytes_range.x)
            break;
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

void DrawDisassembleRow(const DisassembleInstr& row, bool is_pc, ImDrawList* draw_list)
{
    ImGui::TableNextRow();
    ImGui::SameLine();
    ImGui::TableNextColumn();
    
    DisassemblerView::DrawBreakpointButton(row.address, draw_list);
    ImGui::TableNextColumn();

    ImGui::TextUnformatted(row.address == DeviceResources::CPU.pc ? ">" : " ");
    ImGui::TableNextColumn();

    ImGui::PushID(row.address);
    DisassemblerView::Selectable("##row_select", highlight_addr == row.address || is_pc);
    ImGui::PopID();
    ImGui::SameLine();

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

INLINE void DrawTableBorderWithIndex(ImRect winrect, ImGuiTable* table, ImDrawList* draw_list, int index) {
    const float linex = table->Columns[index].MaxX;
    draw_list->AddLine(
        ImVec2(linex, winrect.Min.y),
        ImVec2(linex, winrect.Max.y),
        ImGui::GetColorU32(ImGuiCol_Border)
    );
}

INLINE void DrawTableBorder(ImDrawList* draw_list)
{
    ImGuiTable* table = ImGui::GetCurrentTable();
    ImRect winrect = ImGui::GetCurrentWindow()->Rect();

    DrawTableBorderWithIndex(winrect, table, draw_list, 0);
    DrawTableBorderWithIndex(winrect, table, draw_list, 1);
}

namespace DisassemblerView
{
static bool stop_access = false; // to prevent out-of-bound access/SIGSEGV when the address doesn't in current display ranges
static bool out_of_range = false;
void UpdateDisplayRange(uint16_t address, bool push)
{
    out_of_range = false;
    if(parsed.find(address) == parsed.end())
        out_of_range = true;
    if(!history.empty())
        if(in_range(history.back(), display_addr_end, address))
            return;
    uint16_t display_addr_start = address;
    while (FindPreviousInstruction(display_addr_start)) {}
    // display_addr_start++;

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

void DrawTableContent()
{
    if (ImGui::BeginTable("##instr_view", 6))
    {
        ImGui::TableSetupColumn("##breakpoints_col", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("##pc_col", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("##address_col", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("##bytes_col");
        ImGui::TableSetupColumn("##instrs_col");
        ImGui::TableSetupColumn("##operands_col");

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, ImGui::GetStyle().FramePadding.y));

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
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
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

                DrawDisassembleRow(instr, is_highlight, draw_list);

                if(stop_access) {
                    stop_access = false;
                    break;
                }
            }
        }
        clipper.End();

        ms_io = ImGui::EndMultiSelect();
        selection.ApplyRequests(ms_io);

        ImGui::PopStyleVar();

        DrawTableBorder(draw_list);

        ImGui::EndTable();
    }
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
        DisplayCenteredText("Please load a disk by open menu File->Open.");
    else {
        if(out_of_range)
            DisplayCenteredText("No such instructions exist at given address");
        else DrawTableContent();
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
