#pragma once
#include <renderer.h>

class BasicRenderer : public Renderer
{
public:
	BasicRenderer(Window *window);
	virtual~BasicRenderer();

	void buildProcedural();

	void buildPipeline() VK_OVERRIDE;
	void buildFrameBuffers() VK_OVERRIDE;
	void buildCommandPool() VK_OVERRIDE;
	void buildCommandBuffers() VK_OVERRIDE;
	void buildSemaphores() VK_OVERRIDE;

	void render() VK_OVERRIDE;
};

