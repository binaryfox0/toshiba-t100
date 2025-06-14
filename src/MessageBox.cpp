#include "MessageBox.hpp"
#include <string.h>

#include "imgui.h"

bool show_messagebox = false;
char messagebox_title[64] = {0};
char messagebox_content[64] = {0};

void SetMessageBoxContent(const char* title, const char* content) {
    strncpy(messagebox_title,  title, sizeof(messagebox_title));
    strncpy(messagebox_content,  content, sizeof(messagebox_content));
}
void DrawMessageBox()
{
    if (show_messagebox)
        ImGui::OpenPopup(messagebox_title);  // only triggers once

    // Set a larger default size
    const ImVec2 popup_size = ImVec2(500, 200);

    // Center popup on screen
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(popup_size, ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(messagebox_title, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
        ImGui::Spacing();
        ImGui::TextWrapped("%s", messagebox_content);
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        ImVec2 buttonSize = ImVec2(120, 0);
        float width = ImGui::GetWindowSize().x;
        float buttonPos = (width - buttonSize.x) * 0.5f; // center

        ImGui::SetCursorPosX(buttonPos);
        if (ImGui::Button("OK", buttonSize)) {
            ImGui::CloseCurrentPopup();
            show_messagebox = false;
        }

        ImGui::Spacing();
        ImGui::EndPopup();
    }
}