#include <logger.h>

#include <TimeLib.h>

#include "Tasks.h"
#include "TaskNTP.h"
#include "ProjectConfiguration.h"

NTPTask::NTPTask() :
Task(TASK_NTP, TaskNtp),
m_beginCalled(false),
m_lastEpochTime(0)
{
}

NTPTask::~NTPTask()
{
}

bool NTPTask::setup(System &system)
{
    m_ntpClient.setPoolServerName(system.getUserConfig()->ntpServer.c_str());
    return true;
}

bool NTPTask::loop(System &system)
{
    if (!system.isWifiEthConnected())
    {
        return false;
    }

    if (!m_beginCalled)
    {
        m_ntpClient.begin();
        m_beginCalled = true;
    }

    unsigned long epochTime = 0;
    if (m_ntpClient.update())
    {
        epochTime = m_ntpClient.getEpochTime();

        // Update clock on skew only
        time_t n;
        if ((n = now()) != time_t(epochTime))
        {
            setTime(time_t(epochTime));
            system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "Update current time: %s, diff %d", m_ntpClient.getFormattedTime().c_str(), int(n - epochTime));
        }
    }
    else
    {
        epochTime = m_ntpClient.getEpochTime();
    }

    if (epochTime != m_lastEpochTime)
    {
        m_lastEpochTime = epochTime;
        m_stateInfo     = m_ntpClient.getFormattedTime(m_lastEpochTime);
        m_state         = Okay;
    }

    return true;
}
