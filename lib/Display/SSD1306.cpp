#include "SSD1306.h"

SSD1306::SSD1306(TwoWire *wire, uint8_t address, OLEDDISPLAY_GEOMETRY g) :
OLEDDisplay(g),
m_wire(wire),
m_address(address)
{
    sendInitCommands();
}

SSD1306::~SSD1306()
{
}

void SSD1306::internDisplay(Bitmap *bitmap)
{
    static const uint8_t clist[] =
    {
            PAGEADDR,
            0x00,           // Page start address
            0xFF,           // Page end (not really, but works here)
            COLUMNADDR, 0   // Column start address
    };
    sendCommandList(clist, sizeof(clist));
    sendCommand(getWidth() - 1U);

    uint16_t count = ((getWidth() * getHeight()) / 8);
    uint8_t *ptr = bitmap->m_buffer;

    m_wire->beginTransmission(m_address);
    m_wire->write(0x40U);
    uint8_t bytesOut = 1;

    while (count--)
    {
        if (bytesOut >= I2C_BUFFER_LENGTH)
        {
            m_wire->endTransmission();
            m_wire->beginTransmission(m_address);
            m_wire->write(0x40U);
            bytesOut = 1;
        }
        m_wire->write(*ptr++);
        bytesOut++;
    }
    m_wire->endTransmission();
}

void SSD1306::sendCommand(uint8_t command) {
    m_wire->beginTransmission(m_address);
    m_wire->write(0x80);
    m_wire->write(command);
    m_wire->endTransmission();
}

void SSD1306::sendCommandList(const uint8_t *c, uint8_t n)
{
    m_wire->beginTransmission(m_address);
    m_wire->write(0x00U);
    uint8_t bytesOut = 1;

    while (n--)
    {
        if (bytesOut >= I2C_BUFFER_LENGTH)
        {
            m_wire->endTransmission();
            m_wire->beginTransmission(m_address);
            m_wire->write(0x00U);
            bytesOut = 1;
        }
        m_wire->write(*c++);
        bytesOut++;
    }
    m_wire->endTransmission();
}

