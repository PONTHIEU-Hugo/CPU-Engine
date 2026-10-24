#ifndef PROFILER_H_INCLUDED
#define PROFILER_H_INCLUDED

#include <chrono>

// Additionne dans 'acc' le temps passé dans le bloc où il est déclaré (en ms).
class ScopedTimer
{
public:
    explicit ScopedTimer(double& acc) : m_acc(acc), m_t0(Clock::now()) {}
    ~ScopedTimer()
    {
        m_acc += std::chrono::duration<double, std::milli>(Clock::now() - m_t0).count();
    }

private:
    using Clock = std::chrono::steady_clock;
    double& m_acc;
    Clock::time_point m_t0;
};

#endif