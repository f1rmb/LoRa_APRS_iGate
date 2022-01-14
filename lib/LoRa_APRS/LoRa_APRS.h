#ifndef LORA_H_
#define LORA_H_

#include <Arduino.h>

#include <APRS-Decoder.h>
#include <LoRa.h>
#include <memory>

class LoRa_APRS : public LoRaClass
{
    public:
        LoRa_APRS();

        bool                         checkMessage();
        std::shared_ptr<APRSMessage> getMessage();

        void sendMessage(const std::shared_ptr<APRSMessage> msg);

        void setRxFrequency(long frequency);
        long getRxFrequency() const;

        void setRxGain(uint8_t gain);

        void setTxFrequency(long frequency);
        long getTxFrequency() const;

        float getSNR() const;
        int getRSSI() const;


    private:
        std::shared_ptr<APRSMessage> m_LastReceivedMsg;
        long                         m_RxFrequency;
        long                         m_TxFrequency;
        int                          m_rssi;
        float                        m_snr;
};

#endif
