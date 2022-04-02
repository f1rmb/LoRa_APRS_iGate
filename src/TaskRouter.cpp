#include <logger.h>

#include <TimeLib.h>

#include "Tasks.h"
#include "TaskRouter.h"
#include "ProjectConfiguration.h"

RouterTask::RouterTask(TaskQueue<std::shared_ptr<APRSMessage>> &fromModem, TaskQueue<std::shared_ptr<APRSMessage>> &toModem, TaskQueue<std::shared_ptr<APRSMessage>> &toAprsIs, TaskQueue<std::shared_ptr<APRSMessage>> &toMQTT) :
Task(TASK_ROUTER, TaskRouter),
m_fromModem(fromModem),
m_toModem(toModem),
m_toAprsIs(toAprsIs),
m_toMQTT(toMQTT)
{
    m_visible = false; // Do not display its status on screen
}

RouterTask::~RouterTask()
{
}

bool RouterTask::setup(System &system)
{
    return true;
}

bool RouterTask::loop(System &system)
{
    if (!m_fromModem.empty())
    {
        std::shared_ptr<APRSMessage> modemMsg = m_fromModem.getElement();

        if (system.getUserConfig()->mqtt.active)
        {
            m_toMQTT.addElement(modemMsg);
        }

        if (system.getUserConfig()->aprs_is.active && (modemMsg->getSource() != system.getUserConfig()->callsign))
        {
            std::shared_ptr<APRSMessage> aprsIsMsg = std::make_shared<APRSMessage>(*modemMsg);
            String                       path      = aprsIsMsg->getPath();

            if (((path.indexOf("RFONLY") != -1) || (path.indexOf("NOGATE") != -1) || (path.indexOf("TCPIP") != -1)) == false)
            {
                if (path.isEmpty() == false)
                {
                    path += ",";
                }

                aprsIsMsg->setPath(path + "qAO," + system.getUserConfig()->callsign);

                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "APRS-IS: %s", aprsIsMsg->toString().c_str());
                m_toAprsIs.addElement(aprsIsMsg);
            }
            else
            {
                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "APRS-IS: no forward => RFonly");
            }
        }
        else
        {
            if (system.getUserConfig()->aprs_is.active == false)
            {
                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "APRS-IS: disabled");
            }

            if (modemMsg->getSource() == system.getUserConfig()->callsign)
            {
                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "APRS-IS: no forward => own packet received");
            }
        }

        if (system.getUserConfig()->digi.active && (modemMsg->getSource() != system.getUserConfig()->callsign))
        {
            std::shared_ptr<APRSMessage> digiMsg = std::make_shared<APRSMessage>(*modemMsg);
            String                       path    = digiMsg->getPath();

            // simple loop check
            if ((path.indexOf("WIDE1-1") >= 0) && (path.indexOf(system.getUserConfig()->callsign) == -1))
            {
                // fixme
                digiMsg->setPath(system.getUserConfig()->callsign + "*");

                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "DIGI: %s", digiMsg->toString().c_str());

                m_toModem.addElement(digiMsg);
            }
        }
    }

    m_stateInfo = "Running";
    return true;
}
