
#include "VkRenderer.h"
#include <assert.h>
#include <window.h>

VkRenderer::VkRenderer(Window *pWindow, const std::string &title, bool isDebug)
	: m_pWindow(pWindow), name(title), isDebug(isDebug)
{
	//build_procedural();
}

VkRenderer::~VkRenderer()
{
	//delete_procedural();
}


void VkRenderer::build_procedural()
{
	//device and surface
	build_layers();
	build_instance();
	build_debug();
	build_device();
	//others
	setSurfaceFromWindow(m_pWindow);
	initSurface();
	initSwapchain();
	initImages();
	initDepthStencilImage();
	initRenderPass();
	initFrameBuffer();
	initSynchronization();

	build_command_pool();
	build_command_buffer();

	build_complete_semaphore();

	//addtional build prepare
	build_addtion();
}

void VkRenderer::delete_procedural()
{
	vkQueueWaitIdle(m_queue);			//wait all process
										//---------
	delete_command_pool();
	delete_complete_semaphore();

	//vkQueueWaitIdle(m_queue);
	deleteSynchronization();
	deleteFrameBuffer();
	deleteRenderPass();
	deleteDepthStencilImage();
	deleteImages();
	deleteSwapchain();
	deleteSurface();
	//device
	deleteDevice();
	deleteDebug();
	deleteinstance();
}


void VkRenderer::build_layers()
{
	m_instance_extension.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	m_instance_extension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	m_device_extension.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	if (isDebug) {
		m_debug_callbackcreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		//add call func to callback create info(on init)
		m_debug_callbackcreateInfo.pfnCallback = VulkanDEbugCallback;
		m_debug_callbackcreateInfo.flags =
			//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			//VK_DEBUG_REPORT_DEBUG_BIT_EXT |
			0;
		m_instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
		m_instance_extension.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}
}

void VkRenderer::build_instance()
{
	//SET APLLICATION INFO
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.pApplicationName = "vulkan window";

	//SET INSTANCE CREATE INFO
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledLayerCount = m_instance_layers.size();
	instanceCreateInfo.ppEnabledLayerNames = m_instance_layers.data();
	instanceCreateInfo.enabledExtensionCount = m_instance_extension.size();
	instanceCreateInfo.ppEnabledExtensionNames = m_instance_extension.data();
	//add Callback func info (it must be setted)
	instanceCreateInfo.pNext = &m_debug_callbackcreateInfo;
	

	vkError() << vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
	
	LOG_SECTION(BASE instance created);
	LOG << "create window" << ENDL;
	
}

void VkRenderer::deleteinstance()
{
	vkDestroyInstance(m_instance, nullptr);
	m_instance = nullptr;
}

