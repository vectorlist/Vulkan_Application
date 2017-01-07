#pragma once

#include <vkutils.h>
#include <vdeleter.h>


class Window;
class Renderer
{
public:
	Renderer(Window* window, const std::string &name);
	~Renderer();

	void createInstance();
	void setupDebugCallback();
	void createSurface(Window* window);
	void pickPhysicalDevice();

	VDeleter<VkInstance> m_instance{ vkDestroyInstance };
	VDeleter<VkDebugReportCallbackEXT> callback{ m_instance, vkDebug::DestroyDebugReportCallbackEXT };
	VDeleter<VkSurfaceKHR> surface{ m_instance, vkDestroySurfaceKHR };

	VDeleter<VkSurfaceKHR> m_surface{m_instance,vkDestroySurfaceKHR};
	VkPhysicalDevice m_physical_device;
};
