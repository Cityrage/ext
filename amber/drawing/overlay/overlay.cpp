#include "overlay.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_freetype.h"
#include "../../util/globals.h"
#include "../../util/config/configsystem.h"
#include "../../features/visuals/visuals.h"
#include "../../util/classes/math/math.h"
#include "../../features/misc/modules/nojumpcooldown.h"
#include "../../features/misc/misc.h"
#include "../../util/displaynames.h"
#include "../../features/misc/modules/autofriend.h"
#include "../../util/notification/notification.h"
#include "../../util/avatarmanager/avatarmanager.h"
#include "../../util/avatarmanager/avatarmanager.h"
#include <Windows.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <dxgi.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <ctime>
#include <map>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <thread>
#include <bitset>
#include <functional>
#include <cmath>
#include <unordered_set>
#include <cstring>
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;


// Search icon and font storage
static ImFont* tahoma = nullptr;
static ImFont* imgui_default = nullptr;
static ImFont* verdana = nullptr;

// Removed unused json alias to avoid requiring nlohmann/json here
namespace ImGui
{
    bool SliderFloatManual(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f") {
        bool changed = ImGui::SliderFloat(label, v, v_min, v_max, format);

        std::string popup_id = "manual_input_" + std::string(label);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(popup_id.c_str());
        }

        if (ImGui::BeginPopup(popup_id.c_str())) {
            // Extract display name (text before ##, or skip ## prefix if label starts with ##)
            std::string display_label = label;
            size_t hash_pos = display_label.find("##");
            if (hash_pos != std::string::npos) {
                if (hash_pos == 0) {
                    // Label starts with ##, extract ID and convert to readable name
                    display_label = display_label.substr(2);
                    
                    // Map common IDs to readable names
                    static const std::map<std::string, std::string> id_to_name = {
                        {"smothx", "Smoothness X"}, {"smothy", "Smoothness Y"},
                        {"predx", "Prediction X"}, {"predy", "Prediction Y"},
                        {"SilentPredX", "Prediction X"}, {"SilentPredY", "Prediction Y"},
                        {"FOVRadius", "FOV Radius"}, {"FOVTrans", "FOV Transparency"},
                        {"FOVFillTrans", "Fill Transparency"}, {"SpinSpeed", "Spin Speed"},
                        {"SilentFOVRadius", "FOV Radius"}, {"SilentFOVTrans", "FOV Transparency"},
                        {"SilentFOVFillTrans", "Fill Transparency"}, {"SilentSpinSpeed", "Spin Speed"},
                        {"AimDistance", "Distance"}, {"SilentAimDistance", "Distance"},
                        {"TriggerDelay", "Delay (ms)"},
                        {"StaticSizeScale", "Size Scale"}, {"MaxDistance", "Max Distance"},
                        {"FogStart", "Fog Start"}, {"FogEnd", "Fog End"},
                        {"SonarRadius", "Radius"}, {"SonarSpeed", "Speed"}, {"SonarThickness", "Thickness"},
                        {"SpeedValue", "Speed"}, {"FlySpeed", "Fly Speed"}, {"JumpPowerValue", "Jump Power"},
                        {"OrbitSpeed", "Speed"}, {"OrbitHeight", "Height"}, {"OrbitRange", "Range"},
                        {"Rotate360Speed", "360 Speed"}, {"Rotate360VSpeed", "Vertical Speed"},
                        {"fps_cap", "Max FPS"}
                    };
                    
                    auto it = id_to_name.find(display_label);
                    if (it != id_to_name.end()) {
                        display_label = it->second;
                    }
                } else {
                    // Extract text before ##
                    display_label = display_label.substr(0, hash_pos);
                }
            }
            
            ImGui::Text("Enter value for %s", display_label.c_str());
            static float manual_val = 0.0f;
            static bool initial = true;
            if (initial) {
                manual_val = *v;
                initial = false;
            }

            ImGui::SetNextItemWidth(150.0f);
            if (ImGui::InputFloat("##val", &manual_val, 0.0f, 0.0f, format, ImGuiInputTextFlags_EnterReturnsTrue)) {
                *v = std::clamp(manual_val, v_min, v_max);
                changed = true;
                initial = true;
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::IsWindowAppearing())
                ImGui::SetKeyboardFocusHere(-1);

            if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                initial = true;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        return changed;
    }
    
    bool SliderIntManual(const char* label, int* v, int v_min, int v_max, const char* format = "%d") {
        bool changed = ImGui::SliderInt(label, v, v_min, v_max, format);

        std::string popup_id = "manual_input_int_" + std::string(label);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(popup_id.c_str());
        }

        if (ImGui::BeginPopup(popup_id.c_str())) {
            // Extract display name (text before ##, or skip ## prefix if label starts with ##)
            std::string display_label = label;
            size_t hash_pos = display_label.find("##");
            if (hash_pos != std::string::npos) {
                if (hash_pos == 0) {
                    // Label starts with ##, extract ID and convert to readable name
                    display_label = display_label.substr(2);
                    
                    // Map common IDs to readable names (same as SliderFloatManual)
                    static const std::map<std::string, std::string> id_to_name = {
                        {"smothx", "Smoothness X"}, {"smothy", "Smoothness Y"},
                        {"predx", "Prediction X"}, {"predy", "Prediction Y"},
                        {"SilentPredX", "Prediction X"}, {"SilentPredY", "Prediction Y"},
                        {"FOVRadius", "FOV Radius"}, {"FOVTrans", "FOV Transparency"},
                        {"FOVFillTrans", "Fill Transparency"}, {"SpinSpeed", "Spin Speed"},
                        {"SilentFOVRadius", "FOV Radius"}, {"SilentFOVTrans", "FOV Transparency"},
                        {"SilentFOVFillTrans", "Fill Transparency"}, {"SilentSpinSpeed", "Spin Speed"},
                        {"AimDistance", "Distance"}, {"SilentAimDistance", "Distance"},
                        {"TriggerDelay", "Delay (ms)"},
                        {"StaticSizeScale", "Size Scale"}, {"MaxDistance", "Max Distance"},
                        {"FogStart", "Fog Start"}, {"FogEnd", "Fog End"},
                        {"SonarRadius", "Radius"}, {"SonarSpeed", "Speed"}, {"SonarThickness", "Thickness"},
                        {"SpeedValue", "Speed"}, {"FlySpeed", "Fly Speed"}, {"JumpPowerValue", "Jump Power"},
                        {"OrbitSpeed", "Speed"}, {"OrbitHeight", "Height"}, {"OrbitRange", "Range"},
                        {"Rotate360Speed", "360 Speed"}, {"Rotate360VSpeed", "Vertical Speed"}
                    };
                    
                    auto it = id_to_name.find(display_label);
                    if (it != id_to_name.end()) {
                        display_label = it->second;
                    }
                } else {
                    // Extract text before ##
                    display_label = display_label.substr(0, hash_pos);
                }
            }
            
            ImGui::Text("Enter value for %s", display_label.c_str());
            static int manual_val = 0;
            static bool initial = true;
            if (initial) {
                manual_val = *v;
                initial = false;
            }

            ImGui::SetNextItemWidth(150.0f);
            if (ImGui::InputInt("##val", &manual_val, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue)) {
                *v = std::clamp(manual_val, v_min, v_max);
                changed = true;
                initial = true;
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::IsWindowAppearing())
                ImGui::SetKeyboardFocusHere(-1);

            if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                initial = true;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        return changed;
    }
    bool ColoredButtonV1(const char* label, const ImVec2& size, ImU32 text_color, ImU32 bg_color_1, ImU32 bg_color_2);
}

namespace overlay {
    bool visible = true;
    void load_interface() {
        thread();
    }

}
bool ImGui::ColoredButtonV1(const char* label, const ImVec2& size_arg, ImU32 text_color, ImU32 bg_color_1, ImU32 bg_color_2)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    ImGuiButtonFlags flags = ImGuiButtonFlags_None;
    // if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
     //    flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const bool is_gradient = bg_color_1 != bg_color_2;
    if (held || hovered)
    {
        // Modify colors (ultimately this can be prebaked in the style)
        float h_increase = (held && hovered) ? 0.02f : 0;
        float v_increase = (held && hovered) ? 0.20f : 0;

        ImVec4 bg1f = ColorConvertU32ToFloat4(bg_color_1);
        ColorConvertRGBtoHSV(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
        bg1f.x = ImMin(bg1f.x + h_increase, 1.0f);
        bg1f.z = ImMin(bg1f.z + v_increase, 1.0f);
        ColorConvertHSVtoRGB(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
        bg_color_1 = GetColorU32(bg1f);
        if (is_gradient)
        {
            ImVec4 bg2f = ColorConvertU32ToFloat4(bg_color_2);
            ColorConvertRGBtoHSV(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
            bg2f.z = ImMin(bg2f.z + h_increase, 1.0f);
            bg2f.z = ImMin(bg2f.z + v_increase, 1.0f);
            ColorConvertHSVtoRGB(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
            bg_color_2 = GetColorU32(bg2f);
        }
        else
        {
            bg_color_2 = bg_color_1;
        }
    }
    RenderNavHighlight(bb, id);

#if 0
    // V1 : faster but prevents rounding
    window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max, bg_color_1, bg_color_1, bg_color_2, bg_color_2);
    if (g.Style.FrameBorderSize > 0.0f)
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), 0.0f, 0, g.Style.FrameBorderSize);
#endif

    // V2
    int vert_start_idx = window->DrawList->VtxBuffer.Size;
    window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_color_1, g.Style.FrameRounding);
    int vert_end_idx = window->DrawList->VtxBuffer.Size;
    if (is_gradient)
        ShadeVertsLinearColorGradientKeepAlpha(window->DrawList, vert_start_idx, vert_end_idx, bb.Min, bb.GetBL(), bg_color_1, bg_color_2);
    if (g.Style.FrameBorderSize > 0.0f)
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), g.Style.FrameRounding, 0, g.Style.FrameBorderSize);

    if (g.LogEnabled)
        LogSetNextTextDecoration("[", "]");
    PushStyleColor(ImGuiCol_Text, text_color);
    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    PopStyleColor();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

ImColor VecToColor(ImVec4 Color)
{
    return ImColor(
        (int)(Color.x * 255.0f),
        (int)(Color.y * 255.0f),
        (int)(Color.z * 255.0f),
        (int)(Color.w * 255.0f)
    );
}
ImVec4 ColorFromFloat(const float color[4], float alpha = -1.0f)
{
    float final_alpha = (alpha < 0.0f) ? color[3] : alpha;
    return ImVec4(color[0], color[1], color[2], final_alpha);
}
// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// freetype already included above via local path
#include <map>
static const char* const KeyNames[] = {
    "None", "Mouse 1", "Mouse 2", "Cancel", "MBUTTON", "X1", "X2", "None", "Backspace", "Tab",
    "None", "None", "Clear", "Enter", "None", "None", "Shift", "Control", "Alt", "Pause",
    "Caps Lock", "None", "None", "None", "None", "None", "None", "Esc", "None", "None",
    "None", "None", "Space", "Page Up", "Page Down", "End", "Home", "Left", "Up", "Right",
    "Down", "Select", "Print", "Execute", "Print Screen", "Insert", "Delete", "Help", "0", "1",
    "2", "3", "4", "5", "6", "7", "8", "9", "None", "None",
    "None", "None", "None", "None", "None", "A", "B", "C", "D", "E",
    "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y",
    "Z", "Lwin", "Rwin", "Apps", "None", "Sleep", "Numpad 0", "Numpad 1", "Numpad 2", "Numpad 3",
    "Numpad 4", "Numpad 5", "Numpad 6", "Numpad 7", "Numpad 8", "Numpad 9", "Multiply", "Add", "Separator", "Subtract",
    "Decimal", "Divide", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
    "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17", "F18",
    "F19", "F20", "F21", "F22", "F23", "F24", "None", "None", "None", "None",
    "None", "None", "None", "None", "Num Lock", "Scroll Lock", "None", "None", "None", "None",
    "None", "None", "None", "None", "None", "None", "None", "None", "None", "None",
    "LShift", "RShift", "LControl", "RControl", "LAlt", "RAlt"
};
static const int KeyCodes[] = {
    0x0,  //Undefined
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07, //Undefined
    0x08,
    0x09,
    0x0A, //Reserved
    0x0B, //Reserved
    0x0C,
    0x0D,
    0x0E, //Undefined
    0x0F, //Undefined
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16, //IME On
    0x17,
    0x18,
    0x19,
    0x1A, //IME Off
    0x1B,
    0x1C,
    0x1D,
    0x1E,
    0x1F,
    0x20,
    0x21,
    0x22,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x28,
    0x29,
    0x2A,
    0x2B,
    0x2C,
    0x2D,
    0x2E,
    0x2F,
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,
    0x3A, //Undefined
    0x3B, //Undefined
    0x3C, //Undefined
    0x3D, //Undefined
    0x3E, //Undefined
    0x3F, //Undefined
    0x40, //Undefined
    0x41,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x4A,
    0x4B,
    0x4C,
    0x4B,
    0x4E,
    0x4F,
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5A,
    0x5B,
    0x5C,
    0x5D,
    0x5E, //Rservered
    0x5F,
    0x60, //Numpad1
    0x61, //Numpad2
    0x62, //Numpad3
    0x63, //Numpad4
    0x64, //Numpad5
    0x65, //Numpad6
    0x66, //Numpad7
    0x67, //Numpad8
    0x68, //Numpad8
    0x69, //Numpad9
    0x6A,
    0x6B,
    0x6C,
    0x6D,
    0x6E,
    0x6F,
    0x70, //F1
    0x71, //F2
    0x72, //F3
    0x73, //F4
    0x74, //F5
    0x75, //F6
    0x76, //F7
    0x77, //F8
    0x78, //F9
    0x79, //F10
    0x7A, //F11
    0x7B, //F12
    0x7C, //F13
    0x7D, //F14
    0x7E, //F15
    0x7F, //F16
    0x80, //F17
    0x81, //F18
    0x82, //F19
    0x83, //F20
    0x84, //F21
    0x85, //F22
    0x86, //F23
    0x87, //F24
    0x88, //Unkown
    0x89, //Unkown
    0x8A, //Unkown
    0x8B, //Unkown
    0x8C, //Unkown
    0x8D, //Unkown
    0x8E, //Unkown
    0x8F, //Unkown
    0x90,
    0x91,
    0x92, //OEM Specific
    0x93, //OEM Specific
    0x94, //OEM Specific
    0x95, //OEM Specific
    0x96, //OEM Specific
    0x97, //Unkown
    0x98, //Unkown
    0x99, //Unkown
    0x9A, //Unkown
    0x9B, //Unkown
    0x9C, //Unkown
    0x9D, //Unkown
    0x9E, //Unkown 
    0x9F, //Unkown
    0xA0,
    0xA1,
    0xA2,
    0xA3,
    0xA4,
    0xA5
};


