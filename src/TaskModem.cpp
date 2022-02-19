#include <logger.h>

#include <TimeLib.h>

#include "Tasks.h"
#include "TaskAprsIs.h"
#include "TaskModem.h"
#include "ProjectConfiguration.h"

ModemTask::ModemTask(TaskQueue<std::shared_ptr<APRSMessage>> &fromModem, TaskQueue<std::shared_ptr<APRSMessage>> &toModem) :
Task(TASK_MODEM, TaskModem),
m_initialized(false),
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

    m_stateInfo = ((system.getUserConfig()->lora.rxOnly) ? "RX Only" : "");
    m_initialized = true;
    return true;
}

bool ModemTask::loop(System &system)
{
    if (m_initialized)
    {
        if (m_lora_aprs.checkMessage())
        {
            int rssi = m_lora_aprs.getRSSI();
            float snr = m_lora_aprs.getSNR();

            std::shared_ptr<APRSMessage> msg = m_lora_aprs.getMessage();
            logPrintD("[" + timeString() + "] ");
            logPrintD("Received packet '");
            logPrintD(msg->toString());
            logPrintD("'");
            if (rssi != INT_MAX)
            {
                logPrintD(" RSSI ");
                logPrintD(String(rssi));
            }
            if (std::isnan(snr) == false)
            {
                logPrintD(" SNR ");
                logPrintlnD(String(snr, 2));
            }

            // Add RSSI and SNR, if values are valid and this feature is enabled in configuration
            if (system.getUserConfig()->aprs.add_rssi_and_snr && (rssi != INT_MAX) && (std::isnan(snr) == false))
            {
                String body = msg->getBody()->getData();
                String bodyUpcase = String(body);

                bodyUpcase.toUpperCase();

                // Don't add RSSI and SNR if one of them is already part of the frame.
                if (((bodyUpcase.indexOf(" RSSI: ") >= 0) || (bodyUpcase.indexOf(" SNR: ") >= 0)) == false)
                {
                    String dao;

                    body.trim();

                    // Search for the DAO
                    int daoOffset;
                    if ((daoOffset = body.lastIndexOf('!')) >= 4) // last '!'
                    {
                        if (body[daoOffset - 4] == '!') // previous '!'
                        {
                            dao = body.substring(daoOffset - 4, daoOffset + 1); // extract the DAO
                            body.remove(daoOffset - 4, 5); // Remove the DAO from the current body
                            body.replace("  ", " "); // Get rid of duplicated spaces
                            body.trim();
                            dao = " " + dao; // prepend a space, as DAO will be concatenated with the body message, blindly.
                        }
                    }

                    msg->getBody()->setData(body + " - RSSI: " + rssi + "dBm - SNR: " + String(snr, 2) + "dB" + dao);
                }
            }


            m_fromModem.addElement(msg);
            system.getDisplay().addFrame(std::shared_ptr<DisplayFrame>(new TextFrame("LoRa", msg->encode())));
        }

        if (!m_toModem.empty())
        {
            std::shared_ptr<APRSMessage> msg = m_toModem.getElement();
            m_lora_aprs.sendMessage(msg);
        }
    }
    return true;
}

void ModemTask::shutdown()
{
    if (m_initialized)
    {
        m_lora_aprs.end();
        m_initialized = false;
    }
}
