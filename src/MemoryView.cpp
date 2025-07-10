#include "MemoryView.hpp"

#include <ctype.h>
#include <string.h>

#include <sstream>

#include "imgui.h"

#include "Internal.h"
#include "DeviceResources.hpp"

static auto& mem = DeviceResources::RAM;

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

INLINE bool supported_base(const char* str, uint32_t* out_num)
{
    int out_base = 0;
    const char* backup = str;
    bool add = (str[0] == '-' || str[0] == '+');
    if(add)
        str++;
    if(str[0] == '0') {
        if(tolower(str[1]) == 'x') {
            out_base = 16;
            str += 2;
        } else
            out_base = 10;
    }
    auto func = out_base == 10 ? isdigit : isxdigit;
    char c;
    while(!isspace(c = *str) && c) {
        if(!func(c))
            return false;
        str++;
    }
    if(add)
        *out_num += strtol(backup, 0, out_base);
    else
        *out_num = strtol(backup, 0, out_base);
    return true;
}

static uint32_t cursor_offset = 0;
void DrawMemoryView()
{
    ImGui::BeginChild("##memoryview_content");
    if(ImGui::IsWindowFocused() && ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_G)) {
            if(!ImGui::IsPopupOpen("##mem_goto_popup")) {
            ImVec2 winpos = ImGui::GetWindowPos();
            ImVec2 popup_pos = {winpos.x + ImGui::GetWindowWidth() * 0.5f, winpos.y};
            ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Always, ImVec2(0.5, 0.5f));
            // ImGui::SetNextWindowSize(ImVec2(ImGui::GetWindowWidth(), 0), ImGuiCond_Always);
            ImGui::OpenPopup("##mem_goto_popup");
        }
    }
    static bool need_scroll = false;
    if(ImGui::BeginPopup("##mem_goto_popup")) {
        static char offset_buf[12] = {0}; // sign (+/-) + number (0xXXXXXXXX / XXXXXXXXXX)
        ImGui::TextUnformatted("Jump to byte (enter offset): ");
        ImGui::SameLine();
        ImGui::SetKeyboardFocusHere();
        if(ImGui::InputText("##offset_input", offset_buf, sizeof(offset_buf), 
        ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue)) {
            need_scroll = supported_base(offset_buf, &cursor_offset);
            memset(offset_buf, 0, sizeof(offset_buf));
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if(need_scroll) {
        ImGui::SetScrollY(ImGui::GetTextLineHeightWithSpacing() * (long)(cursor_offset / 16));
        need_scroll = false;
    }

    ImGuiListClipper clipper;
    clipper.Begin(sizeof(mem) / 16);
    while(clipper.Step()) {
        for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            int row_address = i * 16;
            DrawMemoryRow(row_address);
        }
    }
    ImGui::EndChild();
}