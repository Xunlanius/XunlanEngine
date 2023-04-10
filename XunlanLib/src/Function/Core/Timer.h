#pragma once

#include "src/Common/Common.h"
#include "src/Common/WindowsCommon.h"
#include <string>
#include <chrono>

namespace Xunlan
{
    class Timer
    {
    public:

        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;

        void Begin()
        {
            m_start = Clock::now();
        }

        void End()
        {
            std::chrono::nanoseconds deltaNano = Clock::now() - m_start;
            std::chrono::milliseconds deltaMili = std::chrono::duration_cast<std::chrono::milliseconds>(deltaNano);

            m_msAvg += ((float)deltaMili.count() - m_msAvg) / (float)m_counter;
            ++m_counter;

            if (std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - m_prev).count() >= 1)
            {
                OutputDebugStringA("ms Per Frame: ");
                OutputDebugStringA(std::to_string(m_msAvg).c_str());
                OutputDebugStringA((" fps: " + std::to_string(m_counter)).c_str());
                OutputDebugStringA("\n");

                m_msAvg = 0.0f;
                m_counter = 1;
                m_prev = Clock::now();
            }
        }

    private:

        float m_msAvg = 0.0f;
        uint32 m_counter = 1;
        TimePoint m_start = Clock::now();
        TimePoint m_prev = Clock::now();
    };
}