#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <logger.h>
#include <memory>

#include "TaskManager.h"
#include <BoardFinder.h>
#include <Display.h>
#include <configuration.h>

class System
{
    public:
        System();
        ~System();

        void setBoardConfig(BoardConfig const *const boardConfig);
        void setUserConfig(Configuration const *const userConfig);

        BoardConfig const *const   getBoardConfig() const;
        Configuration const *const getUserConfig() const;
        TaskManager               &getTaskManager();
        Display                   &getDisplay();
        bool                       isWifiEthConnected() const;
        void                       connectedViaWifiEth(bool status);
        logging::Logger           &getLogger();

    private:
        BoardConfig const   *m_boardConfig;
        Configuration const *m_userConfig;
        TaskManager          m_taskManager;
        Display              m_display;
        bool                 m_isWifiEthConnected;
        logging::Logger      m_logger;
};

#endif
