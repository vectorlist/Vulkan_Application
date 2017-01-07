#include "renderer.h"
#include <window.h>

Renderer::Renderer(Window* window, const std::string & name)
	: m_window(window)
{
	//build precedural
	createInstance();
	setupDebugCallback();
	createSurface(m_window);
	pickPhysicalDevice();
	//now create logical device
	createLogicalDevice();
	//new create swapchain
	createSwapchain();
	//create image 
	createImageViews();
	//renderpass
	createRenderpass();
	//graphics pipeline
	createPipeline();

}


Renderer::~Renderer()
{

}

void Renderer::createInstance()
{
	if (!enableValidationLayers /*&& !checkValidationLayerSupport()*/) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = vkTool::getRequiredExtenstions();
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, m_instance.replace()) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void Renderer::setupDebugCallback() {
	if (!enableValidationLayers) return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallback;

	if (vkDebug::CreateDebugReportCallbackEXT(m_instance, &createInfo, nullptr, callback.replace()) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug callback!");
	}
}

void Renderer::createSurface(Window * window)
{
	VkWin32SurfaceCreateInfoKHR surface_createInfo{};
	surface_createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_createInfo.hinstance = window->m_instance;
	surface_createInfo.hwnd = window->m_window;

	vkCreateWin32SurfaceKHR(m_instance, &surface_createInfo, nullptr, m_surface.replace());
}

void Renderer::pickPhysicalDevice()
{
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());
	//m_physical_device = devices[0];      //not logical
	//Pick PhysicalDevice
	for (const auto &device : devices)
	{
		//get device if suitable
		if (isDeviceSuitable(device))
		{
			m_physical_device = device;
			break;
		}
		
	}

	if (m_physical_device == VK_NULL_HANDLE)
	{
		throw std::runtime_error("couldnt find suitable physical device");
	}


}

void Renderer::createLogicalDevice()
{
	//we created suitable physical device already
	QueueFamilyIndeice indices = findQueueFamilies(m_physical_device);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
	std::set<int> uniqueQueueFamiles = { indices.graphicsFamily, indices.presentFamily };

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamiles)
	{
		VkDeviceQueueCreateInfo queueCreateInfos = {};
		queueCreateInfos.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos.queueFamilyIndex = queueFamily;
		queueCreateInfos.queueCount = 1;
		queueCreateInfos.pQueuePriorities = &queuePriority;
		queueCreateInfoList.push_back(queueCreateInfos);
	}
	//no features atm
	VkPhysicalDeviceFeatures deviceFeatures = {};
	
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = queueCreateInfoList.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfoList.data();

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) //if has debug layers
	{
		deviceCreateInfo.enabledLayerCount = validationLayers.size();
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}
	//finally get device(logical)
	VkResult err = vkCreateDevice(m_physical_device, &deviceCreateInfo, nullptr, m_device.replace());

	if (err != VK_SUCCESS) VK_ERROR(couldnt create logical device);

	//create Queue
	vkGetDeviceQueue(m_device, indices.graphicsFamily, 0,&m_graphic_queue);
	vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_present_queue);
}

void Renderer::createSwapchain()
{
	//get details first
	SwapChainSupportDetails details = querySwapchainSupport(m_physical_device);

	//get surface format
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(details.formats);

	//get Present mode
	VkPresentModeKHR presentMode = choosePresentMode(details.presentModes);

	//get 2d extent for image
	VkExtent2D extent = chooseSwapcExtent2D(details.capabilities);

	//get enable image count
	uint32_t imageCount = details.capabilities.minImageCount + 1;
	if ((details.capabilities.maxImageCount > 0) &&
		(imageCount > details.capabilities.maxImageCount))
	{
		//limit under max image count support
		imageCount = details.capabilities.maxImageCount;
	}
	//create info
	VkSwapchainCreateInfoKHR swapcahin_createInfo = {};  // = {} for construct defualt as 0 all member
	swapcahin_createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapcahin_createInfo.surface = m_surface;
	swapcahin_createInfo.minImageCount = imageCount;
	swapcahin_createInfo.imageFormat = surfaceFormat.format;
	swapcahin_createInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapcahin_createInfo.imageExtent = extent;
	swapcahin_createInfo.imageArrayLayers = 1;
	swapcahin_createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//set queue family indices
	//get indice from physical device
	QueueFamilyIndeice indices = findQueueFamilies(m_physical_device);
	//for if queue  index seperated
	uint32_t queueFamilyIndice[] = { (uint32_t)indices.graphicsFamily,(uint32_t)indices.presentFamily };

	//if not same queue index
	if (indices.graphicsFamily != indices.presentFamily)
	{
		swapcahin_createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapcahin_createInfo.queueFamilyIndexCount = 2;			//set queue index 2 seperate
		swapcahin_createInfo.pQueueFamilyIndices = queueFamilyIndice;
	}
	else
	{
		//as a default same queue index
		swapcahin_createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	swapcahin_createInfo.preTransform = details.capabilities.currentTransform;
	swapcahin_createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapcahin_createInfo.presentMode = presentMode;
	swapcahin_createInfo.clipped = VK_TRUE;

	//it's for re create
	VkSwapchainKHR oldSwapCahin = m_swapcahin;				//both are null atm
	swapcahin_createInfo.oldSwapchain = oldSwapCahin;

	VkSwapchainKHR newSwapcChain;
	VkResult err = vkCreateSwapchainKHR(m_device, &swapcahin_createInfo, nullptr, &newSwapcChain);
	if (err != VK_SUCCESS) VK_ERROR("failed to create swapchain");

	//now new swapchain to out swapcahin
	m_swapcahin = newSwapcChain;

	//set images
	vkGetSwapchainImagesKHR(m_device, m_swapcahin, &imageCount, nullptr);
	m_images.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapcahin, &imageCount, m_images.data());

	m_image_format = surfaceFormat.format;
	m_swapchain_extent = extent;
	
}

