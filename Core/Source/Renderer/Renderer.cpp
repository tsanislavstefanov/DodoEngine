#include "pch.h"
#include "Renderer.h"
#include "RenderDevice.h"
#include "Core/Application.h"
#include "Core/RenderThread.h"
#include "Diagnostics/Stopwatch.h"
#include "Drivers/Vulkan/VulkanDevice.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // UTILS ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    namespace Utils {

        static RenderDevice* CreateRenderDevice(RenderDeviceType deviceType)
        {
            switch (deviceType)
            {
                case RenderDeviceType::Vulkan: return new VulkanDevice();
                default                      : ASSERT(false, "RenderDeviceType not supported!");
            }

            return nullptr;
        }

    }

    ////////////////////////////////////////////////////////////////
    // RENDERER DATA ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct RendererData
    {
        RenderSettings                    Settings{};
        const size_t                      CommandBufferCount = 2;
        std::vector<RenderCommandBuffer*> CommandBuffers{};
        RenderDevice*                     Device = nullptr;
        size_t                            CommandBufferSubmissionIndex = 0;
    };

    static RendererData s_Data{};

    ////////////////////////////////////////////////////////////////
    // RENDERER ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    const RenderSettings& Renderer::GetSettings()
    {
        return s_Data.Settings;
    }

    void Renderer::SetSettings(const RenderSettings& settings)
    {
        s_Data.Settings = settings;
    }

    void Renderer::Init()
    {
        s_Data.CommandBuffers.resize(s_Data.CommandBufferCount);
        s_Data.CommandBuffers.at(0) = new RenderCommandBuffer();
        s_Data.CommandBuffers.at(1) = new RenderCommandBuffer();
        s_Data.Device               = Utils::CreateRenderDevice(s_Data.Settings.RenderDeviceType);
    }

    void Renderer::RenderThreadProc(RenderThread* renderThread)
    {
        while (renderThread->IsRunning())
        {
            WaitAndRender(renderThread);
        }
    }

    void Renderer::WaitAndRender(RenderThread* renderThread)
    {
        PerformanceStats& performanceStats = Application::GetCurrent().GetStats();

        // Wait for kick and start render.
        {
            Stopwatch stopwatch{};
            renderThread->WaitAndUpdate(RenderThreadState::Kick, RenderThreadState::Busy);
            performanceStats.RenderThreadWaitTime = stopwatch.GetAsMilliseconds();
        }

        Stopwatch stopwatch{};
        GetRenderCommandBuffer()->Execute();
        // Render done.
        renderThread->Update(RenderThreadState::Idle);
        performanceStats.RenderThreadWorkTime = stopwatch.GetAsMilliseconds();
    }

    void Renderer::BeginFrame()
    {
        s_Data.Device->BeginFrame();
    }

    void Renderer::Resize(uint32_t width, uint32_t height)
    {
        s_Data.Device->Resize(width, height);
    }

    void Renderer::SwapBuffers()
    {
        s_Data.CommandBufferSubmissionIndex = (s_Data.CommandBufferSubmissionIndex + 1) % s_Data.CommandBufferCount;
    }

    void Renderer::EndFrame()
    {
        s_Data.Device->EndFrame();
    }

    void Renderer::Dispose()
    {
        s_Data.Device->Dispose();
        delete s_Data.CommandBuffers.at(0);
        delete s_Data.CommandBuffers.at(1);
    }

    RenderCommandBuffer* Renderer::GetSubmissionCommandBuffer()
    {
        return GetCommandBuffer(s_Data.CommandBufferSubmissionIndex);
    }

    RenderCommandBuffer* Renderer::GetRenderCommandBuffer()
    {
        const auto index = (s_Data.CommandBufferSubmissionIndex + 1) % s_Data.CommandBufferCount;
        return GetCommandBuffer(index);
    }

    RenderCommandBuffer* Renderer::GetCommandBuffer(size_t index)
    {
        return s_Data.CommandBuffers.at(index);
    }

}