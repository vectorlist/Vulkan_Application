
//add Commander buffer
#include <VkRenderer.h>
#include <window.h>

void setConsoleGeometry(int x, int y, int w, int h)
{
	HWND console = GetConsoleWindow();
	MoveWindow(console,x, y, w, h,TRUE);
}

int main(int argc, char* argv[])
{
	setConsoleGeometry(15, 15, 890, 1000);
	Window window(512,512,"vulakn window");
	window.loop();

	return 0;
}

