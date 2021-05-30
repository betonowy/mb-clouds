//
// Created by pekopeko on 20.05.2021.
//

#include <iostream>
#include <glm/glm.hpp>
#include <imgui.h>
#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_vulkan.h>
#include <util/misc.h>
#include <util/file/binaryFile.h>
#include <shaders/shader.h>
#include "init.h"

mb::init::init() {
    if (_currentInitInstance) misc::exception("Cannot invoke more than one init instance");

    _currentInitInstance = this;

    if (!_initEverything()) misc::exception("Engine initialization failed");
}

mb::init::~init() {
    _quitEverything();

    _currentInitInstance = nullptr;
}

bool mb::init::_initEverything() {
    if (!_initSdl()) { return false; }

    if (!_getAllVkExtensions()) { return false; }

    if (!_getAllVkLayers()) { return false; }

    if (!_initVkInstance()) { return false; }

    if (!_initDebugCallback()) { return false; }

    if (!_initPhysicalDevice()) { return false; }

    if (!_initLogicalDevice()) { return false; }

    _initDeviceQueue();

    if (!_initSurface()) { return false; }

    if (!_initSwapchain()) { return false; }

    if (!_initSwapchainImageHandles()) { return false; }

    if (!_initSwapchainImageViews()) { return false; }

    if (!_initRenderPass()) { return false; }

    if (!_initGraphicsPipeline()) { return false; }

    if (!_initFramebuffer()) { return false; }

    if (!_initCommandPool()) { return false; }

    if (!_initCommandBuffers()) { return false; }

    _initSyncObjects();

    if (!_initImGui()) { return false; }

    std::cout << "\nVulkan OK!\n\n";

    return true;
}

bool mb::init::_initSdl() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "Unable to initialize SDL\n";
        return false;
    }

    _mainWindow = SDL_CreateWindow(_vulcanAppWindowName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   _vulcanAppDefaultWindowSizeX,
                                   _vulcanAppDefaultWindowSizeY, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
    if (_mainWindow == nullptr) {
        std::cerr << "Unable to create window\n";
        return false;
    }

    return true;
}

bool mb::init::_getAllVkExtensions() {
    unsigned int extensionCount = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(_mainWindow, &extensionCount, nullptr)) {
        std::cerr << "Unable to query the number of Vulkan instance extensions\n";
        return false;
    }

    std::vector<const char *> _constCharPtrExtensions(extensionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(_mainWindow, &extensionCount, _constCharPtrExtensions.data())) {
        std::cerr << "Unable to query the number of Vulkan instance extensions\n";
        return false;
    }

    std::cout << "found " << extensionCount << " Vulkan instance extensions\n";
    for (const auto &name : _constCharPtrExtensions) {
        std::cout << "    - " << name << "\n";
        _vulcanExtensions.emplace_back(name);
    }

    _vulcanExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    std::cout << "\n";
    return true;
}

bool mb::init::_getAllVkLayers() {
    unsigned layerCount = 0;
    if (VK_SUCCESS != vkEnumerateInstanceLayerProperties(&layerCount, nullptr)) {
        std::cerr << "unable to query vulkan instance layer property count\n";
        return false;
    }

    std::vector<VkLayerProperties> layerNames(layerCount);
    if (VK_SUCCESS != vkEnumerateInstanceLayerProperties(&layerCount, layerNames.data())) {
        std::cerr << "unable to retrieve vulkan instance layer names\n";
        return false;
    }

    std::cout << "found " << layerCount << " instance layers:\n";
    std::vector<const char *> validLayerNames;
    const std::set<std::string> &lookupLayers = _requestedLayers();
    for (const auto &name : layerNames) {
        std::cout << "    - " << name.layerName << "\n";
        auto it = lookupLayers.find(std::string(name.layerName));
        if (it != lookupLayers.end())
            _vulcanLayers.emplace_back(name.layerName);
    }

    std::cout << "\napplying " << _vulcanLayers.size() << " Vulkan instance layers\n";
    for (const auto &layer : _vulcanLayers)
        std::cout << "    - " << layer << "\n";

    return true;
}

