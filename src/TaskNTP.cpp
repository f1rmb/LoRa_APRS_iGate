#include <logger.h>

#include <TimeLib.h>

#include "Task.h"
#include "TaskNTP.h"
#include "project_configuration.h"

NTPTask::NTPTask() :
Task(TASK_NTP, TaskNtp),
m_beginCalled(false)
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

    if (m_ntpClient.update())
    {
        setTime(m_ntpClient.getEpochTime());
        logPrintI("Current time: ");
        logPrintlnI(m_ntpClient.getFormattedTime());
    }

    m_stateInfo = m_ntpClient.getFormattedTime();
    m_state     = Okay;
    return true;
}
