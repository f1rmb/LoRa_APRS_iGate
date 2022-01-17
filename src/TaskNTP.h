#ifndef TASK_NTP_H_
#define TASK_NTP_H_

#include <NTPClient.h>
#include <TaskManager.h>

class NTPTask : public Task
{
    public:
        NTPTask();
        virtual ~NTPTask();

        virtual bool setup(System &system) override;
        virtual bool loop(System &system) override;

    private:
        NTPClient      m_ntpClient;
        bool           m_beginCalled;
        unsigned long  m_lastEpochTime;
};

#endif
