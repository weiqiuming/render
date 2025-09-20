#include "VulkanRenderer.h"

#define LOG_TAG "VulkanRenderer"
#define VK_CHECK(x) do{ VkResult r=x; if(r!=VK_SUCCESS){ LOGE("Vk error %d",r);}}while(0)

void VulkanRenderer :: initRenderer() {
    {
        VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO,
                              nullptr,
                              "VulkanClear",
                              0,
                              "VulkanClear",
                              0,
                              VK_API_VERSION_1_0};

        std::vector<const char *> instanceExt = {
                VK_KHR_SURFACE_EXTENSION_NAME,          // 表面支持
                VK_KHR_ANDROID_SURFACE_EXTENSION_NAME  // Android 表面支持
        };
        VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                 nullptr,
                                 0,
                                 &app,
                                 0,
                                 nullptr,
                                 static_cast<uint32_t>(instanceExt.size()),
                                 instanceExt.data()};
        VK_CHECK(vkCreateInstance(&ici, nullptr, &instance));

        uint32_t gpuCount = 0;
        vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
        LOGI("Has %d gpu devices!", gpuCount);

        std::vector<VkPhysicalDevice> gpus(gpuCount);
        VK_CHECK(vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data()));
        LOGI("Using gpu 0!");
        gpu = gpus[0];

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(gpu, &deviceProperties);
        LOGI("deviceName is %s", deviceProperties.deviceName);

        uint32_t qCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &qCount, nullptr);
        LOGI("Has %d DeviceQueueFamilyProperties!", qCount);

        uint32_t queueFamily = 0;
        std::vector<VkQueueFamilyProperties> qprops(qCount);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &qCount, qprops.data());
        for (uint32_t i = 0; i < qCount; ++i) {
            if (qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamily = i;
                LOGI("Using %d DeviceQueueFamilyProperties!", i);
                break;
            }
        }

        std::vector<const char *> deviceExt = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,        // 交换链支持（必须）
        };
        float prio = 1.0f;
        VkDeviceQueueCreateInfo dqci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                     nullptr, 0, queueFamily,
                                     1, &prio};
        VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr,
                               0, 1, &dqci,
                               0, nullptr,
                               static_cast<uint32_t>(deviceExt.size()),
                               deviceExt.data(),
                               nullptr};
        VK_CHECK(vkCreateDevice(gpu, &dci, nullptr, &device));

        vkGetDeviceQueue(device, queueFamily, 0, &queue);

        VkCommandPoolCreateInfo command_pool_info = {};
        command_pool_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.queueFamilyIndex        = queueFamily;
        VK_CHECK(vkCreateCommandPool(device, &command_pool_info, nullptr, &cmdPool));

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
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
        VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
    }

    //create surface & Swapchain
    {
        VkAndroidSurfaceCreateInfoKHR sci{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
                                          nullptr, 0, app_->window};
        VK_CHECK(vkCreateAndroidSurfaceKHR(instance, &sci,
                                           nullptr, &surface));

        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &caps);

        uint32_t fmtCount = 0;
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface,
                                                      &fmtCount, nullptr));
        std::vector<VkSurfaceFormatKHR> fmts(fmtCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface,
                                             &fmtCount,
                                             fmts.data());
        LOGI("Surface supports  %d surface formats",fmtCount);

        VkSwapchainCreateInfoKHR swapChainCreateInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        swapChainCreateInfo.surface = surface;
        swapChainCreateInfo.minImageCount = caps.minImageCount + 1;
        swapChainCreateInfo.imageFormat = fmts[0].format;
        swapChainCreateInfo.imageColorSpace = fmts[0].colorSpace;
        swapChainCreateInfo.imageExtent = caps.currentExtent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;   // 我们要 clear
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.preTransform = caps.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        swapChainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        VK_CHECK(vkCreateSwapchainKHR(device,&swapChainCreateInfo,
                                      nullptr,&swapChain));

        uint32_t imgCount=0;
        vkGetSwapchainImagesKHR(device, swapChain,
                                &imgCount, nullptr);
        LOGI("Has %d swapChainImages",imgCount);
        swapImages.resize(imgCount);
        vkGetSwapchainImagesKHR(device, swapChain,
                                &imgCount,
                                swapImages.data());

        swapImageViews.resize(imgCount);
        for(uint32_t i=0;i<imgCount;++i){
            VkImageViewCreateInfo vci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
            vci.image = swapImages[i];
            vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            vci.format = VK_FORMAT_B8G8R8A8_SRGB;
            vci.components = {VK_COMPONENT_SWIZZLE_IDENTITY};
            vci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT,
                                    0,1,0,1};
            VK_CHECK(vkCreateImageView(device,&vci,
                                       nullptr,&swapImageViews[i]));
        }
    }

    //framebuffer
    {
        frameBuffers.resize(swapImageViews.size());
        for(uint32_t i=0;i<swapImageViews.size();++i){
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;      // 关联的渲染通道
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &swapImageViews[i];        // 使用 Image View
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1;
            VK_CHECK(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBuffers[i]));
        }
    }

    //createCommandBuffers
    {
        commandBuffers.resize(frameBuffers.size());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = cmdPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
        VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[0]));
    }

    //create fence
    {
        fences.resize(commandBuffers.size());
        VkFenceCreateInfo fi{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,nullptr,VK_FENCE_CREATE_SIGNALED_BIT };
        for(auto& f:fences) {
            VK_CHECK(vkCreateFence(device, &fi, nullptr, &f));
        }
    }
    LOGI("Vulkan init done!");
}

