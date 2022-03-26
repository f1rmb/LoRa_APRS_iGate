#include <FTPFilesystem.h>
#include <SPIFFS.h>
#include <logger.h>

#include "Tasks.h"
#include "TaskFTP.h"
#include "ProjectConfiguration.h"

FTPTask::FTPTask() :
Task(TASK_FTP, TaskFtp),
m_beginCalled(false)
{
}

FTPTask::~FTPTask()
{
}

bool FTPTask::setup(System &system)
{
    for (Configuration::Ftp::User user : system.getUserConfig()->ftp.users)
    {
        system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, getName(), "Adding user to FTP Server: %s", user.name.c_str());
        m_ftpServer.addUser(user.name, user.password);
    }

    m_ftpServer.addFilesystem("SPIFFS", &SPIFFS);
    m_stateInfo = "waiting";
    return true;
}

bool FTPTask::loop(System &system)
{
    if (!m_beginCalled)
    {
        m_ftpServer.begin();
        m_beginCalled = true;
    }

    m_ftpServer.handle();

    static bool configWasOpen = false;
    if (configWasOpen && m_ftpServer.countConnections() == 0)
    {
        system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_WARN, getName(), "Maybe the config has been changed via FTP, lets restart now to get the new config...");
        ESP.restart();
    }

    if (m_ftpServer.countConnections() > 0)
    {
        configWasOpen = true;
        m_stateInfo    = "has connection";
    }

    return true;
}
