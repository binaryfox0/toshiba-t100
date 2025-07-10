#include "MenuBar.hpp"

#include "SDL2/SDL.h"
#include "nfd.h"

#include "imgui.h"

#include "DeviceResources.hpp"
#include "MessageBox.hpp"
#include "MainLayout.hpp"
#include "Internal.h"

void OpenDisk() {
    nfdchar_t *outPath = NULL;
    const nfdu8filteritem_t filters[1] = {{"TDISKBASIC image", ".img"}};
    nfdresult_t result = NFD_OpenDialog(&outPath, filters, 1, 0);
        
    if ( result == NFD_OKAY ) {
        DeviceResources::LoadDiskBasic(outPath);
        NFD_FreePathU8(outPath);
    }
    else if(result != NFD_CANCEL) {
        CreateMessageBox("Error", NFD_GetError());
    }
}

static bool show_demo = false;
void DrawMenuBar()
{
    if(show_demo)
        ImGui::ShowDemoWindow();
    if(ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsKeyPressed(ImGuiKey_O, false))
        OpenDisk();
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl-O")) {
                OpenDisk();
            }

            if (ImGui::MenuItem("Exit", "Ctrl+W")) {
                SDL_Event event;
                event.type = SDL_QUIT;
                SDL_PushEvent(&event);
            }
            ImGui::EndMenu();
        }
        
#ifdef BUILD_DEBUG
        if(ImGui::BeginMenu("Debug")) {
            if(ImGui::MenuItem(show_demo ? "Close ImGui Demo" : "Show ImGui Demo")) {
                mainlayout_show = !(show_demo = !show_demo);
            }
            ImGui::EndMenu();
        }
#endif
        ImGui::EndMainMenuBar();
    }

}