#include <logger.h>

#include <TimeLib.h>

#include "Task.h"
#include "TaskAprsIs.h"
#include "TaskModem.h"
#include "ProjectConfiguration.h"

ModemTask::ModemTask(TaskQueue<std::shared_ptr<APRSMessage>> &fromModem, TaskQueue<std::shared_ptr<APRSMessage>> &toModem) :
Task(TASK_MODEM, TaskModem),
m_lora_aprs(),
m_fromModem(fromModem),
m_toModem(toModem)
{
}

ModemTask::~ModemTask()
{
}

bool ModemTask::setup(System &system)
{
    SPI.begin(system.getBoardConfig()->LoraSck, system.getBoardConfig()->LoraMiso, system.getBoardConfig()->LoraMosi, system.getBoardConfig()->LoraCS);
    m_lora_aprs.setPins(system.getBoardConfig()->LoraCS, system.getBoardConfig()->LoraReset, system.getBoardConfig()->LoraIRQ);

    if (!m_lora_aprs.begin(system.getUserConfig()->lora.frequencyRx))
    {
        logPrintlnE("Starting LoRa failed!");
        m_stateInfo = "LoRa-Modem failed";
        m_state     = Error;
        while (true) { delay(10); }
    }

    m_lora_aprs.setRxFrequency(system.getUserConfig()->lora.frequencyRx);
    m_lora_aprs.setRxGain(system.getUserConfig()->lora.gainRx);
    m_lora_aprs.setTxFrequency(system.getUserConfig()->lora.frequencyTx);
    m_lora_aprs.setTxPower(system.getUserConfig()->lora.power);
    m_lora_aprs.setSpreadingFactor(system.getUserConfig()->lora.spreadingFactor);
    m_lora_aprs.setSignalBandwidth(system.getUserConfig()->lora.signalBandwidth);
    m_lora_aprs.setCodingRate4(system.getUserConfig()->lora.codingRate4);
    m_lora_aprs.enableCrc();

    m_stateInfo = "";
    return true;
}

bool ModemTask::loop(System &system)
{
    if (m_lora_aprs.checkMessage())
    {
        std::shared_ptr<APRSMessage> msg = m_lora_aprs.getMessage();
        // msg->getAPRSBody()->setData(msg->getAPRSBody()->getData() + " 123");
        logPrintD("[" + timeString() + "] ");
        logPrintD("Received packet '");
        logPrintD(msg->toString());
        logPrintD("' with RSSI ");
        logPrintD(String(m_lora_aprs.packetRssi()));
        logPrintD(" and SNR ");
        logPrintlnD(String(m_lora_aprs.packetSnr()));

        m_fromModem.addElement(msg);
        system.getDisplay().addFrame(std::shared_ptr<DisplayFrame>(new TextFrame("LoRa", msg->toString())));
    }

    if (!m_toModem.empty())
    {
        std::shared_ptr<APRSMessage> msg = m_toModem.getElement();
        m_lora_aprs.sendMessage(msg);
    }

    return true;
}
