#pragma once

namespace Dodo {

    ////////////////////////////////////////////////////////////////
    // STOPWATCH ///////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////

    class Stopwatch
    {
    public:
        Stopwatch();
        virtual ~Stopwatch() = default;

        void Start();
        void Reset();

        double GetAsMilliseconds() const;

        double GetAsSeconds() const
        {
            return GetAsMilliseconds() * 0.001;
        }

    private:
        std::chrono::steady_clock::time_point m_StartTime{};
    };

}