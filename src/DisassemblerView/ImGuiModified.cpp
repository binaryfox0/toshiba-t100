#include "DisassemblerView/ImGuiModified.hpp"

#include "imgui_internal.h"

// Strpped from imgui internal, commit 89b5a2c3d50e4ca6a0a88378f096d7d05ff1c962
// Default flag ImGuiSelectableFlags_AllowItemOverlap/AllowOverlap (renamed), customized version of ImGuiSelectableFlags_SpanAllColumns
namespace DisassemblerView {
bool Selectable(const char* label, bool selected)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id = window->GetID(label);
    ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImVec2 size(label_size.x, label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ImGui::ItemSize(size, 0.0f);
    
    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
    // const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const bool span_all_columns = true;
    // const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float min_x = pos.x;
    const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
    size.x = ImMax(label_size.x, max_x - min_x);

    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    // FIXME: Not part of layout so not included in clipper calculation, but ItemSize currently doesn't allow offsetting CursorPos.

    ImRect bb(min_x, pos.y, min_x + size.x, pos.y + size.y);
    // const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
    const float spacing_x = style.ItemSpacing.x;
    const float spacing_y = style.ItemSpacing.y;
    const float spacing_L = IM_TRUNC(spacing_x * 0.5f);
    const float spacing_U = IM_TRUNC(spacing_y * 0.50f);
    bb.Min.x -= spacing_L;
    bb.Min.y -= spacing_U;
    // bb.Max.x += (spacing_x - spacing_L);
    bb.Max.y += (spacing_y - spacing_U);

    //if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    bool is_visible;
    // if (spn_all_columns)
    // {
        // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackgroundChannel for every Selectable..
        const float backup_clip_rect_min_x = window->ClipRect.Min.x;
        const float backup_clip_rect_max_x = window->ClipRect.Max.x;
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
        is_visible = ImGui::ItemAdd(bb, id, NULL);
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    // }

    const bool is_multi_select = (g.LastItemData.ItemFlags & ImGuiItemFlags_IsMultiSelect) != 0;
    if (!is_visible)
        if (!is_multi_select || !g.BoxSelectState.UnclipMode || !g.BoxSelectState.UnclipRect.Overlaps(bb)) // Extra layer of "no logic clip" for box-select support (would be more overhead to add to ItemAdd)
            return false;


    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
    // which would be advantageous since most selectable are not selected.
    if (span_all_columns)
    {
        if (g.CurrentTable)
            ImGui::TablePushBackgroundChannel();
        else if (window->DC.CurrentColumns)
            ImGui::PushColumnsBackground();
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasClipRect;
        g.LastItemData.ClipRect = window->ClipRect;
    }
    
    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = ImGuiButtonFlags_AllowOverlap;
    // if ((flags & ImGuiSelectableFlags_AllowOverlap) || (g.LastItemData.ItemFlags & ImGuiItemFlags_AllowOverlap)) { button_flags |= ImGuiButtonFlags_AllowOverlap; }

    // Multi-selection support (header)
    const bool was_selected = selected;
    if (is_multi_select)
    {
        // Handle multi-select + alter button flags for it
        ImGui::MultiSelectItemHeader(id, &selected, &button_flags);
    }

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);

    // Multi-selection support (footer)
    if (is_multi_select)
    {
        ImGui::MultiSelectItemFooter(id, &selected, &pressed);
    }
    else
    {
        // Auto-select when moved into
        // - This will be more fully fleshed in the range-select branch
        // - This is not exposed as it won't nicely work with some user side handling of shift/control
        // - We cannot do 'if (g.NavJustMovedToId != id) { selected = false; pressed = was_selected; }' for two reasons
        //   - (1) it would require focus scope to be set, need exposing PushFocusScope() or equivalent (e.g. BeginSelection() calling PushFocusScope())
        //   - (2) usage will fail with clipped items
        //   The multi-select API aim to fix those issues, e.g. may be replaced with a BeginSelection() API.
        // if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
            // if (g.NavJustMovedToId == id)
                // selected = pressed = true;
    }

    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with keyboard/gamepad
    if (pressed)
    {
        if (!g.NavHighlightItemUnderNav && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            ImGui::SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, ImGui::WindowRectAbsToRel(window, bb)); // (bb == NavRect)
            if (g.IO.ConfigNavCursorVisibleAuto)
                g.NavCursorVisible = false;
        }
    }
    if (pressed)
        ImGui::MarkItemEdited(id);

    if (selected != was_selected)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    // Render
    if (is_visible)
    {
        const bool highlighted = hovered;
        // const bool highlighted = hovered || (flags & ImGuiSelectableFlags_Highlight);
        if (highlighted || selected)
        {
            // Between 1.91.0 and 1.91.4 we made selected Selectable use an arbitrary lerp between _Header and _HeaderHovered. Removed that now. (#8106)
            ImU32 col = ImGui::GetColorU32((held && highlighted) ? ImGuiCol_HeaderActive : highlighted ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
            ImGui::RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
        }
        if (g.NavId == id)
        {
            ImGuiNavRenderCursorFlags nav_render_cursor_flags = ImGuiNavRenderCursorFlags_Compact | ImGuiNavRenderCursorFlags_NoRounding;
            if (is_multi_select)
                nav_render_cursor_flags |= ImGuiNavRenderCursorFlags_AlwaysDraw; // Always show the nav rectangle
            ImGui::RenderNavCursor(bb, id, nav_render_cursor_flags);
        }
    }

    if (span_all_columns)
    {
        if (g.CurrentTable)
            ImGui::TablePopBackgroundChannel();
        else if (window->DC.CurrentColumns)
            ImGui::PopColumnsBackground();
    }

    // Text stays at the submission position. Alignment/clipping extents ignore SpanAllColumns.
    if (is_visible)
        ImGui::RenderTextClipped(pos, ImVec2(ImMin(pos.x + size.x, window->WorkRect.Max.x), pos.y + size.y), label, NULL, &label_size, style.SelectableTextAlign, &bb);

    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && (g.LastItemData.ItemFlags & ImGuiItemFlags_AutoClosePopups))
        ImGui::CloseCurrentPopup();

    // Selectable() always returns a pressed state!
    // Users of BeginMultiSelect()/EndMultiSelect() scope: you may call ImGui::IsItemToggledSelection() to retrieve
    // selection toggle, only useful if you need that state updated (e.g. for rendering purpose) before reaching EndMultiSelect().
    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed; //-V1020
}
}