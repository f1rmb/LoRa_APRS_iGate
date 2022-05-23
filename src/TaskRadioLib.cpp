#include <RadioLib.h>
#include <TimeLib.h>
#include <logger.h>

#include "Tasks.h"
#include "TaskRadioLib.h"

volatile bool  RadioLibTask::m_enableInterrupt = true;  // Need to catch interrupt or not.
volatile bool  RadioLibTask::m_operationDone   = false; // Caught IRQ or not.
static int16_t transmissionState               = RADIOLIB_ERR_NONE;
static bool    transmitFlag                    = false; // Transmitting or not.


RadioLibTask::RadioLibTask(TaskQueue<std::shared_ptr<APRSMessage>> &fromModem, TaskQueue<std::shared_ptr<APRSMessage>> &toModem) :
Task(TASK_RADIOLIB, TaskRadioLib),
m_module(NULL),
m_radio(NULL),
m_rxEnable(false),
m_txEnable(false),
m_fromModem(fromModem),
m_toModem(toModem),
m_preambleDurationMilliSec(0)
{
}

RadioLibTask::~RadioLibTask()
{
    m_radio->clearDio0Action();
}

void RadioLibTask::setFlag(void)
{
    if (m_enableInterrupt == false)
    {
        return;
    }

    m_operationDone = true;
}

bool RadioLibTask::setup(System &system)
{
    const uint16_t preambleLength = 8;
    float freqMHz = (float)m_config.frequencyRx / 1000000;
    float BWkHz   = (float)m_config.signalBandwidth / 1000;

    SPI.begin(system.getBoardConfig()->LoraSck, system.getBoardConfig()->LoraMiso, system.getBoardConfig()->LoraMosi, system.getBoardConfig()->LoraCS);
    m_module = new Module(system.getBoardConfig()->LoraCS, system.getBoardConfig()->LoraIRQ, system.getBoardConfig()->LoraReset);
    m_radio  = new SX1278(m_module);

    m_config = system.getUserConfig()->lora;

    m_rxEnable = true;
    m_txEnable = (m_config.rxOnly == false);

    int16_t state = m_radio->begin(freqMHz, BWkHz, m_config.spreadingFactor, m_config.codingRate4, RADIOLIB_SX127X_SYNC_WORD, m_config.power, preambleLength, m_config.gainRx);

    if (state != RADIOLIB_ERR_NONE)
    {
        switch (state)
        {
            case RADIOLIB_ERR_INVALID_FREQUENCY:
                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] SX1278 init failed, The supplied frequency value (%fMHz) is invalid for this module.", timeString().c_str(), freqMHz);
                m_rxEnable = false;
                m_txEnable = false;
                break;
            case RADIOLIB_ERR_INVALID_BANDWIDTH:
                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] SX1278 init failed, The supplied bandwidth value (%fkHz) is invalid for this module. Should be 7800, 10400, 15600, 20800, 31250, 41700 ,62500, 125000, 250000, 500000.", timeString().c_str(), BWkHz);
                m_rxEnable = false;
                m_txEnable = false;
                break;
            case RADIOLIB_ERR_INVALID_SPREADING_FACTOR:
                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] SX1278 init failed, The supplied spreading factor value (%d) is invalid for this module.", timeString().c_str(), m_config.spreadingFactor);
                m_rxEnable = false;
                m_txEnable = false;
                break;
            case RADIOLIB_ERR_INVALID_CODING_RATE:
                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] SX1278 init failed, The supplied coding rate value (%d) is invalid for this module.", timeString().c_str(), m_config.codingRate4);
                m_rxEnable = false;
                m_txEnable = false;
                break;
            case RADIOLIB_ERR_INVALID_OUTPUT_POWER:
                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] SX1278 init failed, The supplied output power value (%d) is invalid for this module.", timeString().c_str(), m_config.power);
                m_txEnable = false;
                break;
            case RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH:
                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] SX1278 init failed, The supplied preamble length is invalid.", timeString().c_str());
                m_txEnable = false;
                break;
            case RADIOLIB_ERR_INVALID_GAIN:
                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] SX1278 init failed, The supplied gain value (%d) is invalid.", timeString().c_str(), m_config.gainRx);
                m_rxEnable = false;
                break;
            default:
                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] SX1278 init failed, code %d", timeString().c_str(), state);
                m_rxEnable = false;
                m_txEnable = false;
        }

        //m_stateInfo = "LoRa-Modem failed";
        m_state = Error;
    }

    if (m_state != Error)
    {
        state = m_radio->setCRC(true);

        if (state != RADIOLIB_ERR_NONE)
        {
            system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] setCRC failed, code %d", timeString().c_str(), state);
            //m_stateInfo = "LoRa-Modem failed";
            m_state = Error;
        }
    }

    if (m_state != Error)
    {
        m_radio->setDio0Action(setFlag);
    }

    if (m_rxEnable && (m_state != Error))
    {
        int16_t state = startRX(RADIOLIB_SX127X_RXCONTINUOUS);

        if (state != RADIOLIB_ERR_NONE)
        {
            system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] startRX failed, code %d", timeString().c_str(), state);
            m_rxEnable = false;
            //m_stateInfo = "LoRa-Modem failed";
            m_state = Error;
        }
    }

    m_preambleDurationMilliSec = ((uint64_t)(preambleLength + 4) << (m_config.spreadingFactor + 10 /* to milli-sec */)) / m_config.signalBandwidth;
    m_stateInfo = ((m_state == Error) ? "Modem Error" : ((system.getUserConfig()->lora.rxOnly) ? "RX Only" : ""));

    return true;
}