static const char* combo_items_4[3] = { ("Head"), ("HumanoidRootPart"), ("Closest") };
static const char* combo_items_3[3] = { ("Mouse"), ("Camera"), ("Silent") };
void Hotkey(keybind& bind, const ImVec2& size_arg = ImVec2(0, 0), int id = 0)
{
    // Use a unique static variable for each hotkey instance
    static std::map<int, bool> waitingForKeyMap;

    // Initialize if not present
    if (waitingForKeyMap.find(id) == waitingForKeyMap.end()) {
        waitingForKeyMap[id] = false;
    }

    bool& waitingforkey = waitingForKeyMap[id];

    if (!waitingforkey) {
        std::string txt = std::string(KeyNames[bind.key]) + "##" + std::to_string(id);
        ImVec4 windowBgColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

        // Darken the base color a bit for the button's border and background
        ImVec4 darker;
        darker.x = (windowBgColor.x - 0.04f < 0.0f) ? 0.0f : windowBgColor.x - 0.04f;
        darker.y = (windowBgColor.y - 0.04f < 0.0f) ? 0.0f : windowBgColor.y - 0.04f;
        darker.z = (windowBgColor.z - 0.04f < 0.0f) ? 0.0f : windowBgColor.z - 0.04f;
        darker.w = windowBgColor.w;

        // Using the original and darkened colors for button
        ImVec4 borderColor = darker;  // Using darker color for the border
        ImVec4 backgroundColor = windowBgColor;  // Using the base color for the button's background
        if (ImGui::ColoredButtonV1(txt.c_str(), size_arg, ImColor(255, 255, 255), ImColor(backgroundColor), ImColor(borderColor)))
            waitingforkey = true;

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::Selectable("Hold", bind.type == keybind::HOLD)) bind.type = keybind::HOLD;
            if (ImGui::Selectable("Toggle", bind.type == keybind::TOGGLE)) bind.type = keybind::TOGGLE;
            if (ImGui::Selectable("Always", bind.type == keybind::ALWAYS)) bind.type = keybind::ALWAYS;
            ImGui::Separator();
            if (ImGui::Selectable("Clear")) {
                bind.key = 0;  // Reset to None
            }
            ImGui::EndPopup();
        }
    }
    else {
        std::string txt = "...##" + std::to_string(id);
        // Get base color from ImGui style (WindowBg)
        ImVec4 windowBgColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

        // Darken the base color a bit for the button's border and background
        ImVec4 darker;
        darker.x = (windowBgColor.x - 0.04f < 0.0f) ? 0.0f : windowBgColor.x - 0.04f;
        darker.y = (windowBgColor.y - 0.04f < 0.0f) ? 0.0f : windowBgColor.y - 0.04f;
        darker.z = (windowBgColor.z - 0.04f < 0.0f) ? 0.0f : windowBgColor.z - 0.04f;
        darker.w = windowBgColor.w;

        // Using the original and darkened colors for button
        ImVec4 borderColor = darker;  // Using darker color for the border
        ImVec4 backgroundColor = windowBgColor;  // Using the base color for the button's background

        // Call to ColoredButtonV1 with adjusted colors
        ImGui::ColoredButtonV1(txt.c_str(), size_arg, ImColor(255, 255, 255), ImColor(backgroundColor), ImColor(borderColor));


        // Check for key presses
        for (auto& Key : KeyCodes)
        {
            if (GetAsyncKeyState(Key) & 0x8000) {
                bind.key = Key;
                waitingforkey = false;
                break;
            }
        }
    }
}

void RenderKeybindList() {
    if (!globals::misc::keybinds) return;

    ImGui::SetNextWindowSize(ImVec2(180, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(globals::misc::overlay_color[0], globals::misc::overlay_color[1], globals::misc::overlay_color[2], 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ColorFromFloat(globals::misc::Border));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

    ImFont* selected_font = nullptr;
    
    selected_font = tahoma;
    if (globals::misc::font_index == 1) selected_font = imgui_default;
    else if (globals::misc::font_index == 2) selected_font = verdana;

    if (selected_font) ImGui::PushFont(selected_font);

    if (ImGui::Begin("Keybinds", &globals::misc::keybinds, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();

        // Accent line on the left of title
        draw_list->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + 2, pos.y + 20), ImGui::GetColorU32(ImGuiCol_CheckMark));

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8);
        ImGui::Text("Keybinds");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 2));

        auto render_bind = [&](keybind& bind, const char* label) {
            if (bind.key == 0 && bind.type != keybind::ALWAYS) return;

            bool active = bind.enabled;
            if (active) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(globals::misc::ThemeColor[0], globals::misc::ThemeColor[1], globals::misc::ThemeColor[2], 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            }

            ImGui::Text("%s", label);
            ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(KeyNames[bind.key]).x - 8);
            ImGui::Text("%s", KeyNames[bind.key]);

            ImGui::PopStyleColor();
        };

        render_bind(globals::combat::aimbotkeybind, "Aimbot");
        render_bind(globals::combat::silentaimkeybind, "Silent Aim");
        render_bind(globals::misc::speedkeybind, "Speed");
        render_bind(globals::misc::flightkeybind, "Fly");
        render_bind(globals::misc::jumppowerkeybind, "Jump Power");
        render_bind(globals::combat::orbitkeybind, "Orbit");
        render_bind(globals::misc::rotate360keybind, "360 Camera");
        render_bind(globals::combat::triggerbotkeybind, "Triggerbot");
    }
    if (selected_font) ImGui::PopFont();
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}





