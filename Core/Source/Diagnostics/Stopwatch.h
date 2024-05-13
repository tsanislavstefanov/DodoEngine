#pragma once

////////////////////////////////////////////////////////////////
// STOPWATCH ///////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

class Stopwatch
{
public:
    Stopwatch() = default;

    [[nodiscard]] double GetAsMilliseconds() const;

    [[nodiscard]] double GetAsSeconds() const
    {
        return GetAsMilliseconds() * 0.001;
    }

    void Start();

    void Reset();
private:
    std::chrono::steady_clock::time_point m_StartTime{};
};