#include "pch.h"
#include "Stopwatch.h"

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // STOPWATCH ///////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    Stopwatch::Stopwatch()
    {
        Start();
    }

    double Stopwatch::GetAsMilliseconds() const
    {
        auto stopTime = std::chrono::steady_clock::now();
        using Nanoseconds = std::chrono::nanoseconds;
        const auto deltaTime = std::chrono::duration_cast<Nanoseconds>(stopTime - m_StartTime);
        return static_cast<double>(deltaTime.count()) * 1.0E-6;
    }

    void Stopwatch::Start()
    {
        Reset();
    }

    void Stopwatch::Reset()
    {
        m_StartTime = std::chrono::steady_clock::now();
    }

}