bool full_screen(HWND windowHandle)
{
    MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
    if (GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
    {
        RECT windowRect;
        if (GetWindowRect(windowHandle, &windowRect))
        {
            return windowRect.left == monitorInfo.rcMonitor.left
                && windowRect.right == monitorInfo.rcMonitor.right
                && windowRect.top == monitorInfo.rcMonitor.top
                && windowRect.bottom == monitorInfo.rcMonitor.bottom;
        }
    }
    return false;
}


void move_window(HWND hw)
{
    HWND target = FindWindowA(nullptr, "Roblox");
    HWND foregroundWindow = GetForegroundWindow();

    if (target != foregroundWindow && hw != foregroundWindow)
    {
        MoveWindow(hw, 0, 0, 0, 0, true);
        return;
    }

    RECT rect;
    if (!GetWindowRect(target, &rect))
    {
        std::cerr << "failed ot get window rect" << std::endl;
        return;
    }

    int rsize_x = rect.right - rect.left - 17;
    int rsize_y = rect.bottom - rect.top;

    if (full_screen(target))
    {
        rsize_x += 16;
    }
    else
    {
        rsize_y -= 39;
        rect.left += 4 + 5;
        rect.top += 31;
    }

    if (!MoveWindow(hw, rect.left, rect.top, rsize_x, rsize_y, TRUE))
    {
        std::cerr << "failed to move window" << std::endl;
    }

}
std::vector < std::string > notifications;
void overlay::addnotification(const char* log)
{
    notifications.push_back(log);
}
// Main code
void overlay::thread()
{
    static ConfigSystem cfg;
   
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"VROOOO", nullptr };
    ::RegisterClassExW(&wc);
    const HWND hwnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, wc.lpszClassName, L"sdwdadsgf",
        WS_POPUP, 0, 0, GetSystemMetrics(0), GetSystemMetrics(1), nullptr, nullptr, wc.hInstance, nullptr);
    
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    const MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margin);
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        exit(0);
    }

    // Show the window and bring it to front
    ::ShowWindow(hwnd, SW_SHOW);
    ::UpdateWindow(hwnd);
    ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    ::BringWindowToTop(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    

   
    ImGui::StyleColorsDark();
   

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    
    // Initialize avatar manager
    overlay::init_avatar_manager(g_pd3dDevice, g_pd3dDeviceContext);



   
    bool draw = true;
    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.00f);
    ImGuiStyle* style = &ImGui::GetStyle();
    // Main UI accent color: DA477A (hex) -> (218, 71, 122)
    style->Colors[ImGuiCol_CheckMark] = ImColor(218, 71, 122).Value;
    static const char* aimbot_items[3] = { ("Mouse"), ("Camera"), ("Silent aim") };

    if (tahoma == nullptr) {
        ImFontConfig fontConfig;
        fontConfig.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
        fontConfig.PixelSnapH = true;

        const ImWchar* ranges = io.Fonts->GetGlyphRangesChineseFull(); // Includes Japanese/Korean

        // Add Tahoma Bold first to make it the default font at 13px
        tahoma = io.Fonts->AddFontFromFileTTF(("c:\\Windows\\Fonts\\tahomabd.ttf"), 13.0f, &fontConfig);
        fontConfig.MergeMode = true;
        io.Fonts->AddFontFromFileTTF(("c:\\Windows\\Fonts\\msyh.ttc"), 13.0f, &fontConfig, ranges);
        fontConfig.MergeMode = false;

        // Add default font and others as secondary
        imgui_default = io.Fonts->AddFontDefault(&fontConfig);
        
        verdana = io.Fonts->AddFontFromFileTTF(("c:\\Windows\\Fonts\\verdana.ttf"), 13.0f, &fontConfig);
        fontConfig.MergeMode = true;
        io.Fonts->AddFontFromFileTTF(("c:\\Windows\\Fonts\\msyh.ttc"), 13.0f, &fontConfig, ranges);
        fontConfig.MergeMode = false;

        io.Fonts->Build();
    }
    bool done = false;
    addnotification("amber.lol initialized.");
    static bool show_bulk_add = false;
    while (!done)
    {
        
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

      
        
        g_SwapChainOccluded = false;

       
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }
        if (GetAsyncKeyState(globals::misc::menu_hotkey.key) & 1)
            draw = !draw;
      
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // FPS Unlocker Logic
        if (globals::misc::unlock_fps) {
            static uintptr_t task_scheduler = 0;
            if (task_scheduler == 0) {
                task_scheduler = read<uintptr_t>(base_address + offsets::TaskSchedulerPointer);
            }
            if (task_scheduler) {
                // TaskSchedulerMaxFPS is usually 1.0 / target_fps
                write<double>(task_scheduler + offsets::TaskSchedulerMaxFPS, 1.0 / (double)globals::misc::fps_cap);
            }
        }
        else {
            static uintptr_t task_scheduler = 0;
            if (task_scheduler == 0) {
                task_scheduler = read<uintptr_t>(base_address + offsets::TaskSchedulerPointer);
            }
            if (task_scheduler) {
                // Reset to default 240 FPS
                write<double>(task_scheduler + offsets::TaskSchedulerMaxFPS, 1.0 / 240.0);
            }
        }

        if (auto mgr = overlay::get_avatar_manager()) mgr->update();
        auto getbg = GetForegroundWindow();
        // Allow overlay to be focused if menu is open OR if Roblox/overlay is foreground
        globals::focused = draw || (getbg == FindWindowA(0, ("Roblox")) || getbg == hwnd);
        if (globals::focused)
        {
            globals::instances::draw = ImGui::GetBackgroundDrawList();
            visuals::run();
            move_window(hwnd);
            if (globals::misc::enable_notifications)
            {
                static std::vector<float> times;
                static std::vector<float> y_offsets;
                static std::vector<float> alphas;
                static std::vector<std::string> active_notifications;

                // Get screen dimensions for positioning
                ImVec2 screen_size = ImGui::GetIO().DisplaySize;
                float base_x = screen_size.x * 0.5f; // Center horizontally
                float base_y = screen_size.y - 200.0f; // Bottom with some padding

                // Sync our metadata arrays with notifications
                ImDrawList* draw = ImGui::GetBackgroundDrawList();
                float spacing = 25.0f;
                float now = ImGui::GetTime();

                // Add new notifications - start them at the top of the screen to slide down
                if (active_notifications.size() < notifications.size()) {
                    size_t old_size = active_notifications.size();

                    // Add new items to our tracking arrays
                    active_notifications.insert(active_notifications.end(),
                        notifications.begin() + old_size,
                        notifications.end());
                    times.resize(active_notifications.size(), now);

                    // Make new notifications start at the top of the screen
                    for (size_t i = old_size; i < active_notifications.size(); i++) {
                        y_offsets.push_back(0.0f); // Start at top of screen
                        alphas.push_back(1.0f);
                    }
                }

                // Vector to track which notifications should be removed
                std::vector<int> to_remove;

                // Calculate target positions for all notifications - stacking upward from bottom
                float target_y = base_y;
                std::vector<float> target_positions(active_notifications.size());

                for (int i = active_notifications.size() - 1; i >= 0; i--) {
                    target_positions[i] = target_y;
                    target_y -= spacing; // Move up for next notification
                }

                // Process and draw notifications
                for (int i = 0; i < active_notifications.size(); ++i) {
                    float lifetime = now - times[i];

                    // Fade out after 1.5s
                    if (lifetime > 5.0f)
                        alphas[i] = alphas[i] + (0.0f - alphas[i]) * 0.1f;

                    // Smooth Y position - slide down to target position
                    y_offsets[i] = y_offsets[i] + (target_positions[i] - y_offsets[i]) * 0.15f;

                    // Mark for removal if too old or fully transparent
                    if (lifetime > 5.0f || alphas[i] <= 0.01f) {
                        to_remove.push_back(i);
                        continue; // Skip drawing if it's going to be removed
                    }

                    // Calculate center position based on text width
                    ImVec2 text_size = ImGui::CalcTextSize(active_notifications[i].c_str());
                    ImVec2 pos(base_x - text_size.x * 0.5f, y_offsets[i]);

                    // Draw the notification with outline for better visibility
                    ImVec4 color = ImVec4(1, 1, 1, alphas[i]);

                    // Draw text outline
                    draw->AddText(ImVec2(pos.x - 1, pos.y), IM_COL32(0,0,0,255), active_notifications[i].c_str());
                    draw->AddText(ImVec2(pos.x + 1, pos.y), IM_COL32(0,0,0,255), active_notifications[i].c_str());
                    draw->AddText(ImVec2(pos.x, pos.y - 1), IM_COL32(0,0,0,255), active_notifications[i].c_str());
                    draw->AddText(ImVec2(pos.x, pos.y + 1), IM_COL32(0,0,0,255), active_notifications[i].c_str());

                    // Diagonal positions for a more complete outline
                    draw->AddText(ImVec2(pos.x - 1, pos.y - 1), IM_COL32(0,0,0,255), active_notifications[i].c_str());
                    draw->AddText(ImVec2(pos.x + 1, pos.y - 1), IM_COL32(0,0,0,255), active_notifications[i].c_str());
                    draw->AddText(ImVec2(pos.x - 1, pos.y + 1), IM_COL32(0,0,0,255), active_notifications[i].c_str());
                    draw->AddText(ImVec2(pos.x + 1, pos.y + 1), IM_COL32(0,0,0,255), active_notifications[i].c_str());

                    // Main text
                    draw->AddText(pos, ImGui::ColorConvertFloat4ToU32(color), active_notifications[i].c_str());
                }

                // Remove old notifications (in reverse order to maintain valid indices)
                if (!to_remove.empty()) {
                    std::sort(to_remove.begin(), to_remove.end(), std::greater<int>());
                    for (int idx : to_remove) {
                        active_notifications.erase(active_notifications.begin() + idx);
                        times.erase(times.begin() + idx);
                        y_offsets.erase(y_offsets.begin() + idx);
                        alphas.erase(alphas.begin() + idx);
                    }

                    // Synchronize the original notifications vector with our tracking
                    notifications.clear();
                    notifications.insert(notifications.begin(), active_notifications.begin(), active_notifications.end());
                }
            }
        }
        // Auto-respectate logic
        if (!globals::misc::spectate_target_name.empty()) {
            roblox::player target_player;
            bool found = false;
            {
                std::lock_guard<std::mutex> lock(globals::instances::cachedplayers_mutex);
                for (const auto& p : globals::instances::cachedplayers) {
                    if (p.name == globals::misc::spectate_target_name) {
                        target_player = p;
                        found = true;
                        break;
                    }
                }
            }

            if (found && target_player.hrp.is_valid()) {
                roblox::instance current_subject = globals::instances::camera.getSubject();
                if (current_subject.address != target_player.hrp.address) {
                    globals::instances::localplayer.spectate(target_player.hrp.address);
                }
            }
        }

        RenderKeybindList();
        if (draw)
        {
            ImFont* selected_font = tahoma;
            if (globals::misc::font_index == 1) selected_font = imgui_default;
            else if (globals::misc::font_index == 2) selected_font = verdana;

            if (selected_font) ImGui::PushFont(selected_font);
#if 1
            style->Colors[ImGuiCol_WindowBg] = ColorFromFloat(globals::misc::WindowBG);
            style->Colors[ImGuiCol_Border] = ColorFromFloat(globals::misc::Border);
            style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::Child);
            style->Colors[ImGuiCol_PopupBg] = ImVec4(globals::misc::PopupBG[0], globals::misc::PopupBG[1], globals::misc::PopupBG[2], globals::misc::PopupBG[3]);
            style->Colors[ImGuiCol_Text] = ImVec4(globals::misc::Text[0], globals::misc::Text[1], globals::misc::Text[2], globals::misc::Text[3]);
            style->Colors[ImGuiCol_TextDisabled] = ImVec4(globals::misc::TextDisabled[0], globals::misc::TextDisabled[1], globals::misc::TextDisabled[2], globals::misc::TextDisabled[3]);
            style->Colors[ImGuiCol_Header] = ImVec4(globals::misc::Header[0], globals::misc::Header[1], globals::misc::Header[2], globals::misc::Header[3]);
            style->Colors[ImGuiCol_HeaderHovered] = ImVec4(globals::misc::ThemeColor[0], globals::misc::ThemeColor[1], globals::misc::ThemeColor[2], globals::misc::ThemeColor[3]);
            style->Colors[ImGuiCol_HeaderActive] = ImVec4(globals::misc::AccentActive[0], globals::misc::AccentActive[1], globals::misc::AccentActive[2], globals::misc::AccentActive[3]);
            style->Colors[ImGuiCol_Button] = ImVec4(globals::misc::Button[0], globals::misc::Button[1], globals::misc::Button[2], globals::misc::Button[3]);
            style->Colors[ImGuiCol_ButtonHovered] = ImVec4(globals::misc::ButtonHovered[0], globals::misc::ButtonHovered[1], globals::misc::ButtonHovered[2], globals::misc::ButtonHovered[3]);
            style->Colors[ImGuiCol_ButtonActive] = ImVec4(globals::misc::ButtonActive[0], globals::misc::ButtonActive[1], globals::misc::ButtonActive[2], globals::misc::ButtonActive[3]);
            style->Colors[ImGuiCol_FrameBg] = ImVec4(globals::misc::FrameBG[0], globals::misc::FrameBG[1], globals::misc::FrameBG[2], globals::misc::FrameBG[3]);
            style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(globals::misc::FrameBGHovered[0], globals::misc::FrameBGHovered[1], globals::misc::FrameBGHovered[2], globals::misc::FrameBGHovered[3]);
            style->Colors[ImGuiCol_FrameBgActive] = ImVec4(globals::misc::FrameBGActive[0], globals::misc::FrameBGActive[1], globals::misc::FrameBGActive[2], globals::misc::FrameBGActive[3]);
            style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(globals::misc::ScrollbarBG[0], globals::misc::ScrollbarBG[1], globals::misc::ScrollbarBG[2], globals::misc::ScrollbarBG[3]);
            style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(globals::misc::ScrollbarGrab[0], globals::misc::ScrollbarGrab[1], globals::misc::ScrollbarGrab[2], globals::misc::ScrollbarGrab[3]);
            style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(globals::misc::ScrollbarGrabHovered[0], globals::misc::ScrollbarGrabHovered[1], globals::misc::ScrollbarGrabHovered[2], globals::misc::ScrollbarGrabHovered[3]);
            style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(globals::misc::ScrollbarGrabActive[0], globals::misc::ScrollbarGrabActive[1], globals::misc::ScrollbarGrabActive[2], globals::misc::ScrollbarGrabActive[3]);
            style->Colors[ImGuiCol_SliderGrab] = ImVec4(globals::misc::SliderGrab[0], globals::misc::SliderGrab[1], globals::misc::SliderGrab[2], globals::misc::SliderGrab[3]);
            style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(globals::misc::SliderGrabActive[0], globals::misc::SliderGrabActive[1], globals::misc::SliderGrabActive[2], globals::misc::SliderGrabActive[3]);
            style->Colors[ImGuiCol_CheckMark] = ImVec4(globals::misc::ThemeColor[0], globals::misc::ThemeColor[1], globals::misc::ThemeColor[2], globals::misc::ThemeColor[3]);
            style->Colors[ImGuiCol_Separator] = ColorFromFloat(globals::misc::Border);

            style->FramePadding = { 1,1 };
            style->ScrollbarSize = 0.0f;
            style->FrameBorderSize = 1.0f;
            ImGui::SetNextWindowSizeConstraints({ 575, 650 }, { FLT_MAX, FLT_MAX });
            ImGui::SetNextWindowSize({ 575, 650 });
            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
            ImGui::Begin("##JuggedABandReactor", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
            style->Colors[ImGuiCol_Border] = ColorFromFloat(globals::misc::Border);
            {
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 windowPos = ImGui::GetWindowPos();
                ImVec2 windowSize = ImGui::GetWindowSize();

                // Outside Border
                ImGui::GetBackgroundDrawList()->AddRect(
                    { windowPos.x - 2, windowPos.y - 2 },
                    { windowPos.x + windowSize.x + 2, windowPos.y + windowSize.y + 2 },
                    ImColor(VecToColor(ColorFromFloat(globals::misc::OverlayBorder))), 2.0f, 0, 2.0f
                );
                ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
                ImVec4 darker;
                darker.x = (color.x - 0.04f < 0.0f) ? 0.0f : color.x - 0.04f;
                darker.y = (color.y - 0.04f < 0.0f) ? 0.0f : color.y - 0.04f;
                darker.z = (color.z - 0.04f < 0.0f) ? 0.0f : color.z - 0.04f;
                darker.w = color.w;
                drawList->AddRectFilledMultiColor(
                    windowPos,
                    ImVec2(windowPos.x + windowSize.x, windowPos.y + 30),
                    ImColor(darker),      // Bottom-right
                    ImColor(darker),
                    VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_WindowBg)),         // Top-left
                    VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_WindowBg))
                );
            }
            // dynamic current date and time with timezone selection
            std::time_t now = std::time(nullptr);
            std::tm tm_local{};
            localtime_s(&tm_local, &now);

            // Calculate offset time
            std::tm tm_final = tm_local;
            std::string tz_label = "";
            
            // Expanded timezones (Offset from GMT)
            struct Timezone { std::string label; int offset; };
            static const std::vector<Timezone> timezones = {
                {"Local", 0}, // Offset logic used differently for Local
                {"EST", -5}, {"CST", -6}, {"MST", -7}, {"PST", -8},
                {"AST", -4}, {"NST", -3}, {"GMT", 0}, {"CET", 1},
                {"EET", 2}, {"MSK", 3}, {"JST", 9}, {"AEST", 10},
                {"NZDT", 13}, {"BRT", -3}, {"HST", -10}
            };

            if (globals::timezone_index > 0 && globals::timezone_index < timezones.size()) {
                std::tm tm_utc{};
                gmtime_s(&tm_utc, &now);
                
                int offset = timezones[globals::timezone_index].offset;
                tz_label = timezones[globals::timezone_index].label;

                now += offset * 3600;
                gmtime_s(&tm_final, &now);
            } else {
                globals::timezone_index = 0; // Boundary check
                tz_label = "Local";
            }

            char datebuf[64]{};
            std::strftime(datebuf, sizeof(datebuf), "%b. %d. %Y | %I:%M %p", &tm_final);
            
            // Convert month to lowercase
            for (int i = 0; i < 3 && datebuf[i] != '.'; i++) {
                datebuf[i] = std::tolower(datebuf[i]);
            }

            // Custom lowercase for AM/PM
            for (int i = 0; i < sizeof(datebuf) && datebuf[i] != '\0'; i++) {
                if (datebuf[i] == 'A' || datebuf[i] == 'P' || datebuf[i] == 'M') {
                    datebuf[i] = std::tolower(datebuf[i]);
                }
            }

            ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark), "amber.lol");
            ImGui::SameLine(0, 0);
            ImGui::Text(" - %s", datebuf);
            if (ImGui::IsItemClicked()) {
                globals::timezone_index = (globals::timezone_index + 1) % 16;
            }
            ImGui::BeginChild("##content", ImGui::GetContentRegionAvail(), true);
            style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::WindowBG);
            ImGui::BeginChild("##main2", ImGui::GetContentRegionAvail(), true);
            std::string tabs[] = { "Aimbot", "Silent", "Visual", "Movement", "Players", "Settings" };
            static std::string currenttab = "Aimbot";
            float totalWidth = ImGui::GetWindowSize().x;
            size_t tabCount = sizeof(tabs) / sizeof(tabs[0]);
            float spacing = 0.0f;
            ImVec2 SelectedMin;
            ImVec2 SelectedMax;
            ImVec2 LastMin;
            ImVec2 LastMax;
            ImVec2 CurrentMin;
            ImVec2 CurrentMax;

            // Calculate button width while accounting for spacing
            float buttonWidth = (totalWidth - spacing * (tabCount - 1)) / tabCount;

            for (size_t i = 0; i < tabCount; ++i) {
                // First button at (0,0)
                if (i == 0) {
                    ImGui::SetCursorPos({ 0, 1 });
                }
                ImVec4 color = ColorFromFloat(globals::misc::WindowBG);

                // Darken the color a bit (0.04 ~ 10/255)
                ImVec4 darker;
                darker.x = (color.x - 0.04f < 0.0f) ? 0.0f : color.x - 0.04f;
                darker.y = (color.y - 0.04f < 0.0f) ? 0.0f : color.y - 0.04f;
                darker.z = (color.z - 0.04f < 0.0f) ? 0.0f : color.z - 0.04f;
                darker.w = color.w;
                // Create button
                if (tabs[i] == currenttab) {
                    style->FrameBorderSize = 0.0f;
                    if (ImGui::ColoredButtonV1(tabs[i].c_str(), { buttonWidth, 21 }, ImColor(255, 255, 255), ImColor(darker), VecToColor(ColorFromFloat(globals::misc::WindowBG)))) {
                        currenttab = tabs[i];
                    }
                    SelectedMin = ImGui::GetItemRectMin();
                    SelectedMax = ImGui::GetItemRectMax();
                    style->FrameBorderSize = 1.0f;
                    ImGui::GetWindowDrawList()->AddLine(
                        SelectedMin,
                        { SelectedMin.x, SelectedMax.y },
                        VecToColor(ColorFromFloat(globals::misc::Border))

                    );
                    ImGui::GetWindowDrawList()->AddLine(
                        SelectedMax,
                        { SelectedMax.x, SelectedMin.y },
                        VecToColor(ColorFromFloat(globals::misc::Border))

                    );
                    ImGui::GetWindowDrawList()->AddLine(
                        SelectedMin,
                        { SelectedMax.x, SelectedMin.y },
                        VecToColor(ColorFromFloat(globals::misc::Border))

                    );




                }
                else {
                    if (ImGui::ColoredButtonV1(tabs[i].c_str(), { buttonWidth, 21 }, ImColor(120, 120, 120), VecToColor(ColorFromFloat(globals::misc::WindowBG)), ImColor(darker))) {
                        currenttab = tabs[i];
                    }
                    ImGui::GetWindowDrawList()->AddLine(
                        ImGui::GetItemRectMin() - ImVec2(0, 4),
                        { ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y - 4 },
                        VecToColor(ColorFromFloat(globals::misc::Child)),
                        5.5f
                    );
                }

                // Get current button rect
                CurrentMin = ImGui::GetItemRectMin();
                CurrentMax = ImGui::GetItemRectMax();

                // Fill the space between the previous button and current button
                if (i > 0) {
                    ImVec2 fillStart = ImVec2(LastMax.x + 1, LastMin.y - 2);
                    ImVec2 fillEnd = ImVec2(CurrentMin.x - 1, CurrentMax.y - 2);
                    ImGui::GetForegroundDrawList()->AddRectFilled(fillStart, fillEnd, VecToColor(ColorFromFloat(globals::misc::Child)));
                }

                // Store current button position for next iteration
                LastMin = CurrentMin;
                LastMax = CurrentMax;

                // Add spacing except after the last button
                if (i < tabCount - 1) {
                    ImGui::SameLine(0.0f, spacing);
                }
            }
            ImVec2 wp = ImGui::GetWindowPos();

            // **Ensure the underline perfectly aligns**
            ImGui::GetWindowDrawList()->AddLine(
                { wp.x, wp.y + 21 },   // Adjust Y slightly for better alignment
                { SelectedMin.x, wp.y + 21 },
                VecToColor(ColorFromFloat(globals::misc::Border)), 1.5f
            );

            ImGui::GetWindowDrawList()->AddLine(
                { SelectedMax.x, wp.y + 21 },
                { wp.x + totalWidth, wp.y + 21 },
                VecToColor(ColorFromFloat(globals::misc::Border)), 1.5f
            );
            ImGui::GetWindowDrawList()->AddLine(
                { wp.x, SelectedMin.y - 1 },
                { wp.x + ImGui::GetWindowSize().x, SelectedMin.y - 1 },
                VecToColor(ColorFromFloat(globals::misc::Child)), 1.5f
            );
            ImGui::GetForegroundDrawList()->AddLine(wp, wp + ImVec2(ImGui::GetWindowSize().x, 0), VecToColor(ColorFromFloat(globals::misc::Child)));

            ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(LastMax.x, LastMin.y) + ImVec2(1, -1), ImVec2(wp.x + ImGui::GetWindowSize().x, LastMax.y - 1), VecToColor(ColorFromFloat(globals::misc::Child)));
            
            if (currenttab == "Visual")
            {
                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::WindowBG);
                ImGui::BeginChild("##holder", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, false);
                
                // --- Left Column: Player ESP ---
                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::Child);
                ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x / 2 - 5, ImGui::GetContentRegionAvail().y }, true);
                wp = ImGui::GetWindowPos();

                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 1 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                );
                ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );

                ImGui::Text("Player ESP");

                // Master Visuals Toggle
                static bool has_saved_state = false;
                bool toggle = globals::visuals::visuals;
                if (ImGui::Checkbox(toggle ? "Enable" : "Disable", &toggle))
                {
                    globals::visuals::visuals = toggle;
                    if (!toggle)
                    {
                         // Save current state
                        globals::visuals::boxes_prev = globals::visuals::boxes;
                        globals::visuals::health_prev = globals::visuals::health;
                        globals::visuals::name_prev = globals::visuals::name;
                        globals::visuals::toolesp_prev = globals::visuals::toolesp;
                        globals::visuals::distance_prev = globals::visuals::distance;
                        globals::visuals::chams_prev = globals::visuals::chams;
                         globals::visuals::skeletons_prev = globals::visuals::skeletons;
                        globals::visuals::snapline_prev = globals::visuals::snapline;
                        globals::visuals::fog_enabled_prev = globals::visuals::fog_enabled;
                        globals::visuals::sonar_prev = globals::visuals::sonar;
                        globals::visuals::target_only_esp_prev = globals::visuals::target_only_esp;
                        globals::visuals::lockedesp_prev = globals::visuals::lockedesp;
                        
                        has_saved_state = true;

                        // Disable all
                        globals::visuals::boxes = false;
                        globals::visuals::health = false;
                        globals::visuals::name = false;
                        globals::visuals::toolesp = false;
                        globals::visuals::distance = false;
                        globals::visuals::chams = false;
                        globals::visuals::skeletons = false;
                        globals::visuals::snapline = false;
                        globals::visuals::fog_enabled = false;
                        globals::visuals::sonar = false;
                        globals::visuals::target_only_esp = false;
                        globals::visuals::lockedesp = false;
                    }
                    else
                    {
                        // Restore state
                        if (!has_saved_state)
                        {
                            // Default set if nothing was prev saved
                            globals::visuals::boxes = true;
                            globals::visuals::health = true;
                            globals::visuals::name = true;
                        }
                        else
                        {
                            globals::visuals::boxes = globals::visuals::boxes_prev;
                            globals::visuals::health = globals::visuals::health_prev;
                            globals::visuals::name = globals::visuals::name_prev;
                            globals::visuals::toolesp = globals::visuals::toolesp_prev;
                            globals::visuals::distance = globals::visuals::distance_prev;
                             globals::visuals::chams = globals::visuals::chams_prev;
                            globals::visuals::skeletons = globals::visuals::skeletons_prev;
                            globals::visuals::snapline = globals::visuals::snapline_prev;
                            globals::visuals::fog_enabled = globals::visuals::fog_enabled_prev;
                            globals::visuals::sonar = globals::visuals::sonar_prev;
                            globals::visuals::target_only_esp = globals::visuals::target_only_esp_prev;
                            globals::visuals::lockedesp = globals::visuals::lockedesp_prev;
                        }
                    }
                }

                // 1. Boxes
                ImGui::Checkbox(("Box"), &globals::visuals::boxes);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##boxcolor"), globals::visuals::boxcolors, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                
                if (globals::visuals::boxes) {
                    ImGui::Text("Box Type");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    static const char* box_types[] = { "Corners", "Bounding" };
                    ImGui::Combo("##BoxType", &globals::visuals::boxtype, box_types, IM_ARRAYSIZE(box_types));



                    // Box Overlays Dropdown (Only show if Box is enabled)
                    ImGui::Text("Box Overlays");

                    if ((*globals::visuals::box_overlay_flags)[2]) { // Fill
                        ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                        ImGui::SetNextItemWidth(20.0f);
                        ImGui::ColorEdit4("##boxfillcolor", globals::visuals::boxfillcolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    }

                    if ((*globals::visuals::box_overlay_flags)[1]) { // Glow
                        float offset = (*globals::visuals::box_overlay_flags)[2] ? 58.0f : 35.0f;
                        ImGui::SameLine(ImGui::GetWindowWidth() - offset);
                        ImGui::SetNextItemWidth(20.0f);
                        ImGui::ColorEdit4("##glowcolor", globals::visuals::glowcolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                        
                        // Glow Size slider
                        ImGui::Text("Glow Size");
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::SliderFloatManual("##GlowSize", &globals::visuals::glow_size, 1.0f, 100.0f, "%.1f");
                        
                        // Glow Opacity slider
                        ImGui::Text("Glow Opacity");
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::SliderFloatManual("##GlowOpacity", &globals::visuals::glow_opacity, 0.0f, 1.0f, "%.2f");
                    }
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    static std::string preview_box_overlay = "None";
                    if (ImGui::BeginCombo("##BoxOverlayDropdown", preview_box_overlay.c_str())) {
                        bool outline_selected = (*globals::visuals::box_overlay_flags)[0];
                        if (ImGui::Selectable("Outline", &outline_selected, ImGuiSelectableFlags_DontClosePopups)) (*globals::visuals::box_overlay_flags)[0] = outline_selected ? 1 : 0;
                        
                        bool glow_selected = (*globals::visuals::box_overlay_flags)[1];
                        if (ImGui::Selectable("Glow", &glow_selected, ImGuiSelectableFlags_DontClosePopups)) (*globals::visuals::box_overlay_flags)[1] = glow_selected ? 1 : 0;

                        bool fill_selected = (*globals::visuals::box_overlay_flags)[2];
                        if (ImGui::Selectable("Fill", &fill_selected, ImGuiSelectableFlags_DontClosePopups)) (*globals::visuals::box_overlay_flags)[2] = fill_selected ? 1 : 0;
                        
                        ImGui::EndCombo();
                    }
                    // Update preview string
                    preview_box_overlay = "";
                    if ((*globals::visuals::box_overlay_flags)[0]) preview_box_overlay += "Outline, ";
                    if ((*globals::visuals::box_overlay_flags)[1]) preview_box_overlay += "Glow, ";
                    if ((*globals::visuals::box_overlay_flags)[2]) preview_box_overlay += "Fill, ";
                    
                    if (preview_box_overlay.empty()) {
                        preview_box_overlay = "None";
                    } else {
                        preview_box_overlay.erase(preview_box_overlay.size() - 2);
                    }
                }

                
                // 2. Health Bar
                ImGui::Checkbox(("Health Bar"), &globals::visuals::health);
                ImGui::SameLine(ImGui::GetWindowWidth() - 81.0f);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##healthglowcolor"), globals::visuals::healthglowcolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                ImGui::SameLine(ImGui::GetWindowWidth() - 58.0f);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##healthcolor1"), globals::visuals::healthbarcolor1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35.0f);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##healthcolor"), globals::visuals::healthbarcolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                
                 if (globals::visuals::health) {
                    ImGui::Text("Health Style");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    
                    // Build preview text
                    std::string preview_health_style = "";
                    if (globals::visuals::health_bar_outline) preview_health_style += "Outline, ";
                    if (globals::visuals::health_bar_gradient) preview_health_style += "Gradient, ";
                    if (globals::visuals::enable_health_glow) preview_health_style += "Glow, ";
                    if (preview_health_style.empty()) {
                        preview_health_style = "None";
                    } else {
                        preview_health_style.erase(preview_health_style.size() - 2);
                    }
                    
                    if (ImGui::BeginCombo("##HealthStyleDropdown", preview_health_style.c_str())) {
                        if (ImGui::Selectable("Outline", &globals::visuals::health_bar_outline, ImGuiSelectableFlags_DontClosePopups)) {}
                        if (ImGui::Selectable("Gradient", &globals::visuals::health_bar_gradient, ImGuiSelectableFlags_DontClosePopups)) {}
                        if (ImGui::Selectable("Glow", &globals::visuals::enable_health_glow, ImGuiSelectableFlags_DontClosePopups)) {}
                        ImGui::EndCombo();
                    }
                    
                    // Show glow sliders if glow is enabled
                    if (globals::visuals::enable_health_glow) {
                        ImGui::Text("Glow Size");
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::SliderFloatManual("##HealthGlowSize", &globals::visuals::health_glow_size, 1.0f, 100.0f, "%.1f");
                        
                        ImGui::Text("Glow Opacity");
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::SliderFloatManual("##HealthGlowOpacity", &globals::visuals::health_glow_opacity, 0.0f, 1.0f, "%.2f");
                    }
                    
                    ImGui::Text("Health Bar Position");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    static const char* health_positions[] = { "Left", "Right" };
                    ImGui::Combo("##HealthPosition", &globals::visuals::health_bar_position, health_positions, IM_ARRAYSIZE(health_positions));
                }

                // 3. Name ESP with Type
                ImGui::Checkbox(("Name"), &globals::visuals::name);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##namecolor"), globals::visuals::namecolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                if (globals::visuals::name) {
                    ImGui::Text("Name Style");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    static const char* name_types[] = { "Username", "Display Name" };
                    ImGui::Combo("##NameType", &globals::visuals::nametype, name_types, IM_ARRAYSIZE(name_types));
                }

                // 4. Tool Name
                ImGui::Checkbox(("Tool Name"), &globals::visuals::toolesp);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##toolcolor"), globals::visuals::toolespcolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                // 5. Skeleton
                ImGui::Checkbox(("Skeleton"), &globals::visuals::skeletons);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##skeletoncolor"), globals::visuals::skeletonscolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

                // 6. Chams
                ImGui::Checkbox(("Chams"), &globals::visuals::chams);
                if (globals::visuals::chamstype == 1 || globals::visuals::chamstype == 2) { // Fill or Highlight
                    ImGui::SameLine(ImGui::GetWindowWidth() - 58.0f);
                    ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##chamscolor1"), globals::visuals::chamscolor1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                }
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##chamscolor"), globals::visuals::chamscolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                 if (globals::visuals::chams) {
                     ImGui::Text("Chams Style");
                     ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                     static const char* chams_types[] = { "Outline", "Fill", "Highlight" };
                     ImGui::Combo("##ChamsType", &globals::visuals::chamstype, chams_types, IM_ARRAYSIZE(chams_types));
                 }

                // 7. Snaplines
                ImGui::Checkbox(("Snaplines"), &globals::visuals::snapline);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##snaplinecolor"), globals::visuals::snaplinecolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                if (globals::visuals::snapline) {
                    ImGui::Text("Type");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    static const char* snap_types[] = { "Top", "Bottom", "Center", "Crosshair" };
                    ImGui::Combo("##SnapType", &globals::visuals::snaplinetype, snap_types, IM_ARRAYSIZE(snap_types));
                    
                    ImGui::Text("Overlay");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    static const char* snap_overlay_types[] = { "Straight", "Spiderweb" };
                    ImGui::Combo("##SnapOverlay", &globals::visuals::snaplineoverlaytype, snap_overlay_types, IM_ARRAYSIZE(snap_overlay_types)); // Uses same variable? Or flags? Assuming exclusive int.
                }

                // 8. Distance
                ImGui::Checkbox(("Distance"), &globals::visuals::distance);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##distancecolor"), globals::visuals::distancecolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                if (globals::visuals::distance) {
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual("##MaxDistance", &globals::visuals::visual_range, 100, 5000, "%.0f");
                    ImGui::Dummy(ImVec2(0, 1));
                }


                ImGui::EndChild();
                ImGui::SameLine();
                // --- Right Column: Options & Theme ---
                ImGui::BeginChild("##2", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                wp = ImGui::GetWindowPos();

                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 1 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                );
                ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );

                ImGui::Text("Options");
                 // Fog Changer
                ImGui::Checkbox("Fog Changer", &globals::visuals::fog_enabled);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##fogcolor"), globals::visuals::fog_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                if (globals::visuals::fog_enabled) {
                    ImGui::Text("Fog Start");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual("##FogStart", &globals::visuals::fog_start, 0.0f, 500.0f, "%.0f");
                    ImGui::Text("Fog End");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual("##FogEnd", &globals::visuals::fog_end, 0.0f, 5000.0f, "%.0f");
                }
                 // Sonar
                ImGui::Checkbox("Sonar", &globals::visuals::sonar);
                 if (globals::visuals::sonar) {
                    ImGui::Checkbox("Detect Players", &globals::visuals::sonar_detect_players);
                    ImGui::SameLine(ImGui::GetWindowWidth() - 58.0f);
                    ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##sonardetectcolor1"), globals::visuals::sonar_detect_color_in, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    ImGui::SameLine(ImGui::GetWindowWidth() - 35.0f);
                    ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##sonardetectcolor"), globals::visuals::sonar_detect_color_out, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                    
                    ImGui::Text("Radius");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual("##SonarRadius", &globals::visuals::sonar_range, 0.0f, 100.0f, "%.0f");

                    ImGui::Text("Speed");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual("##SonarSpeed", &globals::visuals::sonar_speed, 0.0f, 5.0f, "%.0f");

                    ImGui::Text("Thickness");
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual("##SonarThickness", &globals::visuals::sonar_thickness, 0.0f, 5.0f, "%.0f");
                 }
 

                // Target Only ESP
                ImGui::Checkbox(("Target Only ESP"), &globals::visuals::target_only_esp);

                // Locked ESP
                ImGui::Checkbox(("Locked ESP"), &globals::visuals::lockedesp);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##lockedcolor"), globals::visuals::lockedespcolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);


                // Removed OLD Box Filled Checkbox

                ImGui::EndChild();
                ImGui::EndChild();
            }

            if (currenttab == "Aimbot")
            {
                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::Child);
                ImGui::Dummy({ 0,0 });


                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::WindowBG);
                ImGui::BeginChild("##holder", { ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y }, false);
                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::Child);
                ImGui::BeginChild("##2", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2 }, true);
                wp = ImGui::GetWindowPos();


                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x , wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x  , wp.y + 1 },
                    VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark)), 2.0f
                );
                ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );
                ImGui::Text("Aimbot");
                ImGui::Checkbox(("Aim"), &globals::combat::aimbot);
                ImGui::SameLine();
                {
                    float hotkey_width = 50.0f;
                    float x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - hotkey_width;
                    ImGui::SetCursorPosX(x);
                    Hotkey(globals::combat::aimbotkeybind, ImVec2(hotkey_width, 15), 2);
                }
                ImGui::Checkbox(("Sticky Aim"), &globals::combat::stickyaim);
                ImGui::Checkbox(("Closest Part"), &globals::combat::aimbot_closest_part);
                ImGui::Text(("Type"));
                style->FramePadding = { 3,3 };
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                static const char* aimbot_types[2] = { ("Camera"), ("Mouse") };
                ImGui::Combo(("##AimType"), &globals::combat::aimbottype, aimbot_types, IM_ARRAYSIZE(aimbot_types));
                style->FramePadding = { 1,1 };
                // Removed FOV controls from here per request

                // Aim Part and Air Part under Type
                ImGui::Text(("Aim Part"));
                style->FramePadding = { 3,3 };
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                // Build filtered hit parts without "All"
                static std::vector<const char*> filtered_hit_parts;
                static std::vector<int> hit_index_map;
                if (filtered_hit_parts.empty()) {
                    for (int i = 0; i < IM_ARRAYSIZE(globals::combat::hit_parts); ++i) {
                        if (std::string(globals::combat::hit_parts[i]) != "All") {
                            filtered_hit_parts.push_back(globals::combat::hit_parts[i]);
                            hit_index_map.push_back(i);
                        }
                    }
                }
                {
                    // Preview text for selected aim parts
                    std::string preview;
                    for (size_t i = 0; i < hit_index_map.size(); ++i) {
                        int original = hit_index_map[i];
                        if (original >= 0 && original < (int)globals::combat::aimpart->size() && (*globals::combat::aimpart)[original]) {
                            if (!preview.empty()) preview += ", ";
                            preview += filtered_hit_parts[i];
                        }
                    }
                    if (preview.empty()) preview = "None";
                    if (ImGui::BeginCombo("##AimPart", preview.c_str())) {
                        for (size_t i = 0; i < filtered_hit_parts.size(); ++i) {
                            int original = hit_index_map[i];
                            bool selected = (original >= 0 && original < (int)globals::combat::aimpart->size()) ? (*globals::combat::aimpart)[original] != 0 : false;
                            if (ImGui::Selectable(filtered_hit_parts[i], selected, ImGuiSelectableFlags_DontClosePopups)) {
                                if (original >= 0 && original < (int)globals::combat::aimpart->size()) {
                                    (*globals::combat::aimpart)[original] = selected ? 0 : 1;
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                style->FramePadding = { 1,1 };
                ImGui::Text(("Air Part"));
                style->FramePadding = { 3,3 };
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                {
                    std::string preview;
                    for (size_t i = 0; i < hit_index_map.size(); ++i) {
                        int original = hit_index_map[i];
                        if (original >= 0 && original < (int)globals::combat::airaimpart->size() && (*globals::combat::airaimpart)[original]) {
                            if (!preview.empty()) preview += ", ";
                            preview += filtered_hit_parts[i];
                        }
                    }
                    if (preview.empty()) preview = "None";
                    if (ImGui::BeginCombo("##AirAimPart", preview.c_str())) {
                        for (size_t i = 0; i < filtered_hit_parts.size(); ++i) {
                            int original = hit_index_map[i];
                            bool selected = (original >= 0 && original < (int)globals::combat::airaimpart->size()) ? (*globals::combat::airaimpart)[original] != 0 : false;
                            if (ImGui::Selectable(filtered_hit_parts[i], selected, ImGuiSelectableFlags_DontClosePopups)) {
                                if (original >= 0 && original < (int)globals::combat::airaimpart->size()) {
                                    (*globals::combat::airaimpart)[original] = selected ? 0 : 1;
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                style->FramePadding = { 1,1 };

                ImGui::EndChild();

                ImGui::BeginChild("##3", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                wp = ImGui::GetWindowPos();


                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x , wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x  , wp.y + 1 },
                    VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark)), 2.0f
                ); ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );
                ImGui::Text("FOV");

                ImGui::Checkbox(("Use FOV"), &globals::combat::usefov);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##fovcolor"), globals::combat::fovcolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                ImGui::Checkbox(("Fill FOV"), &globals::combat::fovfill);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##fovfillcolor"), globals::combat::fovfillcolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                ImGui::Checkbox(("Spin FOV"), &globals::combat::spin_fov_aimbot);

                ImGui::Text(("FOV Radius"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##FOVRadius"), &globals::combat::fovsize, 1.0f, 300.0f, "%.0f");

                ImGui::Text(("FOV Transparency"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##FOVTrans"), &globals::combat::fovtransparency, 0.0f, 5.0f, "%.0f");

                ImGui::Text(("Fill Transparency"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##FOVFillTrans"), &globals::combat::fovfilltransparency, 0.0f, 3.0f, "%.0f");

                ImGui::Text(("Spin Speed"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##SpinSpeed"), &globals::combat::spin_fov_aimbot_speed, 0.0f, 3.0f, "%.0f");

                ImGui::Text(("Shape"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                {
                    static const char* shape_items[] = { "Circle", "Square", "Triangle", "Pentagon", "Hexagon", "Octagon" };
                    ImGui::Combo(("##FOVShape"), &globals::combat::fovshape, shape_items, IM_ARRAYSIZE(shape_items));
                }


                ImGui::EndChild();

                ImGui::EndChild();

                ImGui::SameLine();
                ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                wp = ImGui::GetWindowPos();wp = ImGui::GetWindowPos();wp = ImGui::GetWindowPos();


            ImGui::GetWindowDrawList()->AddLine(
                { wp.x + 1, wp.y + 1 },
                { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 1 },
                ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
            );
            ImGui::GetWindowDrawList()->AddShadowRect(
                { wp.x + 1, wp.y + 1 },
                { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 3 },
                ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
            );
            
                ImGui::Text("Controls");
                ImGui::Checkbox(("##smooth_enable"), &globals::combat::smoothing);
                ImGui::SameLine();
                ImGui::Text("Smoothness");
                ImGui::Text(("Smoothness X"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##smothx"), &globals::combat::smoothingx, 1.0f, 100.0f, "%.0f");
                ImGui::Text(("Smoothness Y"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##smothy"), &globals::combat::smoothingy, 1.0f, 100.0f, "%.0f");

                ImGui::Checkbox(("##predictions_enable"), &globals::combat::predictions);
                ImGui::SameLine();
                ImGui::Text("Predictions");
                ImGui::Text(("Prediction X"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##predx"), &globals::combat::predictionsx, 1.0f, 30.0f, "%.0f");
                ImGui::Text(("Prediction Y"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##predy"), &globals::combat::predictionsy, 1.0f, 30.0f, "%.0f");

                ImGui::Text(("Smoothing Style"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::Combo(("##smoothstyle"), &globals::combat::smoothing_style, globals::combat::smoothing_styles, IM_ARRAYSIZE(globals::combat::smoothing_styles));

                // Checks moved under predictions
                ImGui::Checkbox(("Team Check"), &globals::combat::teamcheck);
                ImGui::Checkbox(("Knocked Check"), &globals::combat::knockcheck);
                ImGui::Checkbox(("Wallcheck"), &globals::combat::wallcheck);
                ImGui::Checkbox(("Distance Check"), &globals::combat::rangecheck);
                if (globals::combat::rangecheck) {
                    ImGui::Text(("Distance"));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##AimDistance"), &globals::combat::aim_distance, 10.0f, 5000.0f, "%.0f");
                }

                // Trigger bot moved here with keybind
                ImGui::Checkbox(("Trigger Bot"), &globals::combat::triggerbot);
                ImGui::SameLine();
                {
                    float hotkey_width = 50.0f;
                    float x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - hotkey_width;
                    ImGui::SetCursorPosX(x);
                    Hotkey(globals::combat::triggerbotkeybind, ImVec2(hotkey_width, 15), 9);
                }

                if (globals::combat::triggerbot)
                {
                    ImGui::Text(("Delay (ms)"));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##TriggerDelay"), &globals::combat::triggerbot_delay, 0.0f, 1000.0f, "%.0f");

                    ImGui::Text(("Triggerbot Checks"));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    // Build preview of selected items
                    std::string preview_text;
                    const char* names_ordered[] = { "Spray", "Knife", "Wallet", "Food" };
                    const int map_to_globals[4] = { 0, 3, 1, 2 }; // spray, knife, wallet, food
                    for (int i = 0; i < 4; ++i)
                    {
                        if (globals::combat::triggerbot_item_checks[map_to_globals[i]])
                        {
                            if (!preview_text.empty()) preview_text += ", ";
                            preview_text += names_ordered[i];
                        }
                    }
                    if (preview_text.empty()) preview_text = "None";

                    if (ImGui::BeginCombo("##TriggerbotChecks", preview_text.c_str()))
                    {
                        for (int i = 0; i < 4; ++i)
                        {
                            int idx = map_to_globals[i];
                            bool selected = globals::combat::triggerbot_item_checks[idx];
                            if (ImGui::Selectable(names_ordered[i], selected, ImGuiSelectableFlags_DontClosePopups))
                            {
                                globals::combat::triggerbot_item_checks[idx] = !selected;
                            }
                        }
                        ImGui::EndCombo();
                    }
                }




                ImGui::EndChild();
                ImGui::SameLine();

            }
            if (currenttab == "Players")
            {
                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::Child);
                ImGui::Dummy({ 0,0 });

                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::WindowBG);
                ImGui::BeginChild("##holder_players", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, false);
                
                wp = ImGui::GetWindowPos();
                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 1 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                );
                ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );
                ImGui::Dummy(ImVec2(0, 5));

                // Player search
                static char player_search[128] = "";
                float total_width = ImGui::GetContentRegionAvail().x;
                float search_width = total_width; 
                
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 3)); 
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputTextWithHint("##playersearch", "Search...", player_search, IM_ARRAYSIZE(player_search));
                ImGui::PopStyleVar();
                
                ImGui::Spacing();
                
                // Get all players logic (same as before)
                std::vector<roblox::player> all_players;
                {
                    std::lock_guard<std::mutex> lock(globals::instances::cachedplayers_mutex);
                    all_players = globals::instances::cachedplayers;
                }
                
                // Add bots logic (same as before) for completeness if needed in this block, 
                // but usually the previous loop covered it. Assuming the context replacement covers the whole block 
                // starting from search input down to end of tab.
                // Re-inserting the bot logic briefly for safety if it was inside the replaced block in previous steps.
                 for (auto bot_head : globals::instances::bots) {
                    if (bot_head.is_valid()) {
                        roblox::instance npc_model = bot_head.read_parent();
                        if (npc_model.is_valid() && npc_model.get_class_name() == "Model") {
                            roblox::instance humanoid = npc_model.findfirstchild("Humanoid");
                            if (humanoid.is_valid() && humanoid.read_health() > 0) {
                                roblox::player bot_player;
                                bot_player.name = "NPC";
                                bot_player.displayname = "NPC";
                                bot_player.head = bot_head;
                                bot_player.humanoid = humanoid;
                                bot_player.hrp = npc_model.findfirstchild("HumanoidRootPart");
                                bot_player.instance = npc_model;
                                bot_player.main = npc_model;
                                bot_player.userid.address = 0;
                                bot_player.health = humanoid.read_health();
                                bot_player.maxhealth = humanoid.read_maxhealth();
                                all_players.push_back(bot_player);
                            }
                        }
                    }
                }

                // Filter by search
                std::string search_lower = player_search;
                std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);

                // Initialize static state for selection
                static std::string selected_player_name = "";
                // Filter state
                static int current_filter = 0;
                static const char* filter_items[] = { "All", "Neutral", "Friendly", "Enemy", "Target" };

                // Calculate available height
                float available_height = ImGui::GetContentRegionAvail().y;
                float actions_panel_height = 200.0f; // Adjusted height
                float list_height = available_height - actions_panel_height - 15.0f; // More padding for separators

                // Header for List
                // ImGui::Text("Player List"); // Optional "Player List" text can go here if needed to match "Actions" style
                // but user image just showed the list. The task said "add the lines like how the other section have it".
                // The images show "Actions" with a line under it. The list usually has a header or just starts.
                // I will add a separator block like the bottom ones for consistency if that's what "lines" means.
                // Or maybe the user meant the dark background for the list? "main playerlist section smaller".
                
                // --- Player List Section ---
                // Background consistent with other children
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ColorFromFloat(globals::misc::Child)); 
                // Filter players before rendering to allow clipping
                std::vector<roblox::player*> filtered_players;
                filtered_players.reserve(all_players.size());

                for (auto& player : all_players) {
                    // Search filter
                    std::string display_lower = displaynames::get_best_display(player);
                    std::transform(display_lower.begin(), display_lower.end(), display_lower.begin(), ::tolower);
                    std::string username_lower = player.name;
                    std::transform(username_lower.begin(), username_lower.end(), username_lower.begin(), ::tolower);
                    
                    bool matches_search = search_lower.empty() ||
                        display_lower.find(search_lower) != std::string::npos ||
                        username_lower.find(search_lower) != std::string::npos;
                    
                    if (!matches_search) continue;

                    // Status Filter
                    bool is_friendly = globals::bools::player_status.count(player.name) && globals::bools::player_status[player.name];
                    bool is_enemy = globals::bools::player_status.count(player.name) && !globals::bools::player_status[player.name];
                    bool is_target_only = std::find(globals::visuals::target_only_list.begin(), globals::visuals::target_only_list.end(), player.name) != globals::visuals::target_only_list.end();
                    // bool is_whitelisted = std::find(globals::instances::whitelist.begin(), globals::instances::whitelist.end(), player.name) != globals::instances::whitelist.end();
                    
                    bool pass_filter = false;
                    switch (current_filter) {
                        case 0: pass_filter = true; break; // All
                        case 1: pass_filter = !is_friendly && !is_enemy && !is_target_only; break; // Neutral
                        case 2: pass_filter = is_friendly; break; // Friendly
                        case 3: pass_filter = is_enemy || is_target_only; break; // Enemy (Target Only is auto-enemy)
                        case 4: pass_filter = is_target_only; break; // Target Only
                        // case 5: pass_filter = is_whitelisted; break; // Whitelisted - removed from list for now as user only mentioned Enemy
                    }

                    if (pass_filter) {
                        filtered_players.push_back(&player);
                    }
                }

                ImGui::BeginChild("PlayerListScroll", ImVec2(0, list_height), true);
                ImGui::PopStyleColor();

                if (filtered_players.empty()) {
                    ImGui::Text("No players found");
                } else {
                    for (auto* player_ptr : filtered_players) {
                        auto& player = *player_ptr;
                        
                        ImGui::PushID(player.name.c_str()); 
                        
                        bool is_client = (player.name == globals::instances::lp.name);
                        
                        // Card dimensions
                        float card_height = 30.0f; 
                        ImVec2 card_start = ImGui::GetCursorScreenPos();
                        ImVec2 card_size = ImVec2(ImGui::GetContentRegionAvail().x, card_height);
                        ImDrawList* draw_list = ImGui::GetWindowDrawList();
                        
                        // Selection Logic
                        if (ImGui::InvisibleButton("##player_card", card_size)) {
                            selected_player_name = player.name;
                        }
                        bool card_hovered = ImGui::IsItemHovered();
                        bool is_selected = (selected_player_name == player.name);
                        
                        // Visuals
                        if (is_selected) {
                                draw_list->AddRectFilled(card_start, ImVec2(card_start.x + card_size.x, card_start.y + card_size.y), IM_COL32(50, 50, 55, 255), 0.0f);
                        }
                        if (card_hovered) {
                            draw_list->AddRectFilled(card_start, ImVec2(card_start.x + card_size.x, card_start.y + card_size.y), IM_COL32(255, 255, 255, 15), 0.0f);
                        }

                        // Text Setup
                        float text_x = card_start.x + 10.0f;
                        float text_y = card_start.y + 7.0f; 
                        
                        std::string display = displaynames::get_best_display(player);
                        std::string full_name_text = display + " @" + player.name;
                        
                        ImU32 text_color = is_selected ? IM_COL32(230, 230, 230, 255) : IM_COL32(160, 160, 160, 255);
                        draw_list->AddText(ImVec2(text_x, text_y), text_color, full_name_text.c_str());

                        // Status Color Logic
                        bool is_friendly = globals::bools::player_status.count(player.name) && globals::bools::player_status[player.name];
                        bool is_target_only = std::find(globals::visuals::target_only_list.begin(), 
                            globals::visuals::target_only_list.end(), player.name) != globals::visuals::target_only_list.end();
                        
                        std::string status_label = "Neutral";
                        ImU32 status_color = IM_COL32(150, 150, 150, 255); // Neutral Gray

                        if (is_client) {
                            status_label = "Client";
                            // Dynamic UI color match
                            ImVec4 active_col = ImGui::GetStyle().Colors[ImGuiCol_CheckMark]; 
                            status_color = ImGui::ColorConvertFloat4ToU32(active_col);
                        } else if (is_target_only) {
                            status_label = "Target"; 
                            status_color = IM_COL32(0, 0, 0, 255); // Black as requested
                        } else if (is_friendly) {
                            status_label = "Friendly"; 
                            status_color = IM_COL32(0, 255, 0, 255); // Green
                        } else if (globals::bools::player_status.count(player.name) && !globals::bools::player_status[player.name]) {
                            status_label = "Enemy";
                            status_color = IM_COL32(255, 0, 0, 255); // Red
                        }
                        
                        ImVec2 status_size = ImGui::CalcTextSize(status_label.c_str());
                        float status_x = card_start.x + card_size.x - status_size.x - 10.0f;
                        draw_list->AddText(ImVec2(status_x, text_y), status_color, status_label.c_str());

                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                // --- Bottom Panels ---
                ImGui::Spacing();
                
                // Actions & Custom Panels split
                // We need two children side-by-side. 
                // Left: Actions
                // Right: Custom
                
                float half_width = (ImGui::GetContentRegionAvail().x * 0.5f) - 4.0f;
                
                // Actions Panel
                ImGui::BeginChild("ActionsPanel", ImVec2(half_width, 0), true);
                wp = ImGui::GetWindowPos();
                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 1 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                );
                ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );
                ImGui::Text("Actions");
                ImGui::Dummy(ImVec2(0, 5));
                
                if (!selected_player_name.empty()) {
                    // Re-find player
                    roblox::player* selected_player = nullptr;
                    for (auto& p : all_players) {
                        if (p.name == selected_player_name) {
                            selected_player = &p;
                            break;
                        }
                    }

                    if (selected_player) {
                        bool is_me = (selected_player->name == globals::instances::lp.name);
                        
                        // Button styling helper
                        auto ActionButton = [](const char* label) -> bool {
                            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
                            bool ret = ImGui::Button(label, ImVec2(-1, 22.0f)); // Larger height for better visibility
                            ImGui::PopStyleVar();
                            return ret;
                        };

                        if (!is_me) {
                            static double teleport_time = -10.0;
                            bool show_teleported = (ImGui::GetTime() - teleport_time) < 5.0;
                            std::string teleport_label = show_teleported ? (std::string("Teleported: @") + selected_player->name) : "Teleport To";

                            if (ActionButton(teleport_label.c_str())) {
                                roblox::player lp = globals::instances::lp;
                                if (lp.hrp.is_valid() && selected_player->hrp.is_valid()) {
                                   lp.hrp.write_position(selected_player->hrp.get_pos());
                                   teleport_time = ImGui::GetTime();
                                }
                            }
                            
                            bool is_spectating = (globals::misc::spectate_target_name == selected_player->name);
                            std::string spectate_label = is_spectating ? "Unspectate" : "Spectate";
                            if (ActionButton(spectate_label.c_str())) {
                                if (is_spectating) {
                                    globals::instances::localplayer.unspectate();
                                    globals::misc::spectate_target_name = "";
                                } else {
                                    globals::instances::localplayer.spectate(selected_player->hrp.address);
                                    globals::misc::spectate_target_name = selected_player->name;
                                }
                            }

                             bool is_in_target_list = std::find(globals::visuals::target_only_list.begin(), globals::visuals::target_only_list.end(), selected_player->name) != globals::visuals::target_only_list.end();
                             
                            if (ActionButton(is_in_target_list ? "Remove Target Only" : "Set Target Only")) {
                                 if (is_in_target_list) {
                                    globals::visuals::target_only_list.erase(std::remove(globals::visuals::target_only_list.begin(), globals::visuals::target_only_list.end(), selected_player->name), globals::visuals::target_only_list.end());
                                    globals::bools::player_status.erase(selected_player->name);
                                } else {
                                    globals::visuals::target_only_list.push_back(selected_player->name);
                                    globals::bools::player_status[selected_player->name] = false; // Force enemy
                                    globals::visuals::target_only_esp = true; // Auto-enable Target Only ESP
                                }
                            }



                            bool is_friendly = globals::bools::player_status.count(selected_player->name) && globals::bools::player_status[selected_player->name];
                            bool is_enemy = globals::bools::player_status.count(selected_player->name) && !globals::bools::player_status[selected_player->name];
                            
                            // Single cycling button: Neutral -> Friendly -> Enemy -> Neutral
                            // Disabled when player is in Target Only list
                            std::string status_button_label;
                            if (is_friendly) {
                                status_button_label = "Set as Enemy";
                            } else if (is_enemy) {
                                status_button_label = "Set as Neutral";
                            } else {
                                status_button_label = "Set as Friendly";
                            }
                            
                            if (is_in_target_list) {
                                ImGui::BeginDisabled();
                            }
                            
                            if (ActionButton(status_button_label.c_str())) {
                                if (is_friendly) {
                                    // Friendly -> Enemy
                                    globals::bools::player_status[selected_player->name] = false;
                                } else if (is_enemy) {
                                    // Enemy -> Neutral
                                    globals::bools::player_status.erase(selected_player->name);
                                } else {
                                    // Neutral -> Friendly
                                    globals::bools::player_status[selected_player->name] = true;
                                }
                            }
                            
                            if (is_in_target_list) {
                                ImGui::EndDisabled();
                            }

                        }

                        // Copy Buttons with timed feedback
                        static double username_copy_time = -10.0;
                        static double id_copy_time = -10.0;
                        
                        bool show_user_copied = (ImGui::GetTime() - username_copy_time) < 5.0;
                        std::string copy_user_label = show_user_copied ? (std::string("Copied: @") + selected_player->name) : "Copy Username";
                        if (ActionButton(copy_user_label.c_str())) {
                             ImGui::SetClipboardText(selected_player->name.c_str());
                             username_copy_time = ImGui::GetTime();
                        }
                        
                        bool show_id_copied = (ImGui::GetTime() - id_copy_time) < 5.0;
                        std::string uid_str = std::to_string(selected_player->userid.address);
                        std::string copy_id_label = show_id_copied ? (std::string("Copied: ") + uid_str) : "Copy ID";
                        if (ActionButton(copy_id_label.c_str())) {
                             if (selected_player->userid.address != 0) {
                                ImGui::SetClipboardText(uid_str.c_str());
                                id_copy_time = ImGui::GetTime();
                             }
                        }

                    } else {
                        ImGui::TextDisabled("Player not found.");
                    }
                } else {
                    ImGui::TextDisabled("No player selected.");
                }
                ImGui::EndChild();

                ImGui::SameLine();

                // Custom Panel
                ImGui::BeginChild("CustomPanel", ImVec2(half_width, 0), true);
                wp = ImGui::GetWindowPos();
                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 1 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                );
                ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );
                ImGui::Text("Filters");
                ImGui::Dummy(ImVec2(0, 5));
                
                ImGui::SetNextItemWidth(-1);
                ImGui::Combo("##filter_combo", &current_filter, filter_items, IM_ARRAYSIZE(filter_items));
                
                ImGui::Spacing();

                ImGui::Separator();
                ImGui::Spacing();

                ImGui::Text("Auto Friend Groups");
                ImGui::Dummy(ImVec2(0, 3));

                static char group_id_input[128] = "";
                if (group_id_input[0] == '\0' && !globals::misc::autofriend_group_id.empty()) {
                    strcpy_s(group_id_input, globals::misc::autofriend_group_id.c_str());
                }

                ImGui::Text("Group ID:");
                ImGui::SetNextItemWidth(-1);
                if (ImGui::InputText("##group_id", group_id_input, IM_ARRAYSIZE(group_id_input))) {
                    globals::misc::autofriend_group_id = group_id_input;
                }

                ImGui::Spacing();

                if (ImGui::Button("Auto Add Friend", ImVec2(-1, 18))) {
                    autofriend::g_auto_friend_manager.fetchGroupMembers(globals::misc::autofriend_group_id);
                }

                if (ImGui::Button("Bulk Add Targets", ImVec2(-1, 18))) {
                    show_bulk_add = true;
                }
                
                ImGui::EndChild();
                
                ImGui::EndChild();
            }
            if (currenttab == "Silent")
            {
                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::Child);
                ImGui::Dummy({ 0,0 });

                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::WindowBG);
                ImGui::BeginChild("##holder", { ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y }, false);
                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::Child);
                ImGui::BeginChild("##2", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2 }, true);
                wp = ImGui::GetWindowPos();

                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x , wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x  , wp.y + 1 },
                    VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark)), 2.0f
                );
                ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );
                ImGui::Text("Silent Aim");
                ImGui::Checkbox(("Silent"), &globals::combat::silentaim);
                ImGui::SameLine();
                {
                    float hotkey_width = 50.0f;
                    float x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - hotkey_width;
                    ImGui::SetCursorPosX(x);
                    Hotkey(globals::combat::silentaimkeybind, ImVec2(hotkey_width, 15), 10);
                }
                ImGui::Checkbox(("Sticky Aim"), &globals::combat::stickyaimsilent);
                ImGui::Checkbox(("Closest Part"), &globals::combat::silent_closest_part);
                ImGui::Text(("Type"));
                style->FramePadding = { 3,3 };
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                static const char* silent_types[2] = { ("Freeaim"), ("Mouse") };
                ImGui::Combo(("##SilentType"), &globals::combat::silentaimtype, silent_types, IM_ARRAYSIZE(silent_types));
                style->FramePadding = { 1,1 };

                // Aim Part
                ImGui::Text(("Aim Part"));
                style->FramePadding = { 3,3 };
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                static std::vector<const char*> filtered_hit_parts_silent;
                static std::vector<int> hit_index_map_silent;
                if (filtered_hit_parts_silent.empty()) {
                    for (int i = 0; i < IM_ARRAYSIZE(globals::combat::hit_parts); ++i) {
                        if (std::string(globals::combat::hit_parts[i]) != "All") {
                            filtered_hit_parts_silent.push_back(globals::combat::hit_parts[i]);
                            hit_index_map_silent.push_back(i);
                        }
                    }
                }
                {
                    std::string preview;
                    for (size_t i = 0; i < hit_index_map_silent.size(); ++i) {
                        int original = hit_index_map_silent[i];
                        if (original >= 0 && original < (int)globals::combat::silentaimpart->size() && (*globals::combat::silentaimpart)[original]) {
                            if (!preview.empty()) preview += ", ";
                            preview += filtered_hit_parts_silent[i];
                        }
                    }
                    if (preview.empty()) preview = "None";
                    if (ImGui::BeginCombo("##SilentAimPart", preview.c_str())) {
                        for (size_t i = 0; i < filtered_hit_parts_silent.size(); ++i) {
                            int original = hit_index_map_silent[i];
                            bool selected = (original >= 0 && original < (int)globals::combat::silentaimpart->size()) ? (*globals::combat::silentaimpart)[original] != 0 : false;
                            if (ImGui::Selectable(filtered_hit_parts_silent[i], selected, ImGuiSelectableFlags_DontClosePopups)) {
                                if (original >= 0 && original < (int)globals::combat::silentaimpart->size()) {
                                    (*globals::combat::silentaimpart)[original] = selected ? 0 : 1;
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                }

                 // Air Part
                style->FramePadding = { 1,1 };
                ImGui::Text(("Air Part"));
                style->FramePadding = { 3,3 };
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                {
                    std::string preview;
                    for (size_t i = 0; i < hit_index_map_silent.size(); ++i) {
                        int original = hit_index_map_silent[i];
                        if (original >= 0 && original < (int)globals::combat::airsilentaimpart->size() && (*globals::combat::airsilentaimpart)[original]) {
                            if (!preview.empty()) preview += ", ";
                            preview += filtered_hit_parts_silent[i];
                        }
                    }
                    if (preview.empty()) preview = "None";
                    if (ImGui::BeginCombo("##SilentAirAimPart", preview.c_str())) {
                        for (size_t i = 0; i < filtered_hit_parts_silent.size(); ++i) {
                            int original = hit_index_map_silent[i];
                            bool selected = (original >= 0 && original < (int)globals::combat::airsilentaimpart->size()) ? (*globals::combat::airsilentaimpart)[original] != 0 : false;
                            if (ImGui::Selectable(filtered_hit_parts_silent[i], selected, ImGuiSelectableFlags_DontClosePopups)) {
                                if (original >= 0 && original < (int)globals::combat::airsilentaimpart->size()) {
                                    (*globals::combat::airsilentaimpart)[original] = selected ? 0 : 1;
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                style->FramePadding = { 1,1 };
                ImGui::EndChild();

                ImGui::BeginChild("##3", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                wp = ImGui::GetWindowPos();
                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x , wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x  , wp.y + 1 },
                    VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark)), 2.0f
                );
                 ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );
                ImGui::Text("FOV");
                ImGui::Checkbox(("Use FOV"), &globals::combat::silentaimfov);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##silentfovcolor"), globals::combat::silentaimfovcolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                ImGui::Checkbox(("Fill FOV"), &globals::combat::silentaimfovfill);
                ImGui::SameLine(ImGui::GetWindowWidth() - 35);
                ImGui::SetNextItemWidth(20.0f); ImGui::ColorEdit4(("##silentfovfillcolor"), globals::combat::silentaimfovfillcolor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                ImGui::Checkbox(("Spin FOV"), &globals::combat::spin_fov_silentaim);

                ImGui::Text(("FOV Radius"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##SilentFOVRadius"), &globals::combat::silentaimfovsize, 1.0f, 300.0f, "%.0f");

                ImGui::Text(("FOV Transparency"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##SilentFOVTrans"), &globals::combat::silentaimfovtransparency, 0.0f, 5.0f, "%.0f");

                ImGui::Text(("Fill Transparency"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##SilentFOVFillTrans"), &globals::combat::silentaimfovfilltransparency, 0.0f, 3.0f, "%.0f");

                ImGui::Text(("Spin Speed"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##SilentSpinSpeed"), &globals::combat::spin_fov_silentaim_speed, 0.0f, 3.0f, "%.0f");

                ImGui::Text(("Shape"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                {
                    static const char* shape_items[] = { "Circle", "Square", "Triangle", "Pentagon", "Hexagon", "Octagon" };
                    ImGui::Combo(("##SilentFOVShape"), &globals::combat::silentaimfovshape, shape_items, IM_ARRAYSIZE(shape_items));
                }

                ImGui::EndChild();
                ImGui::EndChild();

                ImGui::SameLine();
                ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                wp = ImGui::GetWindowPos();wp = ImGui::GetWindowPos();wp = ImGui::GetWindowPos();
                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 1 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                );
                ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1  , wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );

                ImGui::Text("Controls");
                ImGui::Checkbox(("Predictions"), &globals::combat::silentpredictions);

                ImGui::Text(("Prediction X"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##SilentPredX"), &globals::combat::silentpredictionsx, 1.0f, 30.0f, "%.0f");
                ImGui::Text(("Prediction Y"));
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::SliderFloatManual(("##SilentPredY"), &globals::combat::silentpredictionsy, 1.0f, 30.0f, "%.0f");
                
                ImGui::Checkbox(("Team Check"), &globals::combat::teamcheck);
                ImGui::Checkbox(("Knocked Check"), &globals::combat::knockcheck);
                ImGui::Checkbox(("Distance Check"), &globals::combat::rangecheck);
                if (globals::combat::rangecheck) {
                    ImGui::Text(("Distance"));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##SilentAimDistance"), &globals::combat::aim_distance, 10.0f, 5000.0f, "%.0f");
                }
                ImGui::Checkbox(("Wallcheck"), &globals::combat::wallcheck);

                ImGui::EndChild();
            }
            if (currenttab == "Movement")
            {
                ImGui::Dummy({ 0,0 });
                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::WindowBG);
                ImGui::BeginChild("##holder", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, false);

                // --- Left Column: Movement ---
                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::Child);
                ImGui::BeginChild("##1", { ImGui::GetContentRegionAvail().x / 2 - 5, ImGui::GetContentRegionAvail().y }, true);
                wp = ImGui::GetWindowPos();

                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 1 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                );
                ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );

                ImGui::Text("Movement");
                
                ImGui::Checkbox(("Speed Hack"), &globals::misc::speed);
                ImGui::SameLine(ImGui::GetWindowWidth() - 55);
                Hotkey(globals::misc::speedkeybind, ImVec2(50, 15), 1);
                
                if (globals::misc::speed) {
                    const char* speed_modes[] = { "Walk Speed", "Velocity" };
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::Combo(("##SpeedMode"), &globals::misc::speedtype, speed_modes, IM_ARRAYSIZE(speed_modes));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##SpeedValue"), &globals::misc::speedvalue, 1.0f, 500.0f, ("Speed: %.0f"));
                }

                ImGui::Checkbox(("Fly"), &globals::misc::flight);
                ImGui::SameLine(ImGui::GetWindowWidth() - 55);
                Hotkey(globals::misc::flightkeybind, ImVec2(50, 15), 7);
                
                if (globals::misc::flight) {
                    const char* fly_modes[] = { "Position", "Velocity" };
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::Combo(("##FlyMode"), &globals::misc::flighttype, fly_modes, IM_ARRAYSIZE(fly_modes));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##FlySpeed"), &globals::misc::flightvalue, 1.0f, 100.0f, ("Fly Speed: %.0f"));
                }

                ImGui::Checkbox(("Jump Power"), &globals::misc::jumppower);
                ImGui::SameLine(ImGui::GetWindowWidth() - 55);
                Hotkey(globals::misc::jumppowerkeybind, ImVec2(50, 15), 8);
                
                if (globals::misc::jumppower) {
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##JumpPowerValue"), &globals::misc::jumppowervalue, 0.0f, 500.0f, ("Jump Power: %.0f"));
                }

                ImGui::Checkbox(("No Jump Cooldown"), &globals::misc::nojumpcooldown);

                ImGui::EndChild();

                ImGui::SameLine();

                // --- Right Column: Extra ---
                ImGui::BeginChild("##2", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                wp = ImGui::GetWindowPos();

                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 1 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                );
                ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );

                ImGui::Text("Extra");
                
                ImGui::Checkbox(("Orbit"), &globals::combat::orbit);
                ImGui::SameLine(ImGui::GetWindowWidth() - 55);
                Hotkey(globals::combat::orbitkeybind, ImVec2(50, 15), 9);
                
                if (globals::combat::orbit) {
                    const char* orbit_modes[] = { "Random", "X Axis", "Y Axis" };
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::Combo(("##OrbitMode"), &globals::combat::orbittype, orbit_modes, IM_ARRAYSIZE(orbit_modes));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##OrbitSpeed"), &globals::combat::orbitspeed, 1.0f, 50.0f, ("Speed: %.0f"));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##OrbitHeight"), &globals::combat::orbitheight, 0.0f, 20.0f, ("Height: %.0f"));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##OrbitRange"), &globals::combat::orbitrange, 1.0f, 50.0f, ("Range: %.0f"));
                }

                ImGui::Checkbox(("360 Camera"), &globals::misc::rotate360);
                ImGui::SameLine(ImGui::GetWindowWidth() - 55);
                Hotkey(globals::misc::rotate360keybind, ImVec2(50, 15), 10);
                
                if (globals::misc::rotate360) {
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##Rotate360Speed"), &globals::misc::rotate360_speed, 1.0f, 30.0f, ("360 Speed: %.0f"));
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##Rotate360VSpeed"), &globals::misc::rotate360_vspeed, 0.0f, 30.0f, ("Vertical Speed: %.0f"));
                }

                ImGui::Checkbox(("Macro"), &globals::misc::macro_enabled);
                ImGui::SameLine(ImGui::GetWindowWidth() - 55);
                Hotkey(globals::misc::macro_keybind, ImVec2(50, 15), 11);

                if (globals::misc::macro_enabled) {
                    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                    ImGui::SliderFloatManual(("##MacroDelay"), &globals::misc::macro_delay, 1.0f, 100.0f, ("Delay: %.0f ms"));
                }

                ImGui::EndChild();
                ImGui::EndChild();
            }
            if (currenttab == "Settings")
            {
                ImGui::Dummy({ 0,0 });
                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::WindowBG);
                ImGui::BeginChild("##holder", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, false);
                
                float col_width = ImGui::GetContentRegionAvail().x / 2 - 5;
                float col_height = ImGui::GetContentRegionAvail().y;

                // --- Left Column ---
                ImGui::BeginChild("##left_col_settings", { col_width, col_height }, false);
                {
                    style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::Child);
                    
                    // Section 1: Settings (Top Left)
                    ImGui::BeginChild("##settings_sec_mini", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 1.7f }, true);
                    wp = ImGui::GetWindowPos();
                    ImGui::GetWindowDrawList()->AddLine(
                        { wp.x + 1, wp.y + 1 },
                        { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 1 },
                        ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                    );
                    ImGui::GetWindowDrawList()->AddShadowRect(
                        { wp.x + 1, wp.y + 1 },
                        { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 3 },
                        ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                    );
                    ImGui::Text("Configs");
                    cfg.render_config_ui();
                    ImGui::EndChild();

                    ImGui::Spacing();

                    // Section 2: Main (Bottom Left)
                    ImGui::BeginChild("##main_sec_mini", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y }, true);
                    wp = ImGui::GetWindowPos();
                    ImGui::GetWindowDrawList()->AddLine(
                        { wp.x + 1, wp.y + 1 },
                        { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 1 },
                        ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                    );
                    ImGui::GetWindowDrawList()->AddShadowRect(
                        { wp.x + 1, wp.y + 1 },
                        { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 3 },
                        ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                    );
                    ImGui::Text("Settings");
                    ImGui::Checkbox("Stream Proof", &globals::misc::streamproof);
                    ImGui::Checkbox("Keybind List", &globals::misc::keybinds);
                                        ImGui::Checkbox("Unlock FPS", &globals::misc::unlock_fps);
                    if (globals::misc::unlock_fps) {
                        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                        ImGui::SliderFloatManual("##fps_cap", &globals::misc::fps_cap, 1.0f, 1000.0f, "Max FPS: %.0f");
                    }
                    ImGui::Text(("Menu Key"));
                    ImGui::SameLine(ImGui::GetWindowWidth() - 55.0f);
                    Hotkey(globals::misc::menu_hotkey, ImVec2(50, 15), 64);
                    ImGui::EndChild();
                }
                ImGui::EndChild();

                ImGui::SameLine();

                // --- Right Column ---
                // Section 3: Theme (Right)
                style->Colors[ImGuiCol_ChildBg] = ColorFromFloat(globals::misc::Child);
                ImGui::BeginChild("##theme_sec", { ImGui::GetContentRegionAvail().x, col_height }, true);
                wp = ImGui::GetWindowPos();
                ImGui::GetWindowDrawList()->AddLine(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 1 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                );
                ImGui::GetWindowDrawList()->AddShadowRect(
                    { wp.x + 1, wp.y + 1 },
                    { wp.x + ImGui::GetWindowSize().x - 1, wp.y + 3 },
                    ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                );
                ImGui::Text("Theme");
                ImGui::Spacing();

                auto themeColorPicker = [&](const char* label, float* color, bool alpha = true) {
                    ImGui::Text(label);
                    ImGui::SameLine(ImGui::GetWindowWidth() - 35.0f);
                    ImGui::SetNextItemWidth(20.0f);
                    ImGui::ColorEdit4((std::string("##") + label).c_str(), color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
                };

                ImGui::BeginChild("##theme_scroll", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
                {
                    themeColorPicker("Accent", globals::misc::ThemeColor);
                    themeColorPicker("Accent active", globals::misc::AccentActive);
                    themeColorPicker("Overlay border", globals::misc::OverlayBorder);
                    themeColorPicker("Window background", globals::misc::WindowBG, false);
                    themeColorPicker("Child background", globals::misc::Child, false);
                    themeColorPicker("Header", globals::misc::Header);
                    themeColorPicker("Popup background", globals::misc::PopupBG);
                    themeColorPicker("Text", globals::misc::Text);
                    themeColorPicker("Text disabled", globals::misc::TextDisabled);
                    themeColorPicker("Border", globals::misc::Border, false);
                    themeColorPicker("Button", globals::misc::Button);
                    themeColorPicker("Button hovered", globals::misc::ButtonHovered);
                    themeColorPicker("Button active", globals::misc::ButtonActive);
                    themeColorPicker("Frame background", globals::misc::FrameBG);
                    themeColorPicker("Frame background hovered", globals::misc::FrameBGHovered);
                    themeColorPicker("Frame background active", globals::misc::FrameBGActive);
                    themeColorPicker("Scrollbar background", globals::misc::ScrollbarBG);
                    themeColorPicker("Scrollbar grab", globals::misc::ScrollbarGrab);
                    themeColorPicker("Scrollbar grab hovered", globals::misc::ScrollbarGrabHovered);
                    themeColorPicker("Scrollbar grab active", globals::misc::ScrollbarGrabActive);
                    themeColorPicker("Slider grab", globals::misc::SliderGrab);
                    themeColorPicker("Slider grab active", globals::misc::SliderGrabActive);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Text("Font");
                    ImGui::SameLine(ImGui::GetWindowWidth() - 110.0f);
                    ImGui::SetNextItemWidth(100.0f);
                    static const char* font_names[] = { "Tahoma", "Default", "Verdana" };
                    ImGui::Combo("##font_changer", &globals::misc::font_index, font_names, IM_ARRAYSIZE(font_names));

                    if (ImGui::Button(("Export Theme"), { ImGui::GetContentRegionAvail().x / 2 - 2, 21 }))
                    {
                        json config;
                        config["theme"]["Accent"] = { globals::misc::ThemeColor[0], globals::misc::ThemeColor[1], globals::misc::ThemeColor[2], globals::misc::ThemeColor[3] };
                        config["theme"]["AccentActive"] = { globals::misc::AccentActive[0], globals::misc::AccentActive[1], globals::misc::AccentActive[2], globals::misc::AccentActive[3] };
                        config["theme"]["OverlayBorder"] = { globals::misc::OverlayBorder[0], globals::misc::OverlayBorder[1], globals::misc::OverlayBorder[2], globals::misc::OverlayBorder[3] };
                        config["theme"]["WindowBG"] = { globals::misc::WindowBG[0], globals::misc::WindowBG[1], globals::misc::WindowBG[2] };
                        config["theme"]["Child"] = { globals::misc::Child[0], globals::misc::Child[1], globals::misc::Child[2] };
                        config["theme"]["Header"] = { globals::misc::Header[0], globals::misc::Header[1], globals::misc::Header[2], globals::misc::Header[3] };
                        config["theme"]["PopupBG"] = { globals::misc::PopupBG[0], globals::misc::PopupBG[1], globals::misc::PopupBG[2], globals::misc::PopupBG[3] };
                        config["theme"]["Text"] = { globals::misc::Text[0], globals::misc::Text[1], globals::misc::Text[2], globals::misc::Text[3] };
                        config["theme"]["TextDisabled"] = { globals::misc::TextDisabled[0], globals::misc::TextDisabled[1], globals::misc::TextDisabled[2], globals::misc::TextDisabled[3] };
                        config["theme"]["Border"] = { globals::misc::Border[0], globals::misc::Border[1], globals::misc::Border[2] };
                        config["theme"]["Button"] = { globals::misc::Button[0], globals::misc::Button[1], globals::misc::Button[2], globals::misc::Button[3] };
                        config["theme"]["ButtonHovered"] = { globals::misc::ButtonHovered[0], globals::misc::ButtonHovered[1], globals::misc::ButtonHovered[2], globals::misc::ButtonHovered[3] };
                        config["theme"]["ButtonActive"] = { globals::misc::ButtonActive[0], globals::misc::ButtonActive[1], globals::misc::ButtonActive[2], globals::misc::ButtonActive[3] };
                        config["theme"]["FrameBG"] = { globals::misc::FrameBG[0], globals::misc::FrameBG[1], globals::misc::FrameBG[2], globals::misc::FrameBG[3] };
                        config["theme"]["FrameBGHovered"] = { globals::misc::FrameBGHovered[0], globals::misc::FrameBGHovered[1], globals::misc::FrameBGHovered[2], globals::misc::FrameBGHovered[3] };
                        config["theme"]["FrameBGActive"] = { globals::misc::FrameBGActive[0], globals::misc::FrameBGActive[1], globals::misc::FrameBGActive[2], globals::misc::FrameBGActive[3] };
                        config["theme"]["ScrollbarBG"] = { globals::misc::ScrollbarBG[0], globals::misc::ScrollbarBG[1], globals::misc::ScrollbarBG[2], globals::misc::ScrollbarBG[3] };
                        config["theme"]["ScrollbarGrab"] = { globals::misc::ScrollbarGrab[0], globals::misc::ScrollbarGrab[1], globals::misc::ScrollbarGrab[2], globals::misc::ScrollbarGrab[3] };
                        config["theme"]["ScrollbarGrabHovered"] = { globals::misc::ScrollbarGrabHovered[0], globals::misc::ScrollbarGrabHovered[1], globals::misc::ScrollbarGrabHovered[2], globals::misc::ScrollbarGrabHovered[3] };
                        config["theme"]["ScrollbarGrabActive"] = { globals::misc::ScrollbarGrabActive[0], globals::misc::ScrollbarGrabActive[1], globals::misc::ScrollbarGrabActive[2], globals::misc::ScrollbarGrabActive[3] };
                        config["theme"]["SliderGrab"] = { globals::misc::SliderGrab[0], globals::misc::SliderGrab[1], globals::misc::SliderGrab[2], globals::misc::SliderGrab[3] };
                        config["theme"]["SliderGrabActive"] = { globals::misc::SliderGrabActive[0], globals::misc::SliderGrabActive[1], globals::misc::SliderGrabActive[2], globals::misc::SliderGrabActive[3] };
                        
                        ImGui::SetClipboardText(config.dump(4).c_str());
                        ImGui::OpenPopup(("Config Exported"));
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(("Import Theme"), { ImGui::GetContentRegionAvail().x, 21 }))
                    {
                        const char* clipboardText = ImGui::GetClipboardText();
                        if (clipboardText && clipboardText[0] != '\0')
                        {
                            try {
                                json config = json::parse(clipboardText);
                                if (config.contains("theme")) {
                                    auto& colors = config["theme"];
                                    auto loadColor = [&](const char* key, float* target, int count) {
                                        if (colors.contains(key)) {
                                            for (int i = 0; i < count; i++) target[i] = colors[key][i].get<float>();
                                        }
                                    };
                                    loadColor("Accent", globals::misc::ThemeColor, 4);
                                    loadColor("AccentActive", globals::misc::AccentActive, 4);
                                    loadColor("OverlayBorder", globals::misc::OverlayBorder, 4);
                                    loadColor("WindowBG", globals::misc::WindowBG, 3);
                                    loadColor("Child", globals::misc::Child, 3);
                                    loadColor("Header", globals::misc::Header, 4);
                                    loadColor("PopupBG", globals::misc::PopupBG, 4);
                                    loadColor("Text", globals::misc::Text, 4);
                                    loadColor("TextDisabled", globals::misc::TextDisabled, 4);
                                    loadColor("Border", globals::misc::Border, 3);
                                    loadColor("Button", globals::misc::Button, 4);
                                    loadColor("ButtonHovered", globals::misc::ButtonHovered, 4);
                                    loadColor("ButtonActive", globals::misc::ButtonActive, 4);
                                    loadColor("FrameBG", globals::misc::FrameBG, 4);
                                    loadColor("FrameBGHovered", globals::misc::FrameBGHovered, 4);
                                    loadColor("FrameBGActive", globals::misc::FrameBGActive, 4);
                                    loadColor("ScrollbarBG", globals::misc::ScrollbarBG, 4);
                                    loadColor("ScrollbarGrab", globals::misc::ScrollbarGrab, 4);
                                    loadColor("ScrollbarGrabHovered", globals::misc::ScrollbarGrabHovered, 4);
                                    loadColor("ScrollbarGrabActive", globals::misc::ScrollbarGrabActive, 4);
                                    loadColor("SliderGrab", globals::misc::SliderGrab, 4);
                                    loadColor("SliderGrabActive", globals::misc::SliderGrabActive, 4);
                                }
                                ImGui::OpenPopup(("Config Imported"));
                            } catch (...) { ImGui::OpenPopup(("Import Error")); }
                        } else { ImGui::OpenPopup(("Import Error")); }
                    }
                }
                ImGui::EndChild(); // ##theme_scroll
                ImGui::EndChild(); // ##theme_sec
                ImGui::EndChild(); // ##holder
            }
   



            ImGui::EndChild();
            ImGui::EndChild();
            ImGui::GetBackgroundDrawList()->AddShadowRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), ImColor(ImVec4(globals::misc::ThemeColor[0], globals::misc::ThemeColor[1], globals::misc::ThemeColor[2], globals::misc::ThemeColor[3])), 100.0f, { 0,0 });

            ImGui::End();


            if (show_bulk_add) {
                ImGui::SetNextWindowSize(ImVec2(450, 400), ImGuiCond_FirstUseEver);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ColorFromFloat(globals::misc::WindowBG));
                ImGui::PushStyleColor(ImGuiCol_Border, ColorFromFloat(globals::misc::Border));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
                
                if (ImGui::Begin("Bulk Add Targets", &show_bulk_add, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
                    // Custom Header
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    float width = ImGui::GetWindowWidth();
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();

                    // Outside Border matching main overlay
                    ImGui::GetBackgroundDrawList()->AddRect(
                        { ImGui::GetWindowPos().x - 2, ImGui::GetWindowPos().y - 2 },
                        { ImGui::GetWindowPos().x + ImGui::GetWindowSize().x + 2, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y + 2 },
                        ImColor(VecToColor(ColorFromFloat(globals::misc::OverlayBorder))), 2.0f, 0, 2.0f
                    );
                    
                    // Header Background
                    draw_list->AddRectFilled(ImVec2(p.x - 10, p.y - 10), ImVec2(p.x + width - 10, p.y + 25), ImColor(ColorFromFloat(globals::misc::Child)));
                    
                    // Drag Logic using InvisibleButton over the header
                    ImGui::SetCursorScreenPos(ImVec2(p.x - 10, p.y - 10));
                    if (ImGui::InvisibleButton("##drag_header", ImVec2(width, 35))) {
                        // Clicked header (focus check handled by ImGui automatically)
                    }
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                        ImVec2 delta = ImGui::GetIO().MouseDelta;
                        ImVec2 win_pos = ImGui::GetWindowPos();
                        ImGui::SetWindowPos(ImVec2(win_pos.x + delta.x, win_pos.y + delta.y));
                    }
                    ImGui::SetCursorScreenPos(p); // Restore cursor

                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); // Center text vertically in header
                    ImGui::Text("Bulk Add Targets");
                    
                    // Add pink accent line under header
                    draw_list->AddLine(
                        ImVec2(p.x - 10, p.y + 25),
                        ImVec2(p.x + width - 10, p.y + 25),
                        ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 2.0f
                    );
                    draw_list->AddShadowRect(
                        ImVec2(p.x - 10, p.y + 25),
                        ImVec2(p.x + width - 10, p.y + 27),
                        ImColor(VecToColor(ImGui::GetStyleColorVec4(ImGuiCol_CheckMark))), 20.f, { 0,0 }
                    );

                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10); // Space after header
                    ImGui::Spacing();

                    static char bulk_audit[1024 * 16] = ""; // 16KB buffer
                    ImGui::Text("Paste list of usernames or display names (one per line):");
                    
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ColorFromFloat(globals::misc::Child));
                    ImGui::PushStyleColor(ImGuiCol_Border, ColorFromFloat(globals::misc::Border));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                    ImGui::InputTextMultiline("##bulk", bulk_audit, IM_ARRAYSIZE(bulk_audit), ImVec2(-1, -40)); // Fill width, leave space for buttons
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(2);
                    
                    ImGui::Dummy(ImVec2(0, 5));

                    // Center buttons
                    float available_width = ImGui::GetContentRegionAvail().x;
                    float button_width = 120.0f;
                    float spacing = 10.0f;
                    float total_button_width = (button_width * 2) + spacing;
                    float start_x = (available_width - total_button_width) * 0.5f;

                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + start_x);

                    ImGui::PushStyleColor(ImGuiCol_Border, ColorFromFloat(globals::misc::Border));
                    if (ImGui::Button("Add All", ImVec2(button_width, 0))) {
                        std::string text = bulk_audit;
                        size_t pos = 0;

                        while (pos < text.length()) {
                            size_t next_pos = text.find('\n', pos);
                            if (next_pos == std::string::npos) next_pos = text.length();

                            std::string line = text.substr(pos, next_pos - pos);
                            
                            // Trim
                            while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t')) line.pop_back();
                            size_t start = 0;
                            while (start < line.length() && (line[start] == ' ' || line[start] == '\t')) start++;
                            line = line.substr(start);

                            if (!line.empty()) {
                                std::string resolved_username = line;
                                bool resolved = false;

                                {
                                    std::lock_guard<std::mutex> lock(globals::instances::cachedplayers_mutex);
                                     for (auto& player : globals::instances::cachedplayers) {
                                        std::string best_display = displaynames::get_best_display(player);
                                        if (_stricmp(player.name.c_str(), line.c_str()) == 0 || 
                                            _stricmp(best_display.c_str(), line.c_str()) == 0) {
                                            resolved_username = player.name;
                                            resolved = true;
                                            std::cout << "[DEBUG] Resolved '" << line << "' -> '" << resolved_username << "'" << std::endl;
                                            break;
                                        }
                                    }
                                }
                                
                                if (!resolved) {
                                     std::cout << "[DEBUG] Could not resolve '" << line << "' - skipping" << std::endl;
                                     pos = next_pos + 1;
                                     continue;
                                }

                                // Add to target list if unique
                                bool exists = false;
                                for (const auto& existing : globals::visuals::target_only_list) {
                                    if (existing == resolved_username) { exists = true; break; }
                                }
                                if (!exists) {
                                    globals::visuals::target_only_list.push_back(resolved_username);
                                }
                                // Force Enemy status
                                globals::bools::player_status[resolved_username] = false;
                            }
                            pos = next_pos + 1;
                        }
                        // Auto-enable features
                        globals::visuals::target_only_esp = true;
                        
                        bulk_audit[0] = '\0';
                        show_bulk_add = false;
                    }
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + spacing);
                    if (ImGui::Button("Cancel", ImVec2(button_width, 0))) {
                        show_bulk_add = false;
                    }
                    ImGui::PopStyleColor();
                }
                ImGui::GetBackgroundDrawList()->AddShadowRect(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), ImColor(ImVec4(globals::misc::ThemeColor[0], globals::misc::ThemeColor[1], globals::misc::ThemeColor[2], globals::misc::ThemeColor[3])), 100.0f, { 0,0 });
                ImGui::End();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(2);
            }
        #endif
            if (selected_font) ImGui::PopFont();
        }
        if (draw)
        {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        }
        else
        {
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
        }
        
        // Always keep overlay on top when menu is open, otherwise follow focus
        if (draw)
        {
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            // Force window to stay on top and ensure it's visible
            BringWindowToTop(hwnd);
        }
        else if (globals::focused)
        {
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        else
        {
            SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        if (globals::misc::streamproof)
        {
            SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
        }
        else
        {
            SetWindowDisplayAffinity(hwnd, WDA_NONE);
        }
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(globals::misc::performance_mode ? 1 : 0, 0);
        
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    exit(0) ;
}



// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
