#include "pch.h"
#include "Stopwatch.h"

////////////////////////////////////////////////////////////////
// STOPWATCH ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

double Stopwatch::GetAsMilliseconds() const
{
    std::chrono::steady_clock::time_point stopTime{};
    stopTime = std::chrono::high_resolution_clock::now();
    using Nanoseconds = std::chrono::nanoseconds;
    const auto dt = std::chrono::duration_cast<Nanoseconds>(stopTime - m_StartTime);
    return dt.count() * 1.0E-6;
}

void Stopwatch::Start()
{
    Reset();
}

void Stopwatch::Reset()
{
    m_StartTime = std::chrono::high_resolution_clock::now();
}