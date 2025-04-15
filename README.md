# ImGui-bgfx example


[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![bgfx](https://img.shields.io/badge/built%20with-bgfx-0170CE.svg)](https://bkaradzic.github.io/bgfx/)
[![ImGui](https://img.shields.io/badge/ImGui-v1.89+-brightgreen.svg)](https://github.com/ocornut/imgui)

**High-performance** multi-viewport ImGui backend powered by bgfx. Designed for cross-platform rendering with minimal dependencies.

## ✨ Features

- **True Multi-viewport Support**: Each ImGui window as a native platform window
- **Zero Runtime Overhead**: Leverages bx for platform-agnostic operations
- **Optimized Rendering**:
  - Vertex buffer reuse (`RendererHasVtxOffset`)
  - Batch command submission
- **Debug Tools**:
  - Real-time render stats overlay
  - GPU memory monitor
  - Viewport layout serialization

## 🚀 Quick Start

### Dependencies

- bgfx (included as submodule)
- ImGui 1.89+
- C++17 compiler

### Integration

```cpp
#include <imgui_impl_bgfx.h>

void init() 
{
    ImGui_ImplBgfx_Init(view_id, [](ImGuiViewport* vp) 
    {
        return (uintptr_t)createPlatformWindow(vp);
    });
}

void render() 
{
    ImGui_ImplBgfx_Render(ViewId, ImGui::GetDrawData(), ClearColor);
}
```

## 🌿 Branch Strategy

This project maintains two specialized branches to serve different use cases:

| Branch | Purpose | Dependencies | Best For |
|--------|---------|--------------|----------|
| **[`master`](https://github.com/calcdqy/bgfx-imgui-example/tree/master)** | Core backend implementation | • bgfx• ImGui• SDL | Engine integrations, custom toolchains |
| **[`quickstart`](https://github.com/calcdqy/bgfx-imgui-example/tree/quickstart)** | Batteries-included template | • All from `master`• spdlog• gflags• SDL_image | Rapid prototyping, learning |

```bash
# Get core implementation (master)
git clone --branch master --depth 1 https://github.com/calcdqy/bgfx-imgui-example.git

# Get full template (quickstart)
git clone --branch quickstart --recurse-submodules https://github.com/calcdqy/bgfx-imgui-example.git
```