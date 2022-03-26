#include <logger.h>

#include "Tasks.h"
#include "TaskAprsIs.h"
#include "ProjectConfiguration.h"

AprsIsTask::AprsIsTask(TaskQueue<std::shared_ptr<APRSMessage>> &toAprsIs) :
Task(TASK_APRS_IS, TaskAprsIs),
m_toAprsIs(toAprsIs)
{
}

AprsIsTask::~AprsIsTask()
{
}

bool AprsIsTask::setup(System &system)
{
    m_aprs_is.setup(system.getUserConfig()->callsign, system.getUserConfig()->aprs_is.passcode, "ESP32-APRS-IS", "0.2");
    return true;
}

bool AprsIsTask::loop(System &system)
{
    if (!system.isWifiEthConnected())
    {
        return false;
    }

    if (!m_aprs_is.connected())
    {
        if (!connect(system))
        {
            m_stateInfo = "not connected";
            m_state     = Error;
            return false;
        }

        m_stateInfo = "connected";
        m_state     = Okay;
        return false;
    }

    m_aprs_is.getAPRSMessage();

    if (!m_toAprsIs.empty())
    {
        std::shared_ptr<APRSMessage> msg = m_toAprsIs.getElement();
        m_aprs_is.sendMessage(msg);
    }

    return true;
}

bool AprsIsTask::connect(System &system)
{
    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "connecting to APRS-IS server: %s on port: %d", system.getUserConfig()->aprs_is.server.c_str(), system.getUserConfig()->aprs_is.port);
    APRS_IS::ConnectionStatus status = m_aprs_is.connect(system.getUserConfig()->aprs_is.server, system.getUserConfig()->aprs_is.port);
    if (status == APRS_IS::ERROR_CONNECTION)
    {
        system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "Something went wrong on connecting! Is the server reachable?");
        system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "Connection failed.");
        return false;
    }
    else if (status == APRS_IS::ERROR_PASSCODE)
    {
        system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "User can not be verified with passcode!");
        system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "Connection failed.");
        return false;
    }

    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "Connected to APRS-IS server!");
    return true;
}
