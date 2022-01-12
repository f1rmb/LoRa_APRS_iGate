/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
 * Copyright (c) 2018 by Fabrice Weinberg
 * Copyright (c) 2019 by Helmut Tschemernjak - www.radioshuttle.de
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ThingPulse invests considerable time and money to develop these open source libraries.
 * Please support us by buying our products (and not the clones) from
 * https://thingpulse.com
 *
 */

#include "OLEDDisplay.h"

OLEDDisplay::OLEDDisplay(OLEDDISPLAY_GEOMETRY g) :
m_geometry(g),
m_displayIsOn(false)
{
}

OLEDDisplay::~OLEDDisplay()
{
}

// cppcheck-suppress unusedFunction
void OLEDDisplay::displayOn()
{
    sendCommand(DISPLAYON);
    m_displayIsOn = true;
}

// cppcheck-suppress unusedFunction
bool OLEDDisplay::isDisplayOn() const
{
    return m_displayIsOn;
}

// cppcheck-suppress unusedFunction
void OLEDDisplay::displayOff()
{
    sendCommand(DISPLAYOFF);
    m_displayIsOn = false;
}

// cppcheck-suppress unusedFunction
bool OLEDDisplay::isDisplayOff() const
{
    return !m_displayIsOn;
}

// cppcheck-suppress unusedFunction
void OLEDDisplay::invertDisplay()
{
    sendCommand(INVERTDISPLAY);
}

// cppcheck-suppress unusedFunction
void OLEDDisplay::normalDisplay()
{
    sendCommand(NORMALDISPLAY);
}

// cppcheck-suppress unusedFunction
void OLEDDisplay::setContrast(uint8_t contrast, uint8_t precharge, uint8_t comdetect)
{
    static const uint8_t clist[] =
    {
            SETPRECHARGE,  // 0xD9
            precharge,     // 0xF1 default, to lower the contrast, put 1-1F
            SETCONTRAST,
            contrast,      // 0-255
            SETVCOMDETECT, // 0xDB, (additionally needed to lower the contrast)
            comdetect,     // 0x40 default, to lower the contrast, put 0
            DISPLAYALLON_RESUME,
            NORMALDISPLAY,
            DISPLAYON
    };
    sendCommandList(clist, sizeof(clist));
}

// cppcheck-suppress unusedFunction
void OLEDDisplay::setBrightness(uint8_t brightness)
{
    uint8_t contrast = brightness * 1.171 - 43;

    if (brightness < 128)
    {
        // Magic values to get a smooth/ step-free transition
        contrast = brightness * 1.171;
    }

    uint8_t precharge = 241;
    if (brightness == 0)
    {
        precharge = 0;
    }

    uint8_t comdetect = brightness / 8;
    setContrast(contrast, precharge, comdetect);
}

// cppcheck-suppress unusedFunction
void OLEDDisplay::resetOrientation()
{
    static const uint8_t clist[] =
    {
            SEGREMAP,
            COMSCANINC
    };
    sendCommandList(clist, sizeof(clist));
}

// cppcheck-suppress unusedFunction
void OLEDDisplay::rotate180()
{
    static const uint8_t clist[] =
    {
            (SEGREMAP | 0x01),
            COMSCANDEC
    };
    sendCommandList(clist, sizeof(clist));
}

// cppcheck-suppress unusedFunction
void OLEDDisplay::flipScreenVertically()
{
    static const uint8_t clist[] =
    {
            SEGREMAP,
            COMSCANDEC
    };
    sendCommandList(clist, sizeof(clist));
}

void OLEDDisplay::display(Bitmap *bitmap)
{
    if (isDisplayOff())
    {
        displayOn();
    }

    internDisplay(bitmap);
}

// cppcheck-suppress unusedFunction
void OLEDDisplay::clear()
{
}

// cppcheck-suppress unusedFunction
uint16_t OLEDDisplay::getWidth()
{
    switch (m_geometry)
    {
        case GEOMETRY_128_64:
        case GEOMETRY_128_32:
            return 128U;
        case GEOMETRY_64_48:
        case GEOMETRY_64_32:
            return 64U;
    }
    return 0U;
}

// cppcheck-suppress unusedFunction
uint16_t OLEDDisplay::getHeight()
{
    switch (m_geometry)
    {
        case GEOMETRY_128_64:
            return 64U;
        case GEOMETRY_64_48:
            return 48U;
        case GEOMETRY_128_32:
        case GEOMETRY_64_32:
            return 32U;
    }
    return 0U;
}

// cppcheck-suppress unusedFunction
void OLEDDisplay::sendInitCommands()
{
    uint8_t startline = (((m_geometry == GEOMETRY_64_32) ? 0x00U : SETSTARTLINE));
    uint8_t contrast = (((m_geometry == GEOMETRY_128_32) ? 0x8FU : 0xCFU));
    uint8_t compins = (((m_geometry == GEOMETRY_128_32) ? 0x02U : 0x12U));

    static const uint8_t clist1[] =
    {
            DISPLAYOFF,
            SETDISPLAYCLOCKDIV,
#if 0
            0xF0U, // Increase speed of the display max ~550kHz
#else
            0x80U, // the suggested ratio 0x80 (~400kHz)
#endif
            SETMULTIPLEX
    };
    sendCommandList(clist1, sizeof(clist1));

    sendCommand((getHeight() - 1));

    static const uint8_t clist2[] =
    {
            SETDISPLAYOFFSET,
            0x00U,
            startline,
            CHARGEPUMP
    };
    sendCommandList(clist2, sizeof(clist2));

    sendCommand(0x14U); // VCC

    static const uint8_t clist3[] =
    {
            MEMORYMODE,
            0x00U,
            SEGREMAP,
            COMSCANINC
    };
    sendCommandList(clist3, sizeof(clist3));

    sendCommand(SETCOMPINS);
    sendCommand(compins);

    sendCommand(SETCONTRAST);
    sendCommand(contrast);

    sendCommand(SETPRECHARGE);
    sendCommand(0xF1U);

    static const uint8_t clist4[] =
    {
            SETVCOMDETECT,  // 0xDB, (additionally needed to lower the contrast)
            0x40U,          // 0x40 default, to lower the contrast, put 0
            DISPLAYALLON_RESUME,
            NORMALDISPLAY,
            DEACTIVATE_SCROLL,
            DISPLAYON
    };
    sendCommandList(clist4, sizeof(clist4));
}
