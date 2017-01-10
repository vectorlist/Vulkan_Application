#pragma once

#include <vkutils.h>
#include <vdeleter.h>
#include <vertex.h>

class Window;
class Renderer
{
public:
	Renderer(Window* window);
	virtual~Renderer();

	Window *m_window;

	virtual void buildProcedural();

	void buildInstance();
	void buildDebug();
	void buildSurface(Window* window);
	void buildPhysicalDevice();
	void buildLogicalDevice();
	void buildSwapChain();
	void buildimageViews();
	void buildRenderPass();
	virtual void buildPipeline() = 0;
	virtual void buildFrameBuffers() =0;
	virtual void buildCommandPool() =0;
	virtual void buildCommandBuffers() =0;
	virtual void buildSemaphores() =0;

	virtual void render() = 0;
	virtual void updateUniformBuffer() {};


	void reInitSwapchain();

	VDeleter<VkInstance> m_instance{ vkDestroyInstance };
	VDeleter<VkDebugReportCallbackEXT> callback{ m_instance, vkDebug::DestroyDebugReportCallbackEXT };
	VDeleter<VkSurfaceKHR> surface{ m_instance, vkDestroySurfaceKHR };

	VDeleter<VkSurfaceKHR> m_surface{m_instance,vkDestroySurfaceKHR};
	VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
	VDeleter<VkDevice> m_device{vkDestroyDevice};

	VkQueue m_graphic_queue;
	VkQueue m_present_queue;

	/* SWAPCHAIN */
	VDeleter<VkSwapchainKHR> m_swapcahin{ m_device, vkDestroySwapchainKHR };
	std::vector<VkImage> m_images;
	std::vector<VDeleter<VkImageView>> m_image_views;
	VkFormat m_image_format;			//image format
	VkExtent2D m_swapchain_extent;		//size 

	/*RENDERPASS*/
	VDeleter<VkRenderPass> m_renderpass{ m_device,vkDestroyRenderPass };

	/*PIPELINE*/
	VDeleter<VkPipelineLayout> m_pipeline_layout{ m_device, vkDestroyPipelineLayout };
	VDeleter<VkPipeline> m_graphic_pipeline{ m_device,vkDestroyPipeline };

	/*FRAMEBUFFER*/
	std::vector<VDeleter<VkFramebuffer>> m_frame_buffers;

	/*COMMAND POOL*/
	VDeleter<VkCommandPool> m_command_pool{ m_device,vkDestroyCommandPool };

	/*COMMAND BUFFERS*/
	std::vector<VkCommandBuffer> m_command_buffers;

	/*SEMAPHORE*/
	//1st semaphore is ready for next image
	VDeleter<VkSemaphore> m_semaphore_image_available{ m_device, vkDestroySemaphore };
	//2st is for inform us when rendering completed
	VDeleter<VkSemaphore> m_semaphore_render_finished{ m_device, vkDestroySemaphore };

	//-----------------  built in functions ------------------
	QueueFamilyIndeice findQueueFamilies(VkPhysicalDevice device);
	bool checkExtensionSuppot(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR> &availableFormats);
	VkPresentModeKHR choosePresentMode(
		const std::vector<VkPresentModeKHR> &availableModes);

	VkExtent2D chooseSwapcExtent2D(const VkSurfaceCapabilitiesKHR &capabilities);
	void createShaderModule(const std::vector<char> &code, VDeleter<VkShaderModule> &shaderModule);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VDeleter<VkBuffer> &buffer,
		VDeleter<VkDeviceMemory> &bufferMemory);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


	//image built in functions
	void createImage(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VDeleter<VkImage> &image,
		VDeleter<VkDeviceMemory> &imageMemory);

	void copyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);

	void transitionImageLayout(
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout);

	//command built in function
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommand(VkCommandBuffer commandBuffer);

	void createImageView(VkImage image, VkFormat format, VDeleter<VkImageView> &imageView);

};

