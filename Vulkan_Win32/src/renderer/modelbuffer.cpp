#include "modelbuffer.h"

#include "stb_image.h"

ModelBuffer::ModelBuffer(Window *window)
	: Renderer(window)
{
	
}


ModelBuffer::~ModelBuffer()
{
}


void ModelBuffer::buildProcedural()
{
	Renderer::buildProcedural();
	buildModel("./obj/sphinx.obj");
	buildDescriptorSetLayout();

	buildPipeline();
	buildCommandPool();
	buildDeapthStencil();				//build after command pool and before framebuffer
	buildFrameBuffers();
	//------------------ texture
	buildTextureImage();
	buildTextureImageView();
	buildTextureSampler();
	//------------
	buildVertexBuffers();
	buildIndiceBuffers();

	buildUniformBuffer();
	buildDescriptorPool();
	buildDescriptorSet();

	buildCommandBuffers();
	buildSemaphores();
}

void ModelBuffer::buildModel(const std::string &filename)
{
	LOG << "loading model....." << ENDL;
	vkMesh::LoadModel(filename, &m_vertices, &m_indices);
	LOG_SECTION("derived loaded model");
}

void ModelBuffer::buildDeapthStencil()
{
	VkFormat depthFormat = findDepthFormat();
	LOG << "find format : " << depthFormat << ENDL;
	createImage(m_swapchain_extent.width, m_swapchain_extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_depth_image, m_depth_image_memory);
	createImageView(m_depth_image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, m_depth_image_view);
	transitionImageLayout(m_depth_image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	LOG_SECTION("derived created depth stencil image");
}

void ModelBuffer::buildTextureImage()
{
	int tex_width, tex_height, tex_channels;
	stbi_uc* pixels = stbi_load("./image/sphinx01.jpg", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

	VkDeviceSize imageSize = tex_width * tex_height * 4;
	if (pixels == nullptr)
		LOG_ASSERT("failed to load image");

	VDeleter<VkImage> stagingImage{ m_device,vkDestroyImage };
	VDeleter<VkDeviceMemory> stagingImageMemory{ m_device, vkFreeMemory };

	createImage(tex_width, tex_height,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingImage, stagingImageMemory);

	VkImageSubresource subsource = {};
	subsource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subsource.mipLevel = 0;
	subsource.arrayLayer = 0;

	VkSubresourceLayout stagingImageLayout;
	vkGetImageSubresourceLayout(m_device, stagingImage, &subsource, &stagingImageLayout);

	void* data;
	vkMapMemory(m_device, stagingImageMemory, 0, imageSize, 0, &data);

	//rowPitch : the number of bytes between each row of texels in an image.
	if (stagingImageLayout.rowPitch == tex_width * 4) {
		memcpy(data, pixels, (size_t)imageSize);
	}
	else
	{
		uint8_t* dataByte = reinterpret_cast<uint8_t*>(data);
		uint64_t row_texel = stagingImageLayout.rowPitch;
		for (int y = 0; y < tex_height; ++y)
		{
			memcpy(&dataByte[y * row_texel], &pixels[y * tex_width * 4], tex_width * 4);
		}
	}

	vkUnmapMemory(m_device, stagingImageMemory);
	//free allocated std image
	stbi_image_free(pixels);

	//finally create image
	createImage(tex_width, tex_height,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_texture_image, m_texture_image_memory);

	transitionImageLayout(stagingImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	transitionImageLayout(m_texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	//then copy
	copyImage(stagingImage, m_texture_image, tex_width, tex_height);

	transitionImageLayout(m_texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	LOG_SECTION("derived loaded vulkan image");
	LOG << "image : " << tex_width << " " << tex_height << ENDL;
}

void ModelBuffer::buildTextureImageView()
{
	//create image view from texture
	createImageView(m_texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_texture_image_view);
	LOG_SECTION("derived created texture image view");
}

void ModelBuffer::buildTextureSampler()
{
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = 16;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	LOG_ERROR("failed to created texture sampler") <<
		vkCreateSampler(m_device, &samplerCreateInfo, nullptr, m_textureSampler.replace());
	LOG_SECTION("derived created texture sampler");
}


void ModelBuffer::buildDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding,samplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = bindings.size();
	layoutInfo.pBindings = bindings.data();

	LOG_ERROR("failed to create descriptor set layouts") <<
		vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, m_descriptorSetLayout.replace());

	LOG_SECTION("derived created descriptor set layouts");
}

void ModelBuffer::buildDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSize = {};
	poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize[0].descriptorCount = 1;
	poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSize.size();
	poolInfo.pPoolSizes = poolSize.data();
	poolInfo.maxSets = 1;

	LOG_ERROR("failed to create descriptor pool") <<
		vkCreateDescriptorPool(m_device, &poolInfo, nullptr, m_descriptor_pool.replace());
}

void ModelBuffer::buildDescriptorSet()
{
	//we create set layout before
	VkDescriptorSetLayout layouts[] = { m_descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptor_pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	LOG_ERROR("failed to allocate destriptor set") <<
		vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptor_set);

	//create descriptor buffer
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = m_uniform_buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UBO);

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = m_texture_image_view;
	imageInfo.sampler = m_textureSampler;

	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = m_descriptor_set;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &bufferInfo;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = m_descriptor_set;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;



	vkUpdateDescriptorSets(m_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

	LOG_SECTION("derived created descriptor sets");

}

void ModelBuffer::buildUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UBO);
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniform_staging_buffer, m_uinform_staging_buffer_memory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_uniform_buffer, m_uinform_buffer_memory);
}

