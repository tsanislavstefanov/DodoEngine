#include "pch.h"
#include "VulkanContext.h"

#ifdef PLATFORM_WINDOWS
#   include <vulkan/vulkan_win32.h>
#endif

#include "Vulkan.h"
#include "Core/Application.h"
#ifdef PLATFORM_WINDOWS
#   include "Platform/Windows/WindowsWindow.h"
#endif
#include "Renderer/Renderer.h"

#ifndef GET_VK_INSTANCE_PROC_ADDR
#   define GET_VK_INSTANCE_PROC_ADDR(INSTANCE, NAME)                                  \
    {                                                                                 \
	    m_Funcs.pfn##NAME = (PFN_vk##NAME)vkGetInstanceProcAddr(INSTANCE, "vk"#NAME); \
	    VERIFY(m_Funcs.pfn##NAME);                                                    \
    }
#endif

#ifndef GET_VK_DEVICE_PROC_ADDR
#   define GET_VK_DEVICE_PROC_ADDR(DEVICE, NAME)                                  \
    {                                                                             \
	    m_Funcs.pfn##NAME = (PFN_vk##NAME)vkGetDeviceProcAddr(DEVICE, "vk"#NAME); \
	    VERIFY(m_Funcs.pfn##NAME);                                                \
    }
#endif

////////////////////////////////////////////////////////////////
// UTILS ///////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

namespace Utils {

    static bool CheckDriverVersionSupport(uint32_t minimumSupportedVersion)
    {
        uint32_t driverVersion = 0;
        VERIFY_VK_RESULT(vkEnumerateInstanceVersion(&driverVersion));
        if (driverVersion < minimumSupportedVersion)
        {
            LOG_CORE_FATAL("Vulkan driver version is out of date...");
            LOG_CORE_FATAL("    Installed driver version: {0}.{1}.{2}.", VK_API_VERSION_MAJOR(driverVersion), VK_API_VERSION_MINOR(driverVersion), VK_API_VERSION_PATCH(driverVersion));
            LOG_CORE_FATAL("    Minimum supported driver version: {0}.{1}.{2}.", VK_API_VERSION_MAJOR(minimumSupportedVersion), VK_API_VERSION_MINOR(minimumSupportedVersion), VK_API_VERSION_PATCH(minimumSupportedVersion));
            return false;
        }

        return true;
    }

    static const char* ConvertMessageSeverityToString(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity)
    {
        switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return "Verbose";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT   : return "Info";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return "Warning";
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT  : return "Error";
        }

        return "Unknown";
    }

    static const char* ConvertMessageTypeToString(VkDebugUtilsMessageTypeFlagsEXT messageType)
    {
        switch (messageType)
        {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    : return "General";
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT : return "Validation";
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: return "Performance";
        }

        return "Unknown";
    }

    static VkPresentModeKHR ConvertToVkPresentMode(VSyncMode vsyncMode)
    {
        switch (vsyncMode)
        {
            case VSyncMode::Disable: return VK_PRESENT_MODE_IMMEDIATE_KHR;
            case VSyncMode::Enable : return VK_PRESENT_MODE_FIFO_KHR;
            case VSyncMode::Mailbox: return VK_PRESENT_MODE_MAILBOX_KHR;
        }

        ASSERT(false, "V-Sync mode not supported!");
    }

}

////////////////////////////////////////////////////////////////
// VULKAN CONTEXT //////////////////////////////////////////////
////////////////////////////////////////////////////////////////

VulkanContext* VulkanContext::s_Context = nullptr;

VulkanContext::VulkanContext()
{
    ASSERT(!s_Context, "VulkanContext instance already exists!");
    s_Context = this;

    InitInstance();
    InitDebugMessenger();
    InitSurface();
    InitAdapter();
    InitDevice();
    InitSwapchain();
}

void VulkanContext::PrepareBuffers()
{
}

void VulkanContext::Resize(uint32_t width, uint32_t height)
{
    m_Surface.Width         = width;
    m_Surface.Height        = height;
    m_Swapchain.NeedsResize = true;
}

void VulkanContext::SwapBuffers()
{
}

void VulkanContext::Dispose()
{
    DisposeSwapchain();
    DisposeSurface();
    DisposeDevice();
    DisposeDebugMessenger();
    DisposeInstance();
}

////////////////////////////////////////////////////////////////
// VULKAN INSTANCE /////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void VulkanContext::InitInstance()
{
    VERIFY(Utils::CheckDriverVersionSupport(m_Instance.ApiVersion));

    // Get supported extensions.
    uint32_t extensionCount = 0;
    VERIFY_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
    std::vector<VkExtensionProperties> extensions(extensionCount);
    VERIFY_VK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()));
    for (const auto& extension : extensions)
    {
        m_Instance.SupportedExtensions.insert(extension.extensionName);
    }

    // Request extensions.
    m_Instance.RequestedExtensions.insert({ VK_KHR_SURFACE_EXTENSION_NAME, true });
    m_Instance.RequestedExtensions.insert({ GetSurfaceExtension(), true});
    m_Instance.RequestedExtensions.insert({ VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false });

    // Enable requested & supported extensions.
    for (auto& [name, required] : m_Instance.RequestedExtensions)
    {
        if (!m_Instance.SupportedExtensions.contains(name))
        {
            if (required) // Required extension not supported..?
            {
                ASSERT(false, "Required instance extension {0} not found, is a driver installed?", name);
            }
            else // Optional extension not supported..?
            {
                LOG_CORE_WARNING("Optional instance extension {0} not found!", name);
                continue;
            }
        }

        m_Instance.EnabledExtensions.insert(name);
    }

    // Find validation layers.
    if (m_Instance.EnabledExtensions.contains(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
    {
        uint32_t layerCount = 0;
        VERIFY_VK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
        std::vector<VkLayerProperties> layers(layerCount);
        VERIFY_VK_RESULT(vkEnumerateInstanceLayerProperties(&layerCount, layers.data()));
        for (const auto& layer : layers)
        {
            if (std::strcmp(m_LayerName, layer.layerName))
            {
                m_LayerFound = true;
                break;
            }
        }
    }

    // Convert enabled extensions to array because Vulkan expects
    // that format.
    uint32_t extensionIndex = 0;
    static constexpr size_t MaxExtensionCount = 64;
    std::array<const char*, MaxExtensionCount> enabledExtensions{};
    for (auto extension : m_Instance.EnabledExtensions)
    {
        enabledExtensions[extensionIndex++] = extension;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Dodo Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "Dodo";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = m_Instance.ApiVersion;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = m_LayerFound ? 1 : 0;
    createInfo.ppEnabledLayerNames     = m_LayerFound ? &m_LayerName : nullptr;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    vkCreateInstance(&createInfo, nullptr, &m_Instance.Handle);
}

const char* VulkanContext::GetSurfaceExtension() const
{
#ifdef PLATFORM_WINDOWS
    return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#endif

    ASSERT(false, "Platform not supported!");
}

void VulkanContext::DisposeInstance()
{
    vkDestroyInstance(m_Instance.Handle, nullptr);
}

////////////////////////////////////////////////////////////////
// VULKAN DEBUG MESSENGER //////////////////////////////////////
////////////////////////////////////////////////////////////////

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::VulkanDebugMessenger::Report(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                           VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                           const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                                           void* userData)
{
    DISCARD_MAYBE_UNUSED(userData);
    LOG_CORE_DEBUG("Vulkan validation report...");
    LOG_CORE_DEBUG("    Severity: {0}.", Utils::ConvertMessageSeverityToString(messageSeverity));
    LOG_CORE_DEBUG("    Type: {0}."    , Utils::ConvertMessageTypeToString(messageType));
    LOG_CORE_DEBUG("    Message: {0}." , callbackData->pMessage);
    return VK_FALSE;
}

void VulkanContext::InitDebugMessenger()
{
    if (m_LayerFound)
    {
        GET_VK_INSTANCE_PROC_ADDR(m_Instance.Handle, CreateDebugUtilsMessengerEXT );
        GET_VK_INSTANCE_PROC_ADDR(m_Instance.Handle, DestroyDebugUtilsMessengerEXT);
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = VulkanDebugMessenger::Report;
        VERIFY_VK_RESULT(m_Funcs.pfnCreateDebugUtilsMessengerEXT(m_Instance.Handle, &createInfo, nullptr, &m_DebugMessenger.Handle));
    }
}

void VulkanContext::DisposeDebugMessenger()
{
    if (m_LayerFound)
    {
        m_Funcs.pfnDestroyDebugUtilsMessengerEXT(m_Instance.Handle, m_DebugMessenger.Handle, nullptr);
    }
}

////////////////////////////////////////////////////////////////
// VULKAN SURFACE //////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void VulkanContext::InitSurface()
{
    const auto& window = Application::GetCurrent().GetWindow();
    m_Surface.Width    = window.GetWidth ();
    m_Surface.Height   = window.GetHeight();

#ifdef PLATFORM_WINDOWS
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd      = static_cast<HWND>(window.GetHandle());
    createInfo.hinstance = static_cast<const WindowsWindow&>(window).GetModule();
    VERIFY_VK_RESULT(vkCreateWin32SurfaceKHR(m_Instance.Handle, &createInfo, nullptr, &m_Surface.Handle));
#endif
}

void VulkanContext::DisposeSurface()
{
    vkDestroySurfaceKHR(m_Instance.Handle, m_Surface.Handle, nullptr);
}

////////////////////////////////////////////////////////////////
// VULKAN ADAPTER //////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void VulkanContext::InitAdapter()
{
    uint32_t adapterCount = 0;
    VERIFY_VK_RESULT(vkEnumeratePhysicalDevices(m_Instance.Handle, &adapterCount, nullptr));
    ASSERT(adapterCount != 0, "Adapters that support Vulkan not found, is a driver installed?");

    std::vector<VkPhysicalDevice> adapters(adapterCount);
    VERIFY_VK_RESULT(vkEnumeratePhysicalDevices(m_Instance.Handle, &adapterCount, adapters.data()));
    for (const auto& candidate : adapters)
    {
        VkPhysicalDeviceProperties Properties{};
        vkGetPhysicalDeviceProperties(candidate, &Properties);
        switch (Properties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            {
                uint32_t extensionCount = 0;
                VERIFY_VK_RESULT(vkEnumerateDeviceExtensionProperties(candidate, nullptr, &extensionCount, nullptr));
                std::vector<VkExtensionProperties> extensions(extensionCount);
                VERIFY_VK_RESULT(vkEnumerateDeviceExtensionProperties(candidate, nullptr, &extensionCount, extensions.data()));
                for (const auto& extension : extensions)
                {
                    m_Adapter.SupportedExtensions.insert(extension.extensionName);
                }

                m_Adapter.Handle     = candidate;
                m_Adapter.Properties = Properties;

                struct Vendor
                {
                    uint32_t    ID   = 0;
                    const char* Name = nullptr;
                };

                static constexpr size_t MaxVendorCount = 6;
                Vendor vendors[MaxVendorCount] = {
                    { 0x1002, "AMD"      },
                    { 0x1010, "ImgTec"   },
                    { 0x10DE, "NVIDIA"   },
                    { 0x13B5, "ARM"      },
                    { 0x5143, "Qualcomm" },
                    { 0x8086, "Intel"    }
                };

                for (size_t i = 0; i < MaxVendorCount; i++)
                {
                    if (vendors[i].ID == Properties.vendorID)
                    {
                        m_Adapter.VendorName = vendors[i].Name;
                        break;
                    }
                }

                break;
            }
        }
    }

    ASSERT(m_Adapter.Handle, "Discrete adapter not found!");
    LOG_CORE_INFO("Selected adapter...");
    LOG_CORE_INFO("    Vendor: {0}."             , m_Adapter.VendorName);
    LOG_CORE_INFO("    Name: {0}."               , m_Adapter.Properties.deviceName);
    LOG_CORE_INFO("    API version: {0}.{1}.{2}.", VK_API_VERSION_MAJOR(m_Adapter.Properties.apiVersion),
                                                   VK_API_VERSION_MINOR(m_Adapter.Properties.apiVersion),
                                                   VK_API_VERSION_PATCH(m_Adapter.Properties.apiVersion));
}

////////////////////////////////////////////////////////////////
// VULKAN QUEUE ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void VulkanContext::FindQueues()
{
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_Adapter.Handle, &queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queues(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_Adapter.Handle, &queueCount, queues.data());

    uint32_t queueIndex = 0;
    for (const auto& queue : queues)
    {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            m_GraphicQueue.FamilyIndex = queueIndex;
            break;
        }

        queueIndex++;
    }

    GET_VK_INSTANCE_PROC_ADDR(m_Instance.Handle, GetPhysicalDeviceSurfaceSupportKHR);
    queueIndex = 0;
    for (const auto& queue : queues)
    {
        VkBool32 supportsSwap = false;
        m_Funcs.pfnGetPhysicalDeviceSurfaceSupportKHR(m_Adapter.Handle,
                                                      queueIndex,
                                                      m_Surface.Handle,
                                                      &supportsSwap);
        if (supportsSwap)
        {
            m_PresentQueue.FamilyIndex = queueIndex;
            break;
        }
    }

    ASSERT(m_GraphicQueue.FamilyIndex.has_value(), "Graphic queue not found!");
    ASSERT(m_PresentQueue.FamilyIndex.has_value(), "Present queue not found!");
}

////////////////////////////////////////////////////////////////
// VULKAN DEVICE ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void VulkanContext::InitDevice()
{
    FindQueues();

    // Add queue create info(s).
    const size_t MaxQueueFamilyIndexCount = 2;
    std::array<uint32_t, MaxQueueFamilyIndexCount> queueFamilyIndices = {
        m_GraphicQueue.FamilyIndex.value(),
        m_PresentQueue.FamilyIndex.value()
    };

    const float defaultQueuePriority = 1.0f;
    for (auto familyIndex : queueFamilyIndices)
    {
        auto& queueCreateInfo            = m_Device.QueueCreateInfos.emplace_back();
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = familyIndex;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &defaultQueuePriority;
    }


    // Request extensions.
    m_Device.RequestedExtensions.insert({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, true });

    // Enable requested & supported extensions.
    for (auto& [name, required] : m_Device.RequestedExtensions)
    {
        if (!m_Adapter.SupportedExtensions.contains(name))
        {
            if (required) // Required extension not supported..?
            {
                ASSERT(false, "Required instance extension {0} not found, is a driver installed?", name);
            }
            else // Optional extension not supported..?
            {
                LOG_CORE_WARNING("Optional instance extension {0} not found!", name);
                continue;
            }
        }

        m_Device.EnabledExtensions.insert(name);
    }

    // Convert enabled extensions to array because Vulkan expects
    // that format.
    uint32_t extensionIndex = 0;
    const size_t MaxExtensionCount = 64;
    std::array<const char*, MaxExtensionCount> enabledExtensions{};
    for (auto extension : m_Instance.EnabledExtensions)
    {
        enabledExtensions[extensionIndex++] = extension;
    }

    // Create device.
    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount    = static_cast<uint32_t>(m_Device.QueueCreateInfos.size());
    deviceInfo.pQueueCreateInfos       = m_Device.QueueCreateInfos.data();
    deviceInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
    deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
    VERIFY_VK_RESULT(vkCreateDevice(m_Adapter.Handle, &deviceInfo, nullptr, &m_Device.Handle));

    // Get queue(s).
    vkGetDeviceQueue(m_Device.Handle, m_GraphicQueue.FamilyIndex.value(), 0, &m_GraphicQueue.Handle);
    vkGetDeviceQueue(m_Device.Handle, m_PresentQueue.FamilyIndex.value(), 0, &m_PresentQueue.Handle);
}

void VulkanContext::DisposeDevice()
{
    vkDestroyDevice(m_Device.Handle, nullptr);
}

////////////////////////////////////////////////////////////////
// VULKAN SWAPCHAIN ////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void VulkanContext::InitSwapchain()
{
    GET_VK_INSTANCE_PROC_ADDR(m_Instance.Handle, GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_VK_INSTANCE_PROC_ADDR(m_Instance.Handle, GetPhysicalDeviceSurfaceFormatsKHR);
    GET_VK_INSTANCE_PROC_ADDR(m_Instance.Handle, GetPhysicalDeviceSurfacePresentModesKHR);

    GET_VK_DEVICE_PROC_ADDR(m_Device.Handle, CreateSwapchainKHR);
    GET_VK_DEVICE_PROC_ADDR(m_Device.Handle, GetSwapchainImagesKHR);
    GET_VK_DEVICE_PROC_ADDR(m_Device.Handle, AcquireNextImageKHR);
    GET_VK_DEVICE_PROC_ADDR(m_Device.Handle, QueuePresentKHR);
    GET_VK_DEVICE_PROC_ADDR(m_Device.Handle, DestroySwapchainKHR);

    RecreateSwapchain();
}

void VulkanContext::RecreateSwapchain()
{
    QuerySwapchainSupportDetails();
    ASSERT(AreSwapchainSupportDetailsAdequate(), "Swapchain support details are not adequate!");

    m_Swapchain.SurfaceFormat = SelectSwapchainSurfaceFormat();
    m_Swapchain.PresentMode   = SelectSwapchainPresentMode();
    m_Swapchain.Extent        = SelectSwapchainExtent();

    const VkSurfaceCapabilitiesKHR& capabilities = m_Swapchain.Capabilities;
    m_Swapchain.ImageCount = capabilities.minImageCount + 1;
    if ((capabilities.maxImageCount > 0) && (m_Swapchain.ImageCount > capabilities.maxImageCount))
    {
        m_Swapchain.ImageCount = capabilities.maxImageCount;
    }

    // Vulkan needs to know how the graphics & present queue
    // will communicate when they don't share the same family index.
    VkSharingMode imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    std::array<uint32_t, 2> queueFamilyIndices{};
    if (m_GraphicQueue.FamilyIndex != m_PresentQueue.FamilyIndex)
    {
        imageSharingMode         = VK_SHARING_MODE_CONCURRENT;
        queueFamilyIndices.at(0) = m_GraphicQueue.FamilyIndex.value();
        queueFamilyIndices.at(1) = m_PresentQueue.FamilyIndex.value();
    }

    // Create swapchain.
    VkSwapchainKHR oldSwapchain = m_Swapchain.Handle;
    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface               = m_Surface.Handle;
    swapchainCreateInfo.minImageCount         = m_Swapchain.ImageCount;
    swapchainCreateInfo.imageFormat           = m_Swapchain.SurfaceFormat.format;
    swapchainCreateInfo.imageColorSpace       = m_Swapchain.SurfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent           = m_Swapchain.Extent;
    swapchainCreateInfo.imageArrayLayers      = 1;
    swapchainCreateInfo.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode      = imageSharingMode;
    swapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
    swapchainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices.empty() ? nullptr : queueFamilyIndices.data();
    swapchainCreateInfo.preTransform          = m_Swapchain.Capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode           = m_Swapchain.PresentMode;
    swapchainCreateInfo.clipped               = VK_TRUE;
    swapchainCreateInfo.oldSwapchain          = oldSwapchain;
    VERIFY_VK_RESULT(m_Funcs.pfnCreateSwapchainKHR(m_Device.Handle, &swapchainCreateInfo, nullptr, &m_Swapchain.Handle));
    if (oldSwapchain)
    {
        m_Funcs.pfnDestroySwapchainKHR(m_Device.Handle, oldSwapchain, nullptr);
    }

    // Create images & views.
    for (VkImageView imageView : m_Swapchain.ImageViews)
    {
        vkDestroyImageView(m_Device.Handle, imageView, nullptr);
    }

    m_Swapchain.ImageViews.clear();
    m_Swapchain.Images.clear();

    m_Funcs.pfnGetSwapchainImagesKHR(m_Device.Handle, m_Swapchain.Handle, &m_Swapchain.ImageCount, nullptr);
    m_Swapchain.Images.resize(m_Swapchain.ImageCount);
    m_Swapchain.ImageViews.resize(m_Swapchain.Images.size());
    m_Funcs.pfnGetSwapchainImagesKHR(m_Device.Handle, m_Swapchain.Handle, &m_Swapchain.ImageCount, m_Swapchain.Images.data());

    for (size_t i = 0; i < m_Swapchain.Images.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image                           = m_Swapchain.Images.at(i);
        createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format                          = m_Swapchain.SurfaceFormat.format;
        createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;
        VERIFY_VK_RESULT(vkCreateImageView(m_Device.Handle, &createInfo, nullptr, &m_Swapchain.ImageViews[i]));
    }

    // Create render pass.
    if (m_Swapchain.RenderPass)
    {
        vkDestroyRenderPass(m_Device.Handle, m_Swapchain.RenderPass, nullptr);
    }

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format         = m_Swapchain.SurfaceFormat.format;
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachmentRef{};
    attachmentRef.attachment = 0;
    attachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &attachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments    = &colorAttachment;
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;
    VERIFY_VK_RESULT(vkCreateRenderPass(m_Device.Handle, &renderPassInfo, nullptr, &m_Swapchain.RenderPass));

    // Create framebuffer(s)
    for (VkFramebuffer framebuffer : m_Swapchain.Framebuffers)
    {
        vkDestroyFramebuffer(m_Device.Handle, framebuffer, nullptr);
    }

    m_Swapchain.Framebuffers.clear();
    m_Swapchain.Framebuffers.resize(m_Swapchain.ImageViews.size());

    for (size_t i = 0; i < m_Swapchain.ImageViews.size(); i++)
    {
        VkImageView attachments[] = { m_Swapchain.ImageViews.at(i) };
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = m_Swapchain.RenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments    = attachments;
        framebufferInfo.width           = m_Swapchain.Extent.width;
        framebufferInfo.height          = m_Swapchain.Extent.height;
        framebufferInfo.layers          = 1;
        VERIFY_VK_RESULT(vkCreateFramebuffer(m_Device.Handle, &framebufferInfo, nullptr, &m_Swapchain.Framebuffers.at(i)));
    }

    // Create command pool(s) & buffers.
    if (m_Swapchain.CmdPool)
    {
        vkDestroyCommandPool(m_Device.Handle, m_Swapchain.CmdPool, nullptr);
    }

    VkCommandPoolCreateInfo cmdPoolInfo{};
    cmdPoolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolInfo.queueFamilyIndex = m_GraphicQueue.FamilyIndex.value();
    VERIFY_VK_RESULT(vkCreateCommandPool(m_Device.Handle, &cmdPoolInfo, nullptr, &m_Swapchain.CmdPool));

    m_Swapchain.CmdBuffers.clear();
    const uint32_t backBufferCount = Renderer::GetSettings().BackBufferCount;
    m_Swapchain.CmdBuffers.resize(backBufferCount);

    for (size_t i = 0; i < backBufferCount; i++)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool        = m_Swapchain.CmdPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        VERIFY_VK_RESULT(vkAllocateCommandBuffers(m_Device.Handle, &allocInfo, &m_Swapchain.CmdBuffers.at(i)));
    }

    // Create semaphores.
    for (size_t i = 0; i < backBufferCount; i++)
    {
        vkDestroySemaphore(m_Device.Handle, m_Swapchain.ImageAvailableSemaphore.at(i), nullptr);
        vkDestroySemaphore(m_Device.Handle, m_Swapchain.RenderCompleteSemaphore.at(i), nullptr);
    }

    m_Swapchain.ImageAvailableSemaphore.clear();
    m_Swapchain.ImageAvailableSemaphore.resize(backBufferCount);

    m_Swapchain.RenderCompleteSemaphore.clear();
    m_Swapchain.RenderCompleteSemaphore.resize(backBufferCount);

    for (size_t i = 0; i < backBufferCount; i++)
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VERIFY_VK_RESULT(vkCreateSemaphore(m_Device.Handle, &semaphoreInfo, nullptr, &m_Swapchain.ImageAvailableSemaphore.at(i)));
        VERIFY_VK_RESULT(vkCreateSemaphore(m_Device.Handle, &semaphoreInfo, nullptr, &m_Swapchain.RenderCompleteSemaphore.at(i)));
    }

    // Fences.
    for (size_t i = 0; i < m_Swapchain.WaitFences.size(); i++)
    {
        vkDestroyFence(m_Device.Handle, m_Swapchain.WaitFences.at(i), nullptr);
    }

    m_Swapchain.WaitFences.clear();
    m_Swapchain.WaitFences.resize(backBufferCount);
    for (size_t i = 0; i < backBufferCount; i++)
    {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VERIFY_VK_RESULT(vkCreateFence(m_Device.Handle, &fenceInfo, nullptr, &m_Swapchain.WaitFences.at(i)));
    }
}

