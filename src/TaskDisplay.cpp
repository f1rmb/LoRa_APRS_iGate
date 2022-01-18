#include <logger.h>

#include "Tasks.h"
#include "TaskDisplay.h"
#include "ProjectConfiguration.h"

DisplayTask::DisplayTask() :
Task(TASK_DISPLAY, TaskDisplay)
{
}

DisplayTask::~DisplayTask()
{
}

bool DisplayTask::setup(System &system)
{
    system.getDisplay().setup(system.getBoardConfig(), system.getUserConfig()->display.turn180);
    std::shared_ptr<StatusFrame> statusFrame = std::shared_ptr<StatusFrame>(new StatusFrame(system.getTaskManager().getTasks()));
    system.getDisplay().setStatusFrame(statusFrame);

    if (!system.getUserConfig()->display.alwaysOn)
    {
        system.getDisplay().activateDisplaySaveMode();
        system.getDisplay().setDisplaySaveTimeout(system.getUserConfig()->display.timeout);
    }

    m_stateInfo = system.getUserConfig()->callsign;
    return true;
}

bool DisplayTask::loop(System &system)
{
    if (system.getUserConfig()->display.overwritePin != 0 && !digitalRead(system.getUserConfig()->display.overwritePin))
    {
        system.getDisplay().activateDisplay();
    }

    system.getDisplay().update();
    return true;
}