void ModelBuffer::buildPipeline()
{
	VkResult err;
	auto vertCode = vkTool::readfile("./shaders/modelbuffer/vert.spv");
	auto fragCode = vkTool::readfile("./shaders/modelbuffer/frag.spv");


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

	//Vertex
	VkPipelineVertexInputStateCreateInfo vertex_inputInfo = {};
	vertex_inputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	/*vertex_inputInfo.vertexBindingDescriptionCount = 0;
	vertex_inputInfo.vertexAttributeDescriptionCount = 0;*/
	auto bindingDescribtion = Vertex::getBindingDescribtion();
	auto attribDescribtion = Vertex::getAttributeDescribtions();

	vertex_inputInfo.vertexBindingDescriptionCount = 1;
	vertex_inputInfo.vertexAttributeDescriptionCount = attribDescribtion.size();
	vertex_inputInfo.pVertexBindingDescriptions = &bindingDescribtion;
	vertex_inputInfo.pVertexAttributeDescriptions = attribDescribtion.data();



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
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	//if multisampling
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//depth stencil
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

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

	/*DESCRIPTOR SET LAYOUTS*/
	VkDescriptorSetLayout setLayouts[] = { m_descriptorSetLayout };

	VkPipelineLayoutCreateInfo pipeline_layoutInfo = {};
	pipeline_layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layoutInfo.setLayoutCount = 1;
	pipeline_layoutInfo.pSetLayouts = setLayouts;		//descriptorsey layout
														//pipeline_layoutInfo.pushConstantRangeCount = 0;

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
	pipelineInfo.pDepthStencilState = &depthStencil;
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

void ModelBuffer::buildFrameBuffers()
{
	m_frame_buffers.resize(m_image_views.size(), VDeleter<VkFramebuffer>{m_device, vkDestroyFramebuffer});

	for (size_t i = 0; i < m_image_views.size(); ++i)
	{
		//VkImageView attachments[] = { m_image_views[i] };
		std::array<VkImageView, 2> attachments = {
			m_image_views[i],m_depth_image_view
		};

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.renderPass = m_renderpass;
		frameBufferCreateInfo.attachmentCount = attachments.size();
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width = m_swapchain_extent.width;
		frameBufferCreateInfo.height = m_swapchain_extent.height;
		frameBufferCreateInfo.layers = 1;

		LOG_ERROR("failed to create frame buffers") <<
			vkCreateFramebuffer(
				m_device, &frameBufferCreateInfo, nullptr, m_frame_buffers[i].replace());
	}

}

void ModelBuffer::buildCommandPool()
{
	QueueFamilyIndeice queueFamilyIndices = findQueueFamilies(m_physical_device);

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

	LOG_ERROR("failed to create command pool")
		<< vkCreateCommandPool(m_device, &cmdPoolInfo, nullptr, m_command_pool.replace());
}

void ModelBuffer::buildCommandBuffers()
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

		//clear value
		std::array<VkClearValue, 2>  clearValues = {};
		clearValues[0].color = { 0.1f, 0.2f, 0.3f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_command_buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphic_pipeline);

		/*-------- VERTEX BUFFERS --------*/
		VkBuffer vertexBuffers[] = { m_vertex_buffer };
		VkDeviceSize offsets[] = { 0 };
		/*BIND VERTEX*/

		vkCmdBindVertexBuffers(m_command_buffers[i], 0, 1, vertexBuffers, offsets);

		/*BIND INDICE*/

		vkCmdBindIndexBuffer(m_command_buffers[i], m_indice_buffer, 0, VK_INDEX_TYPE_UINT32);

		/*UNIFORM BUFFERS*/
		vkCmdBindDescriptorSets(m_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipeline_layout, 0, 1, &m_descriptor_set, 0, nullptr);

		//INDEX DRAW 
		vkCmdDrawIndexed(m_command_buffers[i], m_indices.size(), 1, 0, 0, 0);

		//vkCmdDraw(m_command_buffers[i], m_vertices.size(), 1, 0, 0);	//for defualt glsl

		vkCmdEndRenderPass(m_command_buffers[i]);

		LOG_ERROR("faile to acess to cammand buffers") <<
			vkEndCommandBuffer(m_command_buffers[i]);
	}
	//LOG_SECTION(create command buffers);
}

