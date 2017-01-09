#include "basicrenderer.h"

BasicRenderer::BasicRenderer(Window* window)
	: Renderer(window)
{
	
}


BasicRenderer::~BasicRenderer()
{

}


void BasicRenderer::buildProcedural()
{
	Renderer::buildProcedural();
	buildPipeline();
	buildFrameBuffers();
	buildCommandPool();
	buildCommandBuffers();
	buildSemaphores();
}


void BasicRenderer::buildPipeline()
{
	VkResult err;
	auto vertCode = vkTool::readfile("./shaders/basicVert.spv");
	auto fragCode = vkTool::readfile("./shaders/basicFrag.spv");


	VDeleter<VkShaderModule> vertShaderModule{ m_device, vkDestroyShaderModule };
	VDeleter<VkShaderModule> fragShaderModule{ m_device, vkDestroyShaderModule };

	createShaderModule(vertCode, vertShaderModule);
	createShaderModule(fragCode, fragShaderModule);

	VkPipelineShaderStageCreateInfo vert_stage_createInfo = {};
	vert_stage_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_stage_createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_stage_createInfo.module = vertShaderModule;
	vert_stage_createInfo.pName = "main";

	VkPipelineShaderStageCreateInfo frag_stage_createInfo = {};
	frag_stage_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_stage_createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_stage_createInfo.module = fragShaderModule;
	frag_stage_createInfo.pName = "main";

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
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

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
	color_blend_attachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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
	LOG_ERROR("failed to create pipeline layouts") <<
		vkCreatePipelineLayout(m_device, &pipeline_layoutInfo, nullptr, m_pipeline_layout.replace());


	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaders_stages;
	pipelineInfo.pVertexInputState = &vertex_inputInfo;
	pipelineInfo.pInputAssemblyState = &input_assembly;
	pipelineInfo.pViewportState = &viewport_stage;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &color_blending;
	pipelineInfo.layout = m_pipeline_layout;
	pipelineInfo.renderPass = m_renderpass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	//create graphic pipeline
	LOG_ERROR("failed to create graphic pipeline") <<
		vkCreateGraphicsPipelines(
			m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, m_graphic_pipeline.replace());

}

void BasicRenderer::buildFrameBuffers()
{
	m_frame_buffers.resize(m_image_views.size(), VDeleter<VkFramebuffer>{m_device, vkDestroyFramebuffer});
	for (size_t i = 0; i < m_image_views.size(); ++i)
	{
		VkImageView attachments[] = { m_image_views[i] };

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.renderPass = m_renderpass;
		frameBufferCreateInfo.attachmentCount = 1;
		frameBufferCreateInfo.pAttachments = attachments;
		frameBufferCreateInfo.width = m_swapchain_extent.width;
		frameBufferCreateInfo.height = m_swapchain_extent.height;
		frameBufferCreateInfo.layers = 1;

		LOG_ERROR("failed to create frame buffers") <<
			vkCreateFramebuffer(
				m_device, &frameBufferCreateInfo, nullptr, m_frame_buffers[i].replace());
	}

}

void BasicRenderer::buildCommandPool()
{
	QueueFamilyIndeice queueFamilyIndices = findQueueFamilies(m_physical_device);

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

	LOG_ERROR("failed to create command pool")
		<< vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, m_command_pool.replace());
}

void BasicRenderer::buildCommandBuffers()
{
	if (m_command_buffers.size() > 0)
	{
		//free buffers if has buffers
		vkFreeCommandBuffers(m_device, m_command_pool, m_command_buffers.size(), m_command_buffers.data());
	}
	m_command_buffers.resize(m_frame_buffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_command_buffers.size();

	LOG_ERROR("failed to create command buffers") <<
		vkAllocateCommandBuffers(m_device, &allocInfo, m_command_buffers.data());

	for (size_t i = 0; i < m_command_buffers.size(); ++i)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		vkBeginCommandBuffer(m_command_buffers[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderpass;
		renderPassInfo.framebuffer = m_frame_buffers[i];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = m_swapchain_extent;

		VkClearValue clearValue = { 0.f,0.f,0.f,1.f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(m_command_buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphic_pipeline);
		vkCmdDraw(m_command_buffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(m_command_buffers[i]);

		LOG_ERROR("faile to acess to cammand buffers") <<
			vkEndCommandBuffer(m_command_buffers[i]);
	}
	LOG_SECTION(create command buffers);
}

void BasicRenderer::buildSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult imageErr, renderErr;

	imageErr = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, m_semaphore_image_available.replace());
	renderErr = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, m_semaphore_render_finished.replace());

	if (imageErr != VK_SUCCESS || renderErr != VK_SUCCESS)
		LOG_ASSERT("failed to create semaphores");
}

void BasicRenderer::render()
{
	uint32_t image_index;
	VkResult currentResult;
	currentResult =
		vkAcquireNextImageKHR(m_device, m_swapcahin, VK_UINT64_MAX, m_semaphore_image_available,
			VK_NULL_HANDLE, &image_index);

	if (currentResult == VK_ERROR_OUT_OF_DATE_KHR) {
		//resize or recreate swapchain
		reInitSwapchain();
		LOG << "out of date" << ENDL;
	}
	else if (currentResult != VK_SUCCESS && currentResult != VK_SUBOPTIMAL_KHR)
	{
		LOG_ASSERT("faile to acquire swapchain image");
	}

	VkSubmitInfo submitinfo = {};
	submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	//we need wait semaphore
	VkSemaphore waitSemaphore[] = { m_semaphore_image_available };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitinfo.waitSemaphoreCount = 1;
	submitinfo.pWaitSemaphores = waitSemaphore;
	submitinfo.pWaitDstStageMask = waitStages;

	submitinfo.commandBufferCount = 1;
	//sync cmd buffer to current image
	submitinfo.pCommandBuffers = &m_command_buffers[image_index];

	//siganl semaphore
	VkSemaphore signalSemaphore[] = { m_semaphore_render_finished };
	submitinfo.signalSemaphoreCount = 1;
	submitinfo.pSignalSemaphores = signalSemaphore;

	LOG_ERROR("failed to create queue submit") <<
		vkQueueSubmit(m_graphic_queue, 1, &submitinfo, VK_NULL_HANDLE);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphore;

	VkSwapchainKHR swapCahins[] = { m_swapcahin };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapCahins;

	presentInfo.pImageIndices = &image_index;

	currentResult = vkQueuePresentKHR(m_present_queue, &presentInfo);

	if (currentResult == VK_ERROR_OUT_OF_DATE_KHR || currentResult == VK_SUBOPTIMAL_KHR)
	{
		//recreate swapchain
		reInitSwapchain();
	}
	else if (currentResult != VK_SUCCESS) {
		LOG_ASSERT("faile to acquire swapchain image");
	}

	//Sleep(1000);
	//LOG << image_index << ENDL;
}
