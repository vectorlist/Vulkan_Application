#include <window.h>

uint64_t Window::win32_ids = 0;

//call back poiint for win32
LRESULT CALLBACK window_event_handler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window *window = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
	if(window != nullptr) 
		window->event_handler(hWnd, uMsg, wParam, lParam);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Window::event_handler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		//prepared = false;
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		ValidateRect(m_window, NULL);
		break;
	}
}

void Window::build_window()
{
	WNDCLASSEX win_class{};


	m_instance = GetModuleHandle(nullptr);
	class_name = window_name + " " + std::to_string(win32_ids);
	win32_ids++;			//static

	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = window_event_handler;		//event handler
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = m_instance;
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = class_name.c_str();
	win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	//register class
	
	if (!RegisterClassEx(&win_class)) {
		assert(0 && "failed to register to class");
		fflush(stdout);
		std::exit(-1);
	}

	//properties
	
	DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX ;

	RECT rect = { 0L, 0L, LONG(width), LONG(height) };
	AdjustWindowRectEx(&rect, style, FALSE, ex_style);

	//HWND m_window
	m_window = CreateWindow(
			class_name.c_str(),				//lpClassName,
			window_name.c_str(),			//lpWindowName,
			style,							//dwStyle,
			CW_USEDEFAULT,					//x,
			CW_DEFAULT,						//y,
			rect.right - rect.left,			//nWidth,
			rect.bottom - rect.top,			//nHeight,
			NULL,							//hWndParent,
			NULL,							//hMenu,
			m_instance,						//hInstance,
			NULL							//lpParam
		);

	//position
	uint32_t x = ((GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2) + 200;
	uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom) / 2;
	SetWindowPos(m_window, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	SetWindowLongPtr(m_window, GWLP_USERDATA, (LONG_PTR)this);
	ShowWindow(m_window, SW_SHOW);
	SetForegroundWindow(m_window);
	SetFocus(m_window);
}


void Window::deleteWindow()
{
	DestroyWindow(m_window);
	UnregisterClass(class_name.c_str(), m_instance);
}



