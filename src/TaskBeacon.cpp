#include <logger.h>

#include <OneButton.h>
#include <TimeLib.h>

#include "Tasks.h"
#include "TaskBeacon.h"
#include "ProjectConfiguration.h"
#include "Deg2DDMMMM.h"

OneButton BeaconTask::m_userButton;
bool      BeaconTask::m_send_update;
uint      BeaconTask::m_instances;


BeaconTask::BeaconTask(TaskQueue<std::shared_ptr<APRSMessage>> &toModem, TaskQueue<std::shared_ptr<APRSMessage>> &toAprsIs) :
Task(TASK_BEACON, TaskBeacon),
m_toModem(toModem),
m_toAprsIs(toAprsIs),
m_forceBeaconing(false),
m_ss(1),
m_useGps(false)
{
}

BeaconTask::~BeaconTask()
{
}

void BeaconTask::pushButton()
{
    m_send_update = true;
}

bool BeaconTask::setup(System &system)
{
    if ((m_instances++ == 0) && (system.getBoardConfig()->Button > 0))
    {
        m_userButton = OneButton(system.getBoardConfig()->Button, true, true);
        m_userButton.attachClick(pushButton);
        m_send_update = false;
    }

    if ((m_useGps = system.getUserConfig()->beacon.use_gps))
    {
        if (system.getBoardConfig()->GpsRx != 0)
        {
            m_ss.begin(9600, SERIAL_8N1, system.getBoardConfig()->GpsTx, system.getBoardConfig()->GpsRx);
        }
        else
        {
            system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "NO GPS found.");
            m_useGps = false;
        }
    }

    // setup beacon
    m_beacon_timer.setTimeout(system.getUserConfig()->beacon.timeout * 60 * 1000);

    m_beaconMsg = std::shared_ptr<APRSMessage>(new APRSMessage());
    m_beaconMsg->setSource(system.getUserConfig()->callsign);
    m_beaconMsg->setDestination("APLG01");
    updatePosition(system, system.getUserConfig()->beacon.positionLatitude, system.getUserConfig()->beacon.positionLongitude);

    return true;
}

bool BeaconTask::loop(System &system)
{
    if (m_useGps)
    {
        while (m_ss.available() > 0)
        {
            char c = m_ss.read();
            m_gps.encode(c);
        }
    }

    m_userButton.tick();

    // check for beacon
    if (m_forceBeaconing || m_beacon_timer.hasExpired() || m_send_update)
    {
        if (sendBeacon(system))
        {
            m_forceBeaconing = false;
            m_send_update = false;
            m_beacon_timer.start();
        }
    }

    String mqttState = "";
    if (system.getUserConfig()->mqtt.active)
    {
        MQTTTask *mqTask = (MQTTTask *)system.getTaskManager().getTaskById(TaskMQTT);

        mqttState = ((mqTask->getState() == Okay) ? "M," : "m,");
    }

    String gpsState = "";
    if (m_useGps)
    {
        gpsState = "G,";
    }

    unsigned long diff = m_beacon_timer.getRemainingInSecs();
    m_stateInfo = mqttState + gpsState + ("bcn: " + String(uint32_t(diff / 600)) + String(uint32_t(diff / 60) % 10) + ":" + String(uint32_t(diff / 10) % 6) + String(uint32_t(diff % 10)));

    return true;
}

bool BeaconTask::updatePosition(System &system, double latitude, double longitude)
{
    Deg2DDMMMMPosition pLat, pLong;
    char               latBuf[32], longBuf[32];

    if (m_useGps)
    {
        if (m_gps.location.isUpdated())
        {
            latitude = m_gps.location.lat();
            longitude = m_gps.location.lng();
        }
        else
        {
            return false;
        }
    }

    Deg2DDMMMM::Convert(pLat, latitude, false);
    Deg2DDMMMM::Convert(pLong, longitude, false);
    m_beaconMsg->getBody()->setData(String("=") + Deg2DDMMMM::Format(latBuf, pLat, false) + "L" + Deg2DDMMMM::Format(longBuf, pLong, true) + "&" + system.getUserConfig()->beacon.message);
    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, getName(), "Updated coords: %f %f", latitude, longitude);

    // force beaconing
    if (m_forceBeaconing == false)
    {
        m_forceBeaconing = true;
    }

    return true;
}

bool BeaconTask::sendBeacon(System &system)
{
    if (updatePosition(system, system.getUserConfig()->beacon.positionLatitude, system.getUserConfig()->beacon.positionLongitude) == false)
    {
        return false;
    }

    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "[%s] %s", timeString().c_str(), m_beaconMsg->encode().c_str());

    if (system.getUserConfig()->aprs_is.active)
    {
        m_toAprsIs.addElement(m_beaconMsg);
    }

    if (system.getUserConfig()->digi.beacon)
    {
        m_toModem.addElement(m_beaconMsg);
    }

    system.getDisplay().addFrame(std::shared_ptr<DisplayFrame>(new TextFrame("BEACON", m_beaconMsg->encode())));

    return true;
}
