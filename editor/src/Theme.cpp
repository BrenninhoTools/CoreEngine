#include "Theme.h"

#include <imgui.h>

namespace editor {

void ApplyTheme(float scale) {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 6.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 5.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(9.0f, 5.0f);
    style.ItemSpacing = ImVec2(8.0f, 7.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 5.0f);
    style.DockingSeparatorSize = 2.0f;

    const ImVec4 accent(0.220f, 0.741f, 0.973f, 1.0f);
    const ImVec4 accentHover(0.376f, 0.800f, 0.984f, 1.0f);
    const ImVec4 accentActive(0.130f, 0.620f, 0.860f, 1.0f);

    ImVec4* c = style.Colors;
    c[ImGuiCol_Text] = ImVec4(0.90f, 0.93f, 0.97f, 1.00f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.48f, 0.53f, 0.62f, 1.00f);
    c[ImGuiCol_WindowBg] = ImVec4(0.067f, 0.075f, 0.118f, 1.00f);
    c[ImGuiCol_ChildBg] = ImVec4(0.055f, 0.063f, 0.102f, 1.00f);
    c[ImGuiCol_PopupBg] = ImVec4(0.086f, 0.098f, 0.153f, 0.98f);
    c[ImGuiCol_Border] = ImVec4(0.150f, 0.170f, 0.250f, 1.00f);
    c[ImGuiCol_FrameBg] = ImVec4(0.110f, 0.125f, 0.196f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.150f, 0.170f, 0.260f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.180f, 0.205f, 0.310f, 1.00f);
    c[ImGuiCol_TitleBg] = ImVec4(0.055f, 0.063f, 0.102f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.086f, 0.098f, 0.153f, 1.00f);
    c[ImGuiCol_MenuBarBg] = ImVec4(0.055f, 0.063f, 0.102f, 1.00f);
    c[ImGuiCol_ScrollbarBg] = ImVec4(0.055f, 0.063f, 0.102f, 0.60f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.200f, 0.230f, 0.340f, 1.00f);
    c[ImGuiCol_CheckMark] = accent;
    c[ImGuiCol_SliderGrab] = accent;
    c[ImGuiCol_SliderGrabActive] = accentActive;
    c[ImGuiCol_Button] = ImVec4(0.150f, 0.180f, 0.290f, 1.00f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.200f, 0.240f, 0.380f, 1.00f);
    c[ImGuiCol_ButtonActive] = accentActive;
    c[ImGuiCol_Header] = ImVec4(0.150f, 0.200f, 0.320f, 1.00f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.190f, 0.260f, 0.410f, 1.00f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.220f, 0.320f, 0.500f, 1.00f);
    c[ImGuiCol_Separator] = ImVec4(0.150f, 0.170f, 0.250f, 1.00f);
    c[ImGuiCol_SeparatorHovered] = accentHover;
    c[ImGuiCol_SeparatorActive] = accent;
    c[ImGuiCol_ResizeGrip] = ImVec4(0.220f, 0.741f, 0.973f, 0.25f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(0.220f, 0.741f, 0.973f, 0.60f);
    c[ImGuiCol_ResizeGripActive] = accent;
    c[ImGuiCol_Tab] = ImVec4(0.086f, 0.098f, 0.153f, 1.00f);
    c[ImGuiCol_TabHovered] = ImVec4(0.190f, 0.260f, 0.410f, 1.00f);
    c[ImGuiCol_TabSelected] = ImVec4(0.150f, 0.200f, 0.320f, 1.00f);
    c[ImGuiCol_TabDimmed] = ImVec4(0.070f, 0.080f, 0.126f, 1.00f);
    c[ImGuiCol_TabDimmedSelected] = ImVec4(0.110f, 0.145f, 0.240f, 1.00f);
    c[ImGuiCol_DockingPreview] = ImVec4(0.220f, 0.741f, 0.973f, 0.40f);
    c[ImGuiCol_DockingEmptyBg] = ImVec4(0.040f, 0.045f, 0.075f, 1.00f);
    c[ImGuiCol_TextSelectedBg] = ImVec4(0.220f, 0.741f, 0.973f, 0.30f);
    c[ImGuiCol_NavCursor] = accent;
    c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.55f);

    style.ScaleAllSizes(scale);
}

}