void VulkanContext::QuerySwapchainSupportDetails()
{
    // Get capabilities.
    m_Funcs.pfnGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Adapter.Handle, m_Surface.Handle, &m_Swapchain.Capabilities);

    // Get surface formats.
    uint32_t formatCount = 0;
    m_Funcs.pfnGetPhysicalDeviceSurfaceFormatsKHR(m_Adapter.Handle, m_Surface.Handle, &formatCount, nullptr);
    if (formatCount > 0)
    {
        m_Swapchain.SurfaceFormats.resize(formatCount);
        m_Funcs.pfnGetPhysicalDeviceSurfaceFormatsKHR(m_Adapter.Handle,
                                                      m_Surface.Handle,
                                                      &formatCount,
                                                      m_Swapchain.SurfaceFormats.data());
    }

    // Get present modes.
    uint32_t modeCount = 0;
    m_Funcs.pfnGetPhysicalDeviceSurfacePresentModesKHR(m_Adapter.Handle, m_Surface.Handle, &modeCount, nullptr);
    if (modeCount > 0)
    {
        m_Swapchain.PresentModes.resize(modeCount);
        m_Funcs.pfnGetPhysicalDeviceSurfacePresentModesKHR(m_Adapter.Handle,
                                                           m_Surface.Handle,
                                                           &modeCount,
                                                           m_Swapchain.PresentModes.data());
    }
}

