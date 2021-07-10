//
// Created by pekopeko on 20.05.2021.
//

#ifndef MB_CLOUDS_INIT_H
#define MB_CLOUDS_INIT_H

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vector>
#include <set>
#include <string>

namespace mb {
    class init {
    public:
        init();

        ~init();

        void littleLoop();

        static init *GetInstance();

        vk::PhysicalDevice &GetPhysicalDevice();

        vk::Device &GetLogicalDevice();

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

        bool _initRenderPass();

        bool _initGraphicsPipeline();

        bool _initFramebuffer();

        bool _initCommandPool();

        bool _initCommandBuffers();

        void _initSyncObjects();

        void _initDeviceQueue();

        bool _initImGui();

        void _quitImGui();

        void _quitEverything();

        void _drawFrame();

        void _cleanupSwapChain();

        void _recreateSwapChain();

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL
        _debugCallback(VkDebugReportFlagsEXT flags, [[maybe_unused]] VkDebugReportObjectTypeEXT objType,
                       [[maybe_unused]] uint64_t obj, [[maybe_unused]] size_t location,
                       [[maybe_unused]] int32_t code, const char *layerPrefix, const char *msg,
                       [[maybe_unused]] void *userData);

        vk::DebugReportCallbackEXT _callbackExt{};

        // constants

        static constexpr const char *_vulcanAppWindowName = "Vulkan application";
        static constexpr const char *_vulcanAppEngineName = "Vulkan engine";
        static constexpr int _vulcanAppDefaultWindowSizeX = 800;
        static constexpr int _vulcanAppDefaultWindowSizeY = 600;
        static constexpr vk::PresentModeKHR _vulcanPreferredPresentationMode = vk::PresentModeKHR::eFifoRelaxed;
        static constexpr vk::SurfaceTransformFlagBitsKHR _vulcanPreferredTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
        static constexpr vk::Format _vulcanPreferredFormat = vk::Format::eB8G8R8A8Srgb;
        static constexpr vk::ColorSpaceKHR _vulcanPreferredColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        static constexpr vk::ImageUsageFlags _vulcanImageUsage = vk::ImageUsageFlagBits::eColorAttachment;
        static constexpr int _vulcanMaxFramesInFlight = 3;

        static const std::set<std::string> &_requestedLayers();

        static const std::set<std::string> &_requestedDeviceExtensionNames();

        static const std::vector<vk::ImageUsageFlags> &_requestedImageUsages();

        // static

        inline static init *_currentInitInstance = nullptr;

        // variables

        SDL_Window *_mainWindow{};
        std::vector<std::string> _vulcanExtensions;
        std::vector<std::string> _vulcanLayers;
        unsigned _apiVersion{};
        vk::Instance _vulkanInstance{};
        unsigned _queueGraphicsIndex = -1;
        vk::PhysicalDevice _vulkanPhysicalDevice{};
        vk::Device _vulkanLogicalDevice{};
        vk::SurfaceKHR _vulkanPresentationSurface{};
        vk::SwapchainKHR _vulkanSwapchain{};
        vk::Extent2D _vulkanSwapchainExtent{};
        vk::PipelineLayout _vulkanPipelineLayout{};
        vk::RenderPass _vulkanRenderPass{};
        vk::Pipeline _vulkanGraphicsPipeline{};
        vk::CommandPool _vulkanCommandPool{};
        std::vector<vk::CommandBuffer> _vulkanCommandBuffers;
        std::vector<vk::Framebuffer> _vulkanSwapchainFramebuffers;
        vk::Format _vulkanSwapImagesFormat{};
        std::vector<vk::Image> _vulkanChainImages;
        std::vector<vk::ImageView> _vulkanChainViews;
        vk::Queue _vulkanGraphicsQueue{};

        vk::DynamicLoader dl;

        size_t _vulkanCurrentFrame = 0;

        // synchronisation

        std::vector<vk::Semaphore> _vulkanImageAvailableSemaphores;
        std::vector<vk::Semaphore> _vulkanRenderFinishedSemaphores;
        std::vector<vk::Fence> _vulkanInFlightFences;
        std::vector<vk::Fence> _vulkanImagesInFlight;
    };
}

#endif //MB_CLOUDS_INIT_H
