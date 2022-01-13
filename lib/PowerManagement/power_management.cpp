
#include "power_management.h"

PowerManagement::PowerManagement()
{
}

bool PowerManagement::begin(TwoWire &port)
{
    bool result = (m_axp.begin(port, AXP192_SLAVE_ADDRESS) == AXP_PASS);

    if (result)
    {
        result = (m_axp.setDCDC1Voltage(3300) == AXP_PASS);
    }

    return result;
}

void PowerManagement::activateLoRa()
{
    m_axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
}

// cppcheck-suppress unusedFunction
void PowerManagement::deactivateLoRa()
{
    m_axp.setPowerOutPut(AXP192_LDO2, AXP202_OFF);
}

// cppcheck-suppress unusedFunction
void PowerManagement::activateGPS()
{
    m_axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
}

void PowerManagement::deactivateGPS()
{
    m_axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
}

void PowerManagement::activateOLED()
{
    m_axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
}

// cppcheck-suppress unusedFunction
void PowerManagement::decativateOLED()
{
    m_axp.setPowerOutPut(AXP192_DCDC1, AXP202_OFF);
}
