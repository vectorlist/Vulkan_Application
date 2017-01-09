#include <renderer.h>
#include <window.h>

//-----------------  built in functions ------------------
QueueFamilyIndeice Renderer::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndeice indices;
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
	LOG << queue_family_count << ENDL;
	std::vector<VkQueueFamilyProperties> queue_family_props(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(
		device, &queue_family_count, queue_family_props.data());

	int i = 0;
	for (const auto &queuefamily : queue_family_props)
	{
		if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 support_present = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &support_present);

		if (queuefamily.queueCount > 0 && support_present) {
			indices.presentFamily = i;
		}
		i++;
	}
	if (!indices.isComplete()) {
		LOG_ASSERT("queue indices not supported");
	}
	return indices;
}

bool Renderer::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndeice queue_indice = findQueueFamilies(device);
	//check device extension supported
	bool extensionSupport = checkExtensionSuppot(device);

	bool swapchainSupport = false;
	if (extensionSupport)
	{
		SwapChainSupportDetails details = querySwapchainSupport(device);
		swapchainSupport = (!details.formats.empty() && !details.presentModes.empty());
	}

	return (queue_indice.isComplete() && extensionSupport && swapchainSupport);
}


bool Renderer::checkExtensionSuppot(VkPhysicalDevice device)
{
	uint32_t ext_count = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &ext_count, nullptr);
	std::vector<VkExtensionProperties> extension_list(ext_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &ext_count, extension_list.data());

	LOG << "extension count : " << ext_count << ENDL;
	std::set<std::string> requiredExtension(deviceExtensions.begin(), deviceExtensions.end());

	for (auto &ex : extension_list)
	{
		requiredExtension.erase(ex.extensionName);
	}
	bool support = false;
	if (requiredExtension.empty())
	{
		support = true;
	}

	return support;
}

SwapChainSupportDetails Renderer::querySwapchainSupport(VkPhysicalDevice device)
{
	//this for get swapchain capabilities / formats / presentmodes
	SwapChainSupportDetails details;
	//get capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);
	//get format count
	uint32_t format_counts;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_counts, nullptr);

	if (format_counts != 0)
	{
		details.formats.resize(format_counts);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_counts, details.formats.data());
	}
	//get presentmode
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device, m_surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return{ VK_FORMAT_B8G8R8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	for (const auto& format : availableFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8_UNORM &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			//as a defualt
			return format;
		}
	}
	//if we didnt find just return first format
	return availableFormats[0];
}

VkPresentModeKHR Renderer::choosePresentMode(
	const std::vector<VkPresentModeKHR> &availableModes)
{
	for (const auto& mode : availableModes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return mode;
		}
	}
	//return as a default first in first out mode
	return VK_PRESENT_MODE_FIFO_KHR;
}

//#undef NOMINMAX
VkExtent2D Renderer::chooseSwapcExtent2D(const VkSurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != VK_UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D currentExtent = { m_window->width , m_window->height };

		currentExtent.width = std::max(capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, currentExtent.width));

		currentExtent.height = std::max(capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, currentExtent.height));
		return currentExtent;
	}
}

void Renderer::createShaderModule(const std::vector<char> &code, VDeleter<VkShaderModule> &shaderModule)
{
	VkShaderModuleCreateInfo shader_module_createInfo = {};
	shader_module_createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_createInfo.codeSize = code.size();
	shader_module_createInfo.pCode = (uint32_t*)code.data();
 
	LOG_ERROR("failed to create shader module") <<
		vkCreateShaderModule(
			m_device, &shader_module_createInfo, nullptr, shaderModule.replace());
}

uint32_t Renderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) && 
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}
	LOG_ASSERT("failed to find suitable memory type");
}

void Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties, VDeleter<VkBuffer> &buffer,
	VDeleter<VkDeviceMemory> &bufferMemory)
{
	VkBufferCreateInfo bufferinfo = {};
	bufferinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferinfo.size = size;
	bufferinfo.usage = usage;
	bufferinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	LOG_ERROR("failed to create buffer") <<
		vkCreateBuffer(m_device, &bufferinfo, nullptr, buffer.replace());

	VkMemoryRequirements memRequiredments = {};
	vkGetBufferMemoryRequirements(m_device, buffer, &memRequiredments);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memRequiredments.size;
	allocateInfo.memoryTypeIndex = findMemoryType(memRequiredments.memoryTypeBits, properties);

	LOG_ERROR("failed to allocate buffer memory") <<
	vkAllocateMemory(m_device, &allocateInfo, nullptr, bufferMemory.replace());

	//finally bind 
	vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}


void Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	//test
	//auto s = srcBuffer;
	//auto d = dstBuffer;

	VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
	cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandPool = m_command_pool;
	cmdBufferAllocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &cmdBufferAllocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_graphic_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphic_queue);

	vkFreeCommandBuffers(m_device, m_command_pool, 1, &commandBuffer);

}