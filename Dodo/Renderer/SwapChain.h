#pragma once

#include <atomic>
#include <cstdint>

#include "Memory/Ref.h"

namespace Dodo {

    class SwapChain : public RefCounted
    {
    public:
        struct PerformanceStats
        {
            std::atomic<double> GPUWaitTime = 0.0;
        };

        SwapChain() = default;
        virtual ~SwapChain() = default;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void OnResize(uint32_t width, uint32_t height) = 0;
        const PerformanceStats& GetPerformanceStats() const { return m_PerformanceStats; }

    protected:
        PerformanceStats m_PerformanceStats{};
    };

}