void ModelBuffer::buildSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult imageErr, renderErr;

	imageErr = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, m_semaphore_image_available.replace());
	renderErr = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, m_semaphore_render_finished.replace());

	if (imageErr != VK_SUCCESS || renderErr != VK_SUCCESS)
		LOG_ASSERT("failed to create semaphores");
}

void ModelBuffer::render()
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

void ModelBuffer::buildVertexBuffers()
{
	VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

	VDeleter<VkBuffer> stagingBuffer(m_device, vkDestroyBuffer);
	VDeleter<VkDeviceMemory> stagingBufferMemory(m_device, vkFreeMemory);

	//usage must be src
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	//copy to data
	memcpy(data, m_vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_device, stagingBufferMemory);

	//usage must dst
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertex_buffer, m_vertex_buffer_memory);

	//miss create buffer first
	copyBuffer(stagingBuffer, m_vertex_buffer, bufferSize);
}

void ModelBuffer::buildIndiceBuffers()
{
	VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

	VDeleter<VkBuffer> stagingBuffer{ m_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> stagingBufferMemory{ m_device, vkFreeMemory };

	//create buffer first for staging
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	//trasfer memory
	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_indices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_device, stagingBufferMemory);

	//create buffer for dst(allocate indice memory buffer and memory)
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indice_buffer, m_indice_buffer_memory);

	//staging to buffer
	copyBuffer(stagingBuffer, m_indice_buffer, bufferSize);

}

void ModelBuffer::updateUniformBuffer()
{

	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.f;


	m_ubo.model.rotate(AXIS::Z, 0.002);
	m_ubo.view = Matrix4x4::lookAtVK(vec3f(-4, 0, 2), vec3f(0, 0, 1), vec3f(0, 0, 1));
	float aspect = m_swapchain_extent.width / (float)m_swapchain_extent.height;
	m_ubo.proj = Matrix4x4::perspectiveVK(30.f, aspect, 0.001, 1000.0);

	void* data;
	vkMapMemory(m_device, m_uinform_staging_buffer_memory, 0, sizeof(m_ubo), 0, &data);
	memcpy(data, &m_ubo, sizeof(m_ubo));
	vkUnmapMemory(m_device, m_uinform_staging_buffer_memory);

	copyBuffer(m_uniform_staging_buffer, m_uniform_buffer, sizeof(m_ubo));
	//LOG << "uniform working" << ENDL;

}