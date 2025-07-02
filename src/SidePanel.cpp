#include "SidePanel.hpp"

#include <vector>
#include <algorithm>

#include "DisassemblerView/InstructionTable.hpp"
#include "DisassemblerView/UnselectableTable.hpp"
#include "DeviceResources.hpp"
#include "Internal.h"
#include "UIHelpers.hpp"

#include "imgui.h"

static std::vector<std::pair<const uint16_t, bool>*> bpoints_sorted; // 8n size
void CheckBreakpointListChanged()
{
    static size_t last_size = 0;
    size_t current_size = DeviceResources::CPUBreak.size();

    if (last_size != current_size) {
        bpoints_sorted.clear();
        bpoints_sorted.reserve(DeviceResources::CPUBreak.size());

        for (auto& entry : DeviceResources::CPUBreak) {
            bpoints_sorted.push_back(&entry); // store pointer to map entry
        }

        std::sort(bpoints_sorted.begin(), bpoints_sorted.end(),
                  [](const auto* a, const auto* b) {
                      return a->first < b->first;
                  });
        last_size = current_size;
    }
}

static z80* z80_cpu = &DeviceResources::CPU;
INLINE uint8_t GenerateFlagByte() {
    uint8_t val = 0;
    val |= z80_cpu->cf << 0;
    val |= z80_cpu->nf << 1;
    val |= z80_cpu->pf << 2;
    val |= z80_cpu->xf << 3;
    val |= z80_cpu->hf << 4;
    val |= z80_cpu->yf << 5;
    val |= z80_cpu->zf << 6;
    val |= z80_cpu->sf << 7;
    return val;
}

void DrawRegistersView(const float size, uint8_t*)
{
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
    ImGui::BeginChild("##registerview_child", ImVec2(0, size), ImGuiChildFlags_Border);
    
    if(!DeviceResources::MachineStarted) {
        DisplayCenteredText("Please start machine to view CPU registers");

        // ImGui::PopStyleVar();
        ImGui::EndChild();
        return;
    }

    if (ImGui::BeginTable("##registers_view", 2, 
        ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerH
    ))
    {
        ImGui::TableSetupColumn("Register", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        struct TreeNode {
            const char* name;
            std::string string;
            std::vector<TreeNode> children;
            bool span_all_cols = false;

            void DisplayNode() const {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAllColumns | (span_all_cols ? ImGuiTreeNodeFlags_LabelSpanAllColumns : ImGuiTreeNodeFlags_None);

                if (!children.empty()) {
                    const bool open = ImGui::TreeNodeEx(name, flags | ImGuiTreeNodeFlags_DefaultOpen);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(string.c_str()); // Always show the value, even if collapsed

                    if (open) {
                        for (const auto& child : children) {
                            child.DisplayNode();
                        }
                        ImGui::TreePop();
                    }
                } else {
                    ImGui::TreeNodeEx(name, flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(string.c_str());
                }
            }
        };

        // Build data tree
        uint8_t f = GenerateFlagByte();
        TreeNode node_root[] = {
            {
                "Cycle count", to_hex<uint32_t>(z80_cpu->cyc), {}, false
            },
            {
                "Special-purpose registers", "",
                {
                    {"PC", to_hex<uint16_t>(z80_cpu->pc)},
                    {"SP", to_hex<uint16_t>(z80_cpu->sp)},
                    {"IX", to_hex<uint16_t>(z80_cpu->ix)},
                    {"IY", to_hex<uint16_t>(z80_cpu->iy)},
                },
                true
            },
            {
                "Main registers", "",
                {
                    {"A", to_hex<uint8_t>(z80_cpu->a)},
                    {"B", to_hex<uint8_t>(z80_cpu->b)},
                    {"C", to_hex<uint8_t>(z80_cpu->c)},
                    {"D", to_hex<uint8_t>(z80_cpu->d)},
                    {"E", to_hex<uint8_t>(z80_cpu->e)},
                    {
                        "F", to_hex<uint8_t>(f),
                        {
                            {"SF", to_bin(z80_cpu->sf)},
                            {"ZF", to_bin(z80_cpu->zf)},
                            {"YF", to_bin(z80_cpu->yf)},
                            {"HF", to_bin(z80_cpu->hf)},
                            {"XF", to_bin(z80_cpu->xf)},
                            {"PF", to_bin(z80_cpu->pf)},
                            {"NF", to_bin(z80_cpu->nf)},
                            {"CF", to_bin(z80_cpu->cf)},
                        }
                    },
                    {"H", to_hex<uint8_t>(z80_cpu->h)},
                    {"L", to_hex<uint8_t>(z80_cpu->l)},
                },
                true
            },
            {
                "Register pairs", "",
                {
                    {"AF", to_hex<uint16_t>(z80_cpu->a << 8 | f)},
                    {"BC", to_hex<uint16_t>(z80_cpu->b << 8 | z80_cpu->c)},
                    {"DE", to_hex<uint16_t>(z80_cpu->d << 8 | z80_cpu->e)},
                    {"HL", to_hex<uint16_t>(z80_cpu->h << 8 | z80_cpu->l)}
                },
                true
            },
        };

        for (const auto& node : node_root)
            node.DisplayNode();

        ImGui::EndTable();
    }
    ImGui::EndChild();
    // ImGui::PopStyleVar();
}


void DrawBreakpointPanel(const float size, uint8_t*)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImGuiStyle().WindowPadding);
    ImGui::BeginChild("##breakpoints_table_child", {0, size}, ImGuiChildFlags_Borders);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_TableRowBg));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
    if(ImGui::BeginTable("##breakpoints_table", 3))
    {
        ImGui::TableSetupColumn("##breakpoint_disable-able_col", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("##breakpoint_disabled_checkbox-col", ImGuiTableColumnFlags_WidthFixed);

        ImGuiListClipper clipper;
        clipper.Begin(bpoints_sorted.size());
        while(clipper.Step())
        {
            for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            {
                const auto pair = bpoints_sorted[i];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextUnformatted("-");
                DisassemblerView::DrawBreakpointCircle(draw_list, pair->second ? DisassemblerView::breakpoint_activate : DisassemblerView::breakpoint_disabled);
                ImGui::TableNextColumn();
                ImGui::PushID(pair->first);
                ImGui::Checkbox("##checkbox", &pair->second);
                ImGui::PopID();
                ImGui::TableNextColumn();
                if(DrawHyperlinkButton((to_hex(pair->first)).c_str()))
                {
                    DisassemblerView::UpdateDisplayRange(pair->first, true);
                    DisassemblerView::FocusAddress(pair->first, true);
                }
                // ImGui::Text("%04X", pair->first);
            }
        }
        ImGui::EndTable();

    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);

    ImGui::EndChild();
    ImGui::PopStyleVar();
}

static const float splitter_width = 4.0f;
static float ratio = 0.5f;
static ImVec2 window_size = ImVec2(310, 0);
static ImVec2 panel_size = ImVec2(310, 0);

void UpdateSidePanel(const ImVec2 size)
{
    window_size = size;
    CalculateSplitterPanel(&panel_size, &ratio, window_size.y, false, ImVec2(0.25f, 0.75f));
}

void DrawSidePanel(const float size, uint8_t*)
{
    // UpdateSidePanel(ImVec2(size, window_size.y));
    CheckBreakpointListChanged();

    ImGui::BeginChild("##sidepanel", {size, 0});

    DrawSplitter(1, 
        &panel_size, &ratio, 
        window_size.y, false, 
        DrawRegistersView, 0, DrawBreakpointPanel, 0, 
        ImVec2(0.25f, 0.75f)
    );

    ImGui::EndChild();
}
