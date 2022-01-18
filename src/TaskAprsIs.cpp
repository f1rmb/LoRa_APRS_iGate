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

bool AprsIsTask::connect(const System &system)
{
    logPrintI("connecting to APRS-IS server: ");
    logPrintI(system.getUserConfig()->aprs_is.server);
    logPrintI(" on port: ");
    logPrintlnI(String(system.getUserConfig()->aprs_is.port));

    if (!m_aprs_is.connect(system.getUserConfig()->aprs_is.server, system.getUserConfig()->aprs_is.port))
    {
        logPrintlnE("Connection failed.");
        return false;
    }

    logPrintlnI("Connected to APRS-IS server!");
    return true;
}
