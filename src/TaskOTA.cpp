#include <functional>

#include <logger.h>

#include "Tasks.h"
#include "TaskOTA.h"
#include "TaskWifi.h"
#include "ProjectConfiguration.h"

OTATask::OTATask() :
Task(TASK_OTA, TaskOta),
m_beginCalled(false)
{
}

OTATask::~OTATask()
{
}

void OTATask::onStart(System &system)
{
    String type;
    WifiTask *wifi = (WifiTask *)system.getTaskManager().getTaskById(TaskWifi);

    wifi->enable(false);

    switch (m_ota.getCommand())
    {
        case U_FLASH:
            type = "sketch";
            break;

        case U_SPIFFS:
            type = "filesystem";
            break;

        case U_AUTH:
            type = "auth";
            break;

        default:
            break;
    }
    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "Start updating %s", type.c_str());
}

void OTATask::onEnd(System &system)
{
    WifiTask *wifi = (WifiTask *)system.getTaskManager().getTaskById(TaskWifi);

    wifi->enable(true);
    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "OTA End");
}

void OTATask::onProgress(System &system, unsigned int progress, unsigned int total)
{
    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "Progress: %f%%", (progress / (total / 100)));
}

void OTATask::onError(System &system, ota_error_t error)
{
    WifiTask *wifi = (WifiTask *)system.getTaskManager().getTaskById(TaskWifi);
    String errStr;

    wifi->enable(true);

    switch (error)
    {
        case OTA_AUTH_ERROR:
            errStr = "Auth Failed";
            break;

        case OTA_BEGIN_ERROR:
            errStr = "Begin Failed";
            break;

        case OTA_CONNECT_ERROR:
            errStr = "Connect Failed";
            break;

        case OTA_RECEIVE_ERROR:
            errStr = "Receive Failed";
            break;

        case OTA_END_ERROR:
            errStr = "End Failed";
            break;
    }
    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "Error[%d]: %s", error, errStr.c_str());
}

bool OTATask::setup(System &system)
{
    m_ota.onStart(std::bind(&OTATask::onStart, this, system));
    m_ota.onEnd(std::bind(&OTATask::onEnd, this, system));
    m_ota.onProgress(std::bind(&OTATask::onProgress, this, system, std::placeholders::_1, std::placeholders::_2));
    m_ota.onError(std::bind(&OTATask::onError, this, system, std::placeholders::_1));

    if (system.getUserConfig()->network.hostname.overwrite)
    {
        m_ota.setHostname(system.getUserConfig()->network.hostname.name.c_str());
    }
    else
    {
        m_ota.setHostname(system.getUserConfig()->callsign.c_str());
    }

    m_ota.setPort(3232);

    m_stateInfo = "Running";
    m_state = Okay;
    return true;
}

bool OTATask::loop(System &system)
{
    if (m_beginCalled == false)
    {
        m_ota.begin();
        m_beginCalled = true;
    }

    m_ota.handle();
    return true;
}
