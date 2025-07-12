#include "MenuBar.hpp"

#include <thread>
#include <atomic>

#include "SDL2/SDL.h"
#include "nfd.h"

#include "imgui.h"

#include "DeviceResources.hpp"
#include "MessageBox.hpp"
#include "MainLayout.hpp"
#include "Internal.h"

static std::atomic<bool> dialog_open = false;
void OpenDisk() {
    if(dialog_open.exchange(true)) {
        return;
    }

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

    dialog_open = false;
}

INLINE void HandleGlobalKeyPressed() {
    if(ImGui::IsKeyChordPressed(ImGuiKey_ModCtrl | ImGuiKey_W)) {
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);
    }
    if(ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsKeyPressed(ImGuiKey_O, false))
        std::thread(OpenDisk).detach();
}

static bool show_demo = false;
void DrawMenuBar()
{
    if(show_demo)
        ImGui::ShowDemoWindow();

    HandleGlobalKeyPressed();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl-O")) {
                std::thread(OpenDisk).detach();
            }

            if (ImGui::MenuItem("Exit", "Ctrl-W")) {
                SDL_Event event;
                event.type = SDL_QUIT;
                SDL_PushEvent(&event);
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Settings")) {
            ImGui::EndMenu();
        }
        
#ifdef BUILD_DEBUG
        if(ImGui::BeginMenu("Debug")) {
            if(ImGui::MenuItem(show_demo ? "Close ImGui Demo" : "Show ImGui Demo")) {
                mainlayout_show = !(show_demo = !show_demo);
            }
            if(ImGui::MenuItem(mainlayout_show ? "Hide Main Layout" : "Show Main Layout")) {
                mainlayout_show = !mainlayout_show;
            }
            ImGui::EndMenu();
        }
#endif
        ImGui::EndMainMenuBar();
    }

}