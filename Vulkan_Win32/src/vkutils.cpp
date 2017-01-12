#include <vkutils.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Log Log::proxylog;

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
	size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData)
{
	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

namespace vkDebug
{
	VkResult CreateDebugReportCallbackEXT(
		VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
	{
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugReportCallbackEXT(
		VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr) 
		{
			func(instance, callback, pAllocator);
		}
	}
}


namespace vkTool
{
	std::vector<const char*> getRequiredExtenstions()
	{
		std::vector<const char*> extensions;
		extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}

	std::vector<char> readfile(const std::string & filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open())
			LOG_ASSERT("failed to load a file.");

		size_t filesize = size_t(file.tellg());

		std::vector<char> buffer(filesize);
		file.seekg(0);
		file.read(buffer.data(), filesize);
		file.close();

		return buffer;
	}
}

void Log::log_section(const std::string &msg)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	size_t line_size = (info.srWindow.Right - info.srWindow.Left) / 2;

	size_t size = msg.size();
	std::string bit("");
	std::string line(line_size - (size / 2) - 1, '-');

	bool rounded = false;
	if ((size % 2) == 0)
		rounded = true;

	bit.append(line + ' ').append(msg).append(' ' + line);
	if (!rounded)
		bit.pop_back();

	std::cout << bit << std::endl;
}

void vkMesh::LoadModel(
	const std::string &filename,
	std::vector<Vertex> *vertices,
	std::vector<uint32_t> *indices)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str()))
		LOG_ASSERT(err);

	std::unordered_map<Vertex, int> uniqueVerices = {};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex;
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.st = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f,1.0f,1.0f };

			if (uniqueVerices.count(vertex) == 0)
			{
				uniqueVerices[vertex] = vertices->size();
				vertices->push_back(vertex);
			}
			indices->push_back(uniqueVerices[vertex]);
		}
	}
	/*for (auto i : *vertices)
	{
		LOG << i.st.r << ENDL;
	}*/
	LOG << "vertices : " << vertices->size() << ENDL;
}