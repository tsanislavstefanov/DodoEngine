#pragma once

namespace Dodo {

    struct SwapchainPerformanceStats
    {
        std::atomic<double> gpu_wait_time = 0.0;
    };

    class Swapchain : public RefCounted
    {
    public:
        Swapchain() = default;
        virtual ~Swapchain() = default;

        virtual void begin_frame() = 0;
        virtual void end_frame() = 0;
        virtual void on_resize(uint32_t width, uint32_t height) = 0;
        const SwapchainPerformanceStats& performance_stats() const;

    protected:
        SwapchainPerformanceStats performance_stats_{};
    };

}