bool mb::init::_initVkInstance() {
    std::vector<const char *> layer_names;
    for (const auto &layer : _vulcanLayers)
        layer_names.emplace_back(layer.c_str());

    std::vector<const char *> ext_names;
    for (const auto &ext : _vulcanExtensions)
        ext_names.emplace_back(ext.c_str());

    vkEnumerateInstanceVersion(&_apiVersion);

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = _vulcanAppWindowName;
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = _vulcanAppEngineName;
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instInfo = {};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = nullptr;
    instInfo.flags = 0;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = _vulcanExtensions.size();
    instInfo.ppEnabledExtensionNames = ext_names.data();
    instInfo.enabledLayerCount = _vulcanLayers.size();
    instInfo.ppEnabledLayerNames = layer_names.data();

    switch (vkCreateInstance(&instInfo, nullptr, &_vulkanInstance)) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            std::cerr << "unable to create vulkan instance, cannot find a compatible Vulkan ICD\n";
            return false;
        default:
            std::cerr << "unable to create vulkan instance, other errors\n";
            return false;
    }

    return true;
}

const std::set<std::string> &mb::init::_requestedLayers() {
    static std::set<std::string> layers;
    if (layers.empty()) {
        layers.emplace("VK_LAYER_NV_optimus");
        layers.emplace("VK_LAYER_KHRONOS_validation");
    }
    return layers;
}

