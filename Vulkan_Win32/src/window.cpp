#include "window.h"
#include <Renderer.h>
#include <renderer/basicrenderer.h>
#include <renderer/vertexbufferr.h>

Window::Window(uint32_t width, uint32_t height, const std::string &title)
	: width(width),height(height), window_name(title), isRunning(true),m_renderer(nullptr)
{
	build_window();
	//m_renderer = new VkRenderer(this, title);
	//m_renderer = new Renderer(this, title);
	//m_renderer = new BasicRenderer(this);
	m_renderer = new VertexBufferr(this);
	m_renderer->buildProcedural();
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
		/*if(msg.message == WM_COMMAND)
			if (msg.wParam == BN_CLICKED)
				test();*/
		//if(msg.message == WM_)
		//LOG << "hello" << ENDL;
		m_renderer->render();
		//m_renderer->darwFrame();
	}
	//wait 
	vkDeviceWaitIdle(m_renderer->m_device);
}


