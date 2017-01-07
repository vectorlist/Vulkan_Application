#include "renderer.h"
#include <window.h>

Renderer::Renderer(Window* window, const std::string & name)
{
	//build precedural
	createInstance();
	setupDebugCallback();
	createSurface(window);
	pickPhysicalDevice();
	//now create logical device
	createLogicalDevice();
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
	//m_physical_device = devices[0];      //not logical
	//Pick PhysicalDevice
	for (const auto &device : devices)
	{
		//get device if suitable
		if (isDeviceSuitable(device))
		{
			m_physical_device = device;
			break;
		}
		
	}

	if (m_physical_device == VK_NULL_HANDLE)
	{
		throw std::runtime_error("couldnt find suitable physical device");
	}


}

void Renderer::createLogicalDevice()
{
	//we created suitable physical device already
	QueueFamilyIndeice indices = findQueueFamilies(m_physical_device);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoList;
	std::set<int> uniqueQueueFamiles = { indices.graphicsFamily, indices.presentFamily };

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamiles)
	{
		VkDeviceQueueCreateInfo queueCreateInfos = {};
		queueCreateInfos.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos.queueFamilyIndex = queueFamily;
		queueCreateInfos.queueCount = 1;
		queueCreateInfos.pQueuePriorities = &queuePriority;
		queueCreateInfoList.push_back(queueCreateInfos);
	}
	//no features atm
	VkPhysicalDeviceFeatures deviceFeatures = {};
	
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = queueCreateInfoList.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfoList.data();

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) //if has debug layers
	{
		deviceCreateInfo.enabledLayerCount = validationLayers.size();
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}
	//finally get device(logical)
	VkResult err = vkCreateDevice(m_physical_device, &deviceCreateInfo, nullptr, m_device.replace());

	if (err != VK_SUCCESS) VK_ERROR(couldnt create logical device);
}

QueueFamilyIndeice Renderer::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndeice indices;
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
	LOG << queue_family_count << ENDL;
	std::vector<VkQueueFamilyProperties> queue_family_props(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(
		device, &queue_family_count, queue_family_props.data());


	int i = 0;
	for (const auto &queuefamily : queue_family_props)
	{
		if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			LOG << "enabled queue GRAPHIC BIT index : " << i << ENDL;
			indices.graphicsFamily = i;
		}

		VkBool32 support_present = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &support_present);

		if (queuefamily.queueCount > 0 && support_present) {
			indices.presentFamily = i;
		}
		LOG << "is support ? " << support_present << ENDL;
		i++;
	}

	LOG <<	"graphic :" << indices.graphicsFamily << "   "
		<<  " present" << indices.presentFamily << ENDL;
	if (!indices.isComplete()) {
			throw std::runtime_error("queue is not support from surface");
	}
	return indices;
}

bool Renderer::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndeice queue_indice = findQueueFamilies(device);
	//check device extension supported
	bool extensionSupport = checkExtensionSuppot(device);

	bool swapchainSupport = false;
	if (extensionSupport)
	{
		SwapChainSupportDetails details = querySwapchainSupport(device);
		swapchainSupport = (!details.formats.empty() && !details.presentModes.empty());
	}

	return (queue_indice.isComplete() && extensionSupport && swapchainSupport);
}


bool Renderer::checkExtensionSuppot(VkPhysicalDevice device)
{
	uint32_t ext_count = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &ext_count, nullptr);
	std::vector<VkExtensionProperties> extension_list(ext_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &ext_count, extension_list.data());

	LOG << "extension count : " << ext_count << ENDL;
	std::set<std::string> requiredExtension(deviceExtensions.begin(), deviceExtensions.end());

	for (auto &ex : extension_list)
	{
		requiredExtension.erase(ex.extensionName);
	}
	bool support = false;
	if (requiredExtension.empty())
	{
		support = true;
	}

	return support;
}

SwapChainSupportDetails Renderer::querySwapchainSupport(VkPhysicalDevice device)
{
	//this for get swapchain capabilities / formats / presentmodes
	SwapChainSupportDetails details;
	//get capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);
	//get format count
	uint32_t format_counts;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_counts, nullptr);

	if (format_counts != 0)
	{
		details.formats.resize(format_counts);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_counts, details.formats.data());
	}
	//get presentmode
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device, m_surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