const std::set<std::string> &mb::init::_requestedDeviceExtensionNames() {
    static std::set<std::string> layers;
    if (layers.empty()) {
        layers.emplace(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
    return layers;
}

const std::vector<VkImageUsageFlags> &mb::init::_requestedImageUsages() {
    static std::vector<VkImageUsageFlags> usages;
    if (usages.empty()) {
        usages.emplace_back(_vulcanImageUsage);
    }
    return usages;
}

bool mb::init::_initDebugCallback() {
    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = _debugCallback;

    auto createDebugCallback = [&]() {
        auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
                _vulkanInstance, "vkCreateDebugReportCallbackEXT");

        if (func)
            return func(_vulkanInstance, &createInfo, nullptr, &_callbackExt);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    };

    if (createDebugCallback() != VK_SUCCESS) {
        std::cerr << "unable to create debug report callback extension\n";
        return false;
    }

    return true;
}

VkBool32 mb::init::_debugCallback(VkDebugReportFlagsEXT flags, [[maybe_unused]] VkDebugReportObjectTypeEXT objType,
                                  [[maybe_unused]] uint64_t obj, [[maybe_unused]] size_t location,
                                  [[maybe_unused]] int32_t code, const char *layerPrefix,
                                  const char *msg, [[maybe_unused]] void *userData) {
    if (flags == VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_ERROR_BIT_EXT)
        std::cerr << "validation layer: " << layerPrefix << ": " << msg << std::endl;
    else
        std::cout << "validation layer: " << layerPrefix << ": " << msg << std::endl;
    return VK_FALSE;
}

bool mb::init::_initPhysicalDevice() {

    std::cout << "\n";

    unsigned physicalDeviceCount = 0;

    vkEnumeratePhysicalDevices(_vulkanInstance, &physicalDeviceCount, nullptr);

    if (physicalDeviceCount == 0) {
        std::cerr << "No physical devices found\n";
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(_vulkanInstance, &physicalDeviceCount, physicalDevices.data());

    std::cout << "found " << physicalDeviceCount << " physical device(s):\n";
    std::vector<VkPhysicalDeviceProperties> physicalDeviceProperties(physicalDevices.size());
    auto pdpIterator = physicalDeviceProperties.begin();

    {
        int count = 0;
        for (auto &physicalDevice : physicalDevices) {
            vkGetPhysicalDeviceProperties(physicalDevice, &(*pdpIterator));
            std::cout << "    - " << count << " - " << pdpIterator->deviceName << "\n";
            ++pdpIterator;
            ++count;
        }
    }

    unsigned selectionId = 0;
    if (physicalDeviceCount > 1) {
        while (true) {
            std::cout << "\nselect device: ";
            std::cin >> selectionId;
            if (selectionId >= physicalDeviceCount) {
                std::cout << "invalid selection, expected a value between 0 and " << physicalDeviceCount - 1 << "\n";
                continue;
            }
            break;
        }
    }

    std::cout << "\nselected: " << physicalDeviceProperties[selectionId].deviceName << "\n";
    VkPhysicalDevice selectedDevice = physicalDevices[selectionId];

    unsigned familyQueueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(selectedDevice, &familyQueueCount, nullptr);

    if (familyQueueCount == 0) {
        std::cerr << "device has no family of queues associated with it\n";
        return false;
    }

    std::vector<VkQueueFamilyProperties> queueProperties(familyQueueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(selectedDevice, &familyQueueCount, queueProperties.data());

    int queueNodeIndex = -1;
    for (unsigned int i = 0; i < familyQueueCount; i++) {
        if (queueProperties[i].queueCount > 0 && queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueNodeIndex = (int) i;
            break;
        }
    }

    if (queueNodeIndex < 0) {
        std::cerr << "Unable to find a queue command family that accepts graphics commands\n";
        return false;
    }

    _vulkanPhysicalDevice = selectedDevice;
    _queueGraphicsIndex = queueNodeIndex;

    return true;
}

bool mb::init::_initLogicalDevice() {
    std::vector<const char *> layerNames;
    for (const auto &layer : _vulcanLayers)
        layerNames.emplace_back(layer.c_str());

    uint32_t devicePropertyCount = 0;
    if (vkEnumerateDeviceExtensionProperties(_vulkanPhysicalDevice, nullptr,
                                             &devicePropertyCount, nullptr) != VK_SUCCESS) {
        std::cerr << "Unable to acquire device extension property count\n";
        return false;
    }
    std::cout << "\nfound " << devicePropertyCount << " device extensions\n";

    std::vector<VkExtensionProperties> deviceProperties(devicePropertyCount);
    if (vkEnumerateDeviceExtensionProperties(_vulkanPhysicalDevice, nullptr,
                                             &devicePropertyCount, deviceProperties.data()) != VK_SUCCESS) {
        std::cerr << "Unable to acquire device extension property names\n";
        return false;
    }

    std::vector<const char *> devicePropertyNames;
    const std::set<std::string> &requiredExtensionNames = _requestedDeviceExtensionNames();
    for (const auto &extensionProp : deviceProperties) {
        std::cout << "    - " << extensionProp.extensionName << "\n";
        auto it = requiredExtensionNames.find(std::string(extensionProp.extensionName));
        if (it != requiredExtensionNames.end()) {
            devicePropertyNames.emplace_back(extensionProp.extensionName);
        }
    }

    if (requiredExtensionNames.size() != devicePropertyNames.size()) {
        std::cerr << "not all required device extensions are supported!\n";
        return false;
    }

    std::cout << "\n";
    for (const auto &name : devicePropertyNames)
        std::cout << "applying device extension: " << name << "\n";

    VkDeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = _queueGraphicsIndex;
    queueCreateInfo.queueCount = 1;
    std::vector<float> queuePriority = {1.0f};
    queueCreateInfo.pQueuePriorities = queuePriority.data();
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.flags = 0;

    VkDeviceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.ppEnabledLayerNames = layerNames.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
    createInfo.ppEnabledExtensionNames = devicePropertyNames.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(devicePropertyNames.size());
    createInfo.pNext = nullptr;
    createInfo.pEnabledFeatures = nullptr;
    createInfo.flags = 0;

    if (VK_SUCCESS != vkCreateDevice(_vulkanPhysicalDevice, &createInfo, nullptr, &_vulkanLogicalDevice)) {
        std::cerr << "failed to create logical device!\n";
        return false;
    }

    return true;
}

bool mb::init::_initSurface() {
    if (!SDL_Vulkan_CreateSurface(_mainWindow, _vulkanInstance, &_vulkanPresentationSurface)) {
        std::cerr << "Unable to create Vulkan compatible surface using SDL\n";
        return false;
    }

    VkBool32 supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(_vulkanPhysicalDevice, _queueGraphicsIndex,
                                         _vulkanPresentationSurface, &supported);
    if (!supported) {
        std::cerr << "Surface is not supported by physical device!\n";
        return false;
    }

    return true;
}

bool mb::init::_initSwapchain() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_vulkanPhysicalDevice, _vulkanPresentationSurface,
                                                                &surfaceCapabilities)) {
        std::cerr << "unable to acquire surface capabilities\n";
        return false;
    }

    VkPresentModeKHR presentationMode = _vulcanPreferredPresentationMode;
    uint32_t modeCount = 0;
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(_vulkanPhysicalDevice, _vulkanPresentationSurface,
                                                                &modeCount, nullptr)) {
        std::cerr << "unable to query present mode count for physical device\n";
        return false;
    }

    std::vector<VkPresentModeKHR> availableModes(modeCount);
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(_vulkanPhysicalDevice, _vulkanPresentationSurface,
                                                                &modeCount, availableModes.data())) {
        std::cerr << "unable to query the various present modes for physical device\n";
        return false;
    }

    bool foundPreferredMode = false;

    for (auto &mode : availableModes)
        if (mode == presentationMode) {
            foundPreferredMode = true;
            break;
        }

    if (!foundPreferredMode) {
        std::cout << "unable to obtain preferred display mode, fallback to FIFO\n";
        presentationMode = VK_PRESENT_MODE_FIFO_KHR;
    }

    unsigned swapImageCount = surfaceCapabilities.minImageCount + 1;
    if (swapImageCount > surfaceCapabilities.maxImageCount) swapImageCount = surfaceCapabilities.minImageCount;

    _vulkanSwapchainExtent = {(unsigned int) _vulcanAppDefaultWindowSizeX,
                              (unsigned int) _vulcanAppDefaultWindowSizeY};

    if (surfaceCapabilities.currentExtent.width == 0xFFFFFFF) {
        _vulkanSwapchainExtent.width = glm::clamp<unsigned int>(_vulkanSwapchainExtent.width,
                                                                surfaceCapabilities.minImageExtent.width,
                                                                surfaceCapabilities.maxImageExtent.width);
        _vulkanSwapchainExtent.height = glm::clamp<unsigned int>(_vulkanSwapchainExtent.height,
                                                                 surfaceCapabilities.minImageExtent.height,
                                                                 surfaceCapabilities.maxImageExtent.height);
    } else {
        _vulkanSwapchainExtent = surfaceCapabilities.currentExtent;
    }

    const std::vector<VkImageUsageFlags> &desiredUsages = _requestedImageUsages();
    assert(!desiredUsages.empty());

    VkImageUsageFlags usageFlags = desiredUsages[0];

    for (const auto &desiredUsage : desiredUsages) {
        VkImageUsageFlags imageUsage = desiredUsage & surfaceCapabilities.supportedUsageFlags;
        if (imageUsage != desiredUsage) {
            std::cout << "unsupported image usage flag: " << desiredUsage << "\n";
            return false;
        }

        usageFlags = usageFlags | desiredUsage;
    }

    VkSurfaceTransformFlagBitsKHR transform = surfaceCapabilities.currentTransform;
    if (surfaceCapabilities.supportedTransforms & _vulcanPreferredTransform)
        transform = _vulcanPreferredTransform;
    else
        std::cout << "unsupported surface transform: " << _vulcanPreferredTransform;

    unsigned formatCount = 0;
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(_vulkanPhysicalDevice, _vulkanPresentationSurface,
                                                           &formatCount, nullptr)) {
        std::cerr << "unable to query number of supported surface formats";
        return false;
    }

    std::vector<VkSurfaceFormatKHR> foundFormats(formatCount);
    if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(_vulkanPhysicalDevice, _vulkanPresentationSurface,
                                                           &formatCount, foundFormats.data())) {
        std::cerr << "unable to query all supported surface formats\n";
        return false;
    }

    VkSurfaceFormatKHR imageFormat;

    auto findFormat = [&]() {
        if (foundFormats.size() == 1 && foundFormats[0].format == VK_FORMAT_UNDEFINED) {
            imageFormat.format = _vulcanPreferredFormat;
            imageFormat.colorSpace = _vulcanPreferredColorSpace;
            return;
        }

        for (const auto &foundFormatOuter : foundFormats) {
            if (foundFormatOuter.format == _vulcanPreferredFormat) {
                imageFormat.format = foundFormatOuter.format;
                for (const auto &foundFormatInner : foundFormats) {
                    if (foundFormatInner.colorSpace == _vulcanPreferredColorSpace) {
                        imageFormat.colorSpace = foundFormatInner.colorSpace;
                        return;
                    }
                }

                std::cout << "warning: no matching color space found, picking first available one\n!";
                imageFormat.colorSpace = foundFormats[0].colorSpace;
                return;
            }
        }

        std::cout << "warning: no matching color format found, picking first available one\n";
        imageFormat = foundFormats[0];
    };

    findFormat();

    _vulkanSwapImagesFormat = imageFormat.format;

    VkSwapchainKHR oldSwapchain = _vulkanSwapchain;

    VkSwapchainCreateInfoKHR swapInfo;
    swapInfo.pNext = nullptr;
    swapInfo.flags = 0;
    swapInfo.surface = _vulkanPresentationSurface;
    swapInfo.minImageCount = swapImageCount;
    swapInfo.imageFormat = imageFormat.format;
    swapInfo.imageColorSpace = imageFormat.colorSpace;
    swapInfo.imageExtent = _vulkanSwapchainExtent;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageUsage = usageFlags;
    swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapInfo.queueFamilyIndexCount = 0;
    swapInfo.pQueueFamilyIndices = nullptr;
    swapInfo.preTransform = transform;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapInfo.presentMode = presentationMode;
    swapInfo.clipped = true;
    swapInfo.oldSwapchain = nullptr;
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    if (oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(_vulkanLogicalDevice, oldSwapchain, nullptr);
    }

    if (VK_SUCCESS != vkCreateSwapchainKHR(_vulkanLogicalDevice, &swapInfo, nullptr, &oldSwapchain)) {
        std::cerr << "unable to create swap chain\n";
        return false;
    }

    _vulkanSwapchain = oldSwapchain;
    return true;
}

