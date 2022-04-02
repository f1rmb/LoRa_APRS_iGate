#ifndef TASK_ROUTER_H_
#define TASK_ROUTER_H_

#include <APRSMessage.h>
#include <TaskMQTT.h>
#include <TaskManager.h>

class RouterTask : public Task
{
    public:
        RouterTask(TaskQueue<std::shared_ptr<APRSMessage>> &fromModem, TaskQueue<std::shared_ptr<APRSMessage>> &toModem, TaskQueue<std::shared_ptr<APRSMessage>> &toAprsIs, TaskQueue<std::shared_ptr<APRSMessage>> &toMQTT);
        virtual ~RouterTask();

        virtual bool setup(System &system) override;
        virtual bool loop(System &system) override;

    private:
        TaskQueue<std::shared_ptr<APRSMessage>> &m_fromModem;
        TaskQueue<std::shared_ptr<APRSMessage>> &m_toModem;
        TaskQueue<std::shared_ptr<APRSMessage>> &m_toAprsIs;
        TaskQueue<std::shared_ptr<APRSMessage>> &m_toMQTT;
};

#endif
