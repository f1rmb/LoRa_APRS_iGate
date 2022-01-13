#include "Bitmap.h"
#include "FontConfig.h"
#include "OLEDDisplay.h"
//#include "OLEDDisplayFonts.h"

// cppcheck-suppress unusedFunction
Bitmap::Bitmap(uint16_t width, uint16_t height) :
m_width(width),
m_height(height),
m_buffer(NULL)
{
    allocateBuffer();
}

Bitmap::Bitmap(uint16_t width, uint16_t height, const uint8_t xBuffer[]) :
m_width(width),
m_height(height),
m_buffer(NULL)
{
    allocateBuffer();

    if (m_buffer != NULL)
    {
        // From Adafruit GFX
        uint16_t byteWidth = (m_width + 7) / 8; // Bitmap scanline pad = whole byte
        uint8_t byte = 0;
        uint16_t x = 0, y = 0;

        for (uint16_t j = 0; j < m_height; j++, y++)
        {
            for (uint16_t i = 0; i < m_width; i++)
            {
                if (i & 7)
                {
                    byte >>= 1;
                }
                else
                {
                    byte = xBuffer[j * byteWidth + i / 8];
                }
                // Nearly identical to drawBitmap(), only the bit order
                // is reversed here (left-to-right = LSB to MSB):
                if (byte & 0x01)
                {
                    writePixel(x + i, y, false);
                }
            }
        }
    }
}

// cppcheck-suppress unusedFunction
Bitmap::Bitmap(OLEDDisplay *display) :
m_width(display->getWidth()),
m_height(display->getHeight()),
m_buffer(NULL)
{
    allocateBuffer();
}

// cppcheck-suppress unusedFunction
Bitmap::~Bitmap()
{
    if (m_buffer != NULL)
    {
        delete m_buffer;
    }
}

// cppcheck-suppress unusedFunction
uint16_t Bitmap::getWidth() const
{
    return m_width;
}

// cppcheck-suppress unusedFunction
uint16_t Bitmap::getHeight() const
{
    return m_height;
}

void Bitmap::writePixel(uint16_t x, uint16_t y, bool invert)
{
    if ((x >= 0) && (x < m_width) && (y >= 0) && (y < m_height))
    {
        if (invert)
        {
            m_buffer[x + (y / 8) * m_width] &= ~(1 << (y % 8));
        }
        else
        {
            m_buffer[x + (y / 8) * m_width] |= (1 << (y % 8));
        }
    }
}

// cppcheck-suppress unusedFunction
void Bitmap::setPixel(uint16_t x, uint16_t y)
{
    writePixel(x, y, false);
}

// cppcheck-suppress unusedFunction
void Bitmap::clearPixel(uint16_t x, uint16_t y)
{
    writePixel(x, y, true);
}

// cppcheck-suppress unusedFunction
bool Bitmap::getPixel(uint16_t x, uint16_t y) const
{
    if ((x >= 0) && (x < m_width) && (y >= 0) && (y < m_height))
    {
        return m_buffer[x + (y / 8) * m_width] & (1 << (y % 8));
    }
    return false;
}

// cppcheck-suppress unusedFunction
void Bitmap::clear() {
    memset(m_buffer, 0, (m_width * m_height) / 8);
}

// cppcheck-suppress unusedFunction
void Bitmap::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    int dx  = abs(x1 - x0);
    int dy  = abs(y1 - y0);
    int sx  = x0 < x1 ? 1 : -1;
    int sy  = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    while (true)
    {
        setPixel(x0, y0);
        if ((x0 == x1) && (y0 == y1))
        {
            break;
        }

        int e2 = err;
        if (e2 > -dx)
        {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dy)
        {
            err += dx;
            y0 += sy;
        }
    }
}

// cppcheck-suppress unusedFunction
void Bitmap::drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length)
{
    if ((y < 0) || (y >= m_height))
    {
        return;
    }

    for (uint16_t i = 0; i < length; i++)
    {
        setPixel(x + i, y);
    }
}

// cppcheck-suppress unusedFunction
void Bitmap::drawVerticalLine(uint16_t x, uint16_t y, uint16_t length)
{
    if ((x < 0) || (x >= m_width))
    {
        return;
    }

    for (uint16_t i = 0; i < length; i++)
    {
        setPixel(x, y + i);
    }
}

