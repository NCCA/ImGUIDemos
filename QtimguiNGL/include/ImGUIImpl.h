// ImGui SDL2 binding with OpenGL3
// In this binding, ImTextureID is used to store an OpenGL 'GLuint' texture identifier. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui
#include "imgui.h"
class NGLScene;
IMGUI_API bool   ImGui_ImplQt_Init();
IMGUI_API void   ImGui_ImplQt_Shutdown();
IMGUI_API void ImGui_ImplQt_NewFrame(NGLScene *window);

IMGUI_API bool   ImGui_ImplQt_ProcessEvent(NGLScene *_window);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API void   ImGui_ImplQt_InvalidateDeviceObjects();
IMGUI_API bool   ImGui_ImplQt_CreateDeviceObjects();
