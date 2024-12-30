#pragma once

namespace Dodo {

    class Stopwatch
    {
    public:
        Stopwatch();

        void Now();
        double get_milliseconds() const;
        double GetSeconds() const;

    private:
        std::chrono::steady_clock::time_point m_StartTime{};
    };

}