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
