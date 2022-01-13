#include <map>

#include <APRS-IS.h>
#include <BoardFinder.h>
#include <System.h>
#include <TaskManager.h>
#include <logger.h>
#include <power_management.h>

#include "TaskAprsIs.h"
#include "TaskDisplay.h"
#include "TaskEth.h"
#include "TaskFTP.h"
#include "TaskModem.h"
#include "TaskNTP.h"
#include "TaskOTA.h"
#include "TaskRouter.h"
#include "TaskWifi.h"
#include "ProjectConfiguration.h"

#define VERSION "22.02.2"

TaskQueue<std::shared_ptr<APRSMessage>> toAprsIs;
TaskQueue<std::shared_ptr<APRSMessage>> fromModem;
TaskQueue<std::shared_ptr<APRSMessage>> toModem;

System        LoRaSystem;
Configuration userConfig;

DisplayTask displayTask;
ModemTask   modemTask(fromModem, toModem);
EthTask     ethTask;
WifiTask    wifiTask;
OTATask     otaTask;
NTPTask     ntpTask;
FTPTask     ftpTask;
AprsIsTask  aprsIsTask(toAprsIs);
RouterTask  routerTask(fromModem, toModem, toAprsIs);

void setup()
{
    Serial.begin(115200);
    Logger::instance().setSerial(&Serial);
    delay(500);
    logPrintlnI("LoRa APRS iGate by OE5BPA (Peter Buchegger)");
    logPrintlnI("Version: " VERSION);

    std::list<BoardConfig const *> boardConfigs;
    boardConfigs.push_back(&TTGO_LORA32_V1);
    boardConfigs.push_back(&TTGO_LORA32_V2);
    boardConfigs.push_back(&TTGO_T_Beam_V0_7);
    boardConfigs.push_back(&TTGO_T_Beam_V1_0);
    boardConfigs.push_back(&ETH_BOARD);
    boardConfigs.push_back(&TRACKERD);
    boardConfigs.push_back(&HELTEC_WIFI_LORA_32_V1);
    boardConfigs.push_back(&HELTEC_WIFI_LORA_32_V2);

    ProjectConfigurationManagement confmg;
    confmg.readConfiguration(userConfig);

    BoardFinder        finder(boardConfigs);
    BoardConfig const *boardConfig = finder.getBoardConfig(userConfig.board);
    if (!boardConfig)
    {
        boardConfig = finder.searchBoardConfig();
        if (!boardConfig)
        {
            logPrintlnE("Board config not set and search failed!");
            while (true) { delay(10); }
        }
        else
        {
            userConfig.board = boardConfig->Name;
            confmg.writeConfiguration(userConfig);
            logPrintlnI("will restart board now!");
            ESP.restart();
        }
    }

    logPrintI("Board ");
    logPrintI(boardConfig->Name);
    logPrintlnI(" loaded.");

    if (boardConfig->Type == eTTGO_T_Beam_V1_0)
    {
        Wire.begin(boardConfig->OledSda, boardConfig->OledScl);
        PowerManagement powerManagement;
        if (!powerManagement.begin(Wire))
        {
            logPrintlnI("AXP192 init done!");
        }
        else
        {
            logPrintlnE("AXP192 init failed!");
        }
        powerManagement.activateLoRa();
        powerManagement.activateOLED();
        powerManagement.deactivateGPS();
    }

    LoRaSystem.setBoardConfig(boardConfig);
    LoRaSystem.setUserConfig(&userConfig);
    LoRaSystem.getTaskManager().addTask(&displayTask);
    LoRaSystem.getTaskManager().addTask(&modemTask);
    LoRaSystem.getTaskManager().addTask(&routerTask);

    if (userConfig.aprs_is.active)
    {
        if (boardConfig->Type == eETH_BOARD && !userConfig.wifi.active)
        {
            LoRaSystem.getTaskManager().addAlwaysRunTask(&ethTask);
        }

        if (userConfig.wifi.active)
        {
            LoRaSystem.getTaskManager().addAlwaysRunTask(&wifiTask);
        }

        LoRaSystem.getTaskManager().addTask(&otaTask);
        LoRaSystem.getTaskManager().addTask(&ntpTask);

        if (userConfig.ftp.active)
        {
            LoRaSystem.getTaskManager().addTask(&ftpTask);
        }

        LoRaSystem.getTaskManager().addTask(&aprsIsTask);
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

    if (userConfig.callsign.startsWith("NOCALL"))
    {
        logPrintlnE("You have to change your settings in 'data/is-cfg.json' and upload it via \"Upload File System image\"!");
        LoRaSystem.getDisplay().showStatusScreen("ERROR", "You have to change your settings in 'data/is-cfg.json' and upload it via \"Upload File System image\"!");
        while (true) { delay(10); }
    }

    if ((userConfig.aprs_is.active == false) && (userConfig.digi.active == false))
    {
        logPrintlnE("No mode selected (iGate or Digi)! You have to activate one of iGate or Digi.");
        LoRaSystem.getDisplay().showStatusScreen("ERROR", "No mode selected (iGate or Digi)! You have to activate one of iGate or Digi.");
        while (true) { delay(10); }
    }

    if (userConfig.display.overwritePin != 0)
    {
        pinMode(userConfig.display.overwritePin, INPUT);
        pinMode(userConfig.display.overwritePin, INPUT_PULLUP);
    }

    delay(5000);
    logPrintlnI("setup done...");
}

void loop()
{
    LoRaSystem.getTaskManager().loop(LoRaSystem);
}

