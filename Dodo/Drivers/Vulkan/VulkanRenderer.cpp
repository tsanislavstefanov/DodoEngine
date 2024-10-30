#include "pch.h"
#include "VulkanUtils.h"
#include "VulkanRenderer.h"
#include "VulkanAllocator.h"
#include "Core/File.h"

namespace Dodo {

    namespace Utils {

        static VkBufferUsageFlagBits ConvertToVulkanBufferUsage(BufferUsage bufferUsage)
        {
            switch (bufferUsage)
            {
                case BufferUsage::VertexBuffer: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                default: break;
            }

            DODO_VERIFY(false);
            return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        }

    }

    VulkanRenderer::VulkanRenderer(RenderThread& renderThread, const Window& targetWindow, VSyncMode vsyncMode)
        : Renderer(renderThread)
        , m_Instance(Ref<VulkanInstance>::Create(targetWindow))
        , m_Device(Ref<VulkanDevice>::Create(m_Instance))
        , m_SwapChain(Ref<VulkanSwapChain>::Create(m_Instance, m_Device, targetWindow, vsyncMode))
    {
    }

    VulkanRenderer::~VulkanRenderer()
    {
        m_SwapChain = nullptr;
        m_Device = nullptr;
        m_Instance = nullptr;
    }

    BufferHandle VulkanRenderer::BufferCreate(BufferUsage bufferUsage, size_t size, void* data)
    {
        VulkanBuffer* vulkanBuffer = new VulkanBuffer();
        vulkanBuffer->Size = size;

        Submit([this, bufferUsage, size, data, vulkanBuffer]() mutable
        {
            VulkanAllocator allocator(m_Instance, m_Device);

            if (!data)
            {
                VkBufferCreateInfo bufferCreateInfo{};
                bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferCreateInfo.size  = size;
                bufferCreateInfo.usage = Utils::ConvertToVulkanBufferUsage(bufferUsage);
                vulkanBuffer->MemoryAllocation =
                    allocator.Allocate(bufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, &vulkanBuffer->Buffer);
            }
            else
            {
                VkBufferCreateInfo stagingBufferCreateInfo{};
                stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                stagingBufferCreateInfo.size  = size;
                stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                VkBuffer stagingBuffer = nullptr;

                VmaAllocation stagingBufferAllocation =
                    allocator.Allocate(stagingBufferCreateInfo, VMA_MEMORY_USAGE_CPU_TO_GPU, &stagingBuffer);

                uint8_t* destination = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
                std::memcpy(destination, data, size);
                allocator.UnmapMemory(stagingBufferAllocation);

                VkBufferCreateInfo bufferCreateInfo{};
                bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferCreateInfo.size  = size;
                bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | Utils::ConvertToVulkanBufferUsage(bufferUsage);
                vulkanBuffer->MemoryAllocation = allocator.Allocate(bufferCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, &vulkanBuffer->Buffer);

                VkCommandBuffer commandBuffer = m_Device->AllocateThreadLocalCommandBuffer();
                VkBufferCopy copyRegion{};
                copyRegion.size = size;
                vkCmdCopyBuffer(commandBuffer, stagingBuffer, vulkanBuffer->Buffer, 1, &copyRegion);
                m_Device->FlushCommandBuffer(commandBuffer);
                allocator.Destroy(stagingBuffer, stagingBufferAllocation);
            }
        });

        return BufferHandle(vulkanBuffer);
    }

    void VulkanRenderer::BufferUploadData(BufferHandle bufferHandle, void* data, size_t size, size_t offset)
    {
        Submit([this, bufferHandle, data, size, offset]() mutable
        {
            VulkanAllocator allocator(m_Instance, m_Device);
            VulkanBuffer* vulkanBuffer = reinterpret_cast<VulkanBuffer*>(bufferHandle.Handle);
            uint8_t* destination = allocator.MapMemory<uint8_t>(vulkanBuffer->MemoryAllocation);
            std::memcpy(destination, static_cast<uint8_t*>(data) + offset, size);
            allocator.UnmapMemory(vulkanBuffer->MemoryAllocation);
        });
    }

    void VulkanRenderer::BufferDestroy(BufferHandle bufferHandle)
    {
        Submit([this, bufferHandle]() mutable
        {
            VulkanAllocator allocator(m_Instance, m_Device);
            VulkanBuffer* vulkanBuffer = reinterpret_cast<VulkanBuffer*>(bufferHandle.Handle);
            allocator.Destroy(vulkanBuffer->Buffer, vulkanBuffer->MemoryAllocation);
            delete vulkanBuffer;
        });
    }

    ShaderHandle VulkanRenderer::ShaderCreate(const std::filesystem::path& assetPath)
    {
        VulkanShader* vulkanShader = new VulkanShader{};
        vulkanShader->AssetPath = assetPath;
        
        ShaderHandle shaderHandle(vulkanShader);
        ShaderReload(shaderHandle);

        return shaderHandle;
    }

    void VulkanRenderer::ShaderReload(ShaderHandle shaderHandle, bool forceCompile)
    {
        Submit([this, shaderHandle, forceCompile]() mutable
        {
            auto vulkanShader = reinterpret_cast<VulkanShader*>(shaderHandle.Handle);
            std::string shaderSource = File::ReadAndSkipBOM(vulkanShader->AssetPath);
            if (!shaderSource.empty())
            {
                vulkanShader->Source = std::move(shaderSource);
                m_ShaderCompiler.Recompile(vulkanShader, forceCompile);
            }
        });
    }

    void VulkanRenderer::ShaderDestroy(ShaderHandle shaderHandle)
    {

    }

    void VulkanRenderer::BeginFrame()
    {
        Submit([this]() mutable
        {
            m_SwapChain->BeginFrame();
        });
    }

    void VulkanRenderer::EndFrame()
    {
        Submit([this]() mutable
        {
            m_SwapChain->EndFrame();
        });
    }

    void VulkanRenderer::OnResize(uint32_t width, uint32_t height)
    {
        m_SwapChain->OnResize(width, height);
    }

}