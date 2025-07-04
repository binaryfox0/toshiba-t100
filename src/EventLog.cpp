#include "EventLog.hpp"

#include <vector>
#include <string>
#include <chrono>

#include <stdio.h>
#include <stdint.h>

#include "UIHelpers.hpp"
#include "ResourceManager.hpp"
#include "MessageBox.hpp"

#include "imgui.h"
#include "nfd.h"


// Why they make the way to access function so long
using Clock = std::chrono::high_resolution_clock;

struct event_entry {
    uint64_t ms;
    std::string content;
};
static std::vector<event_entry> events = {};
static Clock::time_point start_tp;

bool eventlog_show = false;

void ResetEventClock() {
    start_tp = Clock::now();
}

void AddNewEvent(const std::string &content) {
    events.push_back({static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_tp).count()), content});
}

void ClearEventLog() {
    events.clear();
}

void SaveEventLog(const char* path) {
    FILE* file = fopen(path, "w");
    for(const auto& event : events) {
        uint64_t ms = event.ms;
        int hours   = static_cast<int>(ms / (1000 * 60 * 60));
        int minutes = static_cast<int>((ms / (1000 * 60)) % 60);
        int seconds = static_cast<int>((ms / 1000) % 60);
        int millis  = static_cast<int>(ms % 10000); // 4-digit precision

        fprintf(file, "[%02d:%02d:%02d.%04d]: %s\n", hours, minutes, seconds, millis, event.content.c_str());
    }
    fclose(file);
}

void DrawEventLog(const float size, uint8_t*)
{
    ImGui::BeginChild("##eventpanel", {0, size}, ImGuiChildFlags_Borders, ImGuiWindowFlags_MenuBar);
    if(ImGui::BeginMenuBar()) {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Save")) {
                nfdchar_t *outPath = NULL;
                nfdresult_t result = NFD_SaveDialog(&outPath, 0, 0, 0, 0);
                if ( result == NFD_OKAY ) {
                    SaveEventLog(outPath);
                    NFD_FreePathU8(outPath);
                }
                else if(result != NFD_CANCEL){
                    CreateMessageBox("Error", NFD_GetError());
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImageResource &icon = ResourceManager::GetImage("delete_forever.png");
    if(ImGui::ImageButton("##delete_forever", icon.texture, icon.size)) {
        events.clear();
        ResetEventClock();
    }
    ImGui::SameLine();
    ImGui::Text("Total event count: %lu\n", events.size());
    ImGui::BeginChild("##events", ImVec2(0, 0), ImGuiChildFlags_Borders);
    if(events.empty())
        DisplayCenteredText("No event was recorded");
    else {
        bool is_at_bottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;
        ImGuiListClipper clipper;
        clipper.Begin(events.size());
        while(clipper.Step()) {
            for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                uint64_t ms = events[i].ms;
                int hours   = static_cast<int>(ms / (1000 * 60 * 60));
                int minutes = static_cast<int>((ms / (1000 * 60)) % 60);
                int seconds = static_cast<int>((ms / 1000) % 60);
                int millis  = static_cast<int>(ms % 10000); // 4-digit precision

                ImGui::Text("[%02d:%02d:%02d.%04d]: %s", hours, minutes, seconds, millis, events[i].content.c_str());
            }
        }
        if(is_at_bottom)
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::EndChild();
}