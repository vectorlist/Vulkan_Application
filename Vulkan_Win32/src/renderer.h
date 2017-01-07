#pragma once

#include <vkutils.h>
#include <vdeleter.h>


class Window;
class Renderer
{
public:
	Renderer(Window* window, const std::string &name);
	~Renderer();

	Window *m_window;

	void createInstance();
	void setupDebugCallback();
	void createSurface(Window* window);
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapchain();
	void createImageViews();
	void createRenderpass();
	void createPipeline();


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

};

