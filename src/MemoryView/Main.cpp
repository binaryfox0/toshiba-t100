#include "MemoryView/Main.hpp"

#include "imgui.h"

#include "Internal.h"
#include "MemoryView/Helpers.hpp"
#include "DeviceResources.hpp"

static const auto& mem = DeviceResources::RAM;

static uint32_t cursor_pos = 0;
static bool need_scroll = 0;

INLINE void OpenGotoPopup()
{
    if(!ImGui::IsPopupOpen("##mem_goto_popup")) {
        ImVec2 winpos = ImGui::GetWindowPos();
        ImVec2 popup_pos = {winpos.x + ImGui::GetWindowWidth() * 0.5f, winpos.y};
        ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Always, ImVec2(0.5, 0.5f));
        ImGui::OpenPopup("##mem_goto_popup");
    }
}

INLINE void DrawGotoPopup()
{
    if(ImGui::BeginPopup("##mem_goto_popup")) {
        static char offset_buf[12] = {0}; // sign (+/-) + number (0xXXXXXXXX / XXXXXXXXXX)
        ImGui::TextUnformatted("Jump to byte (enter offset): ");
        ImGui::SameLine();
        ImGui::SetKeyboardFocusHere();
        if(ImGui::InputText("##offset_input", offset_buf, sizeof(offset_buf), 
        ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue)) {
            need_scroll = MemoryView::CheckNumberRadix(offset_buf, &cursor_pos);
            memset(offset_buf, 0, sizeof(offset_buf));
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}


INLINE void DrawMemoryRow(int address) {
    std::ostringstream ss;
    ss << to_hex<unsigned int>(address, false) << ": ";
    int upper_limit = address + std::min<int>(16, sizeof(mem) - address); 
    for(int i = address; i < upper_limit; i++) {
        ss << to_hex(mem[i], false) << " ";
    }
    ss << "    ";
    for(int i = address; i < upper_limit; i++) {
        char c = mem[i];
        ss << (std::isprint(c) ? std::string(1, c) : ".");
    }
    ImGui::TextUnformatted(ss.str().c_str());
}

namespace MemoryView
{
void Draw()
{
    ImGui::BeginChild("##memoryview_content", {}, ImGuiChildFlags_Borders);
    if(ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_G))
        OpenGotoPopup();
    DrawGotoPopup();

    if(need_scroll) {
        ImGui::SetScrollY(ImGui::GetTextLineHeightWithSpacing() * (long)(cursor_pos / 16));
        need_scroll = false;
    }

    ImGuiListClipper clipper;
    clipper.Begin(sizeof(mem) / 16);
    while(clipper.Step()) {
        for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            DrawMemoryRow(i * 16);
    }
    ImGui::EndChild();
}
}