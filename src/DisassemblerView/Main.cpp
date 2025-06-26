#include "DisassemblerView/Main.hpp"

#include "DisassemblerView/Core.hpp"
#include "DisassemblerView/InstructionTable.hpp"
#include "DisassemblerView/UnselectableTable.hpp"
#include "UIHelpers.hpp"
#include "EventLog.hpp"
#include "DeviceResources.hpp"
#include "ResourceManager.hpp"

#include "Z80Disassembler.h"

#include "imgui.h"

static float scroll_synced = 0.0f;

static auto& history  = DisassemblerView::GetDisassemblerViewHistory();
static const auto& parsed = DisassemblerView::GetDisassemblerParsedGeneral();
static const auto& memory = DisassemblerView::GetDisassemblerMemory();

static float text_input_size = 0.0f;
INLINE void ResizeCheck() {
    static ImVec2 last_size = ImVec2();
    ImVec2 current_size = ImGui::GetContentRegionAvail();
    if(last_size.x != current_size.x || last_size.y != current_size.y) {
        text_input_size = current_size.x;
        last_size = current_size;
    }
}

static const char* pauseresume_active_img = "resume_normal.png";
INLINE void UpdatePauseResumeButtonImage() {
    if(DeviceResources::CPUPause) pauseresume_active_img = "resume_normal.png";
    else pauseresume_active_img = "pause_normal.png";
}

void DrawToolbar()
{
    if(DrawInactiveableButton(
        "arrow_back_normal.png", "arrow_back_grayed.png", true,
        history.size() > 1) || ImGui::Shortcut(ImGuiKey_Escape)
    ) {
        DisassemblerView::UpdateDisplayRange(history[history.size() - 2], false);
        history.pop_back();
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("Go to previous location (Esc)");
    ImGui::SameLine();
    static const char* onoff_active_img = "play_arrow_normal.png";
    static bool cpu_start = false;
    if(DrawInactiveableButton(onoff_active_img, "", true, true) ||
        ImGui::Shortcut(ImGuiKey_F5)
    ) {
        if((cpu_start = !cpu_start)) {
            onoff_active_img = "stop_normal.png";
            AddNewEvent("Machine started!");
        } else {
            UpdatePauseResumeButtonImage();
            DeviceResources::ResetCPU();
            onoff_active_img = "play_arrow_normal.png";
            AddNewEvent("Machine reset!");
        }
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip(cpu_start ? "Stop machine (F5)" : "Start machine (F5)");
    ImGui::SameLine();

    if(DrawInactiveableButton(pauseresume_active_img, "resume_grayed.png", true, cpu_start) ||
        ImGui::Shortcut(ImGuiKey_F8)
    ) {
        DeviceResources::CPUPause = !DeviceResources::CPUPause;
        UpdatePauseResumeButtonImage();
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip(DeviceResources::CPUPause ? "Resume machine (F8)" : "Pause machine (F8)");
    ImGui::SameLine();

    if(DrawInactiveableButton("step_into_normal.png", "step_into_grayed.png", true, cpu_start && DeviceResources::CPUPause) ||
        ImGui::Shortcut(ImGuiKey_F11)
    ) {
        z80& z80_cpu = DeviceResources::CPU;
        uint16_t old_pc = z80_cpu.pc;
        z80_step(&z80_cpu);
        DisassemblerView::FocusAddress(z80_cpu.pc, false);
        if(z80_cpu.pc != old_pc && z80_cpu.pc != old_pc + parsed.at(old_pc).size)
        {
            if(z80_get_type(memory + old_pc) == RET && history.size() > 1) {
                DisassemblerView::UpdateDisplayRange(history[history.size() - 2], false);
                history.pop_back();
            }
            else {
                DisassemblerView::UpdateDisplayRange(z80_cpu.pc, true);
            }
        }
    }
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("Step Into (F11)");
    ImGui::SameLine();

    static char str[5] = "";
    if([&]() -> bool {
        ImageResource& img = ResourceManager::GetImage("jump_to_element.png");
        if(ImGui::ImageButton("##jumptoelement", img.texture, img.size))
            return true;
        ImGui::SameLine();
        ResizeCheck();
        ImGui::SetNextItemWidth(text_input_size);
        if(ImGui::InputTextWithHint("##address", "<hex address>", str, sizeof(str), 
        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal))
            return true;
        return false;
    }()) {
        uint16_t goto_address = strtol(str, 0, 16);
        DisassemblerView::FocusAddress(goto_address, true);
        DisassemblerView::UpdateDisplayRange(goto_address, true);
    }
}

void BreakpointHandle(uint16_t address) {
    DisassemblerView::UpdateDisplayRange(address, true);
    DisassemblerView::FocusAddress(address, false);
    UpdatePauseResumeButtonImage();
}

namespace DisassemblerView
{
void Init(const uint8_t* disassembler_memory, const uint32_t size, const uint16_t load_address)
{
    SetupDisassembler(disassembler_memory, size, load_address);
    InitInstructionTable();
    DeviceResources::BreakHandle = BreakpointHandle;
}

void Draw()
{
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Disassembler");

    DrawToolbar();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImVec4 table_bg_col = ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg); // making button invisible
    ImGui::PushStyleColor(ImGuiCol_Button, table_bg_col);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, table_bg_col);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, table_bg_col);

    ImGui::BeginChild("##disassembler_content", ImVec2(), ImGuiChildFlags_Borders);
    DrawUnselectableTable();
    ImGui::SameLine(0.0f, 0.0f);
    DrawInstructionTable();
    ImGui::EndChild();

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    ImGui::End();
}

float& GetScrollSynced() {
    return scroll_synced;
}
};