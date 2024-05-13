#include "pch.h"
#include "VulkanDevice.h"

////////////////////////////////////////////////////////////////
// VULKAN DEVICE ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////

VulkanDevice::VulkanDevice()
{
    m_Context = std::make_unique<VulkanContext>();
}

void VulkanDevice::BeginFrame()
{
    m_Context->PrepareBuffers();
}

void VulkanDevice::Resize(uint32_t width, uint32_t height)
{
    m_Context->Resize(width, height);
}

void VulkanDevice::EndFrame()
{
    m_Context->SwapBuffers();
}

void VulkanDevice::Dispose()
{
    m_Context->Dispose();
}