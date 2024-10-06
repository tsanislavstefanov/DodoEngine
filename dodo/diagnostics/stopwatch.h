#pragma once

namespace Dodo {

    class Stopwatch
    {
    public:
        Stopwatch();

        void now();

        double milliseconds() const;
        double seconds() const;

    private:
        std::chrono::steady_clock::time_point start_time_{};
    };

}