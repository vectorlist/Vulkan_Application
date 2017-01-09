#pragma once

#include <renderer.h>
#include <vertex.h>

class VertexBufferr : public Renderer
{
public:
	VertexBufferr(Window* window);
	virtual~VertexBufferr();

	void buildProcedural();

	void buildPipeline() VK_OVERRIDE;
	void buildFrameBuffers() VK_OVERRIDE;
	void buildCommandPool() VK_OVERRIDE;
	void buildCommandBuffers() VK_OVERRIDE;
	void buildSemaphores() VK_OVERRIDE;

	//additional
	void buildVertexBuffers();


	
	void render() VK_OVERRIDE;

	/*STAGING*/


	/*VERTEX BUFFER*/
	std::vector<Vertex> m_vertices;
	VDeleter<VkBuffer> m_vertex_buffer{ m_device, vkDestroyBuffer };
	VDeleter<VkDeviceMemory> m_vertex_buffer_memory{ m_device,vkFreeMemory };
};

