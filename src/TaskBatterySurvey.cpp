#include <logger.h>
#include <driver/rtc_io.h>
#include <WiFi.h>
#include <Wire.h>

#include "Tasks.h"
#include "TaskBatterySurvey.h"
#include "TaskModem.h"
#include "TaskWifi.h"
#include "ProjectConfiguration.h"
#include "power_management.h"

#define READING_INTERVAL                5000U
#define BATTERY_SENSE_RESOLUTION_BITS   10
#define ADC_MULTIPLIER                  2.0
#define AREF_VOLTAGE                    3.3


BatterySurveyTask::BatterySurveyTask() :
Task(TASK_BATTERY_SURVEY, TaskBatterySurvey),
m_board(NULL),
m_batteryPin(0),
//m_batteryIsConnected(false),
//m_batteryIsCharging(false),
m_lastBatteryReading(0)
{
    m_visible = false; // Do not display its status on screen
}

BatterySurveyTask::~BatterySurveyTask()
{
}

bool BatterySurveyTask::setup(System &system)
{
    m_board = system.getBoardConfig();
    m_stateInfo = "Not Ready";

    if (m_board->BattPin > 0U)
    {
        m_batteryPin = m_board->BattPin;

        // Initialize ADC
        pinMode(m_batteryPin, INPUT);
        adcAttachPin(m_batteryPin);
        analogReadResolution(BATTERY_SENSE_RESOLUTION_BITS);
        updateVoltageReading(system);
        m_stateInfo = "Running";
    }

    m_state = Okay;
    return true;
}

bool BatterySurveyTask::loop(System &system)
{
    if (m_batteryPin > 0)
    {
        updateVoltageReading(system);
    }

    return true;
}

float BatterySurveyTask::getBatteryVoltage()
{
    return (m_average.GetValue() * 1e-3);
}

//bool BatterySurveyTask::getBatteryIsConnected()
//{
//    return m_batteryIsConnected;
//}
//
//bool BatterySurveyTask::getBatteryIsCharging()
//{
//    return m_batteryIsCharging;
//}

void BatterySurveyTask::updateVoltageReading(System &system)
{
    if ((m_batteryPin > 0) && ((m_lastBatteryReading == 0) || ((millis() - m_lastBatteryReading) > READING_INTERVAL)))
    {
        uint16_t adcValue = analogRead(m_batteryPin);

        m_average.StackValue(uint16_t(nearbyint(1000.0 * ADC_MULTIPLIER * (AREF_VOLTAGE / 1024.0) * adcValue)));

        uint16_t averageVoltage = m_average.GetValue();
        //char buffer[128];
        //sprintf(buffer, "ADC: %u averageV: %umV %.2fV", adcValue, averageVoltage, getBatteryVoltage());
        //logPrintlnD(buffer);

        if (m_board->Type != eTTGO_T_Beam_V1_0) // Cards without an AXP
        {
            // Critical voltage => shutdown
            if (averageVoltage < FLAT_BATTERY_VOLTAGE)
            {
                static const uint8_t rtcGpios[] =
                {
                        /* 0, */ 2, /* 4, */
                        13, /* 14, */ /* 15, */
                        /* 25, */ 26, /* 27, */
                        32, 33, 34, 35, 36, 37 /*, 38, 39 */
                };
                ModemTask *modem = (ModemTask *)system.getTaskManager().getTaskById(TaskModem);
                if (modem)
                {
                    modem->shutdown(); // Shutdown the LoRa module
                }

                system.getDisplay().deactivateDisplay();

                WifiTask *wifi = (WifiTask *)system.getTaskManager().getTaskById(TaskWifi);
                if (wifi)
                {
                    wifi->enable(false);
                }

                for (size_t i = 0; i < sizeof(rtcGpios); i++)
                {
                    rtc_gpio_isolate(gpio_num_t(rtcGpios[i]));
                }

                setCpuFrequencyMhz(10); // Switch to 10MHz
                esp_deep_sleep_start(); // Jump to deep sleep, endlessly
            }
        }

        //m_batteryIsConnected = ((averageVoltage < NO_BATTERY_VOLTAGE) ? false : true);
        //m_batteryIsCharging = ((averageVoltage > CHARGING_BATTERY_VOLTAGE) ? true : false);

        m_lastBatteryReading = millis();
    }
}
