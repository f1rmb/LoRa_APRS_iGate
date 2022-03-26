#include <Arduino.h>

#include "Timer.h"

Timer::Timer() :
m_timeout_ms(0UL),
m_start(0UL),
m_hasExpired(false),
m_isRunning(false)
{
}

void Timer::setTimeout(const unsigned long timeout_ms)
{
    m_timeout_ms = timeout_ms;
}

unsigned long Timer::getRemainingInSecs() const
{
    if (m_isRunning && (m_hasExpired == false))
    {
        return ((m_timeout_ms - (millis() - m_start)) / 1000UL);
    }

    return 0UL;
}

bool Timer::isRunning() const
{
    return m_isRunning;
}

bool Timer::hasExpired()
{
    if (m_isRunning && (m_hasExpired == false))
    {
        if ((millis() - m_start) > m_timeout_ms)
        {
            m_hasExpired = true;
        }
    }

    return m_hasExpired;
}

void Timer::start()
{
    if (m_timeout_ms > 0UL)
    {
        m_start = millis();
        m_hasExpired = false;
        m_isRunning = true;
    }
}

void Timer::stop()
{
    m_hasExpired = false;
    m_isRunning = false;
}

