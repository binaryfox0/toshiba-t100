#include "EventLog.hpp"

#include <vector>
#include <string>
#include <chrono>

#include <stdint.h>

#include "imgui.h"

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

void DrawEventLog()
{
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Event Log");
    if(events.empty())
    {
        const char* text = "No event was recorded.";
        ImVec2 avail = ImGui::GetWindowSize();
        ImVec2 text_size = ImGui::CalcTextSize(text);

        ImGui::SetCursorPos(ImVec2(
            (avail.x - text_size.x) * 0.5f,
            (avail.y - text_size.y) * 0.5f
        ));
        ImGui::TextUnformatted(text);
    }
    else {
        bool is_at_bottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;
        for(const auto& event : events)
        {
            uint64_t ms = event.ms;
            int hours   = static_cast<int>(ms / (1000 * 60 * 60));
            int minutes = static_cast<int>((ms / (1000 * 60)) % 60);
            int seconds = static_cast<int>((ms / 1000) % 60);
            int millis  = static_cast<int>(ms % 10000); // 4-digit precision

            ImGui::Text("[%02d:%02d:%02d.%04d]: %s", hours, minutes, seconds, millis, event.content.c_str());
        }
        if(is_at_bottom)
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::End();
}