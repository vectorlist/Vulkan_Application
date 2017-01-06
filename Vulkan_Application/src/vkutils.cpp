#include "vkutils.h"

vkDebugger vkDebugger::globalVkDebugger;

#define RESULTTOSTR(x) case VK_ ##x: return #x 

std::string vkDebugger::checkError(VkResult code)
{
	if (code != VK_SUCCESS)
	{
		switch (code)
		{
		RESULTTOSTR(NOT_READY);
		RESULTTOSTR(TIMEOUT);
		RESULTTOSTR(EVENT_SET);
		RESULTTOSTR(EVENT_RESET);
		RESULTTOSTR(INCOMPLETE);
		RESULTTOSTR(ERROR_OUT_OF_HOST_MEMORY);
		RESULTTOSTR(ERROR_OUT_OF_DEVICE_MEMORY);
		RESULTTOSTR(ERROR_DEVICE_LOST);
		RESULTTOSTR(ERROR_MEMORY_MAP_FAILED);
		RESULTTOSTR(ERROR_LAYER_NOT_PRESENT);
		RESULTTOSTR(ERROR_EXTENSION_NOT_PRESENT);
		RESULTTOSTR(ERROR_FEATURE_NOT_PRESENT);

		RESULTTOSTR(ERROR_INCOMPATIBLE_DRIVER);
		RESULTTOSTR(ERROR_TOO_MANY_OBJECTS);
		RESULTTOSTR(ERROR_FORMAT_NOT_SUPPORTED);
		RESULTTOSTR(ERROR_FRAGMENTED_POOL);
		RESULTTOSTR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
		RESULTTOSTR(SUBOPTIMAL_KHR);
		RESULTTOSTR(ERROR_OUT_OF_DATE_KHR);
		RESULTTOSTR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
		RESULTTOSTR(ERROR_VALIDATION_FAILED_EXT);
		RESULTTOSTR(ERROR_INVALID_SHADER_NV);
#undef RESTULTTOSTR
		default:
			return "UNKOWN_TYPE";
		}
		assert(0 && "Vulkan Runtime Rrror");
	}
	//if pass here code is VK_SUCESS
	return "SUCCESS";
}

namespace vkTools
{
	uint32_t FindMemoryTypeIndex(
		const VkPhysicalDeviceMemoryProperties *gpu_memory_properties,
		const VkMemoryRequirements *memory_requirements,
		const VkMemoryPropertyFlags required_properties)
	{
		for (uint32_t i = 0; i < gpu_memory_properties->memoryTypeCount; ++i) {
			if (memory_requirements->memoryTypeBits & (1 << i)) {
				if ((gpu_memory_properties->memoryTypes[i].propertyFlags & required_properties) == required_properties) {
					return i;
				}
			}
		}
		//vkQuitAssert("couldnt find property memory type");
		assert(0 && "couldnt find property memory type");
		return UINT32_MAX;
	}
}

void vkDebugger::quitError(const std::string &str) 
{
	assert(0 && str.c_str());
	std::exit(-1);
}

void vkDebugger::log_section(const std::string &msg)
{
	size_t line_size = 38;
	size_t size = msg.size();
	std::string bit("");
	std::string line(line_size - (size / 2), '=');

	bool rounded = false;
	if ((size % 2) == 0)
		rounded = true;

	bit.append(line+' ').append(msg).append(' '+line);
	if (!rounded)
		bit.pop_back();
	std::cout << bit << std::endl;
}

//vkDebugger& resultCheck(const const std::string &type)
//{
//	vkError(type) << 
//	return vkDebugger::globalVkDebugger;
//	//bool check = this << 
//	/*if(type == VK_SUCCESS)
//		LOG << "VK_SUCCESS"
//	vkError() */
//
//	
//}
