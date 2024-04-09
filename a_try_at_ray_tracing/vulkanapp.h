#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_RADIANS
#include <GLFW/glfw3native.h>
//#include<vulkan/vulkan.h>
#include <chrono>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include"glm/gtc/matrix_transform.hpp"
#include <set>
#include <array>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <optional>
#include "glm/glm.hpp"
const std::vector<const char*> validationlayers = {
	"VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
bool checkValidationLayerSupport() {
	uint32_t layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties> available_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
	for (auto &layer : validationlayers) {
		bool found = false;
		for (auto& available : available_layers) {
			if (std::strcmp(layer, available.layerName) == 0) {
				found = true;
				break;
			}
		}
		if (!found)
			return false;
	}
	return true;	
}
struct Material {
	glm::vec3 albedo;
	glm::vec3 emmission_color;
	float emmission_strength;
};
struct Sphere {
	glm::vec3 pos;
	float r;
	int mat_idx;
	Sphere(
		glm::vec3 _pos,
		float _r,
		int _mat_idx
	) {

		pos = _pos;
		r = _r;
		mat_idx = _mat_idx;
	}
};
bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}	
std::vector<const char*> getRequiredExtentions() {
	uint32_t glfw_extentions_count = 0;
	const char** glfw_extentions;
	glfw_extentions = glfwGetRequiredInstanceExtensions(&glfw_extentions_count);
	std::vector<const char*> extentions(glfw_extentions, glfw_extentions + glfw_extentions_count);
	if (enableValidationLayers) {
		extentions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return extentions;
}
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}
struct Vertex {
	glm::vec3 pos;

	static VkVertexInputBindingDescription getInputBindingDescreption() {
		VkVertexInputBindingDescription d{};
		d.binding = 0;
		d.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		d.stride = sizeof(Vertex);
		return d;
	}
	static std::array<VkVertexInputAttributeDescription, 1> getInputAttributeDescription() {
		std::array<VkVertexInputAttributeDescription, 1> as{};
		as[0].binding = 0;
		as[0].location = 0;
		as[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		as[0].offset = offsetof(Vertex, pos);
		return as;
	}
};

const std::vector<Vertex> vertices = {
	{{-1.0f, -1.0f, 0}},
	{{1.0f, -1.0f, 0}},
	{{1.0f, 1.0f, 0}},
	{{-1.0f, 1.0f, 0}}
};
const std::vector<uint16_t> indices = {
	0, 1, 2,
	2, 3, 0
};
class App {
public:
	void run() {
		spheres = { 
			Sphere(glm::vec3(2.3,-0.04,-0.2), 1.f, 0),
			Sphere(glm::vec3(0,-60,0), 59.f, 3),
			Sphere(glm::vec3(0,2.4,69), 28.2f, 2),
			Sphere(glm::vec3(0.2,-0.2,-0.3), 0.88f, 3),
			Sphere(glm::vec3(-1.4,-0.4,-0.1), 0.6f, 5),
			Sphere(glm::vec3(-2.7,-0.65,0.2), 0.4f, 4) 
		};
		initWndow();
		initVulkan();
		mainLoop();
		cleanUp();
	}
private:
	int WIDTH = 800, HIEGHT = 800;
	const int MAX_FRAMES_IN_FLIGHT = 2;
	uint32_t currentFrame = 0;
	GLFWwindow* window;
private:
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugmessenger;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphics_q;
	VkQueue present_q; VkSwapchainKHR swapChain;
	VkSurfaceKHR surface; VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffers;
	VkCommandPool commandPool;
	VkBuffer vbo;
	VkDeviceMemory vbo_mem;
	VkBuffer ibo;
	VkDeviceMemory ibo_mem;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	std::vector<VkBuffer> ubos;
	std::vector<VkDeviceMemory> ubos_mem;
	std::vector<void*> ubos_maped;
	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkBuffer> spheres_ssbos;
	std::vector<VkDeviceMemory> spheres_ssbos_mem;
	int n_spheres = 50;
	std::vector<Sphere> spheres;
	std::vector<Material> materials;
	struct UBO {
		int max_bounces = 5;
		int n_traces = 20;

