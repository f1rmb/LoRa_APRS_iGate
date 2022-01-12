#ifndef BITMAP_H_
#define BITMAP_H_

#include <Arduino.h>
#include <memory>

class OLEDDisplay;

class Bitmap
{
        friend class SSD1306;

    public:
        explicit Bitmap(uint16_t width, uint16_t height);
        explicit Bitmap(uint16_t width, uint16_t height, const uint8_t xBuffer[]);
        explicit Bitmap(OLEDDisplay *display);
        virtual ~Bitmap();

        uint16_t getWidth() const;
        uint16_t getHeight() const;

        void setPixel(uint16_t x, uint16_t y);
        void clearPixel(uint16_t x, uint16_t y);
        bool getPixel(uint16_t x, uint16_t y) const;
        void clear();

        void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
        void drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length);
        void drawVerticalLine(uint16_t x, uint16_t y, uint16_t length);

        void drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

        void drawCircle(uint16_t x0, uint16_t y0, uint16_t radius);
        void fillCircle(uint16_t x0, uint16_t y0, uint16_t radius);
        void drawCircleQuads(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t quads);

        void drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t progress);

        uint16_t  drawChar(uint16_t x, uint16_t y, bool invert, char c);
        uint16_t  drawString(uint16_t x, uint16_t y, bool invert, String text);
        void drawStringf(uint16_t x, uint16_t y, bool invert, char *buffer, String format, ...);
        uint16_t  drawStringLF(uint16_t x, uint16_t y, bool invert, String text);
        void drawStringLFf(uint16_t x, uint16_t y, bool invert, char *buffer, String format, ...);

        // void drawBitmap(uint16_t x, uint16_t y, const Bitmap & bitmap);

    private:
        void allocateBuffer();
        void writePixel(uint16_t x, uint16_t y, bool invert);

    private:
        const uint16_t   m_width;
        const uint16_t   m_height;
        uint8_t         *m_buffer;
};

#endif
