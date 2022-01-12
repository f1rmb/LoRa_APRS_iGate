#include "LoRa_APRS.h"

LoRa_APRS::LoRa_APRS() :
m_RxFrequency(433775000),
m_TxFrequency(433775000)
{
}

bool LoRa_APRS::checkMessage()
{
    if (!parsePacket())
    {
        return false;
    }

    // read header:
    char dummy[4];
    readBytes(dummy, 3);
    if (dummy[0] != '<')
    {
        // is no APRS message, ignore message
        while (available())
        {
            read();
        }

        return false;
    }

    // read APRS data:
    String str;
    while (available())
    {
        str += (char)read();
    }

    m_LastReceivedMsg = std::shared_ptr<APRSMessage>(new APRSMessage());
    m_LastReceivedMsg->decode(str);
    return true;
}

std::shared_ptr<APRSMessage> LoRa_APRS::getMessage()
{
    return m_LastReceivedMsg;
}

void LoRa_APRS::sendMessage(const std::shared_ptr<APRSMessage> msg)
{
    setFrequency(m_TxFrequency);
    String data = msg->encode();
    if (beginPacket())
    {
        // Header:
        write('<');
        write(0xFF);
        write(0x01);
        // APRS Data:
        write((const uint8_t *)data.c_str(), data.length());
        endPacket();
    }
    setFrequency(m_RxFrequency);
}

void LoRa_APRS::setRxFrequency(long frequency)
{
    m_RxFrequency = frequency;
    setFrequency(m_RxFrequency);
}

void LoRa_APRS::setRxGain(uint8_t gain)
{
    setGain(gain);
}

// cppcheck-suppress unusedFunction
long LoRa_APRS::getRxFrequency() const
{
    return m_RxFrequency;
}

void LoRa_APRS::setTxFrequency(long frequency)
{
    m_TxFrequency = frequency;
}

// cppcheck-suppress unusedFunction
long LoRa_APRS::getTxFrequency() const
{
    return m_TxFrequency;
}
