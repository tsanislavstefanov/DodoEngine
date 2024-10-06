#include "pch.h"
#include "swapchain.h"

namespace Dodo {

    const SwapchainPerformanceStats& Swapchain::performance_stats() const
    {
        return performance_stats_;
    }

}