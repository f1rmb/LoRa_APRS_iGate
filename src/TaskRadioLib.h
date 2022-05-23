#ifndef TASK_RADIOLIB_H_
#define TASK_RADIOLIB_H_

#include <APRS-Decoder.h>
#include <BoardFinder.h>
#include <RadioLib.h>
#include <TaskManager.h>

#include "ProjectConfiguration.h"

class RadioLibTask : public Task
{
    public:
        explicit RadioLibTask(TaskQueue<std::shared_ptr<APRSMessage>> &fromModem, TaskQueue<std::shared_ptr<APRSMessage>> &_toModem);
        virtual ~RadioLibTask();

        virtual bool setup(System &system) override;
        virtual bool loop(System &system) override;

        void shutdown();

    private:
        static void setFlag(void);
        int16_t startRX(uint8_t mode);
        int16_t startTX(String &str);

    private:
        Module                                 *m_module;
        SX1278                                 *m_radio;
        Configuration::LoRa                     m_config;
        bool                                    m_rxEnable;
        bool                                    m_txEnable;
        TaskQueue<std::shared_ptr<APRSMessage>> &m_fromModem;
        TaskQueue<std::shared_ptr<APRSMessage>> &m_toModem;
        static volatile bool                    m_enableInterrupt; // Need to catch interrupt or not.
        static volatile bool                    m_operationDone;   // Caught IRQ or not.
        unsigned long                           m_preambleDurationMilliSec;
        Timer                                   m_txWaitTimer;
};

#endif
