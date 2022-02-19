#include <logger.h>

#include <TimeLib.h>

#include "Tasks.h"
#include "TaskRouter.h"
#include "ProjectConfiguration.h"
#include "Deg2DDMMMM.h"

RouterTask::RouterTask(TaskQueue<std::shared_ptr<APRSMessage>> &fromModem, TaskQueue<std::shared_ptr<APRSMessage>> &toModem, TaskQueue<std::shared_ptr<APRSMessage>> &toAprsIs) :
Task(TASK_ROUTER, TaskRouter),
m_fromModem(fromModem),
m_toModem(toModem),
m_toAprsIs(toAprsIs),
m_forceBeaconing(true)
{
}

RouterTask::~RouterTask()
{
}

bool RouterTask::setup(System &system)
{
    // setup beacon
    m_beacon_timer.setTimeout(system.getUserConfig()->beacon.timeout * 60 * 1000);

    m_beaconMsg = std::shared_ptr<APRSMessage>(new APRSMessage());
    m_beaconMsg->setSource(system.getUserConfig()->callsign);
    m_beaconMsg->setDestination("APLG01");
    updatePosition(system, system.getUserConfig()->beacon.positionLatitude, system.getUserConfig()->beacon.positionLongitude);

    return true;
}

bool RouterTask::loop(System &system)
{
    // do routing
    if (!m_fromModem.empty())
    {
        std::shared_ptr<APRSMessage> modemMsg = m_fromModem.getElement();

        if (system.getUserConfig()->aprs_is.active && modemMsg->getSource() != system.getUserConfig()->callsign)
        {
            std::shared_ptr<APRSMessage> aprsIsMsg = std::make_shared<APRSMessage>(*modemMsg);
            String                       path      = aprsIsMsg->getPath();

            if (((path.indexOf("RFONLY") != -1) || (path.indexOf("NOGATE") != -1) || (path.indexOf("TCPIP") != -1)) == false)
            {
                if (!path.isEmpty())
                {
                    path += ",";
                }

                aprsIsMsg->setPath(path + "qAO," + system.getUserConfig()->callsign);

                logPrintD("APRS-IS: ");
                logPrintlnD(aprsIsMsg->toString());
                m_toAprsIs.addElement(aprsIsMsg);
            }
            else
            {
                logPrintlnD("APRS-IS: no forward => RFonly");
            }
        }
        else
        {
            if (!system.getUserConfig()->aprs_is.active)
            {
                logPrintlnD("APRS-IS: disabled");
            }

            if (modemMsg->getSource() == system.getUserConfig()->callsign)
            {
                logPrintlnD("APRS-IS: no forward => own packet received");
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

                logPrintD("DIGI: ");
                logPrintlnD(digiMsg->toString());

                m_toModem.addElement(digiMsg);
            }
        }
    }

    // check for beacon
    if (m_forceBeaconing || m_beacon_timer.hasExpired())
    {
        m_forceBeaconing = false;
        logPrintD("[" + timeString() + "] ");
        logPrintlnD(m_beaconMsg->encode());

        if (system.getUserConfig()->aprs_is.active)
        {
            m_toAprsIs.addElement(m_beaconMsg);
        }

        if (system.getUserConfig()->digi.beacon)
        {
            m_toModem.addElement(m_beaconMsg);
        }

        system.getDisplay().addFrame(std::shared_ptr<DisplayFrame>(new TextFrame("BEACON", m_beaconMsg->encode())));

        m_beacon_timer.start();
    }

    unsigned long diff = m_beacon_timer.getRemainingInSecs();
    m_stateInfo = "beacon " + String(uint32_t(diff / 600)) + String(uint32_t(diff / 60) % 10) + ":" + String(uint32_t(diff / 10) % 6) + String(uint32_t(diff % 10));

    return true;
}

void RouterTask::updatePosition(System &system, double latitude, double longitude)
{
    Deg2DDMMMMPosition pLat, pLong;
    char               latBuf[32], longBuf[32];

    Deg2DDMMMM::Convert(pLat, latitude, false);
    Deg2DDMMMM::Convert(pLong, longitude, false);
    m_beaconMsg->getBody()->setData(String("=") + Deg2DDMMMM::Format(latBuf, pLat, false) + "L" + Deg2DDMMMM::Format(longBuf, pLong, true) + "&" + system.getUserConfig()->beacon.message);
    logPrintlnD("Updated coords: " + String(latitude, 6) + " " + String(longitude, 6));

    // force beaconing
    if (m_forceBeaconing == false)
    {
        m_forceBeaconing = true;
    }
}
