#pragma once
#include <vkrenderer.h>

typedef struct VerticesBuffers
{
	VkDeviceMemory memory;
	VkBuffer buffer;
	VkPipelineVertexInputStateCreateInfo input_state;
	std::vector<VkVertexInputAttributeDescription> inputAttribute;
}VerticesBuffers;

typedef struct VerticesIndices
{
	VkDeviceMemory memory;
	VkBuffer buffer;
	uint32_t count;
}VerticesIndices;

class VkTriangle : public VkRenderer
{
public:
	VkTriangle(Window* window, const std::string &title, bool isDebug = true);
	virtual~VkTriangle();

	void build_addtion() VK_OVERRIDE;
	
	void initRenderPass() VK_OVERRIDE;
	void initFrameBuffer() VK_OVERRIDE;
	
	void render() VK_OVERRIDE;
	void build_shader();

	VerticesBuffers m_vertices;
	VerticesIndices m_indices;
	
	VkPipelineLayout m_pipeline_layout;
	VkPipeline m_pipeline;
	VkDescriptorSetLayout m_describtor_set_layout;
	VkDescriptorSet m_describtor_set;

	VkSemaphore m_present_complete_semaphore;
	VkSemaphore m_render_complete_semaphore;

	std::vector<VkFence> m_wait_fences;
	std::vector<VkCommandBuffer> m_command_buffers;
	
	void build_command_buffers();
	void build_synchronization_primitives();

};

