#include "pch.h"
#include "Stopwatch.h"

////////////////////////////////////////////////////////////////
// STOPWATCH ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

double Stopwatch::GetAsMilliseconds() const
{
    auto stopTime = std::chrono::steady_clock::now();
    using Nanoseconds = std::chrono::nanoseconds;
    const auto dt = std::chrono::duration_cast<Nanoseconds>(stopTime - m_StartTime);
    return static_cast<double>(dt.count()) * 1.0E-6;
}

void Stopwatch::Start()
{
    Reset();
}

void Stopwatch::Reset()
{
    m_StartTime = std::chrono::steady_clock::now();
}