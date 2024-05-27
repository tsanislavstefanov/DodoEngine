#include "pch.h"
#include "Renderer.h"
#include "RendererApi.h"
#include "Core/Application.h"
#include "Core/RenderThread.h"
#include "Drivers/Vulkan/VulkanRenderer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // UTILS ///////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    namespace Utils {

        static RendererApi* CreateRendererApi(RendererApiType apiType)
        {
            switch (apiType)
            {
                case RendererApiType::Vulkan: return new VulkanRenderer();
                default: DODO_ASSERT(false, "Renderer API type not supported!");
            }

            return nullptr;
        }

    }

    ////////////////////////////////////////////////////////////////
    // RENDERER DATA ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    struct RendererData
    {
        RenderSettings Settings{};
        std::vector<RenderCommandQueue*> CommandQueues{};
        RendererApi* RendererApi = nullptr;
        size_t SubmissionCommandQueueIndex = 0;
    };

    static RendererData s_Data{};

    ////////////////////////////////////////////////////////////////
    // RENDERER ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    const RenderSettings& Renderer::GetSettings()
    {
        return s_Data.Settings;
    }

    void Renderer::SetSettings(const RenderSettings &settings)
    {
        s_Data.Settings = settings;
    }

    void Renderer::Init()
    {
        // Schedule commands in one queue while executing the other.
        s_Data.CommandQueues.resize(2);
        s_Data.CommandQueues.at(0) = new RenderCommandQueue();
        s_Data.CommandQueues.at(1) = new RenderCommandQueue();

        s_Data.RendererApi = Utils::CreateRendererApi(s_Data.Settings.RendererApiType);
    }

    void Renderer::Shutdown()
    {
        s_Data.RendererApi->Shutdown();
        delete s_Data.RendererApi;
        for (const auto queue : s_Data.CommandQueues)
        {
            delete queue;
        }
    }

    RenderCommandQueue* Renderer::GetSubmissionCommandQueue()
    {
        return s_Data.CommandQueues.at(s_Data.SubmissionCommandQueueIndex);
    }

    RenderCommandQueue* Renderer::GetRenderCommandQueue()
    {
        const auto index = (s_Data.SubmissionCommandQueueIndex + 1) % s_Data.CommandQueues.size();
        return s_Data.CommandQueues.at(index);
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
        // Wait for kick and start to render.
        {
            Stopwatch waitStopwatch{};
            renderThread->WaitAndUpdate(RenderThreadState::Kick, RenderThreadState::Busy);
            performanceStats.RenderThreadWaitTime = waitStopwatch.GetAsMilliseconds();
        }

        Stopwatch workStopwatch{};
        GetRenderCommandQueue()->Execute();
        // Render done.
        renderThread->Update(RenderThreadState::Idle);
        performanceStats.RenderThreadWorkTime = workStopwatch.GetAsMilliseconds();
    }

    void Renderer::SwapQueues()
    {
        s_Data.SubmissionCommandQueueIndex = (s_Data.SubmissionCommandQueueIndex + 1) % s_Data.CommandQueues.size();
    }

}