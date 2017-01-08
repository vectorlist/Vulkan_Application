
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

#define VK_UINT32_MAX std::numeric_limits<unsigned int>::max()
#define VK_UINT64_MAX std::numeric_limits<uint64_t>::max()

#define VK_OVERRIDE		override
#define VK_ERROR(x) throw std::runtime_error(#x) 
#define LOG_ERROR(x) Log::proxylog(x)
#define LOG_ASSERT(x) Log::proxylog.log_assert(x)

#define LOG std::cout
#define ENDL std::endl

#define LOG_SECTION(x) vkDebug::log_section(#x)

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

	void log_section(const std::string &msg);
}

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
		//assert(0 && "error : ");
		MessageBox(NULL, m_msg.c_str(), "debug error", MB_ICONSTOP);
	}
	void log_assert(const std::string &msg)
	{
		MessageBox(NULL, m_msg.c_str(), "debug error", MB_ICONSTOP);
		assert(0 && m_msg.c_str());
	}

	std::string m_msg;
};




