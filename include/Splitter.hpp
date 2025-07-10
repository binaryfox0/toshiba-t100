#ifndef SPLITTER_HPP
#define SPLITTER_HPP

#include "Internal.h"

#include "imgui_internal.h"

class Splitter {
public:
    typedef void (*panel_draw_func)(const float, Splitter*);

    Splitter() :
        panel1_draw(0), panel2_draw(0), panel_size() {}
    Splitter(
        const char* str_id,
        panel_draw_func draw_panel1, panel_draw_func draw_panel2,
        const float initial_ratio, const float initial_size,
        const __Vec2 ratio_range,
        const bool vertical,
        const int padding_count = 2
    ) :
        panel1_draw(draw_panel1), panel2_draw(draw_panel2),
        ratio(initial_ratio), vertical(vertical), ratio_range(ratio_range), 
        padding_count(padding_count) {
            UpdateSize(initial_size);
            id = ImHashStr(str_id);
    }

    static void InitStyle();

    void Draw();
    void UpdateSize(const float new_size);

    float window_size;
    float ratio;

private:
    static ImGuiStyle* style;
    
    ImGuiID id;
    
    int padding_count;
    panel_draw_func panel1_draw, panel2_draw;
    __Vec2 ratio_range, panel_size;
    bool vertical;
};

#endif