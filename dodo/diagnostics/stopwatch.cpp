#include "pch.h"
#include "stopwatch.h"

namespace Dodo {

    Stopwatch::Stopwatch()
    {
        now();
    }

    void Stopwatch::now()
    {
        start_time_ = std::chrono::steady_clock::now();
    }

    double Stopwatch::milliseconds() const
    {
        const auto stop_time = std::chrono::steady_clock::now();
        using Nanoseconds = std::chrono::nanoseconds;
        const auto duration = std::chrono::duration_cast<Nanoseconds>(stop_time - start_time_);
        return (double)duration.count() * 1.0E-6;
    }

    double Stopwatch::seconds() const
    {
        return milliseconds() * 0.001;
    }

}