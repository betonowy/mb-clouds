//
// Created by pekopeko on 20.05.2021.
//

#ifndef MB_CLOUDS_INIT_H
#define MB_CLOUDS_INIT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <string>

namespace mb {
    class init {
    public:
        init();

        ~init();

        static void littleLoop();

    private:

        bool _initEverything();

        bool _initSdl();

        bool _getAllVkExtensions();

        bool _getAllVkLayers();

        bool _initVkInstance();

        bool _initDebugCallback();

        bool _initPhysicalDevice();

        bool _initLogicalDevice();

        bool _initSurface();

        bool _initSwapchain();

        bool _initSwapchainImageHandles();

        bool _initSwapchainImageViews();

        void _initDeviceQueue();

        void _quitEverything();

        static VKAPI_ATTR VkBool32 VKAPI_CALL
        _debugCallback(VkDebugReportFlagsEXT flags, [[maybe_unused]] VkDebugReportObjectTypeEXT objType,
                       [[maybe_unused]] uint64_t obj, [[maybe_unused]] size_t location,
                       [[maybe_unused]] int32_t code, const char *layerPrefix, const char *msg,
                       [[maybe_unused]] void *userData);

        VkDebugReportCallbackEXT _callbackExt{};

        // constants

        static constexpr const char *_vulcanAppWindowName = "Vulkan application";
        static constexpr const char *_vulcanAppEngineName = "Vulkan engine";
        static constexpr int _vulcanAppDefaultWindowSizeX = 800;
        static constexpr int _vulcanAppDefaultWindowSizeY = 600;
        static constexpr VkPresentModeKHR _vulcanPreferredPresentationMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        static constexpr VkSurfaceTransformFlagBitsKHR _vulcanPreferredTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        static constexpr VkFormat _vulcanPreferredFormat = VK_FORMAT_B8G8R8A8_SRGB;
        static constexpr VkColorSpaceKHR _vulcanPreferredColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        static constexpr VkImageUsageFlags _vulcanImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        static const std::set<std::string> &_requestedLayers();

        static const std::set<std::string> &_requestedDeviceExtensionNames();

        static const std::vector<VkImageUsageFlags> &_requestedImageUsages();

        // variables

        SDL_Window *_mainWindow{};

        std::vector<std::string> _vulcanExtensions;
        std::vector<std::string> _vulcanLayers;
        unsigned _apiVersion{};
        VkInstance _vulkanInstance{};
        unsigned _queueGraphicsIndex = -1;
        VkPhysicalDevice _vulkanPhysicalDevice{};
        VkDevice _vulkanLogicalDevice{};
        VkSurfaceKHR _vulkanPresentationSurface{};
        VkSwapchainKHR _vulkanSwapchain{};
        VkFormat _vulkanSwapImagesFormat{};
        std::vector<VkImage> _vulkanChainImages;
        std::vector<VkImageView> _vulkanChainViews;
        VkQueue _vulkanGraphicsQueue{};
    };
}

#endif //MB_CLOUDS_INIT_H