		glm::uvec2 res;
		glm::vec2 iMouse;
		float cam_dist;
	};
private:
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}
	void initWndow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, 0);
		window = glfwCreateWindow(WIDTH, HIEGHT, "vukan app", nullptr, nullptr);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + 2);
	}
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* user_data
	) {
		if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			std::cerr << "validation layer:" << pCallbackData->pMessage << "TYPE:" << type << "\n";
			return VK_FALSE; 
		}
	}
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}
	void creatInstance() {
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "vulkan app";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "none";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_3;
		VkInstanceCreateInfo creat_info{};
		if (enableValidationLayers && !checkValidationLayerSupport()){
			throw std::runtime_error("layers requested, but not found");
		}
		if (enableValidationLayers) {
			VkDebugUtilsMessengerCreateInfoEXT messengercreation_info{};
			populateDebugMessengerCreateInfo(messengercreation_info);
			creat_info.enabledLayerCount = static_cast<uint32_t>(validationlayers.size());
			creat_info.ppEnabledLayerNames = validationlayers.data();
			creat_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&messengercreation_info;
		}
		else
			creat_info.enabledLayerCount = 0;
		creat_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		creat_info.pApplicationInfo = &app_info;
		uint32_t extention_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extention_count, nullptr);
		{
			std::vector<VkExtensionProperties> extentions(extention_count);
			vkEnumerateInstanceExtensionProperties(nullptr, &extention_count, extentions.data());
			std::cout << "available extensions:\n";
			for (auto& i : extentions) {
				std::cout << "\t" << i.extensionName << "\n";
			}
		}
		auto extentions = getRequiredExtentions();
		creat_info.enabledExtensionCount = static_cast<uint32_t>(extentions.size());
		creat_info.ppEnabledExtensionNames = extentions.data();


		if(vkCreateInstance(&creat_info, nullptr, &instance) != VK_SUCCESS){
			throw std::runtime_error("failed to creat instance!");
		}
	}
	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT creation_info{};
		populateDebugMessengerCreateInfo(creation_info);
		//creation_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		//creation_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		//creation_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
		//creation_info.pfnUserCallback = debugCallback;
		//creation_info.pUserData = nullptr;
		if (CreateDebugUtilsMessengerEXT(instance, &creation_info, nullptr, &debugmessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool is_complete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	QueueFamilyIndices findQFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indicies;
		uint32_t fam_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &fam_count, nullptr);
		std::vector<VkQueueFamilyProperties>fams(fam_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &fam_count, fams.data());
		for (int i = 0; i < fams.size(); i++) {
			if (fams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indicies.graphicsFamily = i;
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				indicies.presentFamily = i;
			}
		}
		return indicies;
	}
	bool isDeviceSuitable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties prop;
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceProperties(device, &prop);
		vkGetPhysicalDeviceFeatures(device, &features);
		QueueFamilyIndices inds = findQFamilies(device);
		bool extensionsSupported = checkDeviceExtensionSupport(device);
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}
		return inds.is_complete() && extensionsSupported && swapChainAdequate;
		
	}
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != UINT_MAX) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
	void pickPhysicalDevice() {
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
		if (!device_count) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
		std::cout << "physical devices: " << device_count << "\n";
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physical_device = device;
				break;
			}
		}

		if (physical_device == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
		VkPhysicalDeviceProperties prop;
		vkGetPhysicalDeviceProperties(physical_device, &prop);

		std::cout << "most sutable: " << prop.deviceName << "\n";
	}
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}
		return details;
	}
	void creatLogicalDevice() {
		//QueueFamilyIndices inds = findQFamilies(physical_device);
		//std::vector<uint32_t> uni_inds = { inds.graphicsFamily.value(), inds.presentFamily.value() };
		//std::vector<VkDeviceQueueCreateInfo> ci;
		//float prio = 1.0f;
		//for (auto i : uni_inds)
		//{
		//	VkDeviceQueueCreateInfo creat_info{};
		//	creat_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		//	creat_info.queueCount = 1;
		//	creat_info.queueFamilyIndex = i;
		//	creat_info.pQueuePriorities = &prio;
		//	ci.push_back(creat_info);
		//}
		//VkPhysicalDeviceFeatures features;
		//vkGetPhysicalDeviceFeatures(physical_device, &features);
		//VkDeviceCreateInfo dcreat_info{};
		//dcreat_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//dcreat_info.pQueueCreateInfos = ci.data();
		//dcreat_info.queueCreateInfoCount = ci.size();
		//dcreat_info.pEnabledFeatures = &features;
		//dcreat_info.enabledExtensionCount = deviceExtensions.size();
		//dcreat_info.ppEnabledExtensionNames = deviceExtensions.data();

		//if (enableValidationLayers) {
		//	dcreat_info.enabledLayerCount = static_cast<uint32_t>(validationlayers.size());
		//	dcreat_info.ppEnabledLayerNames = validationlayers.data();
		//}
		//else {
		//	dcreat_info.enabledLayerCount = 0;
		//}
		//if (vkCreateDevice(physical_device, &dcreat_info, nullptr, &device) != VK_SUCCESS) {
		//	throw std::runtime_error("failed to create logical device!");
		//}
		//vkGetDeviceQueue(device, inds.graphicsFamily.value(), 0, &graphics_q);
		//vkGetDeviceQueue(device, inds.presentFamily.value(), 0, &present_q);
		QueueFamilyIndices indices = findQFamilies(physical_device);
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{}; // No special features are requested for now.
		vkGetPhysicalDeviceFeatures(physical_device, &deviceFeatures);
		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationlayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationlayers.data();
		}
		else {
			deviceCreateInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physical_device, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphics_q);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &present_q);

	}
	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}
	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physical_device);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		QueueFamilyIndices indices = findQFamilies(physical_device);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}
	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());
		for (int i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo creat_info{};
			creat_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			creat_info.image = swapChainImages[i];
			creat_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			creat_info.format = swapChainImageFormat;
			creat_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			creat_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			creat_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			creat_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			creat_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			creat_info.subresourceRange.baseMipLevel = 0;
			creat_info.subresourceRange.levelCount = 1;
			creat_info.subresourceRange.baseArrayLayer = 0;
			creat_info.subresourceRange.layerCount = 1;
			if (vkCreateImageView(device, &creat_info, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}
	VkShaderModule createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
		return shaderModule;
	}
	void createGraphicsPipeline() {
		auto vertShaderCode = readFile("vert.spv");
		auto fragShaderCode = readFile("frag.spv");
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		auto binding = Vertex::getInputBindingDescreption();
		auto attrib = Vertex::getInputAttributeDescription();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = attrib.size();
		vertexInputInfo.pVertexAttributeDescriptions = attrib.data();
		vertexInputInfo.pVertexBindingDescriptions = &binding;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; 
		rasterizer.rasterizerDiscardEnable = VK_FALSE; 
		rasterizer.lineWidth = 1.0f; 
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; 
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1; // Optional
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}
	void createRenderPass() {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}
	void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (int i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachement[]{
				swapChainImageViews[i]
			};
			VkFramebufferCreateInfo creat_info{};
			creat_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			creat_info.renderPass = renderPass;
			creat_info.attachmentCount = 1;
			creat_info.pAttachments = attachement;
			creat_info.width = swapChainExtent.width;
			creat_info.height = swapChainExtent.height;
			creat_info.layers = 1;

			if (vkCreateFramebuffer(device, &creat_info, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}
	void createCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQFamilies(physical_device);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}
	void createCommandBuffer() {
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

	}
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;
		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		VkBuffer buffers[] = { vbo };
		VkDeviceSize offs[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offs);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
		vkCmdBindIndexBuffer(commandBuffer, ibo, 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffer);
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);
		for (int i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		throw std::runtime_error("failed to find suitable memory type!");
	}
	void copyBuffer(VkBuffer& src_buff, VkBuffer& dst_buff, VkDeviceSize size) {
		VkCommandBufferAllocateInfo alloc_i{};
		alloc_i.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_i.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_i.commandPool = commandPool;
		alloc_i.commandBufferCount = 1;
		VkCommandBuffer cmdbo;
		vkAllocateCommandBuffers(device, &alloc_i, &cmdbo);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmdbo, &beginInfo);
		VkBufferCopy copy{};
		copy.srcOffset = 0;
		copy.dstOffset = 0;
		copy.size = size;
		vkCmdCopyBuffer(cmdbo, src_buff, dst_buff, 1, &copy);
		vkEndCommandBuffer(cmdbo);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdbo;

		vkQueueSubmit(graphics_q, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphics_q);
		vkFreeCommandBuffers(device, commandPool, 1, &cmdbo);
	}
	void creatDescriptorSetLayout() {
		std::array<VkDescriptorSetLayoutBinding, 2>uboLayoutBinding{};
		uboLayoutBinding[0].binding = 0;
		uboLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding[0].descriptorCount = 1;
		uboLayoutBinding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		uboLayoutBinding[0].pImmutableSamplers = nullptr; // Optional
		uboLayoutBinding[1].binding = 1;
		uboLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		uboLayoutBinding[1].descriptorCount = 1;
		uboLayoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		uboLayoutBinding[1].pImmutableSamplers = nullptr; // Optional
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 2;
		layoutInfo.pBindings = uboLayoutBinding.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
	void createDescriptorPool() {
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 2;
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}
	void createDescriptorSets() {
		{
			ubos.resize(MAX_FRAMES_IN_FLIGHT);
			ubos_maped.resize(MAX_FRAMES_IN_FLIGHT);
			ubos_mem.resize(MAX_FRAMES_IN_FLIGHT);
			for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				creatBuffer(sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, ubos[i], ubos_mem[i]);
				vkMapMemory(device, ubos_mem[i], 0, sizeof(UBO), 0, &ubos_maped[i]);

			}
		}
		{
			spheres_ssbos.resize(MAX_FRAMES_IN_FLIGHT);
			spheres_ssbos_mem.resize(MAX_FRAMES_IN_FLIGHT);
			int buff_s = spheres.size() * sizeof(Sphere);
			VkBuffer sbo;
			VkDeviceMemory sbo_mem;
			creatBuffer(buff_s, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sbo, sbo_mem);

			void* data;
			vkMapMemory(device, sbo_mem, 0, buff_s, 0, &data);
			memcpy(data, spheres.data(), buff_s);
			vkUnmapMemory(device, sbo_mem);
			for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				creatBuffer(buff_s, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, spheres_ssbos[i], spheres_ssbos_mem[i]);
				copyBuffer(sbo, spheres_ssbos[i], buff_s);
			}
		}
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();
		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			std::array<VkWriteDescriptorSet, 2> descriptorWrite{};
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = ubos[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UBO);
				descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[0].dstSet = descriptorSets[i];
				descriptorWrite[0].dstBinding = 0;
				descriptorWrite[0].dstArrayElement = 0;
				descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite[0].descriptorCount = 1;
				descriptorWrite[0].pBufferInfo = &bufferInfo;
				descriptorWrite[0].pImageInfo = nullptr; // Optional
				descriptorWrite[0].pTexelBufferView = nullptr; // Optional
			}

			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = spheres_ssbos[i];
				bufferInfo.offset = 0;
				bufferInfo.range = VK_WHOLE_SIZE;
				descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite[1].dstSet = descriptorSets[i];
				descriptorWrite[1].dstBinding = 1;
				descriptorWrite[1].dstArrayElement = 0;
				descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorWrite[1].descriptorCount = 1;
				descriptorWrite[1].pBufferInfo = &bufferInfo;
				descriptorWrite[1].pImageInfo = nullptr; // Optional
				descriptorWrite[1].pTexelBufferView = nullptr; // Optional
			}
			vkUpdateDescriptorSets(device, 2, descriptorWrite.data(), 0, nullptr);
		}
		//std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		//VkDescriptorSetAllocateInfo allocInfo{};
		//allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		//allocInfo.descriptorPool = descriptorPool;
		//allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		//allocInfo.pSetLayouts = layouts.data();
		//descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		//if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		//	throw std::runtime_error("failed to allocate descriptor sets!");
		//}
		//for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		//	VkDescriptorBufferInfo bufferInfo{};
		//	bufferInfo.buffer = uniformBuffers[i];
		//	bufferInfo.offset = 0;
		//	bufferInfo.range = sizeof(UniformBufferObject);
		//	VkWriteDescriptorSet descriptorWrite{};
		//	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//	descriptorWrite.dstSet = descriptorSets[i];
		//	descriptorWrite.dstBinding = 0;
		//	descriptorWrite.dstArrayElement = 0;
		//	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//	descriptorWrite.descriptorCount = 1;
		//	descriptorWrite.pBufferInfo = &bufferInfo;
		//	vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		//}
	}
	void initVulkan() {
		creatInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		creatLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createCommandPool();
		createCommandBuffer();
		createDescriptorPool();
		creatDescriptorSetLayout();
		createDescriptorSets();
		createGraphicsPipeline();
		createFramebuffers();
		createSyncObjects();
	}

	float lerp(float a, float b, float t) {
		return ((a == b) ? b : (a + (b - a) * t));
	}

	double x = 0, y = 0;
	double lx = 0, ly = 0;
	double cx = 400, cy = 400;
	bool first_click = true;
	bool first_stop = true;
	float dist_to_cam = 3;
	void drawFrame() {
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		uint32_t imageIndex;
		vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && first_click) {
			glfwGetCursorPos(window, &lx, &ly);
			first_click = false;
			first_stop = true;
		}
		else if (first_stop) {
			glfwGetCursorPos(window, &x, &y);
			cx += x - lx;
			cy += y - ly;
			first_click = true;
			first_stop = false;
		}
		if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
			dist_to_cam += 0.05;
		if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
			dist_to_cam = glm::clamp(dist_to_cam - 0.05f, 0.0001f, 99999999999999999999999999999999.99999f);
		UBO ubo{};
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		ubo.max_bounces = 5;
		ubo.res = glm::uvec2(WIDTH, HIEGHT);
		ubo.iMouse = glm::vec2(lerp(-1., 1., cx / WIDTH) * 2, lerp(1., -1., cy / HIEGHT) * 2);
		ubo.cam_dist = dist_to_cam;
		memcpy(ubos_maped[currentFrame], &ubo, sizeof(ubo));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(graphics_q, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(present_q, &presentInfo);
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
	//VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory
	void creatBuffer(VkDeviceSize size, VkBufferUsageFlags use, VkMemoryPropertyFlags props, VkBuffer& buff, VkDeviceMemory& mem) {
		VkBufferCreateInfo creat_info{};
		creat_info.usage = use;
		creat_info.size = size;
		creat_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		creat_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		if (vkCreateBuffer(device, &creat_info, nullptr, &buff) != VK_SUCCESS)
			throw std::runtime_error("failed to create buff");
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buff, &memRequirements);

		VkMemoryAllocateInfo alloc_i{};
		alloc_i.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_i.allocationSize = memRequirements.size;
		alloc_i.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, props);
		if (vkAllocateMemory(device, &alloc_i, nullptr, &mem) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(device, buff, mem, 0);
	}
	void mainLoop() {

		{
			//VkBufferCreateInfo creat_info{};
			//creat_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			//creat_info.size = vertices.size() * sizeof(Vertex);
			//creat_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//creat_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			//if (vkCreateBuffer(device, &creat_info, nullptr, &vbo) != VK_SUCCESS)
			//	throw std::runtime_error("failed to create vbo");
			//VkMemoryRequirements memRequirements;
			//vkGetBufferMemoryRequirements(device, vbo, &memRequirements);
			//VkMemoryAllocateInfo alloc_i{};
			//alloc_i.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			//alloc_i.allocationSize = memRequirements.size;
			//alloc_i.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			//if (vkAllocateMemory(device, &alloc_i, nullptr, &vbo_mem) != VK_SUCCESS) {
			//	throw std::runtime_error("failed to allocate vertex buffer memory!");
			//}
			//vkBindBufferMemory(device, vbo, vbo_mem, 0);
			VkBuffer sbo;
			VkDeviceMemory sbo_mem;
			creatBuffer(vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sbo, sbo_mem);

			void* data;
			vkMapMemory(device, sbo_mem, 0, vertices.size() * sizeof(Vertex), 0, &data);
			memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
			vkUnmapMemory(device, sbo_mem);

			creatBuffer(vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vbo, vbo_mem);
			copyBuffer(sbo, vbo, vertices.size() * sizeof(Vertex));
			vkDestroyBuffer(device, sbo, nullptr);
			vkFreeMemory(device, sbo_mem, nullptr);
			
		}
		{
			//VkBufferCreateInfo creat_info{};
			//creat_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			//creat_info.size = vertices.size() * sizeof(Vertex);
			//creat_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			//creat_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			//if (vkCreateBuffer(device, &creat_info, nullptr, &vbo) != VK_SUCCESS)
			//	throw std::runtime_error("failed to create vbo");
			//VkMemoryRequirements memRequirements;
			//vkGetBufferMemoryRequirements(device, vbo, &memRequirements);
			//VkMemoryAllocateInfo alloc_i{};
			//alloc_i.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			//alloc_i.allocationSize = memRequirements.size;
			//alloc_i.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			//if (vkAllocateMemory(device, &alloc_i, nullptr, &vbo_mem) != VK_SUCCESS) {
			//	throw std::runtime_error("failed to allocate vertex buffer memory!");
			//}
			//vkBindBufferMemory(device, vbo, vbo_mem, 0);
			VkBuffer sbo;
			VkDeviceMemory sbo_mem;
			creatBuffer(indices.size() * sizeof(uint16_t), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sbo, sbo_mem);

			void* data;
			vkMapMemory(device, sbo_mem, 0, indices.size() * sizeof(uint16_t), 0, &data);
			memcpy(data, indices.data(), indices.size() * sizeof(uint16_t));
			vkUnmapMemory(device, sbo_mem);

			creatBuffer(indices.size() * sizeof(uint16_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ibo, ibo_mem);
			copyBuffer(sbo, ibo, indices.size() * sizeof(uint16_t));
			vkDestroyBuffer(device, sbo, nullptr);
			vkFreeMemory(device, sbo_mem, nullptr);

		}
		//{
		//	int buff_s = spheres.size() * sizeof(Sphere);
		//	VkBuffer sbo;
		//	VkDeviceMemory sbo_mem;
		//	creatBuffer(buff_s, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sbo, sbo_mem);

		//	void* data;
		//	vkMapMemory(device, sbo_mem, 0, buff_s, 0, &data);
		//	memcpy(data, spheres.data(),buff_s);
		//	vkUnmapMemory(device, sbo_mem);
		//	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		//		creatBuffer(buff_s, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ssbos[i], ssbos_mem[i]);
		//		copyBuffer(sbo, ssbos[i], buff_s);
		//	}


		//}
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(device);
	}
	void cleanUp() {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyBuffer(device, vbo, nullptr);
		vkFreeMemory(device, vbo_mem, nullptr);
		vkDestroyBuffer(device, ibo, nullptr);
		vkFreeMemory(device, ibo_mem, nullptr);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device, spheres_ssbos[i], nullptr);
			vkFreeMemory(device, spheres_ssbos_mem[i], nullptr);
			vkDestroyBuffer(device, ubos[i], nullptr);
			vkFreeMemory(device, ubos_mem[i], nullptr);
		}
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
		vkDestroyCommandPool(device, commandPool, nullptr);
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugmessenger, nullptr);
		}
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
		system("PAUSE");
	}
};