#include <map>

#include <APRS-IS.h>
#include <BoardFinder.h>
#include <System.h>
#include <TaskManager.h>
#include <logger.h>
#include <power_management.h>

#include "TaskAprsIs.h"
#include "TaskBeacon.h"
#include "TaskDisplay.h"
#include "TaskEth.h"
#include "TaskFTP.h"
#include "TaskMQTT.h"
#include "TaskModem.h"
#include "TaskNTP.h"
#include "TaskOTA.h"
#include "TaskRouter.h"
#include "TaskWifi.h"
#include "TaskBatterySurvey.h"
#include "ProjectConfiguration.h"

#define VERSION "22.13.0"
#define MODULE_NAME "Main"

TaskQueue<std::shared_ptr<APRSMessage>> toAprsIs;
TaskQueue<std::shared_ptr<APRSMessage>> fromModem;
TaskQueue<std::shared_ptr<APRSMessage>> toModem;
TaskQueue<std::shared_ptr<APRSMessage>> toMQTT;

System            LoRaSystem;
Configuration     userConfig;

DisplayTask       displayTask;
ModemTask         modemTask(fromModem, toModem);
EthTask           ethTask;
WifiTask          wifiTask;
OTATask           otaTask;
NTPTask           ntpTask;
FTPTask           ftpTask;
MQTTTask          mqttTask(toMQTT);
AprsIsTask        aprsIsTask(toAprsIs);
RouterTask        routerTask(fromModem, toModem, toAprsIs, toMQTT);
BeaconTask        beaconTask(toModem, toAprsIs);
BatterySurveyTask battTask;

volatile bool syslogSet = false;

