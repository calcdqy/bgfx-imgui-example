#include <iostream>
#include <cstdlib>
#include <ctime>
#include <bx/bx.h>
#include <bx/file.h>
#include <bx/filepath.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include "imgui_impl_bgfx.h"

void SetupLightTheme()
{
    ImGui::StyleColorsLight();
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.94f);

    colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.80f, 0.80f, 0.80f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.75f, 0.75f, 0.75f, 0.67f);

    colors[ImGuiCol_Button] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.70f, 0.70f, 0.70f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.60f, 0.60f, 0.60f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);

    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.ScrollbarRounding = 5.0f;
}
void SetupDarkTheme()
{
    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 *colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);

    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 0.67f);

    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.30f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
}