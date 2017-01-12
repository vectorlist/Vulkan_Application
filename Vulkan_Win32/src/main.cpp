
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
	Window window(820, 672, "Vulkan Application");
	window.loop();
	
	/*std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	vkMesh::LoadModel("./obj/chalet.obj", &vertices, &indices);*/


	return 0;
}

