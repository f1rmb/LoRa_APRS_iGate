#ifndef TASK_MANAGER_H_
#define TASK_MANAGER_H_

#include <Arduino.h>
#include <list>
#include <memory>

#include <logger.h>

#include <BoardFinder.h>
#include <Display.h>
#include <configuration.h>

#include "TaskQueue.h"

class System;

typedef uint8_t TaskID_t;

enum TaskDisplayState
{
    Error,
    Warning,
    Okay,
};

class Task
{
    public:
        Task(String &name, TaskID_t taskId) :
            m_enabled(true),
            m_state(Okay),
            m_stateInfo("Booting"),
            m_visible(true),
            m_name(name),
            m_taskId(taskId)
        {
        }
        Task(const char *name, TaskID_t taskId) :
            m_enabled(true),
            m_state(Okay),
            m_stateInfo("Booting"),
            m_visible(true),
            m_name(name),
            m_taskId(taskId)
        {
        }
        virtual ~Task()
        {
        }

        bool isEnabled()
        {
            return m_enabled;
        }

        virtual void enable(bool value)
        {
            m_enabled = value;
        }

        String getName() const
        {
            return m_name;
        }

        TaskID_t getTaskId() const
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

        bool isVisible()
        {
            return m_visible;
        }

        virtual bool setup(System &system) = 0;
        virtual bool loop(System &system)  = 0;

    protected:
        bool             m_enabled;
        TaskDisplayState m_state;
        String           m_stateInfo;
        bool             m_visible;

    private:
        String           m_name;
        TaskID_t         m_taskId;
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
        Task const       *getTaskByName(const String &name) const;
        Task const       *getTaskById(TaskID_t id) const;

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