bool mb::init::_initSwapchainImageHandles() {
    unsigned int imageCount = 0;
    if (VK_SUCCESS != vkGetSwapchainImagesKHR(_vulkanLogicalDevice, _vulkanSwapchain,
                                              &imageCount, nullptr)) {
        std::cerr << "unable to get number of images in swap chain\n";
        return false;
    }

    _vulkanChainImages.resize(imageCount);
    if (VK_SUCCESS != vkGetSwapchainImagesKHR(_vulkanLogicalDevice, _vulkanSwapchain,
                                              &imageCount, _vulkanChainImages.data())) {
        std::cerr << "unable to get image handles from swap chain\n";
        return false;
    }

    return true;
}

bool mb::init::_initSwapchainImageViews() {
    _vulkanChainViews.resize(_vulkanChainImages.size());

    for (size_t i = 0; i < _vulkanChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = _vulkanChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = _vulkanSwapImagesFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (VK_SUCCESS != vkCreateImageView(_vulkanLogicalDevice, &createInfo, nullptr, &_vulkanChainViews[i])) {
            std::cerr << "failed to create image views!\n";
            return false;
        }
    }

    return true;
}

void mb::init::_initDeviceQueue() {
    vkGetDeviceQueue(_vulkanLogicalDevice, _queueGraphicsIndex, 0, &_vulkanGraphicsQueue);
}

