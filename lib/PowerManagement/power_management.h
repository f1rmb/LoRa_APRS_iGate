#ifndef POWER_MANAGEMENT_H_
#define POWER_MANAGEMENT_H_

#include <Arduino.h>
#include <axp20x.h>

class PowerManagement
{
    public:
        PowerManagement();
        bool begin(TwoWire &port);

        void activateLoRa();
        void deactivateLoRa();

        void activateGPS();
        void deactivateGPS();

        void activateOLED();
        void deactivateOLED();

    private:
        AXP20X_Class m_axp;
};

#endif
