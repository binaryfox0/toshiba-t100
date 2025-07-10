#ifndef SIDE_PANEL_HPP
#define SIDE_PANEL_HPP

#include <stdint.h>

#include "Splitter.hpp"
#include "imgui.h"

void DrawSidePanel(const float size, Splitter* sp);
void UpdateSidePanel(const float size);

#endif