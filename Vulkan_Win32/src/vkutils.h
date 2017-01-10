
#pragma once

#include <vulkan/vulkan.h>
#include <Windows.h>
#include <vector>
#include <io.h>
#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <array>
#include <set>
#include <limits>
#include <keycode.h>
#include <chrono>

//---------------------
#include <matrix4x4.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//----------------------------

#define LOG std::cout
#define ENDL std::endl

#ifdef __cplusplus
#define VK_UINT32_MAX std::numeric_limits<unsigned int>::max()
//#define VK_UINT64_MAX std::numeric_limits<uint64_t>::max()
#define VK_UINT64_MAX UINT64_MAX
#else
#define VK_UINT32_MAX UINT32_MAX
#define VK_UINT64_MAX UINT64_MAX
#endif

#define VK_OVERRIDE		override

#define LOG_ERROR(x) Log::proxylog(x)
#define LOG_ASSERT(x) Log::proxylog.log_assert(x)
#define LOG_SECTION(x) Log::proxylog.log_section(x)

#define CLASS_TYPE(x) Log::proxylog.get_class_type(x)

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

extern VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
	size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);


namespace vkDebug
{
	VkResult CreateDebugReportCallbackEXT(
		VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);

	void DestroyDebugReportCallbackEXT(
		VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);

	
}

   /*TYPEDEF*/
struct QueueFamilyIndeice
{
	int graphicsFamily = -1;
	int presentFamily = -1;
	bool isComplete() { return graphicsFamily >= 0 && presentFamily >= 0; }
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct UBO
{
	/*Matrix4x4 model;
	Matrix4x4 view;
	Matrix4x4 proj;*/
	glm::mat4x4 model;
	glm::mat4x4 view;
	glm::mat4x4 proj;
};

namespace vkTool
{
	std::vector<const char*> getRequiredExtenstions();
	std::vector<char> readfile(const std::string &filename);
}

class Log
{
public:
	static Log proxylog;

	Log& operator()(const std::string &msg)
	{
		//for << operator return this object
		m_msg = msg;
		return *this;
	}
	void operator<<(const VkResult &code)
	{
		if (code == VK_SUCCESS) return;
		MessageBox(NULL, m_msg.c_str(), "debug error", MB_ICONSTOP);
	}
	void log_assert(const std::string &msg)
	{
		MessageBox(NULL, msg.c_str(), "assert error", MB_ICONSTOP);
		assert(0 && m_msg.c_str());
		std::exit(1);
	}

	void log_section(const std::string &msg);

	template<typename T>
	std::string get_class_type(T object)
	{
		std::string tname = typeid(object).name();
		return tname;
	}
	std::string m_msg;
};



