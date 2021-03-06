
#pragma once

#include <vkutils.h>

class Renderer;
class Window
{
public:
	Window(uint32_t width, uint32_t height, const std::string &title);
	~Window();

	void event_handler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND					m_window;
	HINSTANCE				m_instance;
	uint32_t width;
	uint32_t height;
private:
	bool isRunning;
	void build_window();

	Renderer* m_renderer;

	std::string window_name;

	static uint64_t win32_ids;
	
	std::string				class_name;
public:
	void loop();
	//test
	void test(float x, float y);
	struct
	{
		float x=  0.0f;
		float y = 0.0f;
	}mousePos;
	bool onMove = false;
	bool viewchange = false;
protected:

	void deleteWindow();

	void resize();

	
};

