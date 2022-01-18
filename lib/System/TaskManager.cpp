#include "TaskManager.h"
#include <FontConfig.h>
#include <logger.h>

TaskManager::TaskManager()
{
}

void TaskManager::addTask(Task *task)
{
    m_tasks.push_back(task);
}

void TaskManager::addAlwaysRunTask(Task *task)
{
    m_alwaysRunTasks.push_back(task);
}

std::list<Task *> TaskManager::getTasks()
{
    std::list<Task *> tasks = m_alwaysRunTasks;
    std::copy(m_tasks.begin(), m_tasks.end(), std::back_inserter(tasks));

    return tasks;
}

Task const *TaskManager::getTaskByName(const String &name) const
{
    for (Task *elem : m_alwaysRunTasks)
    {
        if (elem->getName() == name)
        {
            return (elem);
        }
    }

    for (Task *elem : m_tasks)
    {
        if (elem->getName() == name)
        {
            return (elem);
        }
    }

    return NULL;
}

Task const *TaskManager::getTaskById(TaskID_t id) const
{
    for (Task *elem : m_alwaysRunTasks)
    {
        if (elem->getTaskId() == id)
        {
            return (elem);
        }
    }

    for (Task *elem : m_tasks)
    {
        if (elem->getTaskId() == id)
        {
            return (elem);
        }
    }

    return NULL;
}


bool TaskManager::setup(System &system)
{
    logPrintlnV("will setup all tasks...");
    for (Task *elem : m_alwaysRunTasks)
    {
        logPrintD("call setup from ");
        logPrintlnD(elem->getName());
        elem->setup(system);
    }

    for (Task *elem : m_tasks)
    {
        logPrintD("call setup from ");
        logPrintlnD(elem->getName());
        elem->setup(system);
    }

    m_nextTask = m_tasks.begin();
    return true;
}

bool TaskManager::loop(System &system)
{
    // logPrintlnD("will loop all tasks...");
    for (Task *elem : m_alwaysRunTasks)
    {
        // logPrintD("call loop from ");
        // logPrintlnD(elem->getName());
        elem->loop(system);
    }

    if (m_nextTask == m_tasks.end())
    {
        m_nextTask = m_tasks.begin();
    }

    bool ret = (*m_nextTask)->loop(system);
    ++m_nextTask;

    return ret;
}

// cppcheck-suppress unusedFunction
void StatusFrame::drawStatusPage(Bitmap &bitmap)
{
    int y = 0;

    for (Task *task : m_tasks)
    {
        int x = bitmap.drawString(0, y, false, (task->getName()).substring(0, task->getName().indexOf("Task")));
        x     = bitmap.drawString(x, y, false, ":  ");

        if (task->getStateInfo().length() == 0)
        {
            switch (task->getState())
            {
                case Error:
                    bitmap.drawString(x, y, false, "Error");
                    break;
                case Warning:
                    bitmap.drawString(x, y, false, "Warning");
                default:
                    break;
            }
            bitmap.drawString(x, y, false, "Okay");
        }
        else
        {
            bitmap.drawString(x, y, false, task->getStateInfo());
        }

        y += (getSystemFont()->heightInPixel + 1);
    }
}
