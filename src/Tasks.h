#ifndef TASKS_H_
#define TASKS_H_

#include "TaskManager.h"

enum TaskIDs : TaskID_t
{
    TaskAprsIs = 1,
    TaskEth,
    TaskFtp,
    TaskRadioLib,
    TaskNtp,
    TaskOta,
    TaskWifi,
    TaskRouter,
    TaskDisplay,
    TaskBatterySurvey,
    TaskMQTT,
    TaskBeacon,
    TaskSize,
};

#define TASK_APRS_IS        "AprsIsTask"
#define TASK_ETH            "EthTask"
#define TASK_FTP            "FTPTask"
#define TASK_RADIOLIB       "RadioLibTask"
#define TASK_NTP            "NTPTask"
#define TASK_OTA            "OTATask"
#define TASK_WIFI           "WifiTask"
#define TASK_ROUTER         "RouterTask"
#define TASK_DISPLAY        "DisplayTask"
#define TASK_BATTERY_SURVEY "BattTask"
#define TASK_MQTT           "MQTTTask"
#define TASK_BEACON         "BeaconTask"

#endif
