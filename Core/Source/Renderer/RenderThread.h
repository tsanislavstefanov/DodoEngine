#pragma once

#include "RenderCommandQueue.h"
#include "Core/ThreadPolicy.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // RENDER THREAD ///////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class RenderThread
    {
    public:
        template<typename RenderCommand>
        static inline void Submit(RenderCommand&& cmd)
        {
            auto& submissionQueue = s_Instance->GetSubmissionQueue();
            submissionQueue.Submit<RenderCommand>(std::forward<RenderCommand>(cmd));
        }

        ////////////////////////////////////////////////////////////
        // STATE ///////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////

        enum class State
        {
            Idle,
            Kick,
            Busy,
            AutoCount,
            None
        };

        ////////////////////////////////////////////////////////////
        // PERFORMANCE STATS ///////////////////////////////////////
        ////////////////////////////////////////////////////////////

        struct PerformanceStats
        {
            double WaitTime = 0.0;
            double WorkTime = 0.0;
        };

        RenderThread(ThreadPolicy threadPolicy);

        void Dispatch();
        void Pump();
        void BlockUntilRenderComplete();
        void NextFrame();
        void Stop();

        bool IsRunning() const
        {
            return m_IsRunning;
        }
        
        const PerformanceStats& GetPerformanceStats() const
        {
            return m_PerformanceStats;
        }

    private:
        static void RenderProc(RenderThread* renderThread);
        static void WaitAndRender(RenderThread* renderThread);

        void Kick();
        void Wait(State state);
        void Update(State state);
        void WaitAndUpdate(State waitForState, State newState);
        void SwapQueues();

        RenderCommandQueue& GetSubmissionQueue()
        {
            return m_CommandQueues.at(m_SubmissionQueueIndex);
        }

        RenderCommandQueue& GetRenderQueue()
        {
            const size_t index = (m_SubmissionQueueIndex + 1) % MaxCommandQueueCount;
            return m_CommandQueues.at(index);
        }

        static constexpr uint32_t MaxCommandQueueCount = 2;
        static RenderThread* s_Instance;
        ThreadPolicy m_ThreadPolicy = ThreadPolicy::None;
        bool m_IsRunning = false;
        std::thread m_Thread{};
        State m_State = State::None;
        PerformanceStats m_PerformanceStats{};
        std::mutex m_Mutex{};
        std::condition_variable m_ConditionVar{};
        std::vector<RenderCommandQueue> m_CommandQueues{};
        size_t m_SubmissionQueueIndex = 0;
    };

}