bool mb::init::_initImGui() {
    int w, h;
    SDL_GetWindowSize(_mainWindow, &w, &h);

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForVulkan(_mainWindow);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = _vulkanInstance;
    init_info.PhysicalDevice = _vulkanPhysicalDevice;
    init_info.Device = _vulkanLogicalDevice;
    init_info.QueueFamily = _queueGraphicsIndex;
    init_info.Queue = _vulkanGraphicsQueue;

    return true;
}

void mb::init::_quitImGui() {

}

void mb::init::_quitEverything() {
    _quitImGui();

    for (size_t i = 0; i < _vulcanMaxFramesInFlight; i++) {
        vkDestroySemaphore(_vulkanLogicalDevice, _vulkanRenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(_vulkanLogicalDevice, _vulkanImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(_vulkanLogicalDevice, _vulkanInFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(_vulkanLogicalDevice, _vulkanCommandPool, nullptr);

    for (auto &framebuffer : _vulkanSwapchainFramebuffers) {
        vkDestroyFramebuffer(_vulkanLogicalDevice, framebuffer, nullptr);
    }

    vkDestroyPipeline(_vulkanLogicalDevice, _vulkanGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(_vulkanLogicalDevice, _vulkanPipelineLayout, nullptr);
    vkDestroyRenderPass(_vulkanLogicalDevice, _vulkanRenderPass, nullptr);

    for (auto &imageView : _vulkanChainViews)
        vkDestroyImageView(_vulkanLogicalDevice, imageView, nullptr);

    vkDestroySwapchainKHR(_vulkanLogicalDevice, _vulkanSwapchain, nullptr);
    vkDestroyDevice(_vulkanLogicalDevice, nullptr);
    // Destroy debug report callback
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(_vulkanInstance,
                                                                            "vkDestroyDebugReportCallbackEXT");
    if (func) func(_vulkanInstance, _callbackExt, nullptr);
    vkDestroySurfaceKHR(_vulkanInstance, _vulkanPresentationSurface, nullptr);
    vkDestroyInstance(_vulkanInstance, nullptr);
    SDL_Quit();
}

bool mb::init::_initGraphicsPipeline() {
    mb::shader vertCode("../res/shaders/shader.vert.spv");
    mb::shader fragCode("../res/shaders/shader.frag.spv");

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};

    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertCode.GetShaderModule();
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};

    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragCode.GetShaderModule();
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};

    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_vulkanSwapchainExtent.width);
    viewport.height = static_cast<float>(_vulkanSwapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};

    scissor.offset = {0, 0};
    scissor.extent = _vulkanSwapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};

    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};

    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};

    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};

    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

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

    VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (VK_SUCCESS != vkCreatePipelineLayout(_vulkanLogicalDevice, &pipelineLayoutInfo,
                                             nullptr, &_vulkanPipelineLayout)) {
        misc::exception("failed to create pipeline layout");
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
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = _vulkanPipelineLayout;
    pipelineInfo.renderPass = _vulkanRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (VK_SUCCESS != vkCreateGraphicsPipelines(_vulkanLogicalDevice, VK_NULL_HANDLE, 1,
                                                &pipelineInfo, nullptr, &_vulkanGraphicsPipeline)) {
        misc::exception("failed to create graphics pipeline");
    }

    return true;
}