void VkRenderer::build_device()
{
	//SET PHISICALDEVICE
	uint32_t gpu_count = 0;
	vkEnumeratePhysicalDevices(m_instance, &gpu_count, nullptr);
	std::vector<VkPhysicalDevice> physical_devices(gpu_count);
	vkEnumeratePhysicalDevices(m_instance, &gpu_count, physical_devices.data());
	m_physical_device = physical_devices[0];
	LOG << "enabled GPU's : " << gpu_count << ENDL;

	//SET PHYICAL DEVICE PROPERTIES
	vkGetPhysicalDeviceProperties(m_physical_device, &m_device_properties);

	//SET PHYICAL DEVICE MEMORY PROPERTIES
	vkGetPhysicalDeviceMemoryProperties(m_physical_device, &m_device_memory_properties);
	//features
	vkGetPhysicalDeviceFeatures(m_physical_device, &m_device_features);

	

	//SET PHYSICAL DEVICE QUEUE FAMILY PROPERTIES
	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &familyCount, nullptr);
	std::vector<VkQueueFamilyProperties> familyProperties(familyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &familyCount, familyProperties.data());
	bool found = false;
	for (uint32_t i = 0; i < familyCount; ++i)
	{
		if (familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			found = true;
			m_graphics_faimily_index = i;
		}
	}
	if (!found) {
		vkQuitAssert("Queue family support graphics Found");
	}

	//SET INSTANCE LAYERS
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> layers_properties(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layers_properties.data());
	LOG << " Instacne layer : "<< layerCount << ENDL;

	for (auto &i : layers_properties) {
		LOG << "  " << i.layerName << "t\t\ | " << i.description << ENDL;
	}

	//SET DEVICE LAYERS
	uint32_t deiviceLayercount = 0;
	vkEnumerateDeviceLayerProperties(m_physical_device,&deiviceLayercount, nullptr);
	std::vector<VkLayerProperties> device_layers_properties(deiviceLayercount);
	vkEnumerateDeviceLayerProperties(m_physical_device,&deiviceLayercount, device_layers_properties.data());
	LOG << " Device Layer : " << deiviceLayercount << ENDL;

	for (auto &i : device_layers_properties) {
		LOG << "  " << i.layerName << "t\t\ | " << i.description << ENDL;
	}


	float queuePriorities[]{ 1.0f };
	VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueFamilyIndex = m_graphics_faimily_index;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

	VkDeviceCreateInfo deviceCreateinfo{};
	deviceCreateinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateinfo.queueCreateInfoCount = 1;
	deviceCreateinfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	//deviceCreateinfo.enabledLayerCount = m_device_layers.size();
	//deviceCreateinfo.ppEnabledLayerNames = m_device_layers.data();
	deviceCreateinfo.enabledExtensionCount = m_device_extension.size();
	deviceCreateinfo.ppEnabledExtensionNames = m_device_extension.data();

	vkError() << vkCreateDevice(m_physical_device, &deviceCreateinfo, nullptr, &m_device);
	
	//Set Queue(in 32bit dosent work nullptr)
	vkGetDeviceQueue(m_device, m_graphics_faimily_index, 0, &m_queue);
	LOG_SECTION(device created);
}

void VkRenderer::deleteDevice()
{
	vkDestroyDevice(m_device, nullptr);
	m_device = VK_NULL_HANDLE;
}


//Callback Func
//typedef VkBool32(VKAPI_PTR *PFN_vkDebugReportCallbackEXT)(
//	VkDebugReportFlagsEXT                       flags,
//	VkDebugReportObjectTypeEXT                  objectType,
//	uint64_t                                    object,
//	size_t                                      location,
//	int32_t                                     messageCode,
//	const char*                                 pLayerPrefix,
//	const char*                                 pMessage,
//	void*                                       pUserData);
VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDEbugCallback(
	VkDebugReportFlagsEXT flag,
	VkDebugReportObjectTypeEXT objType,
	uint64_t object,
	size_t location,
	int32_t msg_code,
	const char* player_prefix,
	const char* msg,
	void* user_data)
{
	std::string bit("");
	if (flag == VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		bit.append("INFO : ");
	}
	if (flag == VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		bit.append("WARNNING : ");
	}
	if (flag == VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		bit.append("PERFORMANCE : ");
	}
	if (flag == VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		bit.append("ERROR : ");
	}
	if (flag == VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		bit.append("DEBUG : ");
	}

	bit.append(" [").append(player_prefix).append( "] ").append(msg);
	LOG << bit << ENDL;
	//LOG << 
	if (flag & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		MessageBox(NULL, bit.c_str(), "Vulkan Error", 0);
	return false;
}

//debug
//call back Pointer function
PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = nullptr;

void VkRenderer::build_debug()
{
	//SET CREATE REPORT CALLBACK FUNC(after created vulkan Instance)
	//PFN_vkVoidFunction vkGetInstanceProcAddr(
	//	VkInstance                                  instance,
	//	const char*                                 pName);
	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)
		vkGetInstanceProcAddr(m_instance,"vkCreateDebugReportCallbackEXT");

	fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
		vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");

	if (nullptr == fvkCreateDebugReportCallbackEXT || nullptr == fvkDestroyDebugReportCallbackEXT) {
		//LOG << "failed to load callback pointer" << ENDL;
		vkQuitAssert("Vulkan ASSERT : Can't Fetch Debug function Pointer");
	}

	//assign to member debug report
	fvkCreateDebugReportCallbackEXT(m_instance, &m_debug_callbackcreateInfo,
		nullptr, &m_debug_report);
}

