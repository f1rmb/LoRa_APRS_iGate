#include <WiFi.h>
#include <logger.h>

#include "Tasks.h"
#include "TaskEth.h"
#include "TaskWifi.h"
#include "TaskRouter.h"
#include "ProjectConfiguration.h"

extern RouterTask routerTask;

WifiTask::WifiTask() :
Task(TASK_WIFI, TaskWifi),
m_oldWifiStatus(WL_IDLE_STATUS)
{
}

WifiTask::~WifiTask()
{
}

bool WifiTask::setup(System &system)
{
    // Don't save WiFi configuration in flash
    WiFi.persistent(false);

    // Set WiFi to station mode
    WiFi.mode(WIFI_STA);

    WiFi.onEvent(WiFiEvent);
    if (system.getUserConfig()->network.hostname.overwrite)
    {
        WiFi.setHostname(system.getUserConfig()->network.hostname.name.c_str());
    }
    else
    {
        WiFi.setHostname(system.getUserConfig()->callsign.c_str());
    }

    if (!system.getUserConfig()->network.DHCP)
    {
        WiFi.config(system.getUserConfig()->network.static_.ip, system.getUserConfig()->network.static_.gateway, system.getUserConfig()->network.static_.subnet, system.getUserConfig()->network.static_.dns1, system.getUserConfig()->network.static_.dns2);
    }

    for (Configuration::Wifi::AP ap : system.getUserConfig()->wifi.APs)
    {
        logPrintD("Looking for AP: ");
        logPrintlnD(ap.SSID);
        m_wiFiMulti.addAP(ap.SSID.c_str(), ap.password.c_str());
    }

    return true;
}

void WifiTask::enable(bool value)
{
    Task::enable(value);

    if (value == false)
    {
        WiFi.mode(WIFI_OFF);
    }
}

bool WifiTask::loop(System &system)
{
    if (m_enabled == false)
    {
        return true;
    }

    String oldSSID = WiFi.SSID();
    const uint8_t wifi_status = m_wiFiMulti.run();
    bool ssidHasChanged = (WiFi.SSID() != oldSSID);

    if (wifi_status != WL_CONNECTED)
    {
        system.connectedViaWifiEth(false);

        // Set position back to global coordinates
        if (system.getUserConfig()->wifi.active && (m_oldWifiStatus == WL_CONNECTED))
        {
            routerTask.updatePosition(system, system.getUserConfig()->beacon.positionLatitude, system.getUserConfig()->beacon.positionLongitude);
        }

        logPrintlnE("WiFi not connected!");
        m_oldWifiStatus = wifi_status;
        m_stateInfo     = "WiFi not connected";
        m_state         = Error;
        return false;
    }
    else if ((wifi_status != m_oldWifiStatus) || ssidHasChanged)
    {
        uint8_t prevStatus = m_oldWifiStatus;

        logPrintD("IP address: ");
        logPrintlnD(WiFi.localIP().toString());
        m_oldWifiStatus = wifi_status;

        if (((prevStatus != WL_CONNECTED) && (wifi_status == WL_CONNECTED)) || ssidHasChanged)
        {
            system.connectedViaWifiEth(true);
            m_stateInfo = WiFi.localIP().toString();
            m_state     = Okay;

            // Update position, accordingly to connected AP
            if (system.getUserConfig()->wifi.active)
            {
                for (Configuration::Wifi::AP ap : system.getUserConfig()->wifi.APs)
                {
                    if (ap.SSID == WiFi.SSID())
                    {
                        routerTask.updatePosition(system, ap.positionLatitude, ap.positionLongitude);
                        break;
                    }
                }
            }

            return true;
        }

        return false;
    }

    return true;
}
