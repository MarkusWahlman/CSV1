#include "custom.h"

#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#include "GLFW/glfw3.h"

static UINT GLFWMouseToWindowsVK(int GLFWMouse)
{
    switch (GLFWMouse)
    {
        case GLFW_MOUSE_BUTTON_1:
            return VK_LBUTTON;
        case GLFW_MOUSE_BUTTON_2:
            return VK_RBUTTON;
        case GLFW_MOUSE_BUTTON_3:
            return VK_MBUTTON;
        case GLFW_MOUSE_BUTTON_4:
            return VK_XBUTTON1;
        case GLFW_MOUSE_BUTTON_5:
            return VK_XBUTTON2;
        default:
            return 0;
    }
       
}

static std::string VKMouseToString(UINT VKMouse)
{
    switch (VKMouse)
    {
    case VK_LBUTTON:
        return "M1";
    case VK_RBUTTON:
        return "M2";
    case VK_MBUTTON:
        return "M3";
    case VK_XBUTTON1:
        return "M4";
    case VK_XBUTTON2:
        return "M5";
    default:
        return "";
    }
}

namespace ImGui
{
    void TextCentered(const std::string& text) {
        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text(text.c_str());
    }

    void ImageCentered(ImTextureID user_texture_id, const ImVec2& size)
    {
        SetCursorPosX((GetWindowSize().x - size.x) * 0.5f);
        ImGui::Image(user_texture_id, size);
    }

    bool HelpMarker(const std::string& desc) {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
        return IsItemHovered();
    }

    void InputKeyAndMouse(const std::string& keyLabel, const std::string& mouseLabel, UINT* Key, UINT* MouseButton, int CurrentMouseButton)
    {
        HKL KeyboardLayout = GetKeyboardLayout(0);

        static char KeyInput[2] = "\0";
        KeyInput[0] = (char)MapVirtualKeyEx(*Key, MAPVK_VK_TO_CHAR, KeyboardLayout);

        PushItemWidth(48);
        if (ImGui::InputText(keyLabel.c_str(), KeyInput, 2))
        {
            if (KeyInput[0] == '\0')
                *Key = 0;
            else
                *Key = (UINT)VkKeyScanExA(KeyInput[0], KeyboardLayout);
        }

        static int LastMouseButton = 0;
        static std::string MouseInput = "";

        SameLine();

        MouseInput = VKMouseToString(*MouseButton);
        ImGui::InputText(mouseLabel.c_str(), &MouseInput, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
        if (IsItemActive())
        {
            if (ImGui::IsKeyPressed(ImGuiKey_Backspace))
            {
                *MouseButton = 0;
                MouseInput = "";
            }

            if (LastMouseButton != CurrentMouseButton)
            {
                LastMouseButton = CurrentMouseButton;
                *MouseButton = GLFWMouseToWindowsVK(CurrentMouseButton);
            }
        }

        PopItemWidth();
    }

    void AddUnderLine(ImColor col_)
    {
        ImVec2 min = GetItemRectMin();
        ImVec2 max = GetItemRectMax();
        min.y = max.y;
        GetWindowDrawList()->AddLine(
            min, max, col_, 1.0f);
    }
}