bool VulkanContext::AreSwapchainSupportDetailsAdequate() const
{
    return !m_Swapchain.SurfaceFormats.empty()
        && !m_Swapchain.PresentModes.empty();
}

VkSurfaceFormatKHR VulkanContext::SelectSwapchainSurfaceFormat() const
{
    VkSurfaceFormatKHR targetFormat{};
    targetFormat.format     = VK_FORMAT_B8G8R8A8_SRGB;
    targetFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    // Target surface format has to be compatible with ImGui when enabled!
    if (Application::GetCurrent().GetSpecs().EnableImGui)
    {
        targetFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
    }

    for (const auto& format : m_Swapchain.SurfaceFormats)
    {
        if (format.format == targetFormat.format && format.colorSpace == targetFormat.colorSpace)
        {
            return format;
        }
    }

    return m_Swapchain.SurfaceFormats[0];
}

VkPresentModeKHR VulkanContext::SelectSwapchainPresentMode() const
{
    VkPresentModeKHR targetMode = Utils::ConvertToVkPresentMode(Renderer::GetSettings().VSyncMode);
    // Target present mode has to be compatible with ImGui when enabled!
    if (Application::GetCurrent().GetSpecs().EnableImGui)
    {
        targetMode = VK_PRESENT_MODE_FIFO_KHR;
    }

    for (VkPresentModeKHR mode : m_Swapchain.PresentModes)
    {
        if (mode == targetMode)
        {
            return mode;
        }
    }

    return m_Swapchain.PresentModes[0];
}

VkExtent2D VulkanContext::SelectSwapchainExtent() const
{
    const VkSurfaceCapabilitiesKHR& capabilities = m_Swapchain.Capabilities;
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    VkExtent2D extent{};
    extent.width  = std::clamp(m_Surface.Width , capabilities.minImageExtent.width , capabilities.maxImageExtent.width );
    extent.height = std::clamp(m_Surface.Height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return extent;
}

void VulkanContext::DisposeSwapchain()
{
    m_Funcs.pfnDestroySwapchainKHR(m_Device.Handle, m_Swapchain.Handle, nullptr);
}