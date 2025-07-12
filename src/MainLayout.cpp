#include "MainLayout.hpp"

#include "UIHelpers.hpp"
#include "Splitter.hpp"
#include "SidePanel.hpp"
#include "DisassemblerView/Main.hpp"
#include "MemoryView/Main.hpp"
#include "EventLog.hpp"

#include "imgui.h"

bool mainlayout_show = true;

static Splitter main_splitter = Splitter("##mainpanel_splitter", [](const float size, Splitter* sp){
    ImGui::BeginChild("##mainpanel_top", ImVec2(0, size));
    if(ImGui::BeginTabBar("##mainpanel_tab"))
    {
        if(ImGui::BeginTabItem("Disassembler")) {
            DisassemblerView::Draw(0);
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Memory")) {
            MemoryView::Draw();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::EndChild();
}, DrawEventLog, 0.7, {}, {0.3f, 0.7f}, false, 4);

void DrawMainPanel(const float size, Splitter* sp);
static Splitter splitter("##mainsplitter", 
    DrawSidePanel, DrawMainPanel, 0.25f, {}, {0.25f, 0.75f}, true, 2);


static ImVec2 window_size = {};
void UpdateMainLayoutSize(const ImVec2 size) {
    window_size = size;
    splitter.UpdateSize(size.x);
    main_splitter.UpdateSize(size.y);
    UpdateSidePanel(size.y);
}

void DrawMainPanel(const float size, Splitter* sp)
{
    ImGui::BeginChild("##main_panel", {size, 0}, ImGuiChildFlags_Borders);
    main_splitter.Draw();
    // DisassemblerView::Draw(0);
    ImGui::EndChild();
}

void DrawMainLayout()
{
    static const auto flags = 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::SetNextWindowPos({0, ImGui::GetFrameHeight()});
    ImGui::SetNextWindowSize(window_size);

    ImGui::Begin("##main_layout", 0, flags);
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());

    splitter.Draw();

    // ImGui::PopStyleVar();
    ImGui::End();
}