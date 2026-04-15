#pragma once

// Lightweight wrapper to provide ImAdd namespace used by the project.
// It maps to core Dear ImGui functions and stubs any optional helpers.

#include "../imgui.h"

namespace ImAdd {
    inline bool BeginChild(const char* str_id, const ImVec2& size, bool border = false, ImGuiWindowFlags flags = 0) {
        return ImGui::BeginChild(str_id, size, border, flags);
    }

    inline void EndChild() {
        ImGui::EndChild();
    }

    inline bool Button(const char* label, const ImVec2& size = ImVec2(0, 0)) {
        return ImGui::Button(label, size);
    }

    // Project references this; provide a harmless no-op by default.
    inline void ClearWidgetStates() {}
}

// Provide helper used by project to convert float[4] colors
namespace ImGui {
    inline ImU32 ColorConvert(const float c[4]) {
        return ImGui::GetColorU32(ImVec4(c[0], c[1], c[2], c[3]));
    }
}
