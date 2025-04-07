#include "main.h"
#include "roboto_font.h"

DEFINE_bool(nogui, false, "Cancels UI rendering");
DEFINE_bool(debug, false, "Set debug mode");
bgfx::ViewId MainViewId = 0;
SDL_WindowFlags window_flags;
SDL_Window *MainWindow = nullptr;

static bool (*Original_ImGui_Begin)(const char *, bool *, ImGuiWindowFlags) = nullptr;

bool Hooked_ImGui_Begin(const char *name, bool *p_open, ImGuiWindowFlags flags)
{
    flags |= ImGuiWindowFlags_NoTitleBar;
    return Original_ImGui_Begin(name, p_open, flags);
}

int main(int argc, char *argv[])
{
    // parser args
    gflags::ParseCommandLineFlags(&argc, &argv, false); // Setup spdlog
    time_t t = time(NULL);
    struct tm *local_tm = localtime(&t);
    char logfilename[80];
    strftime(logfilename, 80, "logs/%Y-%m-%d-", local_tm);
    int *i = new int(1);
    for (;; (*i)++)
    {
        if (!(fs::exists(fs::path(logfilename + std::to_string(*i) + ".log"))))
        {
            try
            {
                spdlog::stdout_color_mt("test error1");
                spdlog::basic_logger_mt("test error2", logfilename + std::to_string(*i) + ".log");
            }
            catch (const spdlog::spdlog_ex &ex)
            {
                std::cerr << "Log initialization failed: " << ex.what() << "\n";
                return 1;
            }
            break;
        }
    }
    std::vector<spdlog::sink_ptr> sinks;
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    if (FLAGS_debug)
    {
        consoleSink->set_level(spdlog::level::debug);
    }
    else
        consoleSink->set_level(spdlog::level::info);
    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfilename + std::to_string(*i) + ".log");
    fileSink->set_level(spdlog::level::info);
    sinks.push_back(consoleSink);
    sinks.push_back(fileSink);
    auto logger = std::make_shared<spdlog::logger>("main_logger", begin(sinks), end(sinks));
    logger->set_pattern("[%H:%M:%S] %^[%t/%l]%$: %v");
    logger->set_level(spdlog::level::debug);
    spdlog::set_default_logger(logger);
    spdlog::info("Log initialization successful");
    spdlog::debug("Debug mode on");

    delete i;
    i = nullptr;

    // Init funchook and hook
    funchook_t *funchook = funchook_create();
    if (!funchook)
    {
        spdlog::critical("Failed to create funchook instance\n");
        return -1;
    }
    spdlog::info("Funchook initialization successful");

    Original_ImGui_Begin = ImGui::Begin;
    int ret = funchook_prepare(funchook,
                               reinterpret_cast<void **>(&Original_ImGui_Begin),
                               reinterpret_cast<void *>(Hooked_ImGui_Begin));

    if (ret != 0)
    {
        spdlog::critical("Hook preparation failed: {}\n", funchook_error_message(funchook));
        funchook_destroy(funchook);
        return -1;
    }

    ret = funchook_install(funchook, 0);
    if (ret != 0)
    {
        spdlog::critical("Hook installation failed: {}\n", funchook_error_message(funchook));
        funchook_destroy(funchook);
        return -1;
    }

    // Init SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        spdlog::critical("Error: SDL_Init(): {}\n", SDL_GetError());
        return -1;
    }
    spdlog::info("SDL initialization successful");

    // Create gui window
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_RAWINPUT, "0");
    window_flags = (SDL_WindowFlags)(SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_UTILITY | SDL_WINDOW_BORDERLESS);
    MainWindow = SDL_CreateWindow("MainWindow", 0, 0, window_flags);
    if (!MainWindow)
    {
        spdlog::critical("SDL window creation failed: {}\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    SDL_PumpEvents();

    // Init bgfx
    const char *sdlDriver = SDL_GetCurrentVideoDriver();
    spdlog::info("Using SDL driver: {}", sdlDriver);
    bgfx::PlatformData pd = {};
    pd.ndt = nullptr;

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD // Linux or BSD
    SDL_PropertiesID props = SDL_GetWindowProperties(MainWindow);
    if (strcmp(sdlDriver, "wayland") == 0)
    {
        pd.nwh = SDL_GetPointerProperty(props, "SDL.window.wayland.surface", nullptr);
        pd.ndt = SDL_GetPointerProperty(props, "SDL.window.wayland.display", nullptr);
        spdlog::debug("Wayland surface: {}, display: {}", pd.nwh, pd.ndt);
    }
    else
    {
        pd.ndt = SDL_GetPointerProperty(props, "SDL.window.x11.display", nullptr);
        pd.nwh = (void *)(uintptr_t)SDL_GetNumberProperty(props, "SDL.window.x11.window", 0);
        spdlog::debug("X11 window: {}, display: {}", pd.nwh, pd.ndt);
    }

    if (!pd.nwh || (!pd.ndt && strcmp(sdlDriver, "x11") == 0))
    {
        spdlog::critical("Failed to get native handle for driver: {}", sdlDriver);
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

    // bgfx::renderFrame();
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
    bgfx::setViewRect(MainViewId, 0, 0, 1, 1);
    spdlog::info("BGFX initialization successful");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Viewports
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
    io.ConfigViewportsNoDecoration = false;
    io.ConfigViewportsNoAutoMerge = true;
    io.ConfigViewportsNoTaskBarIcon = false;
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
    funchook_uninstall(funchook, 0);
    funchook_destroy(funchook);
    SDL_DestroyWindow(MainWindow);
    SDL_Quit();

    return 0;
}