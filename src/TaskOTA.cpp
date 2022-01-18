#include <logger.h>

#include "Tasks.h"
#include "TaskOTA.h"
#include "ProjectConfiguration.h"

OTATask::OTATask() :
Task(TASK_OTA, TaskOta),
m_beginCalled(false)
{
}

OTATask::~OTATask()
{
}

bool OTATask::setup(System &system)
{
    m_ota.onStart([&]()
    {
        String type;
        if (m_ota.getCommand() == U_FLASH)
        {
            type = "sketch";
        }
        else // U_SPIFFS
        {
            type = "filesystem";
        }
        logPrintlnI("Start updating " + type);
    })
    .onEnd([]()
    {
        logPrintlnI("");
        logPrintlnI("OTA End");
    })
    .onProgress([](unsigned int progress, unsigned int total)
    {
        logPrintI("Progress: ");
        logPrintI(String(progress / (total / 100)));
        logPrintlnI("%");
    })
    .onError([](ota_error_t error)
    {
        logPrintE("Error[");
        logPrintE(String(error));
        logPrintE("]: ");
        if (error == OTA_AUTH_ERROR)
        {
            logPrintlnE("Auth Failed");
        }
        else if (error == OTA_BEGIN_ERROR)
        {
            logPrintlnE("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR)
        {
            logPrintlnE("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
            logPrintlnE("Receive Failed");
        }
        else if (error == OTA_END_ERROR)
        {
            logPrintlnE("End Failed");
        }
    });

    if (system.getUserConfig()->network.hostname.overwrite)
    {
        m_ota.setHostname(system.getUserConfig()->network.hostname.name.c_str());
    }
    else
    {
        m_ota.setHostname(system.getUserConfig()->callsign.c_str());
    }

    m_stateInfo = "";
    return true;
}

bool OTATask::loop(System &system) {

    if (!m_beginCalled)
    {
        m_ota.begin();
        m_beginCalled = true;
    }

    m_ota.handle();
    return true;
}
