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

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

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
    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

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

    vk::Result res = vk::enumerateInstanceVersion(&_apiVersion);

    vk::ApplicationInfo appInfo{
            .sType = vk::StructureType::eApplicationInfo,
            .pNext = nullptr,
            .pApplicationName = _vulcanAppWindowName,
            .applicationVersion = 1,
            .pEngineName = _vulcanAppEngineName,
            .engineVersion = 1,
            .apiVersion = VK_API_VERSION_1_2
    };

    vk::InstanceCreateInfo instInfo{
            .sType = vk::StructureType::eImageCreateInfo,
            .pNext = nullptr,
            .flags = {},
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(_vulcanLayers.size()),
            .ppEnabledLayerNames = layer_names.data(),
            .enabledExtensionCount = static_cast<uint32_t>(_vulcanExtensions.size()),
            .ppEnabledExtensionNames = ext_names.data()
    };

    switch (vk::createInstance(&instInfo, nullptr, &_vulkanInstance)) {
        case vk::Result::eSuccess:
            break;
        case vk::Result::eErrorIncompatibleDriver:
            std::cerr << "unable to create vulkan instance, cannot find a compatible Vulkan ICD\n";
            return false;
        default:
            std::cerr << "unable to create vulkan instance, other errors\n";
            return false;
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(_vulkanInstance);

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

const std::vector<vk::ImageUsageFlags> &mb::init::_requestedImageUsages() {
    static std::vector<vk::ImageUsageFlags> usages;
    if (usages.empty()) {
        usages.emplace_back(_vulcanImageUsage);
    }
    return usages;
}

bool mb::init::_initDebugCallback() {
    vk::DebugReportCallbackCreateInfoEXT createInfo{
            .sType = vk::StructureType::eDebugReportCallbackCreateInfoEXT,
            .flags = vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning,
            .pfnCallback = _debugCallback
    };

    auto createDebugCallback = [&]() -> vk::Result {
        auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
                _vulkanInstance, "vkCreateDebugReportCallbackEXT");

        if (func)
            return vk::Result(
                    func(_vulkanInstance, reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT *>(&createInfo),
                         nullptr, reinterpret_cast<VkDebugReportCallbackEXT_T **>(&_callbackExt)));
        else
            return vk::Result::eErrorExtensionNotPresent;
    };

    if (createDebugCallback() != vk::Result::eSuccess) {
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

    vk::Result res = _vulkanInstance.enumeratePhysicalDevices(&physicalDeviceCount, nullptr);

    if (physicalDeviceCount == 0) {
        std::cerr << "No physical devices found\n";
        return false;
    }

    std::vector<vk::PhysicalDevice> physicalDevices(physicalDeviceCount);
    res = _vulkanInstance.enumeratePhysicalDevices(&physicalDeviceCount, physicalDevices.data());

    std::cout << "found " << physicalDeviceCount << " physical device(s):\n";
    std::vector<vk::PhysicalDeviceProperties> physicalDeviceProperties(physicalDevices.size());
    auto pdpIterator = physicalDeviceProperties.begin();

    {
        int count = 0;
        for (auto &physicalDevice : physicalDevices) {
            physicalDevice.getProperties(&(*pdpIterator));
//            vkGetPhysicalDeviceProperties(physicalDevice, &(*pdpIterator));
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
    vk::PhysicalDevice selectedDevice = physicalDevices[selectionId];

    unsigned familyQueueCount = 0;
    selectedDevice.getQueueFamilyProperties(&familyQueueCount, nullptr);

    if (familyQueueCount == 0) {
        std::cerr << "device has no family of queues associated with it\n";
        return false;
    }

    std::vector<vk::QueueFamilyProperties> queueProperties(familyQueueCount);
    selectedDevice.getQueueFamilyProperties(&familyQueueCount, queueProperties.data());

    int queueNodeIndex = -1;
    for (unsigned int i = 0; i < familyQueueCount; i++) {
        if (queueProperties[i].queueCount > 0 && queueProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
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

    if (vk::Result::eSuccess != _vulkanPhysicalDevice.enumerateDeviceExtensionProperties(nullptr,
                                                                                         &devicePropertyCount,
                                                                                         nullptr)) {
        std::cerr << "Unable to acquire device extension property count\n";
        return false;
    }
    std::cout << "\nfound " << devicePropertyCount << " device extensions\n";

    std::vector<vk::ExtensionProperties> deviceProperties(devicePropertyCount);
    if (vk::Result::eSuccess != _vulkanPhysicalDevice.enumerateDeviceExtensionProperties(nullptr,
                                                                                         &devicePropertyCount,
                                                                                         deviceProperties.data())) {
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

    std::vector<float> queuePriority = {1.0f};
    vk::DeviceQueueCreateInfo queueCreateInfo{
            .sType = vk::StructureType::eDeviceQueueCreateInfo,
            .pNext = nullptr,
            .flags = {},
            .queueFamilyIndex = _queueGraphicsIndex,
            .queueCount = 1,
            .pQueuePriorities = queuePriority.data()
    };

    vk::DeviceCreateInfo createInfo{
            .sType = vk::StructureType::eDeviceCreateInfo,
            .pNext = nullptr,
            .flags = {},
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCreateInfo,
            .enabledLayerCount = static_cast<uint32_t>(layerNames.size()),
            .ppEnabledLayerNames = layerNames.data(),
            .enabledExtensionCount = static_cast<uint32_t>(devicePropertyNames.size()),
            .ppEnabledExtensionNames = devicePropertyNames.data(),
            .pEnabledFeatures = nullptr
    };

    if (vk::Result::eSuccess != _vulkanPhysicalDevice.createDevice(&createInfo, nullptr, &_vulkanLogicalDevice)) {
        std::cerr << "failed to create logical device!\n";
        return false;
    }

    return true;
}

bool mb::init::_initSurface() {
    if (!SDL_Vulkan_CreateSurface(_mainWindow, _vulkanInstance,
                                  reinterpret_cast<VkSurfaceKHR *>(&_vulkanPresentationSurface))) {
        std::cerr << "Unable to create Vulkan compatible surface using SDL\n";
        return false;
    }

    if (!_vulkanPhysicalDevice.getSurfaceSupportKHR(_queueGraphicsIndex, _vulkanPresentationSurface)) {
        std::cerr << "Surface is not supported by physical device!\n";
        return false;
    }

    return true;
}

bool mb::init::_initSwapchain() {
    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    if (vk::Result::eSuccess != _vulkanPhysicalDevice.getSurfaceCapabilitiesKHR(
            _vulkanPresentationSurface, &surfaceCapabilities)) {
        std::cerr << "unable to acquire surface capabilities\n";
        return false;
    }

    vk::PresentModeKHR presentationMode = _vulcanPreferredPresentationMode;
    uint32_t modeCount = 0;
    if (vk::Result::eSuccess != _vulkanPhysicalDevice.getSurfacePresentModesKHR(
            _vulkanPresentationSurface, &modeCount,
            nullptr)) {
        std::cerr << "unable to query present mode count for physical device\n";
        return false;
    }

    std::vector<vk::PresentModeKHR> availableModes(modeCount);
    if (vk::Result::eSuccess != _vulkanPhysicalDevice.getSurfacePresentModesKHR(
            _vulkanPresentationSurface, &modeCount, availableModes.data())) {
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
        presentationMode = vk::PresentModeKHR::eFifo;
    }

    unsigned swapImageCount = surfaceCapabilities.minImageCount + 1;
    if (swapImageCount > surfaceCapabilities.maxImageCount) swapImageCount = surfaceCapabilities.minImageCount;

    _vulkanSwapchainExtent
            .setWidth(_vulcanAppDefaultWindowSizeX)
            .setHeight(_vulcanAppDefaultWindowSizeY);

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

    const std::vector<vk::ImageUsageFlags> &desiredUsages = _requestedImageUsages();
    assert(!desiredUsages.empty());

    vk::ImageUsageFlags usageFlags = desiredUsages[0];

    for (const auto &desiredUsage : desiredUsages) {
        vk::ImageUsageFlags imageUsage = desiredUsage & surfaceCapabilities.supportedUsageFlags;
        if (imageUsage != desiredUsage) {
            std::cout << "unsupported image usage flag: " << to_string(desiredUsage) << "\n";
            return false;
        }

        usageFlags = usageFlags | desiredUsage;
    }

    vk::SurfaceTransformFlagBitsKHR transform = surfaceCapabilities.currentTransform;
    if (surfaceCapabilities.supportedTransforms & _vulcanPreferredTransform)
        transform = _vulcanPreferredTransform;
    else
        std::cout << "unsupported surface transform: " << to_string(_vulcanPreferredTransform);

    unsigned formatCount = 0;
    if (vk::Result::eSuccess != _vulkanPhysicalDevice.getSurfaceFormatsKHR(
            _vulkanPresentationSurface, &formatCount,
            nullptr)) {
        std::cerr << "unable to query number of supported surface formats";
        return false;
    }

    std::vector<vk::SurfaceFormatKHR> foundFormats(formatCount);
    if (vk::Result::eSuccess != _vulkanPhysicalDevice.getSurfaceFormatsKHR(
            _vulkanPresentationSurface,
            &formatCount, foundFormats.data())) {
        std::cerr << "unable to query all supported surface formats\n";
        return false;
    }

    vk::SurfaceFormatKHR imageFormat;

    auto findFormat = [&]() {
        if (foundFormats.size() == 1 && foundFormats[0].format == vk::Format::eUndefined) {
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

    vk::SwapchainKHR oldSwapchain = nullptr;
    vk::SwapchainCreateInfoKHR swapInfo{
            .sType = vk::StructureType::eSwapchainCreateInfoKHR,
            .pNext = nullptr,
            .flags = {},
            .surface = _vulkanPresentationSurface,
            .minImageCount = swapImageCount,
            .imageFormat = imageFormat.format,
            .imageColorSpace = imageFormat.colorSpace,
            .imageExtent = _vulkanSwapchainExtent,
            .imageArrayLayers = 1,
            .imageUsage = usageFlags,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = transform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = presentationMode,
            .clipped = true,
            .oldSwapchain = nullptr,
    };

    if (oldSwapchain) {
        vk::Result res = _vulkanLogicalDevice.getSwapchainStatusKHR(oldSwapchain);
        if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR) {
            _vulkanLogicalDevice.destroySwapchainKHR(oldSwapchain, nullptr);
        }
    }

    if (vk::Result::eSuccess != _vulkanLogicalDevice.createSwapchainKHR(&swapInfo, nullptr, &oldSwapchain)) {
        std::cerr << "unable to create swap chain\n";
        return false;
    }

    _vulkanSwapchain = oldSwapchain;
    return true;
}

bool mb::init::_initSwapchainImageHandles() {
    unsigned int imageCount = 0;
    if (vk::Result::eSuccess != _vulkanLogicalDevice.getSwapchainImagesKHR(
            _vulkanSwapchain, &imageCount, nullptr)) {
        std::cerr << "unable to get number of images in swap chain\n";
        return false;
    }

    _vulkanChainImages.resize(imageCount);
    if (vk::Result::eSuccess != _vulkanLogicalDevice.getSwapchainImagesKHR(
            _vulkanSwapchain, &imageCount, _vulkanChainImages.data())) {
        std::cerr << "unable to get image handles from swap chain\n";
        return false;
    }

    return true;
}

bool mb::init::_initSwapchainImageViews() {
    _vulkanChainViews.resize(_vulkanChainImages.size());

    for (size_t i = 0; i < _vulkanChainImages.size(); i++) {
        vk::ImageViewCreateInfo createInfo{
                .sType = vk::StructureType::eImageViewCreateInfo,
                .image = _vulkanChainImages[i],
                .viewType = vk::ImageViewType::e2D,
                .format = _vulkanSwapImagesFormat,
                .components = {
                        .r = vk::ComponentSwizzle::eIdentity,
                        .g = vk::ComponentSwizzle::eIdentity,
                        .b = vk::ComponentSwizzle::eIdentity,
                        .a = vk::ComponentSwizzle::eIdentity
                },
                .subresourceRange = {
                        .aspectMask = vk::ImageAspectFlagBits::eColor,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                }
        };

        if (vk::Result::eSuccess != _vulkanLogicalDevice.createImageView(&createInfo, nullptr, &_vulkanChainViews[i])) {
            std::cerr << "failed to create image views!\n";
            return false;
        }
    }

    return true;
}

void mb::init::_initDeviceQueue() {
    _vulkanLogicalDevice.getQueue(_queueGraphicsIndex, 0, &_vulkanGraphicsQueue);
}

bool mb::init::_initImGui() {
    int w, h;
    SDL_GetWindowSize(_mainWindow, &w, &h);

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForVulkan(_mainWindow);
//    ImGui_ImplVulkan_InitInfo init_info = {};
//    init_info.Instance = _vulkanInstance;
//    init_info.PhysicalDevice = _vulkanPhysicalDevice;
//    init_info.Device = _vulkanLogicalDevice;
//    init_info.QueueFamily = _queueGraphicsIndex;
//    init_info.Queue = _vulkanGraphicsQueue;

    return true;
}

void mb::init::_quitImGui() {

}

void mb::init::_quitEverything() {
    _quitImGui();

    _cleanupSwapChain();

    for (size_t i = 0; i < _vulcanMaxFramesInFlight; i++) {
        _vulkanLogicalDevice.destroySemaphore(_vulkanRenderFinishedSemaphores[i], nullptr);
        _vulkanLogicalDevice.destroySemaphore(_vulkanImageAvailableSemaphores[i], nullptr);
        _vulkanLogicalDevice.destroyFence(_vulkanInFlightFences[i], nullptr);
    }

    _vulkanLogicalDevice.destroyCommandPool(_vulkanCommandPool, nullptr);

    vkDestroyDevice(_vulkanLogicalDevice, nullptr);
    // Destroy debug report callback
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(_vulkanInstance,
                                                                            "vkDestroyDebugReportCallbackEXT");
    if (func) func(_vulkanInstance, _callbackExt, nullptr);
    vkDestroySurfaceKHR(_vulkanInstance, _vulkanPresentationSurface, nullptr);
    vkDestroyInstance(_vulkanInstance, nullptr);
    SDL_Quit();
}

void mb::init::_cleanupSwapChain() {
    for (auto &_vulkanSwapchainFramebuffer : _vulkanSwapchainFramebuffers) {
        vkDestroyFramebuffer(_vulkanLogicalDevice, _vulkanSwapchainFramebuffer, nullptr);
    }

    _vulkanLogicalDevice.freeCommandBuffers(_vulkanCommandPool,
                                            _vulkanCommandBuffers.size(),
                                            _vulkanCommandBuffers.data());

    _vulkanLogicalDevice.destroyPipeline(_vulkanGraphicsPipeline, nullptr);
    _vulkanLogicalDevice.destroyPipelineLayout(_vulkanPipelineLayout, nullptr);
    _vulkanLogicalDevice.destroyRenderPass(_vulkanRenderPass, nullptr);

    for (auto &_vulkanChainView : _vulkanChainViews) {
        _vulkanLogicalDevice.destroyImageView(_vulkanChainView, nullptr);
    }

    _vulkanLogicalDevice.destroySwapchainKHR(_vulkanSwapchain, nullptr);
}

void mb::init::_recreateSwapChain() {
    uint32_t windowFlags = SDL_GetWindowFlags(_mainWindow);

    while (windowFlags & SDL_WINDOW_MINIMIZED) {
        windowFlags = SDL_GetWindowFlags(_mainWindow);
        SDL_WaitEvent(nullptr);
    }

    _vulkanLogicalDevice.waitIdle();

    _cleanupSwapChain();

    _initSwapchain();
    _initSwapchainImageHandles();
    _initSwapchainImageViews();
    _initRenderPass();
    _initGraphicsPipeline();
    _initFramebuffer();
    _initCommandBuffers();
}

bool mb::init::_initGraphicsPipeline() {
    mb::shader vertCode("../res/shaders/shader.vert.spv");
    mb::shader fragCode("../res/shaders/shader.frag.spv");

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
            .sType = vk::StructureType::ePipelineShaderStageCreateInfo,
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vertCode.GetShaderModule(),
            .pName = "main"
    };

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
            .sType = vk::StructureType::ePipelineShaderStageCreateInfo,
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fragCode.GetShaderModule(),
            .pName = "main"
    };

    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = vk::StructureType::ePipelineVertexInputStateCreateInfo,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo,
            .topology = vk::PrimitiveTopology::eTriangleList,
            .primitiveRestartEnable = VK_FALSE
    };

    vk::Viewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(_vulkanSwapchainExtent.width),
            .height = static_cast<float>(_vulkanSwapchainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
    };

    vk::Rect2D scissor{
            .offset = {0, 0},
            .extent = _vulkanSwapchainExtent
    };

    vk::PipelineViewportStateCreateInfo viewportState{
            .sType = vk::StructureType::ePipelineViewportStateCreateInfo,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer{
            .sType = vk::StructureType::ePipelineRasterizationStateCreateInfo,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = vk::PolygonMode::eFill,
            .cullMode = vk::CullModeFlagBits::eBack,
            .frontFace = vk::FrontFace::eClockwise,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{
            .sType = vk::StructureType::ePipelineMultisampleStateCreateInfo,
            .rasterizationSamples = vk::SampleCountFlagBits::e1,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE

    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = vk::BlendFactor::eOne,
            .dstColorBlendFactor = vk::BlendFactor::eZero,
            .colorBlendOp = vk::BlendOp::eAdd,
            .srcAlphaBlendFactor = vk::BlendFactor::eOne,
            .dstAlphaBlendFactor = vk::BlendFactor::eZero,
            .alphaBlendOp = vk::BlendOp::eAdd,
            .colorWriteMask = vk::ColorComponentFlagBits::eR |
                              vk::ColorComponentFlagBits::eG |
                              vk::ColorComponentFlagBits::eB |
                              vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
            .sType = vk::StructureType::ePipelineColorBlendStateCreateInfo,
            .logicOpEnable = VK_FALSE,
            .logicOp = vk::LogicOp::eCopy,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants{{0.0f, 0.0f, 0.0f, 0.0f}}
    };

    vk::DynamicState dynamicStates[] = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eLineWidth
    };

//    VkPipelineDynamicStateCreateInfo dynamicState{};
//    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//    dynamicState.dynamicStateCount = 2;
//    dynamicState.pDynamicStates = dynamicStates;

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
            .sType = vk::StructureType::ePipelineLayoutCreateInfo,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr
    };

    if (vk::Result::eSuccess !=
        _vulkanLogicalDevice.createPipelineLayout(&pipelineLayoutInfo, nullptr, &_vulkanPipelineLayout)) {
        misc::exception("failed to create pipeline layout");
    }

    vk::GraphicsPipelineCreateInfo pipelineInfo{
            .sType = vk::StructureType::eGraphicsPipelineCreateInfo,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &colorBlending,
            .pDynamicState = nullptr,
            .layout = _vulkanPipelineLayout,
            .renderPass = _vulkanRenderPass,
            .subpass = 0,
            .basePipelineHandle = nullptr,
            .basePipelineIndex = -1
    };

    if (vk::Result::eSuccess !=
        _vulkanLogicalDevice.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &_vulkanGraphicsPipeline)) {
        misc::exception("failed to create graphics pipeline");
    }

    return true;
}

mb::init *mb::init::GetInstance() { return _currentInitInstance; }

vk::PhysicalDevice &mb::init::GetPhysicalDevice() { return _vulkanPhysicalDevice; }

vk::Device &mb::init::GetLogicalDevice() { return _vulkanLogicalDevice; }

bool mb::init::_initRenderPass() {
    vk::AttachmentDescription colorAttachment{
            .format = _vulkanSwapImagesFormat,
            .samples = vk::SampleCountFlagBits::e1,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
            .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
            .initialLayout = vk::ImageLayout::eUndefined,
            .finalLayout = vk::ImageLayout::ePresentSrcKHR
    };

    vk::AttachmentReference colorAttachmentReference{
            .attachment = 0,
            .layout = vk::ImageLayout::eColorAttachmentOptimal
    };

    vk::SubpassDescription subpass{
            .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentReference
    };

    vk::SubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
            .srcAccessMask = {},
            .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
    };

    vk::RenderPassCreateInfo renderPassInfo{
            .sType = vk::StructureType::eRenderPassCreateInfo,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency
    };

    if (vk::Result::eSuccess != _vulkanLogicalDevice.createRenderPass(&renderPassInfo, nullptr, &_vulkanRenderPass)) {
        misc::exception("failed to create render pass!");
    }

    return true;
}

bool mb::init::_initFramebuffer() {
    _vulkanSwapchainFramebuffers.resize(_vulkanChainViews.size());

    for (size_t i = 0; i < _vulkanChainViews.size(); i++) {
        vk::ImageView attachments[] = {
                _vulkanChainViews[i]
        };

        vk::FramebufferCreateInfo framebufferInfo{
                .sType = vk::StructureType::eFramebufferCreateInfo,
                .renderPass = _vulkanRenderPass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = _vulkanSwapchainExtent.width,
                .height = _vulkanSwapchainExtent.height,
                .layers = 1
        };

        if (vk::Result::eSuccess != _vulkanLogicalDevice.createFramebuffer(&framebufferInfo,
                                                                           nullptr, &_vulkanSwapchainFramebuffers[i])) {
            misc::exception("failed to create framebuffer");
        }
    }

    return true;
}

bool mb::init::_initCommandPool() {
    vk::CommandPoolCreateInfo poolInfo{
            .sType = vk::StructureType::eCommandPoolCreateInfo,
            .flags = {},
            .queueFamilyIndex = _queueGraphicsIndex
    };

    if (vk::Result::eSuccess != _vulkanLogicalDevice.createCommandPool(&poolInfo, nullptr, &_vulkanCommandPool)) {
        misc::exception("failed to allocate command buffers");
    }

    return true;
}

bool mb::init::_initCommandBuffers() {
    _vulkanCommandBuffers.resize(_vulkanSwapchainFramebuffers.size());

    vk::CommandBufferAllocateInfo allocInfo{
            .sType = vk::StructureType::eCommandBufferAllocateInfo,
            .commandPool = _vulkanCommandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = static_cast<uint32_t>(_vulkanCommandBuffers.size())
    };

    if (vk::Result::eSuccess != _vulkanLogicalDevice.allocateCommandBuffers(&allocInfo, _vulkanCommandBuffers.data())) {
        misc::exception("failed to allocate command buffers");
    }

    for (size_t i = 0; i < _vulkanCommandBuffers.size(); i++) {
        vk::CommandBufferBeginInfo beginInfo{
                .sType = vk::StructureType::eCommandBufferBeginInfo,
                .flags = {},
                .pInheritanceInfo = nullptr
        };

        if (vk::Result::eSuccess != _vulkanCommandBuffers[i].begin(&beginInfo)) {
            misc::exception("failed to begin recording command buffer");
        }

        vk::ClearValue clearColor{};
        clearColor.color.setFloat32({0, 0, 0, 0});

        vk::RenderPassBeginInfo renderPassInfo{
                .sType = vk::StructureType::eRenderPassBeginInfo,
                .renderPass = _vulkanRenderPass,
                .framebuffer = _vulkanSwapchainFramebuffers[i],
                .renderArea{
                        .offset{
                                .x = 0,
                                .y = 0
                        },
                        .extent = _vulkanSwapchainExtent
                },
                .clearValueCount = 1,
                .pClearValues = &clearColor
        };

        _vulkanCommandBuffers[i].beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

        _vulkanCommandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, _vulkanGraphicsPipeline);

        _vulkanCommandBuffers[i].draw(3, 1, 0, 0);

        _vulkanCommandBuffers[i].endRenderPass();

        _vulkanCommandBuffers[i].end();
    }

    return true;
}

void mb::init::_initSyncObjects() {
    _vulkanImageAvailableSemaphores.resize(_vulcanMaxFramesInFlight);
    _vulkanRenderFinishedSemaphores.resize(_vulcanMaxFramesInFlight);
    _vulkanInFlightFences.resize(_vulcanMaxFramesInFlight);
    _vulkanImagesInFlight.resize(_vulkanChainImages.size(), VK_NULL_HANDLE);

    vk::SemaphoreCreateInfo semaphoreInfo{
            .sType = vk::StructureType::eSemaphoreCreateInfo
    };

    vk::FenceCreateInfo fenceInfo{
            .sType = vk::StructureType::eFenceCreateInfo,
            .flags = vk::FenceCreateFlagBits::eSignaled
    };

    for (size_t i = 0; i < _vulcanMaxFramesInFlight; i++) {
        if (vk::Result::eSuccess !=
            _vulkanLogicalDevice.createSemaphore(&semaphoreInfo, nullptr, &_vulkanImageAvailableSemaphores[i]) ||
            vk::Result::eSuccess !=
            _vulkanLogicalDevice.createSemaphore(&semaphoreInfo, nullptr, &_vulkanRenderFinishedSemaphores[i]) ||
            vk::Result::eSuccess !=
            _vulkanLogicalDevice.createFence(&fenceInfo, nullptr, &_vulkanInFlightFences[i])) {
            misc::exception("failed to create sync objects");
        }
    }
}

void mb::init::_drawFrame() {
    unsigned int imageIndex;
    vk::Result res = _vulkanLogicalDevice.waitForFences(1, &_vulkanInFlightFences[_vulkanCurrentFrame],
                                                        VK_TRUE, UINT64_MAX);

    res = _vulkanLogicalDevice.acquireNextImageKHR(_vulkanSwapchain, UINT64_MAX,
                                                   _vulkanImageAvailableSemaphores[_vulkanCurrentFrame],
                                                   VK_NULL_HANDLE, &imageIndex);

    if (res == vk::Result::eErrorOutOfDateKHR) {
        _recreateSwapChain();
        return;
    } else if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR) {
        misc::exception("failed to acquire swap chain image!");
    }

    if (_vulkanImagesInFlight[imageIndex]) {
        res = _vulkanLogicalDevice.waitForFences(1, &_vulkanImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    _vulkanImagesInFlight[imageIndex] = _vulkanInFlightFences[_vulkanCurrentFrame];

    vk::Semaphore waitSemaphores[] = {_vulkanImageAvailableSemaphores[_vulkanCurrentFrame]};

    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::Semaphore signalSemaphores[] = {_vulkanRenderFinishedSemaphores[_vulkanCurrentFrame]};

    vk::SubmitInfo submitInfo{
            .sType = vk::StructureType::eSubmitInfo,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &_vulkanCommandBuffers[imageIndex],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores
    };

    vk::Result d = _vulkanLogicalDevice.resetFences(1, &_vulkanInFlightFences[_vulkanCurrentFrame]);

    if (vk::Result::eSuccess !=
        _vulkanGraphicsQueue.submit(1, &submitInfo, _vulkanInFlightFences[_vulkanCurrentFrame])) {
        misc::exception("failed to submit draw command buffer");
    }

    vk::SwapchainKHR swapchains[] = {_vulkanSwapchain};

    vk::PresentInfoKHR presentInfo{
            .sType = vk::StructureType::ePresentInfoKHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapchains,
            .pImageIndices = &imageIndex,
            .pResults = nullptr
    };

    res = _vulkanGraphicsQueue.presentKHR(&presentInfo);

    _vulkanCurrentFrame = (_vulkanCurrentFrame + 1) % _vulcanMaxFramesInFlight;
}

void mb::init::littleLoop() {
    bool run = true;

    static auto t1 = SDL_GetTicks();
    static auto t2 = t1;
    int frameCount = 0;

    while (run) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                run = false;
            }
        }

        _drawFrame();

        t2 = SDL_GetTicks();
        frameCount++;

        if (t2 - t1 >= 1000) {
            int fps = int(round(float(frameCount) / float(t2 - t1) * 1000));

            std::stringstream ss;
            ss << _vulcanAppWindowName << " FPS: " << fps;

            SDL_SetWindowTitle(_mainWindow, ss.str().c_str());

            t1 = t2;
            frameCount = 0;
        }
    }

    _vulkanLogicalDevice.waitIdle();
}
