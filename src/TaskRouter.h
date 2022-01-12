#ifndef TASK_ROUTER_H_
#define TASK_ROUTER_H_

#include <APRSMessage.h>
#include <TaskManager.h>

class RouterTask : public Task
{
    public:
        RouterTask(TaskQueue<std::shared_ptr<APRSMessage>> &fromModem, TaskQueue<std::shared_ptr<APRSMessage>> &toModem, TaskQueue<std::shared_ptr<APRSMessage>> &toAprsIs);
        virtual ~RouterTask();

        virtual bool setup(System &system) override;
        virtual bool loop(System &system) override;

    private:
        TaskQueue<std::shared_ptr<APRSMessage>> &m_fromModem;
        TaskQueue<std::shared_ptr<APRSMessage>> &m_toModem;
        TaskQueue<std::shared_ptr<APRSMessage>> &m_toAprsIs;
        std::shared_ptr<APRSMessage>             m_beaconMsg;
        Timer                                    m_beacon_timer;
        bool                                     m_firstRun;
};

#endif