// cppcheck-suppress unusedFunction
void Bitmap::drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    drawHorizontalLine(x, y, width);
    drawVerticalLine(x, y, height);
    drawVerticalLine(x + width - 1, y, height);
    drawHorizontalLine(x, y + height - 1, width);
}

// cppcheck-suppress unusedFunction
void Bitmap::fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    for (uint16_t i = 0; i < width; i++)
    {
        drawVerticalLine(x + i, y, height);
    }
}

// cppcheck-suppress unusedFunction
void Bitmap::drawCircle(uint16_t x0, uint16_t y0, uint16_t radius)
{
    int x  = 0;
    int y  = radius;
    int dp = 1 - radius;

    do
    {
        if (dp < 0)
        {
            dp = dp + (x++) * 2 + 3;
        }
        else
        {
            dp = dp + (x++) * 2 - (y--) * 2 + 5;
        }

        setPixel(x0 + x, y0 + y); // For the 8 octants
        setPixel(x0 - x, y0 + y);
        setPixel(x0 + x, y0 - y);
        setPixel(x0 - x, y0 - y);
        setPixel(x0 + y, y0 + x);
        setPixel(x0 - y, y0 + x);
        setPixel(x0 + y, y0 - x);
        setPixel(x0 - y, y0 - x);
    } while (x < y);

    setPixel(x0 + radius, y0);
    setPixel(x0, y0 + radius);
    setPixel(x0 - radius, y0);
    setPixel(x0, y0 - radius);
}

// cppcheck-suppress unusedFunction
void Bitmap::fillCircle(uint16_t x0, uint16_t y0, uint16_t radius)
{
    int x  = 0;
    int y  = radius;
    int dp = 1 - radius;

    do
    {
        if (dp < 0)
        {
            dp = dp + (x++) * 2 + 3;
        }
        else
        {
            dp = dp + (x++) * 2 - (y--) * 2 + 5;
        }

        drawHorizontalLine(x0 - x, y0 - y, 2 * x);
        drawHorizontalLine(x0 - x, y0 + y, 2 * x);
        drawHorizontalLine(x0 - y, y0 - x, 2 * y);
        drawHorizontalLine(x0 - y, y0 + x, 2 * y);
    } while (x < y);

    drawHorizontalLine(x0 - radius, y0, 2 * radius);
}

// cppcheck-suppress unusedFunction
void Bitmap::drawCircleQuads(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t quads)
{
    int x  = 0;
    int y  = radius;
    int dp = 1 - radius;

    while (x < y)
    {
        if (dp < 0)
        {
            dp = dp + (x++) * 2 + 3;
        }
        else
        {
            dp = dp + (x++) * 2 - (y--) * 2 + 5;
        }

        if (quads & 0x1)
        {
            setPixel(x0 + x, y0 - y);
            setPixel(x0 + y, y0 - x);
        }

        if (quads & 0x2)
        {
            setPixel(x0 - y, y0 - x);
            setPixel(x0 - x, y0 - y);
        }

        if (quads & 0x4)
        {
            setPixel(x0 - y, y0 + x);
            setPixel(x0 - x, y0 + y);
        }

        if (quads & 0x8)
        {
            setPixel(x0 + x, y0 + y);
            setPixel(x0 + y, y0 + x);
        }
    }

    if ((quads & 0x1) && (quads & 0x8))
    {
        setPixel(x0 + radius, y0);
    }

    if ((quads & 0x4) && (quads & 0x8))
    {
        setPixel(x0, y0 + radius);
    }

    if ((quads & 0x2) && (quads & 0x4))
    {
        setPixel(x0 - radius, y0);
    }

    if ((quads & 0x1) && (quads & 0x2))
    {
        setPixel(x0, y0 - radius);
    }
}

// cppcheck-suppress unusedFunction
void Bitmap::drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t progress)
{
    int radius       = height / 2;
    int xRadius      = x + radius;
    int yRadius      = y + radius;
    int doubleRadius = 2 * radius;
    int innerRadius  = radius - 2;

    drawCircleQuads(xRadius, yRadius, radius, 0b00000110);
    drawHorizontalLine(xRadius, y, width - doubleRadius + 1);
    drawHorizontalLine(xRadius, y + height, width - doubleRadius + 1);
    drawCircleQuads(x + width - radius, yRadius, radius, 0b00001001);

    uint16_t maxProgressWidth = (width - doubleRadius + 1) * progress / 100;

    fillCircle(xRadius, yRadius, innerRadius);
    fillRect(xRadius + 1, y + 2, maxProgressWidth, height - 3);
    fillCircle(xRadius + maxProgressWidth, yRadius, innerRadius);
}