void Renderer::createImageViews()
{
	//set same as images count
	m_image_views.resize(m_images.size(), VDeleter<VkImageView>{m_device,vkDestroyImageView});

	for (uint32_t i = 0; i < m_image_views.size(); ++i)
	{
		VkImageViewCreateInfo imageView_createInfo = {};
		imageView_createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageView_createInfo.image = m_images[i];
		imageView_createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageView_createInfo.format = m_image_format;
		imageView_createInfo.components = {			//rgba
			VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY
		};
		imageView_createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageView_createInfo.subresourceRange.baseMipLevel = 0;
		imageView_createInfo.subresourceRange.levelCount = 1;
		imageView_createInfo.subresourceRange.baseArrayLayer = 0;
		imageView_createInfo.subresourceRange.layerCount = 1;

		VkResult err = vkCreateImageView(
			m_device, &imageView_createInfo, nullptr, m_image_views[i].replace());
		if (err != VK_SUCCESS) VK_ERROR(failed to create image views);
	}
}

void Renderer::createRenderpass()
{
	//color 
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_image_format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//ref
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//subpass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	//render pass info
	VkRenderPassCreateInfo renderpass_createInfo = {};
	renderpass_createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpass_createInfo.attachmentCount = 1;
	renderpass_createInfo.pAttachments = &colorAttachment;
	renderpass_createInfo.subpassCount = 1;
	renderpass_createInfo.pSubpasses = &subpass;
	renderpass_createInfo.dependencyCount = 1;
	renderpass_createInfo.pDependencies = &dependency;

	VkResult err = vkCreateRenderPass(m_device, &renderpass_createInfo, nullptr, m_renderpass.replace());
	if (err != VK_SUCCESS) VK_ERROR(failed to create renderpass);
	//int r_count = m_renderpass.replace()
	LOG_SECTION(render pass created);
}

void Renderer::createPipeline()
{
	VkResult err;
	std::vector<char> vertCode = vkTool::readfile("./shaders/vert.spv");
	std::vector<char> fragCode = vkTool::readfile("./shaders/frag.spv");
	

	VDeleter<VkShaderModule> vertShaderModule{ m_device, vkDestroyShaderModule };
	VDeleter<VkShaderModule> fragShaderModule{ m_device, vkDestroyShaderModule };
	
	createShaderModule(vertCode, vertShaderModule);
	createShaderModule(vertCode, fragShaderModule);

	VkPipelineShaderStageCreateInfo vert_stage_createInfo = {};
	vert_stage_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_stage_createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_stage_createInfo.module = vertShaderModule;
	vert_stage_createInfo.pName = "main";

	VkPipelineShaderStageCreateInfo frag_stage_createInfo = {};
	vert_stage_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_stage_createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	vert_stage_createInfo.module = fragShaderModule;
	vert_stage_createInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaders_stages[] = { vert_stage_createInfo ,frag_stage_createInfo };

	VkPipelineVertexInputStateCreateInfo vertex_inputInfo = {};
	vertex_inputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_inputInfo.vertexBindingDescriptionCount = 0;
	vertex_inputInfo.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;
	
	//viewport
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)m_swapchain_extent.width;
	viewport.height = (float)m_swapchain_extent.height;

	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = m_swapchain_extent;

	VkPipelineViewportStateCreateInfo viewport_stage = {};
	viewport_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_stage.viewportCount = 1;
	viewport_stage.pViewports = &viewport;
	viewport_stage.scissorCount = 1;
	viewport_stage.pScissors = &scissor;

	//raster
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	//if multisampling
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//color blend
	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;
	//pipeline layout
	VkPipelineLayoutCreateInfo pipeline_layoutInfo = {};
	pipeline_layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layoutInfo.setLayoutCount = 0;
	pipeline_layoutInfo.pushConstantRangeCount = 0;

	//create pipeline layout
	err = vkCreatePipelineLayout(m_device, &pipeline_layoutInfo, nullptr, m_pipeline_layout.replace());
	if (err != VK_SUCCESS) VK_ERROR(failed to create pipelinelayout);

	VkGraphicsPipelineCreateInfo pipeline_createInfo = {};
	pipeline_createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_createInfo.stageCount = 2;
	pipeline_createInfo.pStages = shaders_stages;
	pipeline_createInfo.pVertexInputState = &vertex_inputInfo;
	pipeline_createInfo.pInputAssemblyState = &input_assembly;
	pipeline_createInfo.pViewportState = &viewport_stage;
	pipeline_createInfo.pRasterizationState = &rasterizer;
	pipeline_createInfo.pMultisampleState = &multisampling;
	pipeline_createInfo.pColorBlendState = &color_blending;
	pipeline_createInfo.layout = m_pipeline_layout;			//member
	pipeline_createInfo.renderPass = m_renderpass;
	pipeline_createInfo.subpass = 0;
	pipeline_createInfo.basePipelineHandle = VK_NULL_HANDLE;

	//create graphic pipeline
	VkResult error;
	/*error = vkCreateGraphicsPipelines(
		m_device,VK_NULL_HANDLE,1 ,&pipeline_createInfo, nullptr, m_graphic_pipeline.replace());*/
	//if (err != VK_SUCCESS) VK_ERROR(failed to create pipelinelayout);


	/*error = vkCreateGraphicsPipelines(
		m_device,VK_NULL_HANDLE,1 ,&pipeline_createInfo, nullptr, &m_graphic_p);
	if (err != VK_SUCCESS) VK_ERROR(failed to create pipelinelayout);
	int g = 7;*/

}


