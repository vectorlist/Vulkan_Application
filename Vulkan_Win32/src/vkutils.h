
#pragma once

#include <vulkan/vulkan.h>
#include <Windows.h>
#include <vector>
#include <io.h>
#include <iostream>
#include <string>
#include <assert.h>
#include <array>
#include <set>

#define VK_OVERRIDE		override
#define VK_ERROR(x) throw std::runtime_error(#x)
#define LOG std::cout
#define ENDL std::endl

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

	/*static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType, 
		uint64_t obj, size_t location, int32_t code,
		const char* layerPrefix, const char* msg, void* userData);*/

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
}




