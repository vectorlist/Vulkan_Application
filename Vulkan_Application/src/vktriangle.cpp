#include "VkTriangle.h"

VkTriangle::VkTriangle(Window* window, const std::string &title, bool isDebug)
	: VkRenderer(window,title,isDebug)
{

}


VkTriangle::~VkTriangle()
{

}


void VkTriangle::build_addtion()
{
	//add addtional function from derive class
}

void VkTriangle::initRenderPass()
{
	//LOG << "do nothing" << ENDL;
	VkAttachmentDescription color_attachment = {};
	//color_attachment.flags =   flags;
	color_attachment.format = m_surface_format.format;
	color_attachment.samples  =  VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	//color dont load
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_createInfo = {};
	render_pass_createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_createInfo.attachmentCount = 1;
	render_pass_createInfo.pAttachments = &color_attachment;
	render_pass_createInfo.subpassCount = 1;
	render_pass_createInfo.pSubpasses = &subpass;
	render_pass_createInfo.dependencyCount = 1;
	render_pass_createInfo.pDependencies = &dependency;

	vkError() << vkCreateRenderPass(m_device, &render_pass_createInfo, nullptr, &m_render_pass);

	LOG_SECTION(create triangle renderpass);

}

void VkTriangle::initFrameBuffer()
{
	//st'll working far now
	m_frame_buffers.resize(m_swapchain_image_count);
	for (uint32_t i = 0; i < m_swapchain_image_count; ++i)
	{
		//store imageview by index
		VkImageView attachment[] = { m_image_views[i] };
		VkFramebufferCreateInfo frame_buffer_createinfo = {};
		frame_buffer_createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frame_buffer_createinfo.renderPass = m_render_pass;
		frame_buffer_createinfo.attachmentCount = 1;
		//imageview as attachment
		frame_buffer_createinfo.pAttachments = attachment;
		frame_buffer_createinfo.width = m_surface_width;
		frame_buffer_createinfo.height = m_surface_height;
		frame_buffer_createinfo.layers = 1;
		vkError() << vkCreateFramebuffer(
			m_device, &frame_buffer_createinfo, nullptr, &m_frame_buffers[i]);
	}
	LOG_SECTION(TRI create frame buffer);
	LOG << "frame buffer counts : " << m_swapchain_image_count << ENDL;
}

void VkTriangle::build_shader()
{
	VkShaderModuleCreateInfo shader_createInfo = {};
	shader_createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_createInfo.codeSize = 0;			//yet
	//shader_createInfo.pCode = 
}

void VkTriangle::build_command_buffers()
{
	VkCommandBufferBeginInfo command_bufferInfo = {};
	command_bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_bufferInfo.pNext = nullptr;

	VkClearValue clear_value[2];
	//clear_value[0].color = { {0.0f,0.0f,0.2,1.0f} };
	//clear_value[0].depthStencil = { 1.0f, 0 };



}

void VkTriangle::build_synchronization_primitives()
{
	VkSemaphoreCreateInfo semaphore_createInfo = {};
	semaphore_createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphore_createInfo.pNext = nullptr;

	vkError() << vkCreateSemaphore(
		m_device, &semaphore_createInfo, nullptr, &m_present_complete_semaphore);
	vkError() << vkCreateSemaphore(
		m_device, &semaphore_createInfo, nullptr, &m_render_complete_semaphore);

	VkFenceCreateInfo fence_createInfo = {};
	fence_createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	//m_wait_fences.resize()

}

void VkTriangle::render()
{
	//LOG << "working" << ENDL;
}