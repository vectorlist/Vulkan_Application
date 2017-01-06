
#pragma once

#include <vulkan/vulkan.h>
#include <Windows.h>
#include <vector>
#include <io.h>
#include <iostream>
#include <string>
#include <assert.h>
#include <array>

#define VK_OVERRIDE		override

#define LOG std::cout
#define ENDL std::endl

#define LOG_SECTION(x) vkDebugger::globalVkDebugger.log_section(#x)
#define vkError() vkDebugger::globalVkDebugger

//#define vkError(x)				\
//{								\
//								\
//	LOG << (x) <<" is result success" << ENDL;						\
//	vkDebugger::globalVkDebugger									\
//}								\

#define vkQuitAssert(x) vkDebugger::globalVkDebugger.quitError(x)

class vkDebugger
{
public:

	static std::string checkError(VkResult code);
	//singletone global
	static vkDebugger globalVkDebugger;
	static void quitError(const std::string &str);
	void log_section(const std::string &msg);
	bool operator<<(const VkResult &code);

	//friend vkDebugger& resultCheck(const const std::string &type);
};

namespace vkTools
{
	uint32_t FindMemoryTypeIndex(
		const VkPhysicalDeviceMemoryProperties *gpu_memory_properties,
		const VkMemoryRequirements *memory_requirements,
		const VkMemoryPropertyFlags required_properties);
}

inline bool vkDebugger::operator<<(const VkResult &code)
{
	if (code == VK_SUCCESS) return true;
	std::cout << "Fatal Error : " << checkError(code) << std::endl;
	return false;

}



