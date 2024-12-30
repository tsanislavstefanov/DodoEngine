#include "pch.h"
#include "Stopwatch.h"

namespace Dodo {

    Stopwatch::Stopwatch()
    {
        Now();
    }

    void Stopwatch::Now()
    {
        m_StartTime = std::chrono::steady_clock::now();
    }

    double Stopwatch::get_milliseconds() const
    {
        const auto stopTime = std::chrono::steady_clock::now();
        using Nanoseconds = std::chrono::nanoseconds;
        const auto duration = std::chrono::duration_cast<Nanoseconds>(stopTime - m_StartTime);
        return static_cast<double>(duration.count()) * 1.0E-6;
    }

    double Stopwatch::GetSeconds() const
    {
        return get_milliseconds() * 0.001;
    }

}