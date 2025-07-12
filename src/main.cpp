#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <filesystem>

#include "Internal.h"

#if defined(PLATFORM_LINUX)
#   include <unistd.h>
#elif defined(PLATFORM_WINDOWS)
#   include <windows.h>        // SetProcessDPIAware, GetLastError
#else
#   include <mach-o/dyld.h>
#endif

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include <SDL2/SDL.h>
#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

#include <nfd.h>

#include "MessageBox.hpp"
#include "ResourceManager.hpp"
#include "DeviceResources.hpp"
#include "EventLog.hpp"
#include "Debugging.h"
#include "DeviceResources.hpp"
#include "Splitter.hpp"
#include "MainLayout.hpp"
#include "MenuBar.hpp"

namespace fs = std::filesystem;

bool always_true = true;


struct GUIElement {
    void (*handle)(void);
    bool *show;
} gui_elements[] = {
    {DrawMessageBox, &messagebox_show},
    {DrawMainLayout, &mainlayout_show}
};

bool GetProgramParentDirectory(fs::path& parent_dir)
{
#if defined(PLATFORM_LINUX)
    size_t bufsiz = PATH_MAX;
    char* buf = 0;
    while(true) {
        buf = (char*)malloc(bufsiz);
        if(!buf) return false;
        ssize_t len = readlink("/proc/self/exe", buf, bufsiz - 1);
        if(len == -1) {
            free(buf);
            return false;
        }
        if(len < bufsiz - 1) {
            buf[len] = '\0';
            break;
        }
        free(buf);
        bufsiz *= 2;
    }
#elif defined(PLATFORM_WIN32)
    DWORD bufsiz = PATH_MAX;
    char* buf = 0;
    while(true) {
        buf = (char*)malloc(bufsiz);
        if(!buf) return false;
        DWORD len = GetModuleFileNameA(NULL, buf, bufsiz);
        if(len == 0) {
            free(buf);
            return false;
        }
        if(len < bufsiz - 1) {
            break;
        }
        free(buf);
        bufsiz *= 2;
    }
#else
    uint32_t bufsiz = 0;
    _NSGetExecutablePath(NULL, &bufsiz);
    char* buf = (char*)malloc(bufsiz);
    if(!buf) return false;
    if(_NSGetExecutablePath(buf, &bufsiz) != 0) {
        free(buf)
        return false;
    }
#endif
    parent_dir = fs::path(buf).parent_path();
    free(buf);
    return true;
}

void LoadAllImageResources()
{
    fs::path path;
    // based on cwd (current working directory)
    if(!fs::exists("./resources/images")) {
        warn("\"resources\" directory not found at cwd. Searching at program directory");
        fs::path parent_path;
        if(!GetProgramParentDirectory(parent_path)) {
            error("unable to retrive program parent directory");
            return;
        }
        path = parent_path / "resources" / "images";
        if(!fs::exists(path)) {
            error("\"resources\" directory not found at proram directory");
            return;
        }
    } else { path = "./resources/images"; }
    for(const auto& entry: fs::directory_iterator(path))
        if(entry.is_regular_file()) {
            ResourceManager::LoadImage(entry.path(), entry.path().filename());
        }
}



bool done = false;

int main(int, char**)
{
    // Setup SDL
#ifdef _WIN32
    ::SetProcessDPIAware();
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        error("failed to initialize the following component: SDL");
        return 1;
    }
    info("SDL was initialized successfully");

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    if(NFD_Init() != NFD_OKAY) {
        error("failed to initialize the following component: nativefiledialog-extended/NFD");
        SDL_Quit();
        return 1;
    }
    info("NFD was initialized successfully");

    // Create window with SDL_Renderer graphics context
    float main_scale = ImGui_ImplSDL2_GetContentScaleForDisplay(0);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow(
        "ToshibaT100 TDISKBASIC emulator", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        (int)(1024 * main_scale), (int)(576 * main_scale), 
        window_flags);
    if (window == nullptr) {
        error("failed to create SDL_Window");
        SDL_Quit();
        return 1;
    }
    info("create SDL_Window successfully");
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        error("failed to create SDL_Renderer for SDL_Window");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    info("create SDL_Renderer successfully");
    LogSDLRendererInfo(renderer);

    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    style.TabRounding = 0.0f;
    style.Colors[ImGuiCol_Text] = ImVec4(212.0f/255, 212.0f/255, 212.0f/255, 1);

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    info("ImGui was initialized successfully, version: %s", IMGUI_VERSION);

    Splitter::InitStyle();
    ResetEventClock();
    ResourceManager::Init(renderer);
    LoadAllImageResources();

#ifdef BUILD_DEBUG
    // Too lazy to manually loading anything...
    DeviceResources::LoadDiskBasic("/home/binaryfox0/proj/toshiba-t100-pc/images/TDISKBASIC.img");
#endif

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4();

    // Main loop
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(1000 / 60);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        static ImVec2 last_size = ImVec2(0.0f, 0.0f);
        ImVec2 current_size = ImGui::GetMainViewport()->Size;
        if(current_size.x != last_size.x || current_size.y != last_size.y) {
            info("window size changed");
            ImVec2 real_size = ImVec2(current_size.x, current_size.y - ImGui::GetFrameHeight());
            UpdateMainLayoutSize(real_size);
            last_size = current_size;
        }
        for(int i = 0; i < sizeof(gui_elements) / sizeof(gui_elements[0]); i++)
            if(*(gui_elements[i].show))
                gui_elements[i].handle();

        DrawMenuBar();

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    DeviceResources::FreeResources();

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    NFD_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}