mb::init *mb::init::GetInstance() { return _currentInitInstance; }

VkPhysicalDevice &mb::init::GetPhysicalDevice() { return _vulkanPhysicalDevice; }

VkDevice &mb::init::GetLogicalDevice() { return _vulkanLogicalDevice; }

bool mb::init::_initRenderPass() {
    VkAttachmentDescription colorAttachment{};

    colorAttachment.format = _vulkanSwapImagesFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentReference{};

    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};

    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    VkSubpassDependency dependency{};

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};

    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (VK_SUCCESS != vkCreateRenderPass(_vulkanLogicalDevice, &renderPassInfo, nullptr, &_vulkanRenderPass)) {
        misc::exception("failed to create render pass!");
    }

    return true;
}

bool mb::init::_initFramebuffer() {
    _vulkanSwapchainFramebuffers.resize(_vulkanChainViews.size());

    for (size_t i = 0; i < _vulkanChainViews.size(); i++) {
        VkImageView attachments[] = {
                _vulkanChainViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};

        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _vulkanRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = _vulkanSwapchainExtent.width;
        framebufferInfo.height = _vulkanSwapchainExtent.height;
        framebufferInfo.layers = 1;

        if (VK_SUCCESS != vkCreateFramebuffer(_vulkanLogicalDevice, &framebufferInfo,
                                              nullptr, &_vulkanSwapchainFramebuffers[i])) {
            misc::exception("failed to create framebuffer");
        }
    }

    return true;
}

bool mb::init::_initCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};

    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = _queueGraphicsIndex;
    poolInfo.flags = 0;

    if (VK_SUCCESS != vkCreateCommandPool(_vulkanLogicalDevice, &poolInfo, nullptr, &_vulkanCommandPool)) {
        misc::exception("failed to allocate command buffers");
    }

    return true;
}