void VulkanRenderer::render() {
    uint32_t frameIndex;
    frameIndex = 0;

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
                          VK_NULL_HANDLE,
                          fences[frameIndex], &imageIndex);

    vkWaitForFences(device,1,&fences[frameIndex],VK_TRUE,UINT64_MAX);
    vkResetFences(device,1,&fences[frameIndex]);

    VkCommandBuffer cb = commandBuffers[imageIndex];
    vkResetCommandBuffer(cb,0);

    VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(cb,&bi));

    VkClearColorValue ccv{};
    ccv.float32[0] = (float)imageIndex / (float)swapImages.size();
    ccv.float32[1] = 0.0f;
    ccv.float32[2] = (float)imageIndex / (float)swapImages.size();
    ccv.float32[3] = 1.0f;
    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.layerCount = 1;
    subresourceRange.levelCount = 1;
    vkCmdClearColorImage(cb,swapImages[imageIndex],
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&ccv,1,&subresourceRange);

    VK_CHECK(vkEndCommandBuffer(cb));

    VkSubmitInfo si{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    si.commandBufferCount=1; si.pCommandBuffers=&cb;
    VK_CHECK(vkQueueSubmit(queue,1,&si,fences[frameIndex]));

    VkPresentInfoKHR pi{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    pi.swapchainCount=1; pi.pSwapchains=&swapChain; pi.pImageIndices=&imageIndex;
    VK_CHECK(vkQueuePresentKHR(queue,&pi));

}
VulkanRenderer::~VulkanRenderer() noexcept {

    for (VkFence fence : fences) {
        if (fence != VK_NULL_HANDLE) {
            vkDestroyFence(device, fence, nullptr);
        }
    }

    for (VkCommandBuffer cmdBuf : commandBuffers) {
        if (cmdBuf != VK_NULL_HANDLE){
            vkFreeCommandBuffers(device,
                                 cmdPool,
                                 1,
                                 &cmdBuf);
        }
    }

    if (cmdPool != VK_NULL_HANDLE){
        vkDestroyCommandPool(device,cmdPool, NULL);
    }

    for (VkFramebuffer frameBuffer : frameBuffers){
        if (frameBuffer != VK_NULL_HANDLE){
            vkDestroyFramebuffer(device, frameBuffer, nullptr);
        }
    }


    for (VkImageView swapImageView:swapImageViews){
        if (swapImageView != VK_NULL_HANDLE){
            vkDestroyImageView(device,swapImageView,nullptr);
        }
    }

    if (swapChain != VK_NULL_HANDLE){
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    if (surface != VK_NULL_HANDLE){
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }

    if (renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device, renderPass, nullptr);
    }

    if (device != VK_NULL_HANDLE) {
        vkDestroyDevice(device, nullptr);
    }


    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
    }

    LOGI("VulkanRender exit done!");
}

