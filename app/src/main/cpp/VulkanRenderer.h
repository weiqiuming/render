#ifndef ANDROIDGLINVESTIGATIONS_VULKANRENDERER_H
#define ANDROIDGLINVESTIGATIONS_VULKANRENDERER_H

#include "Renderer.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

class VulkanRenderer : public Renderer {
public:
    //GLESRenderer(android_app *pApp);
    using Renderer::Renderer;
    ~VulkanRenderer() noexcept;
    void initRenderer() override ;
    void render() override;

private:
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkCommandPool    cmdPool{VK_NULL_HANDLE};
    VkRenderPass renderPass;
    VkQueue queue;

    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;

    std::vector<VkImage> swapImages;
    std::vector<VkImageView> swapImageViews;
    std::vector<VkFramebuffer> frameBuffers;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkFence> fences;
};

#endif //ANDROIDGLINVESTIGATIONS_VULKANRENDERER_H