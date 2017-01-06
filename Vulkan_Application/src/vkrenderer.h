#pragma once

#include <vkutils.h>
#include <window.h>

class Window;
class VkRenderer 
{
public:
	VkRenderer(Window* window, const std::string &title, bool isDebug = true);
	virtual~VkRenderer();

//protected:
	virtual void build_procedural();
	virtual void delete_procedural();

	void build_instance();
	void deleteinstance();

	void build_layers();
	void build_debug();
	void deleteDebug();

	void build_device();
	void deleteDevice();

	Window* m_pWindow;
	//vulkan
	bool isDebug;
	std::string name;

	VkInstance m_instance = VK_NULL_HANDLE;
	/*--------------- DEVICE -------------*/
	VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
	VkDevice m_device	= VK_NULL_HANDLE;
	VkQueue m_queue;
	VkPhysicalDeviceProperties m_device_properties;
	VkPhysicalDeviceMemoryProperties m_device_memory_properties;
	VkPhysicalDeviceFeatures m_device_features;
	uint32_t m_graphics_faimily_index;

	/*------------- SURFACE ------------*/
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;									//surface
	VkSurfaceFormatKHR	m_surface_format = {};				//surface format
	VkSurfaceCapabilitiesKHR m_surface_capablilities = {};	//cap abilities
	uint32_t m_surface_width;
	uint32_t m_surface_height;

	/*------------- SWAPCHAIN -------------*/
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	uint32_t m_swapchain_image_count = 2;
	uint32_t m_active_swapchain_image_id = UINT32_MAX;

	/*-------------- IMAGE ----------------*/
	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_image_views;

	/*----------- DEPTH STENCIL ------------*/
	VkImage				m_depth_stencil_image = VK_NULL_HANDLE;
	VkImageView			m_depth_stencil_image_view = VK_NULL_HANDLE;
	VkFormat			m_depth_stencil_format = VK_FORMAT_UNDEFINED;
	VkDeviceMemory		m_depth_stencil_memory = VK_NULL_HANDLE;

	bool				m_depth_stencil_available = false;

	/*-------------- RENDER PASS -----------------*/
	VkRenderPass		m_render_pass = VK_NULL_HANDLE;

	/*--------- FRAME BUFFER -------------*/
	std::vector<VkFramebuffer> m_frame_buffers;

	/*--------- SYNC ----------*/
	VkFence m_swapchain_image_available = VK_NULL_HANDLE;

	//LAYER
	std::vector<const char*> m_instance_layers;
	std::vector<const char*> m_device_layers;
	//EXTENSIONS
	std::vector<const char*> m_instance_extension;
	std::vector<const char*> m_device_extension;
	//callback for debug
	VkDebugReportCallbackEXT m_debug_report = VK_NULL_HANDLE;
	VkDebugReportCallbackCreateInfoEXT m_debug_callbackcreateInfo;
	
	VkRenderPass getRenderPass();
	VkFramebuffer getActiveFrameBuffer();
	VkExtent2D getSurfaceSize();

	void setSurfaceFromWindow(Window* pWindow);

	void initSurface();
	void deleteSurface();

	//swapchain
	void initSwapchain();
	void deleteSwapchain();

	//images
	void initImages();
	void deleteImages();

	//depth stencil image
	void initDepthStencilImage();
	void deleteDepthStencilImage();

	//render pass
	virtual void initRenderPass();
	void deleteRenderPass();

	//set pipeline cache


	//framebuffers
	virtual void initFrameBuffer();
	void deleteFrameBuffer();

	//sync
	void initSynchronization();
	void deleteSynchronization();

	//prepare
	/*---------------------------------------------------------*/
	VkCommandPool m_command_pool;
	
	void build_command_pool();
	void delete_command_pool();

	VkCommandBuffer m_command_buffer;
	void build_command_buffer();
	void delete_command_buffer();

	//semaphore for complete
	VkSemaphore m_complete_semaphore =  VK_NULL_HANDLE;
	void build_complete_semaphore();
	void delete_complete_semaphore();

	float frame = 0.0f;
	
	//render method
	void begin();
	void end();

	//----------------------- override
	virtual void build_addtion() {};
	virtual void render();

};


inline VkRenderPass VkRenderer::getRenderPass()
{
	return m_render_pass;
}
inline VkFramebuffer VkRenderer::getActiveFrameBuffer()
{
	return m_frame_buffers[m_active_swapchain_image_id];
}
inline VkExtent2D VkRenderer::getSurfaceSize()
{
	return{ m_surface_width, m_surface_height };
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDEbugCallback(	VkDebugReportFlagsEXT flag,
													VkDebugReportObjectTypeEXT objType,
													uint64_t object,
													size_t location,
													int32_t msg_code,
													const char* player_prefix,
													const char* msg,
													void* user_data);