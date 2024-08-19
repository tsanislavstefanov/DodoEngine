#pragma once

#include <vulkan/vulkan.h>

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // VULKAN DEBUG MESSENGER //////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class VulkanDebugMessenger : public RefCounted
    {
    public:
        VulkanDebugMessenger(VkInstance instance);

        void Destroy();

    private:
        VkInstance               m_Instance  = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_Messenger = VK_NULL_HANDLE;
    };

}