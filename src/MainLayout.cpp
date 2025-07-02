#include "MainLayout.hpp"

#include "UIHelpers.hpp"
#include "SidePanel.hpp"
#include "DisassemblerView/Main.hpp"
#include "imgui.h"

static ImVec2 window_size = {};
static ImVec2 panel_size = {};
static float ratio = 0.25f;
void UpdateMainLayoutSize(const ImVec2 size) {
    window_size = size;
    CalculateSplitterPanel(&panel_size, &ratio, window_size.x, true);
    UpdateSidePanel(ImVec2(panel_size.x, window_size.y));
}

static ImVec2 mainpanel_size = {};
static ImVec2 mainpanel_panel_size = {};
static float mainpanel_ratio = 0.7f;
void UpdateMainPanelSize(const ImVec2 size) {
    CalculateSplitterPanel(&mainpanel_panel_size, &mainpanel_ratio, mainpanel_size.y, false);
}

// void DrawMainPanel(const float size, uint8_t*)
// {
    // ImGui::BeginChild("##main_panel", {size, 0});
    // DrawSplitter(3, &mainpanel_panel_size, &mainpanel_ratio, )
    // ImGui::EndChild();
// }

void DrawMainLayout()
{
    static const auto flags = 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGui::SetNextWindowPos({0, ImGui::GetFrameHeight()});
    ImGui::SetNextWindowSize(window_size);

    ImGui::Begin("##main_layout", 0, flags);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());

    DrawSplitter(2, &panel_size, &ratio, window_size.x, true, DrawSidePanel, 0, DisassemblerView::Draw, 0, ImVec2(0.25f, 0.75f));

    ImGui::PopStyleVar();
    ImGui::End();
}