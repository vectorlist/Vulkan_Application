#include <renderer.h>
#include <window.h>

//-----------------  built in functions ------------------
QueueFamilyIndeice Renderer::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndeice indices;
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
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

void Renderer::createImage(
	uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VDeleter<VkImage> &image,
	VDeleter<VkDeviceMemory> &imageMemory)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	//imageInfo.pNext;
	//imageInfo.flags;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = format;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers =1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = tiling;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	//imageInfo.queueFamilyIndexCount;
	//imageInfo.pQueueFamilyIndices;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	LOG_ERROR("failed to create vulkan image") <<
		vkCreateImage(m_device, &imageInfo, nullptr, image.replace());

	//allocate memory

	VkMemoryRequirements memRequirements = {};
	vkGetImageMemoryRequirements(m_device, image, &memRequirements); //get requirements

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	LOG_ERROR("failed to allocate vulkan image") <<
		vkAllocateMemory(m_device, &allocInfo, nullptr, imageMemory.replace());

	//bind memory each other
	vkBindImageMemory(m_device, image, imageMemory, 0);
}

void Renderer::copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageSubresourceLayers subResources = {};
	subResources.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResources.baseArrayLayer = 0;
	subResources.mipLevel = 0;
	subResources.layerCount = 1;

	VkImageCopy region = {};
	region.srcSubresource = subResources;
	region.dstSubresource = subResources;
	region.srcOffset = { 0,0,0 };
	region.dstOffset = { 0,0,0 };
	region.extent.width = width;
	region.extent.height = height;
	region.extent.depth = 1;

	vkCmdCopyImage(
		commandBuffer,
		srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &region
	);
	endSingleTimeCommand(commandBuffer);
}

void Renderer::transitionImageLayout(
	VkImage image,
	VkFormat format,
	VkImageLayout oldLayout,
	VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}
	else if (	oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
				newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if (	oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
				newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else {
		LOG_ASSERT("unsupported layout transition");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);
	endSingleTimeCommand(commandBuffer);

}

VkCommandBuffer Renderer::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocinfo = {};
	allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocinfo.commandPool = m_command_pool;
	allocinfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &allocinfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void Renderer::endSingleTimeCommand(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_graphic_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphic_queue);

	vkFreeCommandBuffers(m_device, m_command_pool, 1, &commandBuffer);
}

void Renderer::createImageView(VkImage image, VkFormat format, VDeleter<VkImageView> &imageView)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	LOG_ERROR("failed to create texture image view") <<
		vkCreateImageView(m_device, &viewInfo, nullptr, imageView.replace());

}

