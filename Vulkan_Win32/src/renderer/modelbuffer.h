#pragma once

#include <renderer.h>
class ModelBuffer : public Renderer
{
public:
	ModelBuffer(Window* window);
	virtual~ModelBuffer();

	void buildProcedural() VK_OVERRIDE;

	void buildPipeline() VK_OVERRIDE;
	void buildFrameBuffers() VK_OVERRIDE;
	void buildCommandPool() VK_OVERRIDE;
	void buildCommandBuffers() VK_OVERRIDE;
	void buildSemaphores() VK_OVERRIDE;

	//additional
	void buildVertexBuffers();
	void buildIndiceBuffers();

	void render() VK_OVERRIDE;

	/*MODEL*/
	void buildModel(const std::string &filename);

	/*DEPTH STENCIL*/

	void buildDeapthStencil();
	VDeleter<VkImage> m_depth_image{ m_device, vkDestroyImage };
	VDeleter<VkImageView> m_depth_image_view{ m_device, vkDestroyImageView };
	VDeleter<VkDeviceMemory> m_depth_image_memory{ m_device,vkFreeMemory };


	/*TEXTURE BUFFER*/
	void buildTextureImage();
	void buildTextureImageView();
	void buildTextureSampler();
	VDeleter<VkImage> m_texture_image{ m_device, vkDestroyImage };
	VDeleter<VkDeviceMemory> m_texture_image_memory{ m_device, vkFreeMemory };

	VDeleter<VkImageView> m_texture_image_view{ m_device,vkDestroyImageView };
	VDeleter<VkSampler> m_textureSampler{ m_device, vkDestroySampler };


	/*UNIFORM BUFFER*/
	UBO m_ubo;

	void buildDescriptorSetLayout();
	void buildDescriptorPool();
	void buildDescriptorSet();
	void buildUniformBuffer();

	void updateUniformBuffer() VK_OVERRIDE;


	VDeleter<VkDescriptorSetLayout> m_descriptorSetLayout{ m_device,vkDestroyDescriptorSetLayout };
	//VDeleter<VkPipelineLayout> m_pipelineLayout{ m_device,vkDestroyPipelineLayout };

	VDeleter<VkBuffer> m_uniform_staging_buffer{ m_device,vkDestroyBuffer };
	VDeleter<VkDeviceMemory> m_uinform_staging_buffer_memory{ m_device,vkFreeMemory };

	VDeleter<VkBuffer> m_uniform_buffer{ m_device,vkDestroyBuffer };
	VDeleter<VkDeviceMemory> m_uinform_buffer_memory{ m_device,vkFreeMemory };

	VDeleter<VkDescriptorPool> m_descriptor_pool{ m_device,vkDestroyDescriptorPool };
	VkDescriptorSet m_descriptor_set;

	/*VERTEX BUFFER*/
	std::vector<Vertex> m_vertices;
	VDeleter<VkBuffer> m_vertex_buffer{ m_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> m_vertex_buffer_memory{ m_device,vkFreeMemory };

	/*INDEX BUFFER*/
	std::vector<uint32_t> m_indices;
	VDeleter<VkBuffer> m_indice_buffer{ m_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> m_indice_buffer_memory{ m_device,vkFreeMemory };
};