void setup()
{
    Serial.begin(115200);
    LoRaSystem.getLogger().setSerial(&Serial);
    LoRaSystem.getLogger().enableColor(false);
    setWiFiLogger(&LoRaSystem.getLogger());
    delay(500);
    LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, MODULE_NAME, "LoRa APRS iGate by OE5BPA (Peter Buchegger), F1RMB fork");
    LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, MODULE_NAME, "Version: %s", VERSION);

    std::list<BoardConfig const *> boardConfigs;
    boardConfigs.push_back(&TTGO_LORA32_V1);
    boardConfigs.push_back(&TTGO_LORA32_V2);
    boardConfigs.push_back(&TTGO_T_Beam_V0_7);
    boardConfigs.push_back(&TTGO_T_Beam_V1_0);
    boardConfigs.push_back(&ETH_BOARD);
    boardConfigs.push_back(&TRACKERD);
    boardConfigs.push_back(&HELTEC_WIFI_LORA_32_V1);
    boardConfigs.push_back(&HELTEC_WIFI_LORA_32_V2);

    ProjectConfigurationManagement confmg(LoRaSystem.getLogger());
    confmg.readConfiguration(LoRaSystem.getLogger(), userConfig);

    BoardFinder        finder(boardConfigs);
    BoardConfig const *boardConfig = finder.getBoardConfig(userConfig.board);
    if (!boardConfig)
    {
        boardConfig = finder.searchBoardConfig(LoRaSystem.getLogger());
        if (!boardConfig)
        {
            LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, MODULE_NAME, "Board config not set and search failed!");
            while (true) { delay(10); }
        }
        else
        {
            userConfig.board = boardConfig->Name;
            confmg.writeConfiguration(LoRaSystem.getLogger(), userConfig);
            LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, MODULE_NAME, "will restart board now!");
            ESP.restart();
        }
    }

    btStop();

    LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, MODULE_NAME, "Board %s loaded.", boardConfig->Name.c_str());

    if (boardConfig->Type == eTTGO_T_Beam_V1_0)
    {
        Wire.begin(boardConfig->OledSda, boardConfig->OledScl);
        PowerManagement powerManagement;
        if (powerManagement.begin(Wire))
        {
            LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, MODULE_NAME, "AXP192 init done!");
        }
        else
        {
            LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, MODULE_NAME, "AXP192 init failed!");
        }
        powerManagement.activateLoRa();
        powerManagement.activateOLED();

        if (userConfig.beacon.use_gps)
        {
            powerManagement.activateGPS();
        }
        else
        {
            powerManagement.deactivateGPS();
        }

        // Use the user button to lit the screen, if overwritePin is not defined
        if ((userConfig.display.alwaysOn == false) && (userConfig.display.overwritePin == 0))
        {
            userConfig.display.overwritePin = 38;
        }
    }
    else if (boardConfig->Type == eTTGO_T_Beam_V0_7)
    {
        HardwareSerial ss(1);
        const uint8_t gnssPowerOff[] = { 0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B };

        //
        // Powering OFF the GNSS
        //
        ss.begin(9600, SERIAL_8N1, 12, 15);

        // Wait for some incoming data
        unsigned long startTime = millis();
        while ((millis() - startTime) < 3000)
        {
            if (ss.available() > 0)
            {
                break;
            }
            else
            {
                delay(1);
            }
        }

        // Send PowerOff
        for (size_t i = 0; i < (sizeof(gnssPowerOff) / sizeof(gnssPowerOff[0])); i++)
        {
            ss.write(gnssPowerOff[i]);
        }

        // Use the user button to lit the screen, if overwritePin is not defined
        if ((userConfig.display.alwaysOn == false) && (userConfig.display.overwritePin == 0))
        {
            userConfig.display.overwritePin = 39;
        }
    }

    LoRaSystem.setBoardConfig(boardConfig);
    LoRaSystem.setUserConfig(&userConfig);
    LoRaSystem.getTaskManager().addTask(&displayTask);
    LoRaSystem.getTaskManager().addTask(&modemTask);
    LoRaSystem.getTaskManager().addTask(&routerTask);
    LoRaSystem.getTaskManager().addTask(&beaconTask);

    bool tcpip = false;

    if (userConfig.wifi.active)
    {
        LoRaSystem.getTaskManager().addAlwaysRunTask(&wifiTask);
        LoRaSystem.getTaskManager().addTask(&otaTask);
        tcpip = true;
    }
    else if (boardConfig->Type == eETH_BOARD)
    {
        LoRaSystem.getTaskManager().addAlwaysRunTask(&ethTask);
        tcpip = true;
    }

    if (tcpip)
    {
        LoRaSystem.getTaskManager().addTask(&ntpTask);

        if (userConfig.ftp.active)
        {
            LoRaSystem.getTaskManager().addTask(&ftpTask);
        }

        if (userConfig.aprs_is.active)
        {
            LoRaSystem.getTaskManager().addTask(&aprsIsTask);
        }

        if (userConfig.mqtt.active)
        {
            LoRaSystem.getTaskManager().addTask(&mqttTask);
        }
    }

    // Disable (power off) wifi module if not used.
    if (userConfig.wifi.active == false)
    {
        wifiTask.enable(false);
    }

    // User want to override the battery pin (change number or disabling it (0))
    // This would only works if the board doesn't have a Power chip.
    if ((boardConfig->HasPowerChip == false) && (userConfig.tweaks.voltagePin > -1))
    {
        LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, MODULE_NAME, "Override battery sensing pin to: %u", userConfig.tweaks.voltagePin);
        ((BoardConfig *)boardConfig)->BattPin = uint8_t(userConfig.tweaks.voltagePin);
    }

    if (boardConfig->BattPin > 0U)
    {
        LoRaSystem.getTaskManager().addTask(&battTask);
    }

    LoRaSystem.getTaskManager().setup(LoRaSystem);

    if ((LoRaSystem.getDisplay().getWidth() == 128) && (LoRaSystem.getDisplay().getHeight() == 64))
    {
        LoRaSystem.getDisplay().showSpashScreen(VERSION);
    }
    else
    {
        LoRaSystem.getDisplay().showSpashScreen("LoRa  APRS  iGate", VERSION);
    }

    //
    // Config sanity checks
    //
    if (userConfig.callsign.startsWith("NOCALL"))
    {
        LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, MODULE_NAME, "You have to change your settings in 'data/is-cfg.json' and upload it via 'Upload File System image'!");
        LoRaSystem.getDisplay().showStatusScreen("ERROR", "You have to change your settings in 'data/is-cfg.json' and upload it via \"Upload File System image\"!");
        while (true) { delay(10); }
    }

    if ((userConfig.aprs_is.active == false) && (userConfig.digi.active == false))
    {
        LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, MODULE_NAME, "No mode selected (iGate or Digi)! You have to activate one of iGate or Digi.");
        LoRaSystem.getDisplay().showStatusScreen("ERROR", "No mode selected (iGate or Digi)! You have to activate one of iGate or Digi.");
        while (true) { delay(10); }
    }

    if (userConfig.mqtt.active &&
            ((userConfig.mqtt.server.length() == 0) || (userConfig.mqtt.port == 0) ||
                    (userConfig.mqtt.name.length() == 0) || (userConfig.mqtt.password.length() == 0) ||
                    (userConfig.mqtt.topic.length() == 0)))
    {
        LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, MODULE_NAME, "The MQTT configuration is wrong or incomplete! Please fix this.");
        LoRaSystem.getDisplay().showStatusScreen("ERROR", "The MQTT configuration is wrong or incomplete! Please fix this.");
        while (true) { delay(10); }
    }

    if (userConfig.display.overwritePin != 0)
    {
        // Display's overwritePin collides with board Button pin.
        if (userConfig.display.overwritePin == boardConfig->Button)
        {
            userConfig.display.overwritePin = 0;
        }
        else
        {
            pinMode(userConfig.display.overwritePin, INPUT);
            pinMode(userConfig.display.overwritePin, INPUT_PULLUP);
        }
    }

    delay(5000);
    LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, MODULE_NAME, "setup done...");
}

void loop()
{
    LoRaSystem.getTaskManager().loop(LoRaSystem);

    if (LoRaSystem.isWifiEthConnected() && LoRaSystem.getUserConfig()->syslog.active && (syslogSet == false))
    {
        LoRaSystem.getLogger().setSyslogServer(LoRaSystem.getUserConfig()->syslog.server, LoRaSystem.getUserConfig()->syslog.port, LoRaSystem.getUserConfig()->callsign);
#if 0
        LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, MODULE_NAME, "System connected after a restart to the network, syslog server set");
#else
        LoRaSystem.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, MODULE_NAME,
                "System connected after a restart to the network, syslog server set (%s / %d / %s)",
                LoRaSystem.getUserConfig()->syslog.server.c_str(), LoRaSystem.getUserConfig()->syslog.port, LoRaSystem.getUserConfig()->callsign.c_str());
#endif
        syslogSet = true;
    }
}
