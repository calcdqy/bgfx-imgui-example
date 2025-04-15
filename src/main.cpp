#include "main.h"
#include "roboto_font.h"

bgfx::ViewId MainViewId = 0;
SDL_WindowFlags window_flags;
SDL_Window *MainWindow = nullptr;

int main(int argc, char *argv[])
{
    // Init SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        std::cerr << "Error: SDL_Init(): " << SDL_GetError() << std::endl;
        return -1;
    }

    // Create gui window
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_RAWINPUT, "0");
    window_flags = (SDL_WindowFlags)(SDL_WINDOW_HIGH_PIXEL_DENSITY);
    MainWindow = SDL_CreateWindow("MainWindow", 1280, 720, window_flags);
    if (!MainWindow)
    {
        std::cerr << "SDL window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }
    SDL_PumpEvents();

    // Init bgfx
    const char *sdlDriver = SDL_GetCurrentVideoDriver();
    bgfx::PlatformData pd = {};
    pd.ndt = nullptr;

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD // Linux or BSD
    SDL_PropertiesID props = SDL_GetWindowProperties(MainWindow);
    if (std::strcmp(sdlDriver, "wayland") == 0)
    {
        pd.nwh = SDL_GetPointerProperty(props, "SDL.window.wayland.surface", nullptr);
        pd.ndt = SDL_GetPointerProperty(props, "SDL.window.wayland.display", nullptr);
    }
    else
    {
        pd.ndt = SDL_GetPointerProperty(props, "SDL.window.x11.display", nullptr);
        pd.nwh = (void *)(uintptr_t)SDL_GetNumberProperty(props, "SDL.window.x11.window", 0);
    }

    if (!pd.nwh || (!pd.ndt && strcmp(sdlDriver, "x11") == 0))
    {
        std::cerr << "Failed to get native handle for driver: " << sdlDriver << std::endl;
        return -1;
    }
#elif BX_PLATFORM_OSX     // Apple
    SDL_PropertiesID props = SDL_GetWindowProperties(MainWindow);
    pd.nwh = SDL_GetPointerProperty(props, "SDL.window.cocoa.window", nullptr);
#elif BX_PLATFORM_WINDOWS // Windows
    SDL_PropertiesID props = SDL_GetWindowProperties(MainWindow);
    pd.nwh = SDL_GetPointerProperty(props, "SDL.window.win32.hwnd", nullptr);
#else                     // Others
#error Your system is not supported!
#endif

    bgfx::setPlatformData(pd);

    bgfx::Init bgfxInit;
    bgfxInit.debug = true;
    bgfxInit.type = bgfx::RendererType::Count;
    bgfxInit.resolution.width = 1280;
    bgfxInit.resolution.height = 720;
    bgfxInit.platformData = pd;
    bgfxInit.resolution.reset = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X16;
    bgfx::init(bgfxInit);

    bgfx::setViewClear(MainViewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
    bgfx::setViewRect(MainViewId, 1280, 720, 1, 1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows
    io.IniFilename = NULL;
    io.LogFilename = NULL;

    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas = false;
    ImFont *font = io.Fonts->AddFontFromMemoryTTF(
        Roboto_Regular_ttf,
        Roboto_Regular_ttf_len,
        16.0f,
        &font_config,
        io.Fonts->GetGlyphRangesDefault());

    // Setup style
    ImGuiStyle &style = ImGui::GetStyle();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    style.WindowRounding = 10.0f;
    style.ChildRounding = 5.0f;
    style.PopupRounding = 20.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 5.0f;

    // Init ImGui
    // Create a Lambda to get window handle
    auto viewport_window_handle = [](ImGuiViewport *viewport) -> void *
    {
        const char *driver = SDL_GetCurrentVideoDriver();
        if (!viewport)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Null viewport");
            return nullptr;
        }
        if (viewport == ImGui::GetMainViewport())
        {
            SDL_Window *main_window = SDL_GetWindowFromID(1);
            if (!main_window)
                return (void *)nullptr;

            SDL_PropertiesID props = SDL_GetWindowProperties(main_window);
#if BX_PLATFORM_WINDOWS
            return SDL_GetPointerProperty(props, "SDL.window.win32.hwnd", nullptr);
#elif BX_PLATFORM_OSX
            return SDL_GetPointerProperty(props, "SDL.window.cocoa.window", nullptr);
#elif BX_PLATFORM_LINUX
            if (strcmp(driver, "wayland") == 0)
            {
                return SDL_GetPointerProperty(props, "SDL.window.wayland.surface", nullptr);
            }
            else
            {
                return (void *)(uintptr_t)SDL_GetNumberProperty(props, "SDL.window.x11.window", 0);
            }
#endif
        }
        SDL_WindowID window_id = (SDL_WindowID)(intptr_t)viewport->PlatformHandle;
        SDL_Window *window = SDL_GetWindowFromID(window_id);
        if (!window)
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                         "Failed to get SDL_Window from ID: %u", window_id);
            return nullptr;
        }
        SDL_PropertiesID props = SDL_GetWindowProperties(window);
        if (!props)
        {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to get window properties");
            return nullptr;
        }

#if BX_PLATFORM_WINDOWS
        return SDL_GetPointerProperty(props, "SDL.window.win32.hwnd", nullptr);
#elif BX_PLATFORM_OSX
        return SDL_GetPointerProperty(props, "SDL.window.cocoa.window", nullptr);
#elif BX_PLATFORM_LINUX
        if (strcmp(driver, "wayland") == 0)
        {
            return SDL_GetPointerProperty(props, "SDL.window.wayland.surface", nullptr);
        }
        else
        {
            return (void *)(uintptr_t)SDL_GetNumberProperty(props, "SDL.window.x11.window", 0);
        }
#else
#error "Unsupported platform!"
#endif
    };

    ImGui_ImplSDL3_InitForOther(MainWindow);
    ImGui_ImplBgfx_Init(MainViewId, viewport_window_handle);

    // Main loop
    bool isClose = true;
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    float f = 0;
    int counter = 0;
    SDL_SystemTheme theme = SDL_GetSystemTheme();
    bool isDark = (theme == SDL_SYSTEM_THEME_DARK);
    if (isDark)
        SetupDarkTheme();
    else
        SetupLightTheme();

    while (isClose)
    {

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);

            Uint32 MainWindowID = SDL_GetWindowID(MainWindow);
            SDL_Window *EventWindow = SDL_GetWindowFromID(event.window.windowID);

            if (event.type == SDL_EVENT_QUIT)
            {
                isClose = false;
            }
            else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == MainWindowID)
            {
                isClose = false;
            }
            else if (event.type == SDL_EVENT_WINDOW_RESIZED && event.window.windowID == MainWindowID)
            {
                bgfx::reset(event.window.data1, event.window.data2, BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X16);
            }
        }

        ImGui_ImplBgfx_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        ImGui::ShowMetricsWindow();

        {
            ImGui::Begin("Hello, world!", &isClose);  // Create a window called "Hello, world!" and append into it.
            ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            if (isDark)
            {
                if (ImGui::Button("Change light"))
                {
                    isDark = false;
                    SetupLightTheme();
                }
            }
            else
            {
                if (ImGui::Button("Change dark"))
                {
                    isDark = true;
                    SetupDarkTheme();
                }
            }

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplBgfx_Render(MainViewId, ImGui::GetDrawData(), 0x443355FF);

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        bgfx::frame();
    }

    // Clean up
    ImGui_ImplBgfx_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    io.Fonts->Clear();
    ImGui::DestroyContext();
    bgfx::shutdown();
    SDL_DestroyWindow(MainWindow);
    SDL_Quit();

    return 0;
}