void VkRenderer::deleteDebug()
{
	fvkDestroyDebugReportCallbackEXT(m_instance, m_debug_report, nullptr);
	m_debug_report = VK_NULL_HANDLE;
}

void VkRenderer::setSurfaceFromWindow(Window * pWindow)
{
	//const HINSTANCE* pInstance = pWindow->instance();
	//const HWND* pWindow = pWindow->window();

	VkWin32SurfaceCreateInfoKHR surface_createInfo{};
	surface_createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_createInfo.hwnd = pWindow->m_window;
	surface_createInfo.hinstance = pWindow->m_instance;
	surface_createInfo.pNext = NULL;
	
	vkError() << vkCreateWin32SurfaceKHR(
		m_instance, &surface_createInfo, nullptr, &m_surface);

}


void VkRenderer::initSurface()
{
	VkPhysicalDevice gpu = m_physical_device;
	VkBool32 WSI_supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(gpu,
		m_graphics_faimily_index,
		m_surface, &WSI_supported);
	if (!WSI_supported) vkQuitAssert("WSI is not supported");

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, m_surface, &m_surface_capablilities);
	if (m_surface_capablilities.currentExtent.width < UINT_MAX) {
		m_surface_width = m_surface_capablilities.currentExtent.width;
		m_surface_height = m_surface_capablilities.currentExtent.height;
	}


	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_surface, &format_count, nullptr);
	if (format_count == 0) vkQuitAssert("Surface Format Missing");

	std::vector<VkSurfaceFormatKHR> formats(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_surface, &format_count, formats.data());
	if (formats[0].format == VK_FORMAT_UNDEFINED) {
		m_surface_format.format = VK_FORMAT_B8G8R8_UNORM;
		m_surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	}
	else {
		m_surface_format = formats[0];
	}
	LOG_SECTION(surface created);
	LOG << "surface width : " << m_surface_width << " surface height : " << m_surface_height << ENDL;
}

void VkRenderer::deleteSurface()
{
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
}

void VkRenderer::initSwapchain()
{
	if (m_swapchain_image_count < m_surface_capablilities.minImageCount + 1)
		m_swapchain_image_count = m_surface_capablilities.minImageCount + 1;
	if (m_surface_capablilities.maxImageCount > 0) {
		if (m_swapchain_image_count > m_surface_capablilities.maxImageCount)
			m_swapchain_image_count = m_surface_capablilities.maxImageCount;
	}
	LOG << "Swap change image count : " << m_swapchain_image_count << ENDL;
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

	uint32_t presentModeCount = 0;
	vkError() << vkGetPhysicalDeviceSurfacePresentModesKHR(
		m_physical_device, m_surface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkError() << vkGetPhysicalDeviceSurfacePresentModesKHR(
		m_physical_device, m_surface, &presentModeCount, presentModes.data());

	for (auto mode : presentModes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) present_mode = mode;
	}


	VkSwapchainCreateInfoKHR swapchainInfo{};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = m_surface;
	swapchainInfo.minImageCount = m_swapchain_image_count;
	swapchainInfo.imageFormat = m_surface_format.format;
	swapchainInfo.imageColorSpace = m_surface_format.colorSpace;
	swapchainInfo.imageExtent.width = m_surface_width;
	swapchainInfo.imageExtent.height = m_surface_height;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.queueFamilyIndexCount = 0;
	swapchainInfo.pQueueFamilyIndices = nullptr;
	swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.presentMode = present_mode;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;


	vkError() << vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &m_swapchain);
	vkError() << vkGetSwapchainImagesKHR(
		m_device, m_swapchain, &m_swapchain_image_count, nullptr);

	LOG_SECTION(swapchain created);
}

