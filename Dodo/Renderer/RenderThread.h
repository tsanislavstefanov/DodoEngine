#pragma once

#include "RenderCommandQueue.h"
#include "Concurrency/Thread.h"

namespace Dodo {

    enum class RenderThreadPolicy
    {
        SingleThreaded,
        MultiThreaded,
        AutoCount,
        None
    };

    class RenderThread
    {
    public:
        struct PerformanceStats
        {
            std::atomic<double> WaitTime = 0.0;
            std::atomic<double> WorkTime = 0.0;
            std::atomic<uint32_t> ExecutedCommandCount = 0;
        };

        RenderThread() = default;

        void Start(RenderThreadPolicy policy = RenderThreadPolicy::SingleThreaded);
        void WaitUntilRenderComplete();
        void Submit(RenderCommand&& command);
        void Flush();
        void Stop();
        bool IsRunning() const { return m_IsRunning; }
        const PerformanceStats& GetPerformanceStats() const { return m_PerformanceStats; }

    private:
        enum class State
        {
            Idle,
            Kick,
            Busy,
            AutoCount,
            None
        };

        static void Main(RenderThread* renderThread);

        void WaitForState(State state);
        void TransitionToState(State state);
        void Kick();
        void Pump();
        size_t GetRenderQueueIndex() const { return (m_SubmissionQueueIndex + 1) % m_CommandQueues.size(); }
        void Render();

        RenderThreadPolicy m_ThreadPolicy = RenderThreadPolicy::None;
        bool m_IsRunning = false;
        Ref<Thread> m_Thread = nullptr;
        std::mutex m_Mutex{};
        State m_State = State::Idle;
        std::condition_variable m_ConditionVar{};
        static constexpr size_t MaxCommandQueueCount = 2;
        std::array<RenderCommandQueue, MaxCommandQueueCount> m_CommandQueues{};
        size_t m_SubmissionQueueIndex = 0;
        PerformanceStats m_PerformanceStats{};
    };

}