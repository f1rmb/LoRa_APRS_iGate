#include <Arduino.h>

#include "Timer.h"

Timer::Timer() :
m_timeout_ms(0UL),
m_nextTimeout(0UL)
{
}

void Timer::setTimeout(const unsigned long timeout_ms)
{
    m_timeout_ms = timeout_ms;
}

unsigned long Timer::getRemainingInSecs() const
{
    if (m_nextTimeout == 0UL)
    {
        return 0UL;
    }

    return (m_nextTimeout - millis()) / 1000;
}

// cppcheck-suppress unusedFunction
bool Timer::isRunning() const
{
    return (m_nextTimeout > 0UL);
}

bool Timer::hasExpired()
{
    return (millis() > m_nextTimeout);
}

void Timer::start()
{
    m_nextTimeout = (millis() + m_timeout_ms);
}

// cppcheck-suppress unusedFunction
void Timer::stop()
{
    m_nextTimeout = 0UL;
}

