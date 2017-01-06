#include "window.h"
#include <VkRenderer.h>
#include <vktriangle.h>

Window::Window(uint32_t width, uint32_t height, const std::string &title)
	: width(width),height(height), window_name(title), isRunning(true),m_renderer(nullptr)
{
	build_window();
	m_renderer = new VkRenderer(this, title);
	//m_renderer = new VkTriangle(this, title);
	m_renderer->build_procedural();
}


Window::~Window()
{
	deleteWindow();
	if (m_renderer)
		delete m_renderer;
}

void Window::loop()
{
	MSG msg;
	//MSG m = GetMessage();
	while (TRUE)
	{
		if (PeekMessage(&msg, /*window*/NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			break;
		}
		//LOG << "hello" << ENDL;
		m_renderer->render();
	}
}


