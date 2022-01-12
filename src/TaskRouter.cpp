#include <logger.h>

#include <TimeLib.h>

#include "Task.h"
#include "TaskRouter.h"
#include "project_configuration.h"

String create_lat_aprs(double lat);
String create_long_aprs(double lng);

RouterTask::RouterTask(TaskQueue<std::shared_ptr<APRSMessage>> &fromModem, TaskQueue<std::shared_ptr<APRSMessage>> &toModem, TaskQueue<std::shared_ptr<APRSMessage>> &toAprsIs) :
Task(TASK_ROUTER, TaskRouter),
m_fromModem(fromModem),
m_toModem(toModem),
m_toAprsIs(toAprsIs)
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
    String lat = create_lat_aprs(system.getUserConfig()->beacon.positionLatitude);
    String lng = create_long_aprs(system.getUserConfig()->beacon.positionLongitude);
    m_beaconMsg->getBody()->setData(String("=") + lat + "L" + lng + "&" + system.getUserConfig()->beacon.message);

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

                aprsIsMsg->setPath(path + "qAR," + system.getUserConfig()->callsign);

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
    if (m_beacon_timer.hasExpired())
    {
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

        system.getDisplay().addFrame(std::shared_ptr<DisplayFrame>(new TextFrame("BEACON", m_beaconMsg->toString())));

        m_beacon_timer.start();
    }

    unsigned long diff = m_beacon_timer.getRemainingInSecs();
    m_stateInfo = "beacon " + String(uint32_t(diff / 600)) + String(uint32_t(diff / 60) % 10) + ":" + String(uint32_t(diff / 10) % 6) + String(uint32_t(diff % 10));

    return true;
}
