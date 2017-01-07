
//add Commander buffer
#include <Renderer.h>
#include <window.h>

void setConsoleGeometry(int x, int y, int w, int h)
{
	HWND console = GetConsoleWindow();
	MoveWindow(console,x, y, w, h,TRUE);
}

int main(int argc, char* argv[])
{
	setConsoleGeometry(15, 15, 890, 1000);
	try
	{
		Window window(512, 512, "Vulkan Application");

		window.loop();
	}
	catch (const std::runtime_error& e)
	{
		LOG << e.what() << ENDL;
		//std::cerr << e.what() << std::endl;
		MessageBox(NULL, e.what(), NULL, MB_ICONSTOP);
		return EXIT_FAILURE;
	}
	

	return 0;
}

