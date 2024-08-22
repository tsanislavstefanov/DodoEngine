#include "pch.h"
#include "Renderer.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDERER DATA ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    static constexpr uint32_t MaxCommandQueueCount = 2;
    static std::vector<RenderCommandQueue> s_CommandQueues{};
    static size_t s_SubmissionQueueIndex = 0;
    static Renderer::PerformanceStats s_PerformanceStats{};

    ////////////////////////////////////////////////////////////////
    // RENDERER ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    void Renderer::Init(const RendererSpecs& specs)
    {
        // Create command queues.
        for (size_t i = 0; i < MaxCommandQueueCount; i++)
        {
            s_CommandQueues.emplace_back();
        }
    }

    void Renderer::Destroy()
    {
    }

    void Renderer::Submit(const RenderCommand& cmd)
    {
        GetSubmissionQueue().Submit(cmd);
    }

    void Renderer::WaitAndRender(RenderThread* renderThread)
    {
        Stopwatch waitStopwatch{};
        renderThread->WaitAndUpdate(RenderThread::State::Kick, RenderThread::State::Busy);
        s_PerformanceStats.WaitTime = waitStopwatch.GetAsMilliseconds();

        Stopwatch workStopwatch{};
        GetRenderQueue().Execute();
        // Render done!
        renderThread->Update(RenderThread::State::Idle);
        s_PerformanceStats.WorkTime = workStopwatch.GetAsMilliseconds();
    }

    void Renderer::SwapQueues()
    {
        s_SubmissionQueueIndex = (s_SubmissionQueueIndex + 1) % MaxCommandQueueCount;
    }

    const Renderer::PerformanceStats& Renderer::GetPerformanceStats() const
    {
        return s_PerformanceStats;
    }

    RenderCommandQueue& GetSubmissionQueue()
    {
        return s_CommandQueues.at(s_SubmissionQueueIndex);
    }

    RenderCommandQueue& GetRenderQueue()
    {
        const size_t index = (s_SubmissionQueueIndex + 1) % MaxCommandQueueCount;
        return s_CommandQueues.at(index);
    }

}