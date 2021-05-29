//
// Created by pekopeko on 20.05.2021.
//

#include <iostream>
#include <glm/glm.hpp>
#include "init.h"

mb::init::init() { _initEverything(); }

mb::init::~init() { _quitEverything(); }

bool mb::init::_initEverything() {
    if (!_initSdl()) { return false; }

    if (!_getAllVkExtensions()) { return false; }

    if (!_getAllVkLayers()) { return false; }

    if (!_initVkInstance()) { return false; }

    if (!_initDebugCallback()) { return false; }

    if (!_initPhysicalDevice()) { return false; }

    if (!_initLogicalDevice()) { return false; }

    if (!_initSurface()) { return false; }

    if (!_initSwapchain()) { return false; }

    if (!_initSwapchainImageHandles()) { return false; }

    if (!_initSwapchainImageViews()) { return false; }

    _initDeviceQueue();

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

    VkExtent2D swapImageExtent = {(unsigned int) _vulcanAppDefaultWindowSizeX,
                                  (unsigned int) _vulcanAppDefaultWindowSizeY};

    if (surfaceCapabilities.currentExtent.width == 0xFFFFFFF) {
        swapImageExtent.width = glm::clamp<unsigned int>(swapImageExtent.width,
                                                         surfaceCapabilities.minImageExtent.width,
                                                         surfaceCapabilities.maxImageExtent.width);
        swapImageExtent.height = glm::clamp<unsigned int>(swapImageExtent.height,
                                                          surfaceCapabilities.minImageExtent.height,
                                                          surfaceCapabilities.maxImageExtent.height);
    } else {
        swapImageExtent = surfaceCapabilities.currentExtent;
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
    swapInfo.imageExtent = swapImageExtent;
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

void mb::init::_quitEverything() {
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

void mb::init::littleLoop() {
    bool run = true;
    while (run) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                run = false;
            }
        }
    }
}
