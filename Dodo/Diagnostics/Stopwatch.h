#pragma once

namespace Dodo {

    class Stopwatch
    {
    public:
        Stopwatch();

        void Now();
        double GetMilliseconds() const;
        double GetSeconds() const;

    private:
        std::chrono::steady_clock::time_point m_StartTime{};
    };

}