#pragma once
#include <bgfx/bgfx.h>
#include <imgui.h>
#include <functional>

typedef std::function<void *(ImGuiViewport *)> ImGuiBgfx_ViewportHandleConverter;

/**
 * @brief Initializes the ImGui BGFX backend with a custom window handle converter.
 *
 * @param viewId The BGFX view ID to use for rendering.
 * @param viewportToNativeFn A lambda/functor that converts ImGuiViewport* to native window handle.
 *                           Signature: void*(ImGuiViewport*)
 *
 * Example usage with different backends:
 *
 * // GLFW
 * ImGui_ImplBgfx_Init(255, [](ImGuiViewport* vp) {
 *     return glfwGetWin32Window((GLFWwindow*)vp->PlatformHandle); // Windows
 *     // return glfwGetX11Window((GLFWwindow*)vp->PlatformHandle); // Linux
 * });
 *
 * // SDL
 * ImGui_ImplBgfx_Init(255, [](ImGuiViewport* vp) {
 *     SDL_SysWMinfo wmInfo;
 *     SDL_GetWindowWMInfo((SDL_Window*)vp->PlatformHandle, &wmInfo);
 *     return wmInfo.info.win.window; // Platform-specific extraction
 * });
 *
 * // Custom backend (if PlatformHandle is already native)
 * ImGui_ImplBgfx_Init(255, [](ImGuiViewport* vp) {
 *     return vp->PlatformHandle; // No conversion needed
 * });
 */
void ImGui_ImplBgfx_Init(int view, ImGuiBgfx_ViewportHandleConverter converter);
void ImGui_ImplBgfx_Shutdown();
void ImGui_ImplBgfx_NewFrame();
void ImGui_ImplBgfx_Render(const bgfx::ViewId viewId, ImDrawData *draw_data, uint32_t clearColor);

// Use if you want to reset your rendering device without losing ImGui state.
void ImGui_ImplBgfx_InvalidateDeviceObjects();
void ImGui_ImplBgfx_CreateDeviceObjects();

void *native_window_handle(ImGuiViewport *viewport);