void VkRenderer::deleteSwapchain()
{
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

void VkRenderer::initImages()
{
	m_images.resize(m_swapchain_image_count);
	m_image_views.resize(m_swapchain_image_count);

	vkError() << vkGetSwapchainImagesKHR(
		m_device, m_swapchain, &m_swapchain_image_count, m_images.data());
	for (uint32_t i = 0; i < m_swapchain_image_count; ++i) {
		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = m_images[i];
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = m_surface_format.format;
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;

		vkError() << vkCreateImageView(
			m_device, &imageViewInfo, nullptr, &m_image_views[i]);
	}


}

void VkRenderer::deleteImages()
{
	if (m_image_views.size()) {
		//VkDevice device = m_renderer->m_device;
		for (auto view : m_image_views)
			vkDestroyImageView(m_device, view, nullptr);
	}
}

void VkRenderer::initDepthStencilImage()
{
	//set Format
	std::vector<VkFormat> try_formats{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM
	};
	for (auto f : try_formats) {
		//input format properties
		VkFormatProperties format_properties{};
		vkGetPhysicalDeviceFormatProperties(
			m_physical_device, f, &format_properties);
		if (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			m_depth_stencil_format = f;
			break;
			//LOG << "supported depth stencil format" << ENDL;
		}
	}
	if (m_depth_stencil_format == VK_FORMAT_UNDEFINED)
		vkError("Depth stencil format not selected");
	for (auto f : try_formats) {
		if (m_depth_stencil_format == f)
			m_depth_stencil_available = true;
	}
	LOG << "Stencil available : " << m_depth_stencil_available << ENDL;
	LOG << "Depth Stencil Format : " << m_depth_stencil_format << ENDL;

	VkImageCreateInfo image_createInfo = {};
	image_createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_createInfo.flags = 0;
	image_createInfo.imageType = VK_IMAGE_TYPE_2D;
	image_createInfo.format = m_depth_stencil_format;
	image_createInfo.extent.width = m_surface_width;
	image_createInfo.extent.height = m_surface_height;
	image_createInfo.extent.depth = 1;
	image_createInfo.mipLevels = 1;
	image_createInfo.arrayLayers = 1;
	image_createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	image_createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_createInfo.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
	image_createInfo.pQueueFamilyIndices = nullptr;
	image_createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//create depth image
	vkError() << vkCreateImage(
		m_device, &image_createInfo, nullptr, &m_depth_stencil_image);

	//Set Memory after create image

	VkMemoryRequirements image_memory_requirements{};
	//get image memory requirements
	vkGetImageMemoryRequirements(m_device, m_depth_stencil_image, &image_memory_requirements);

	auto physicalDeviceMemoryProperties = m_device_memory_properties;

	uint32_t memory_index = vkTools::FindMemoryTypeIndex(
		&physicalDeviceMemoryProperties, &image_memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	LOG << "Available Memory Index :" << memory_index << ENDL;

	//allocating memory
	VkMemoryAllocateInfo memory_allocateInfo = {};
	memory_allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocateInfo.allocationSize = image_memory_requirements.size;		//from current image
	memory_allocateInfo.memoryTypeIndex = memory_index;

	//Allocate
	vkError() << vkAllocateMemory(
		m_device, &memory_allocateInfo, nullptr, &m_depth_stencil_memory);
	//bind
	vkError() << vkBindImageMemory(
		m_device, m_depth_stencil_image, m_depth_stencil_memory, 0);

	VkImageViewCreateInfo image_view_createInfo = {};
	image_view_createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_createInfo.image = m_depth_stencil_image;
	image_view_createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_createInfo.format = m_depth_stencil_format;
	image_view_createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT |
		(m_depth_stencil_available ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);

	image_view_createInfo.subresourceRange.baseMipLevel = 0;
	image_view_createInfo.subresourceRange.levelCount = 1;
	image_view_createInfo.subresourceRange.baseArrayLayer = 0;
	image_view_createInfo.subresourceRange.layerCount = 1;

	//create depth imageview
	vkError() << vkCreateImageView(
		m_device, &image_view_createInfo, nullptr, &m_depth_stencil_image_view);
}

void VkRenderer::deleteDepthStencilImage()
{
	vkDestroyImageView(m_device, m_depth_stencil_image_view, nullptr);
	vkFreeMemory(m_device, m_depth_stencil_memory, nullptr);
	vkDestroyImage(m_device, m_depth_stencil_image, nullptr);
}

void VkRenderer::initRenderPass()
{
	std::array<VkAttachmentDescription, 2> attachments = {};
	//depth stencil
	attachments[0].flags = 0;
	attachments[0].format = m_depth_stencil_format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	//surface
	attachments[1].flags = 0;
	attachments[1].format = m_surface_format.format;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//we dont need stencil
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //dont get it(it's extension)

																  //sub pass(depth stencil)
	VkAttachmentReference subpass_0_depth_stencil_attachment = {};
	subpass_0_depth_stencil_attachment.attachment = 0;
	subpass_0_depth_stencil_attachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	//sub pass(color)
	std::array<VkAttachmentReference, 1> subpass_0_color_attachments = {};
	subpass_0_color_attachments[0].attachment = 1;			//depth is 0
	subpass_0_color_attachments[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//now buld a subpass describtion by referrence
	std::array<VkSubpassDescription, 1> sub_passes = {};
	sub_passes[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	sub_passes[0].colorAttachmentCount = subpass_0_color_attachments.size();
	sub_passes[0].pColorAttachments = subpass_0_color_attachments.data();
	sub_passes[0].pDepthStencilAttachment = &subpass_0_depth_stencil_attachment;


	//finnaly we build Renderpass info
	VkRenderPassCreateInfo render_pass_createInfo = {};
	render_pass_createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_createInfo.attachmentCount = attachments.size();
	render_pass_createInfo.pAttachments = attachments.data();
	render_pass_createInfo.subpassCount = sub_passes.size();
	render_pass_createInfo.pSubpasses = sub_passes.data();


	vkError() << vkCreateRenderPass(
		m_device, &render_pass_createInfo, nullptr, &m_render_pass);
}

void VkRenderer::deleteRenderPass()
{
	vkDestroyRenderPass(m_device, m_render_pass, nullptr);
}

void VkRenderer::initFrameBuffer()
{
	//resize frame buffer as swapchain size
	m_frame_buffers.resize(m_swapchain_image_count);
	for (uint32_t i = 0; i < m_swapchain_image_count; ++i)
	{
		//set view as a attachments
		std::array<VkImageView, 2> attachments = {};
		attachments[0] = m_depth_stencil_image_view;				//attachments view [0] : depth
		attachments[1] = m_image_views[i];							//attachment current color view

		VkFramebufferCreateInfo frame_buffer_createInfo = {};
		frame_buffer_createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_createInfo.renderPass = m_render_pass;
		frame_buffer_createInfo.attachmentCount = attachments.size();
		frame_buffer_createInfo.pAttachments = attachments.data();
		frame_buffer_createInfo.width = m_surface_width;
		frame_buffer_createInfo.height = m_surface_height;
		frame_buffer_createInfo.layers = 1;			//as a defualt

													//build framebbuer to m_frame_buffer list
		vkError() << vkCreateFramebuffer(
			m_device, &frame_buffer_createInfo, nullptr, &m_frame_buffers[i]);
	}
}

void VkRenderer::deleteFrameBuffer()
{
	for (auto buffer : m_frame_buffers) {
		vkDestroyFramebuffer(m_device, buffer, nullptr);
	}
}

void VkRenderer::initSynchronization()
{
	//build a fence
	VkFenceCreateInfo fence_createInfo = {};
	fence_createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkError() << vkCreateFence(m_device, &fence_createInfo, nullptr, &m_swapchain_image_available);

}

void VkRenderer::deleteSynchronization()
{
	vkDestroyFence(m_device, m_swapchain_image_available, nullptr);
}

//------------------------------------------------------

void VkRenderer::build_command_pool()
{
	VkCommandPoolCreateInfo command_pool_createInfo = {};
	command_pool_createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
		| VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_createInfo.queueFamilyIndex = m_graphics_faimily_index;

	vkError() << vkCreateCommandPool(
		m_device, &command_pool_createInfo, nullptr, &m_command_pool);

	LOG_SECTION(BASE create command pool);
	LOG << "queue family index : "  << command_pool_createInfo.queueFamilyIndex << ENDL;
}

void VkRenderer::delete_command_pool()
{
	vkDestroyCommandPool(m_device, m_command_pool, nullptr);
}

void VkRenderer::build_command_buffer()
{
	VkCommandBufferAllocateInfo command_buffer_allocInfo = {};
	command_buffer_allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocInfo.commandPool = m_command_pool;
	command_buffer_allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocInfo.commandBufferCount = 1;

	vkError() << vkAllocateCommandBuffers(
		m_device, &command_buffer_allocInfo, &m_command_buffer);
	LOG_SECTION(BASE create buffers);
	LOG << "command buffers : "<< command_buffer_allocInfo.commandBufferCount << ENDL;
}

void VkRenderer::delete_command_buffer()
{
	
}

void VkRenderer::build_complete_semaphore()
{
	VkSemaphoreCreateInfo semaphore_createInfo = {};
	semaphore_createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkError() << vkCreateSemaphore(
		m_device, &semaphore_createInfo, nullptr, &m_complete_semaphore);

	LOG_SECTION(BASE create complete semaphore);

}
void VkRenderer::delete_complete_semaphore()
{
	vkDestroySemaphore(m_device, m_complete_semaphore, nullptr);
}

void VkRenderer::begin()
{
	vkAcquireNextImageKHR(
		m_device,										//device
		m_swapchain,									//swapchain
		UINT32_MAX,										//timeout
		VK_NULL_HANDLE,                                 //semaphore
		m_swapchain_image_available,					//fence
		&m_active_swapchain_image_id);					//pImageIndex

	vkWaitForFences(m_device, 1, &m_swapchain_image_available, VK_TRUE, UINT64_MAX);
	vkResetFences(m_device, 1, &m_swapchain_image_available);
	vkQueueWaitIdle(m_queue);
}
void VkRenderer::end()
{
	VkResult present_result = VkResult::VK_RESULT_MAX_ENUM;
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	//using complete semaphore
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_complete_semaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.pImageIndices = &m_active_swapchain_image_id;
	presentInfo.pResults = &present_result;

	vkQueuePresentKHR(m_queue, &presentInfo);
}



void VkRenderer::render()
{
	begin();
	
	VkCommandBufferBeginInfo command_beginInfo = {};
	command_beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(m_command_buffer, &command_beginInfo);

	VkRect2D render_area;
	render_area.offset.x = 0;
	render_area.offset.y = 0;
	render_area.extent = getSurfaceSize();

	std::array<VkClearValue,2> clear_values {};
	clear_values[0].depthStencil.depth = 0.0f;
	clear_values[0].depthStencil.stencil = 0;
	clear_values[1].color.float32[0] = 0.2f;
	clear_values[1].color.float32[1] = 0.3f;
	clear_values[1].color.float32[2] = 0.4f;
	clear_values[1].color.float32[3] = 1.0f;

	VkRenderPassBeginInfo render_pass_beginInfo{};
	render_pass_beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_beginInfo.renderPass = m_render_pass;
	render_pass_beginInfo.framebuffer = getActiveFrameBuffer();
	render_pass_beginInfo.renderArea = render_area;
	render_pass_beginInfo.clearValueCount = clear_values.size();
	render_pass_beginInfo.pClearValues = clear_values.data();

	vkCmdBeginRenderPass(
		m_command_buffer, &render_pass_beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdEndRenderPass(m_command_buffer);

	vkEndCommandBuffer(m_command_buffer);

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 0;
	submit_info.pWaitSemaphores = nullptr;					//no using wait_semaphore atm
	submit_info.pWaitDstStageMask = nullptr;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &m_command_buffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &m_complete_semaphore;	//signal semaphore as complete semaphore

	vkQueueSubmit(m_queue, 1, &submit_info, VK_NULL_HANDLE);
	
	end();
}

