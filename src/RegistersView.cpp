#include "RegistersView.hpp"

#include <vector>

#include "Internal.h"
#include "DeviceEmulator.hpp"

#include "imgui.h"

bool registersview_show = false;
INLINE uint8_t GenerateFlagByte() {
    uint8_t val = 0;
    val |= z80_cpu.cf << 0;
    val |= z80_cpu.nf << 1;
    val |= z80_cpu.pf << 2;
    val |= z80_cpu.xf << 3;
    val |= z80_cpu.hf << 4;
    val |= z80_cpu.yf << 5;
    val |= z80_cpu.zf << 6;
    val |= z80_cpu.sf << 7;
    return val;
}
void DrawRegistersView()
{
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Registers");

    if (ImGui::BeginTable("##registers_view", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody))
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
                    const bool open = ImGui::TreeNodeEx(name, flags);
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
                "Cycle count", to_hex<uint32_t>(z80_cpu.cyc), {}, false
            },
            {
                "Special-purpose registers", "",
                {
                    {"PC", to_hex<uint16_t>(z80_cpu.pc)},
                    {"SP", to_hex<uint16_t>(z80_cpu.sp)},
                    {"IX", to_hex<uint16_t>(z80_cpu.ix)},
                    {"IY", to_hex<uint16_t>(z80_cpu.iy)},
                },
                true
            },
            {
                "Main registers", "",
                {
                    {"A", to_hex<uint8_t>(z80_cpu.a)},
                    {"B", to_hex<uint8_t>(z80_cpu.b)},
                    {"C", to_hex<uint8_t>(z80_cpu.c)},
                    {"D", to_hex<uint8_t>(z80_cpu.d)},
                    {"E", to_hex<uint8_t>(z80_cpu.e)},
                    {
                        "F", to_hex<uint8_t>(f),
                        {
                            {"SF", to_bin(z80_cpu.sf)},
                            {"ZF", to_bin(z80_cpu.zf)},
                            {"YF", to_bin(z80_cpu.yf)},
                            {"HF", to_bin(z80_cpu.hf)},
                            {"XF", to_bin(z80_cpu.xf)},
                            {"PF", to_bin(z80_cpu.pf)},
                            {"NF", to_bin(z80_cpu.nf)},
                            {"CF", to_bin(z80_cpu.cf)},
                        }
                    },
                    {"H", to_hex<uint8_t>(z80_cpu.h)},
                    {"L", to_hex<uint8_t>(z80_cpu.l)},
                },
                true
            },
            {
                "Register pairs", "",
                {
                    {"AF", to_hex<uint16_t>(z80_cpu.a << 8 | f)},
                    {"BC", to_hex<uint16_t>(z80_cpu.b << 8 | z80_cpu.c)},
                    {"DE", to_hex<uint16_t>(z80_cpu.d << 8 | z80_cpu.e)},
                    {"HL", to_hex<uint16_t>(z80_cpu.h << 8 | z80_cpu.l)}
                },
                true
            },
        };

        for (const auto& node : node_root)
            node.DisplayNode();

        ImGui::EndTable();
    }

    ImGui::End();
}