// cppcheck-suppress unusedFunction
uint16_t Bitmap::drawChar(uint16_t x, uint16_t y, bool invert, char c)
{
    fontDesc_t const *font = getSystemFont();

    if (c == ' ')
    {
        return (x + font->widthInPixel * 4 / 10);
    }

    // Out of boundaries
    unsigned char cu = (unsigned char)c;
    if (cu < font->firstChar || cu > font->lastChar)
    {
        cu = '?';
    }

    uint16_t firstPixelBitPos = 0;
    for (uint16_t i = 0; i < (cu - font->firstChar); i++)
    {
        firstPixelBitPos = firstPixelBitPos + font->pData[i];
    }
    firstPixelBitPos = firstPixelBitPos * font->heightInPixel;

    unsigned char const *pDataStart   = &(font->pData[font->lastChar - font->firstChar + 1]);
    const uint16_t       top          = y;
    const uint16_t       widthInPixel = font->pData[cu - font->firstChar];
    for (uint16_t i = 0; i < widthInPixel * font->heightInPixel; i++)
    {
        uint16_t bytePos = firstPixelBitPos / 8;
        uint16_t bitPos  = firstPixelBitPos % 8;

        if (pDataStart[bytePos] & (1 << bitPos))
        {
            writePixel(x, y, invert);
        }
        /*
        else
        {
            clearPixel(x, y);
        }
        */

        firstPixelBitPos++;
        y++;
        if (y == (top + font->heightInPixel))
        {
            y = top;
            x++;
        }
    }

    return (x + FONT_CHAR_SPACING);
}

// cppcheck-suppress unusedFunction
uint16_t Bitmap::drawString(uint16_t x, uint16_t y, bool invert, String text)
{
    uint16_t next_x = x;

    for (uint16_t i = 0; i < text.length(); i++) {
        next_x = drawChar(next_x, y, invert, text[i]);
    }

    return next_x;
}

// cppcheck-suppress unusedFunction
void Bitmap::drawStringf(uint16_t x, uint16_t y, bool invert, char *buffer, String format, ...)
{
    va_list myargs;
    va_start(myargs, format);
    vsprintf(buffer, format.c_str(), myargs);
    va_end(myargs);
    drawString(x, y, invert, buffer);
}

// cppcheck-suppress unusedFunction
uint16_t Bitmap::drawStringLF(uint16_t x, uint16_t y, bool invert, String text)
{
    fontDesc_t const *font   = getSystemFont();
    uint16_t          next_x = x;

    for (uint16_t i = 0; i < text.length(); i++)
    {
        if ((next_x + font->widthInPixel) > m_width)
        {
            next_x = 0;
            y += font->heightInPixel;
        }
        next_x = drawChar(next_x, y, invert, text[i]);
    }

    return next_x;
}

// cppcheck-suppress unusedFunction
void Bitmap::drawStringLFf(uint16_t x, uint16_t y, bool invert, char *buffer, String format, ...)
{
    va_list myargs;
    va_start(myargs, format);
    vsprintf(buffer, format.c_str(), myargs);
    va_end(myargs);
    drawStringLF(x, y, invert, buffer);
}

/*void Bitmap::drawBitmap(uint16_t x, uint16_t y, const Bitmap & bitmap)
{
        if(_width < x + bitmap.getWidth() || _height < y + bitmap.getHeight())
        {
                return;
        }

        for(int _x = 0; _x < bitmap.getWidth(); _x++)
        {
                for(int _y = 0; _y < bitmap.getHeight(); _y++)
                {
                        if(bitmap.getPixel(_x, _y))
                        {
                                // _buffer[x + (y / 8) * _width] |= (1 << (y % 8));
                                // return _buffer[x + (y / 8) * _width] & (1 << (y % 8));
                                Serial.print(_x);
                                Serial.print(" ");
                                Serial.println(_y);
                                setPixel(x + _x, y + _y);
                        }
                        else
                        {
                                clearPixel(x + _x, y + _y);
                        }
                }
        }
}*/

// cppcheck-suppress unusedFunction
void Bitmap::allocateBuffer()
{
    m_buffer = new uint8_t[m_width * m_height / 8];
    clear();
}
