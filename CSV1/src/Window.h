#pragma once

#include "GLFW/glfw3.h"
#include "RenderAPI/Texture.h"

namespace MAYUMI
{
	class Window
	{
	public:
		Window();
	   ~Window();

	   void Run();
	private:
		GLFWwindow* m_Window;
	};
}


