#include <logger.h>

#include <TimeLib.h>

#include "Task.h"
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
        setTime((epochTime = m_ntpClient.getEpochTime()));
        logPrintI("Current time: ");
        logPrintlnI(m_ntpClient.getFormattedTime(epochTime));
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
