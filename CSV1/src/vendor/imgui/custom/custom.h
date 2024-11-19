#pragma once

#include <Windows.h>
#include <iostream>
#include "imgui/imgui_impl_opengl3.h"

namespace ImGui
{
	void TextCentered(const std::string& text);
	void ImageCentered(ImTextureID user_texture_id, const ImVec2& size);
	bool HelpMarker(const std::string& desc);
	void InputKeyAndMouse(const std::string& keyLabel, const std::string& mouseLabel, UINT* Key, UINT* MouseButton, int CurrentMouseButton);
	void AddUnderLine(ImColor col_);
}