bool mb::init::_initCommandBuffers() {
    _vulkanCommandBuffers.resize(_vulkanSwapchainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo{};

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _vulkanCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = _vulkanCommandBuffers.size();

    if (VK_SUCCESS != vkAllocateCommandBuffers(_vulkanLogicalDevice, &allocInfo, _vulkanCommandBuffers.data())) {
        misc::exception("failed to allocate command buffers");
    }

    for (size_t i = 0; i < _vulkanCommandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};

        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if (VK_SUCCESS != vkBeginCommandBuffer(_vulkanCommandBuffers[i], &beginInfo)) {
            misc::exception("failed to begin recording command buffer");
        }

        VkRenderPassBeginInfo renderPassInfo{};

        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _vulkanRenderPass;
        renderPassInfo.framebuffer = _vulkanSwapchainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _vulkanSwapchainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(_vulkanCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(_vulkanCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _vulkanGraphicsPipeline);

        vkCmdDraw(_vulkanCommandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(_vulkanCommandBuffers[i]);

        if (VK_SUCCESS != vkEndCommandBuffer(_vulkanCommandBuffers[i])) {
            misc::exception("failed to record command buffer");
        }
    }

    return true;
}

void mb::init::_initSyncObjects() {
    _vulkanImageAvailableSemaphores.resize(_vulcanMaxFramesInFlight);
    _vulkanRenderFinishedSemaphores.resize(_vulcanMaxFramesInFlight);
    _vulkanInFlightFences.resize(_vulcanMaxFramesInFlight);
    _vulkanImagesInFlight.resize(_vulkanChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};

    VkFenceCreateInfo fenceInfo{};

    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < _vulcanMaxFramesInFlight; i++) {
        if (VK_SUCCESS != vkCreateSemaphore(_vulkanLogicalDevice, &semaphoreInfo,
                                            nullptr, &_vulkanImageAvailableSemaphores[i]) ||
            VK_SUCCESS != vkCreateSemaphore(_vulkanLogicalDevice, &semaphoreInfo,
                                            nullptr, &_vulkanRenderFinishedSemaphores[i]) ||
            VK_SUCCESS != vkCreateFence(_vulkanLogicalDevice, &fenceInfo,
                                        nullptr, &_vulkanInFlightFences[i])) {
            misc::exception("failed to create sync objects");
        }
    }
}

void mb::init::_drawFrame() {
    unsigned int imageIndex;

    vkWaitForFences(_vulkanLogicalDevice, 1, &_vulkanInFlightFences[_vulkanCurrentFrame], VK_TRUE, UINT64_MAX);

    vkAcquireNextImageKHR(_vulkanLogicalDevice, _vulkanSwapchain, UINT64_MAX,
                          _vulkanImageAvailableSemaphores[_vulkanCurrentFrame], VK_NULL_HANDLE, &imageIndex);

    if (_vulkanImagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(_vulkanLogicalDevice, 1, &_vulkanImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    _vulkanImagesInFlight[imageIndex] = _vulkanInFlightFences[_vulkanCurrentFrame];

    VkSemaphore waitSemaphores[] = {_vulkanImageAvailableSemaphores[_vulkanCurrentFrame]};

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSemaphore signalSemaphores[] = {_vulkanRenderFinishedSemaphores[_vulkanCurrentFrame]};

    VkSubmitInfo submitInfo{};

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_vulkanCommandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(_vulkanLogicalDevice, 1, &_vulkanInFlightFences[_vulkanCurrentFrame]);

    if (VK_SUCCESS != vkQueueSubmit(_vulkanGraphicsQueue, 1, &submitInfo, _vulkanInFlightFences[_vulkanCurrentFrame])) {
        misc::exception("failed to submit draw command buffer");
    }

    VkSwapchainKHR swapchains[] = {_vulkanSwapchain};

    VkPresentInfoKHR presentInfo{};

    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(_vulkanGraphicsQueue, &presentInfo);

    _vulkanCurrentFrame = (_vulkanCurrentFrame + 1) % _vulcanMaxFramesInFlight;
}

void mb::init::littleLoop() {
    bool run = true;
    while (run) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                run = false;
            }
        }

        _drawFrame();
    }

    vkDeviceWaitIdle(_vulkanLogicalDevice);
}
