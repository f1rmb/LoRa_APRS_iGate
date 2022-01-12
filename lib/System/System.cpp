
#include "System.h"

System::System() :
m_boardConfig(0),
m_userConfig(0),
m_isWifiEthConnected(false)
{
}

System::~System()
{
}

void System::setBoardConfig(BoardConfig const *const boardConfig)
{
    m_boardConfig = boardConfig;
}

void System::setUserConfig(Configuration const *const userConfig)
{
    m_userConfig = userConfig;
}

BoardConfig const *const System::getBoardConfig() const
{
    return m_boardConfig;
}

Configuration const *const System::getUserConfig() const
{
    return m_userConfig;
}

TaskManager &System::getTaskManager()
{
    return m_taskManager;
}

Display &System::getDisplay()
{
    return m_display;
}

bool System::isWifiEthConnected() const
{
    return m_isWifiEthConnected;
}

void System::connectedViaWifiEth(bool status)
{
    m_isWifiEthConnected = status;
}
