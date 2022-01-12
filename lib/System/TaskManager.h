#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

#include <Arduino.h>
#include <list>
#include <memory>

#include <BoardFinder.h>
#include <Display.h>
#include <configuration.h>

#include "TaskQueue.h"

class System;

enum TaskDisplayState
{
    Error,
    Warning,
    Okay,
};

class Task
{
    public:
        Task(String &name, int taskId) :
            m_state(Okay),
            m_stateInfo("Booting"),
            m_name(name),
            m_taskId(taskId)
        {
        }
        Task(const char *name, int taskId) :
            m_state(Okay),
            m_stateInfo("Booting"),
            m_name(name),
            m_taskId(taskId)
        {
        }
        virtual ~Task()
        {
        }

        String getName() const
        {
            return m_name;
        }
        int getTaskId() const
        {
            return m_taskId;
        }

        TaskDisplayState getState() const
        {
            return m_state;
        }
        String getStateInfo() const
        {
            return m_stateInfo;
        }

        virtual bool setup(System &system) = 0;
        virtual bool loop(System &system)  = 0;

    protected:
        TaskDisplayState m_state;
        String           m_stateInfo;

    private:
        String m_name;
        int    m_taskId;
};

class TaskManager
{
    public:
        TaskManager();
        ~TaskManager()
        {
        }

        void              addTask(Task *task);
        void              addAlwaysRunTask(Task *task);
        std::list<Task *> getTasks();

        bool setup(System &system);
        bool loop(System &system);

    private:
        std::list<Task *>           m_tasks;
        std::list<Task *>::iterator m_nextTask;
        std::list<Task *>           m_alwaysRunTasks;
};

class StatusFrame : public DisplayFrame
{
    public:
        explicit StatusFrame(const std::list<Task *> &tasks) : m_tasks(tasks)
        {
        }
        virtual ~StatusFrame()
        {
        }
        void drawStatusPage(Bitmap &bitmap) override;

    private:
        std::list<Task *> m_tasks;
};

#include "System.h"

#endif