bool RadioLibTask::loop(System &system)
{
    if (m_state != Error)
    {
        if (m_operationDone) // occurs interrupt.
        {
            m_enableInterrupt = false;

            if (transmitFlag) // transmitted.
            {
                if (transmissionState == RADIOLIB_ERR_NONE)
                {
                    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, getName(), "[%s] TX done", timeString().c_str());
                }
                else
                {
                    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] transmitFlag failed, code %d", timeString().c_str(), transmissionState);
                }

                m_operationDone = false;
                transmitFlag = false;

                m_txWaitTimer.setTimeout(m_preambleDurationMilliSec * 2);
                m_txWaitTimer.start();

            }
            else // received.
            {
                String str;
                int16_t state = m_radio->readData(str);

                if (state != RADIOLIB_ERR_NONE)
                {
                    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] readData failed, code %d", timeString().c_str(), state);
                }
                else
                {
                    if (str.substring(0, 3) != "<\xff\x01")
                    {
                        system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, getName(),
                                "[%s] Unknown packet '%s' with RSSI %.0fdBm, SNR %.2fdB and FreqErr %dHz",
                                timeString().c_str(), m_radio->getRSSI(), m_radio->getSNR(), -m_radio->getFrequencyError());
                    }
                    else
                    {
                        std::shared_ptr<APRSMessage> msg = std::shared_ptr<APRSMessage>(new APRSMessage());
                        float rssi = m_radio->getRSSI();
                        float snr = m_radio->getSNR();

                        msg->decode(str.substring(3));

                        // Add RSSI and SNR
                        if (system.getUserConfig()->aprs.add_rssi_and_snr)
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

                                msg->getBody()->setData(body + " - RSSI: " + String(rssi, 0) + "dBm - SNR: " + String(snr, 2) + "dB" + dao);
                            }
                        }

                        m_fromModem.addElement(msg);
                        system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, getName(),
                                "[%s] Received packet '%s' with RSSI %.0fdBm, SNR %.2fdB and FreqErr %fHz",
                                timeString().c_str(), msg->toString().c_str(), rssi, snr, -m_radio->getFrequencyError());
                        system.getDisplay().addFrame(std::shared_ptr<DisplayFrame>(new TextFrame("LoRa", msg->toString().c_str())));
                    }
                }

                m_operationDone = false;
            }

            if (m_rxEnable)
            {
                int16_t state = startRX(RADIOLIB_SX127X_RXCONTINUOUS);

                if (state != RADIOLIB_ERR_NONE)
                {
                    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] startRX failed, code %d", timeString().c_str(), state);
                    m_rxEnable = false;
                }
            }

            m_enableInterrupt = true;
        }
        else // not interrupt.
        {
            if (m_txWaitTimer.hasExpired())
            {
                if (transmitFlag)
                {
                    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, getName(), "[%s] TX signal detected. Waiting TX", timeString().c_str());
                }
                else // Not currently transmitting
                {
                    if (m_toModem.empty() == false)
                    {
                        if ((m_config.frequencyRx == m_config.frequencyTx) && ((m_radio->getModemStatus() & 0x01) == 0x01))
                        {
                            // TODO: REMOVE ME
                            system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, getName(), "[%s] RX signal detected. Waiting TX", timeString().c_str());
                        }
                        else
                        {
                            std::shared_ptr<APRSMessage> msg = m_toModem.getElement();

                            if (m_txEnable)
                            {
                                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, getName(), "[%s] Transmitting packet '%s'", timeString().c_str(), msg->toString().c_str());

                                int16_t state = startTX("<\xff\x01" + msg->encode());

                                if (state != RADIOLIB_ERR_NONE)
                                {
                                    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, getName(), "[%s] startTX failed, code %d", timeString().c_str(), state);
                                    m_txEnable = false;
                                }
                            }
                            else
                            {
                                // TODO: REMOVE ME
                                system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, getName(), "[%s] TX is not enabled", timeString().c_str());
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

void RadioLibTask::shutdown()
{
    if (m_state != Error)
    {
        m_radio->sleep();
        m_module->end();
        m_state = Error;
    }
}

int16_t RadioLibTask::startRX(uint8_t mode)
{
    if (m_config.frequencyTx != m_config.frequencyRx)
    {
        int16_t state = m_radio->setFrequency((float)m_config.frequencyRx / 1000000);

        if (state != RADIOLIB_ERR_NONE)
        {
            return state;
        }
    }

    return m_radio->startReceive(0, mode);
}

int16_t RadioLibTask::startTX(String &str)
{
    if (m_config.frequencyTx != m_config.frequencyRx)
    {
        int16_t state = m_radio->setFrequency((float)m_config.frequencyTx / 1000000);

        if (state != RADIOLIB_ERR_NONE)
        {
            return state;
        }
    }

    transmissionState = (m_txEnable ? m_radio->startTransmit(str) : RADIOLIB_ERR_NONE);
    transmitFlag = (m_txEnable ? true : false);

    return RADIOLIB_ERR_NONE;
}
