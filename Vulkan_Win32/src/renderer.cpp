#include "renderer.h"
#include <window.h>

Renderer::Renderer(Window* window, const std::string & name)
{
	createInstance();
	setupDebugCallback();
	createSurface(window);
	pickPhysicalDevice();
}


Renderer::~Renderer()
{

}

void Renderer::createInstance()
{
	if (!enableValidationLayers /*&& !checkValidationLayerSupport()*/) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = vkTool::getRequiredExtenstions();
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, m_instance.replace()) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void Renderer::setupDebugCallback() {
	if (!enableValidationLayers) return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallback;

	if (vkDebug::CreateDebugReportCallbackEXT(m_instance, &createInfo, nullptr, callback.replace()) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug callback!");
	}
}

void Renderer::createSurface(Window * window)
{
	VkWin32SurfaceCreateInfoKHR surface_createInfo{};
	surface_createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_createInfo.hinstance = window->m_instance;
	surface_createInfo.hwnd = window->m_window;

	vkCreateWin32SurfaceKHR(m_instance, &surface_createInfo, nullptr, m_surface.replace());
}

void Renderer::pickPhysicalDevice()
{
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);
	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

	m_physical_device = devices[0];

	//find queue fimiles
	QueueFamilyIndeice indices;
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, nullptr);
	LOG << queue_family_count << ENDL;
	std::vector<VkQueueFamilyProperties> queue_family_props(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queue_family_count, queue_family_props.data());

	int i = 0;
	int found_present = -1;
	int found_graphic = -1;
	for (const auto &queuefamily : queue_family_props)
	{
		if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			LOG << "enabled queue GRAPHIC BIT index : " << i << ENDL;
			found_graphic = i;
		}

		VkBool32 support_present = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, m_surface, &support_present);

		if (queuefamily.queueCount > 0 && support_present) {
			found_present = i;
		}
		LOG << "is support ? " << support_present << ENDL;
		i++;
	}

	LOG << "graphic :" << found_graphic << "   "<<  " present" <<found_present << ENDL;
	if (found_graphic == -1 || found_present == -1) {
		throw std::runtime_error("queue is not support from surface");
	}
}



