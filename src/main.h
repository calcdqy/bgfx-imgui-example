#include <iostream>
#include <cstdlib>
#include <filesystem>
namespace fs = std::filesystem;
#include <ctime>
#include <gflags/gflags.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdl_bgfx.h>
#include <funchook.h>

void SetupImGuiStyleLight() {}
void SetupImGuiStyleDark() {}