#include "Display.h"
#include <TaskManager.h>
#include <logger.h>

static const uint8_t bootscreen[] =
{
        0x00, 0x60, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x00, 0x78, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x7f, 0x00, 0x00,
        0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xfc,
        0xe0, 0x1f, 0x00, 0x0c, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf7, 0xfd, 0xf8, 0x0f, 0xf8, 0x3e, 0xe0, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x17, 0xfd, 0x7c, 0x0c, 0xf8, 0x7f,
        0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x17, 0xfd,
        0x7c, 0x0c, 0x9c, 0x73, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xf7, 0xfd, 0xf8, 0x0f, 0x8c, 0x61, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x01, 0xc0, 0x1f, 0x0c, 0x61,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x01,
        0x00, 0x7f, 0x0c, 0x61, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x01, 0x00, 0x00,
        0x00, 0x00, 0x10, 0x01, 0x00, 0x78, 0xfc, 0x7f, 0x00, 0x00, 0x00, 0xfe,
        0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x10, 0x01, 0xf0, 0x40, 0xfc, 0x7f,
        0x00, 0x00, 0xc0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x10, 0x01,
        0xf8, 0x03, 0xfc, 0x7f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x00, 0x00,
        0x00, 0x00, 0x10, 0x01, 0xf8, 0x03, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f,
        0x00, 0xfe, 0x03, 0x00, 0x00, 0x00, 0xf0, 0x01, 0x1c, 0x03, 0x00, 0x00,
        0x00, 0x00, 0xf0, 0x01, 0x00, 0xf0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0c, 0x03, 0xfc, 0x7f, 0x00, 0x00, 0x70, 0xc0, 0x7f, 0xc0, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x0c, 0x03, 0xfc, 0x7f, 0x00, 0x00, 0x20, 0xf8,
        0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x03, 0xfc, 0x7f,
        0x00, 0x00, 0x00, 0xfe, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xfc, 0x7f, 0x7c, 0x00, 0x00, 0x00, 0x80, 0x7f, 0xc0, 0x3f, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0xf0, 0x03, 0x00, 0x00, 0x80, 0x07,
        0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0x80, 0x1f,
        0xe0, 0x0f, 0x00, 0x01, 0x00, 0x10, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x7c, 0xe0, 0x0f, 0x00, 0xc0, 0x7f, 0x00, 0xff, 0xff,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x7c, 0xe0, 0x0f, 0x00, 0xf0,
        0xff, 0x03, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0xf8, 0x3e, 0x00, 0x3f,
        0xe0, 0x0f, 0x00, 0xfc, 0xe0, 0x07, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00,
        0xf8, 0x7f, 0xe0, 0x07, 0xe0, 0x0f, 0x00, 0x18, 0x00, 0x03, 0xff, 0xff,
        0x1f, 0x00, 0x00, 0x00, 0x9c, 0x73, 0xfc, 0x00, 0xe0, 0x0f, 0x00, 0x00,
        0x00, 0x00, 0x3f, 0xc0, 0x3f, 0x00, 0x00, 0x00, 0x8c, 0x61, 0xfc, 0x7f,
        0xe0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x3f, 0x00, 0x00, 0x00,
        0x0c, 0x61, 0xfc, 0x7f, 0xe0, 0x0f, 0x00, 0xc0, 0x7f, 0x00, 0x3f, 0x80,
        0x3f, 0xf8, 0x3f, 0x00, 0x0c, 0x61, 0xfc, 0x7f, 0xe0, 0x0f, 0x00, 0xf0,
        0xff, 0x01, 0x3f, 0x80, 0x3f, 0xfe, 0xff, 0x00, 0xfc, 0x7f, 0x00, 0x00,
        0xe0, 0x0f, 0x00, 0xf8, 0xff, 0x03, 0x3f, 0x80, 0x3f, 0xff, 0xff, 0x00,
        0xfc, 0x7f, 0x60, 0x40, 0xe0, 0x0f, 0x00, 0xfc, 0xf9, 0x03, 0x3f, 0xc0,
        0x3f, 0x7f, 0xfe, 0x01, 0xfc, 0x7f, 0xf8, 0x71, 0xe0, 0x0f, 0x00, 0xfe,
        0xe0, 0x07, 0xff, 0xff, 0x9f, 0x1f, 0xf8, 0x01, 0x00, 0x00, 0xf8, 0x7f,
        0xe0, 0x0f, 0x00, 0x7e, 0xe0, 0x07, 0xff, 0xff, 0x8f, 0x1f, 0xf8, 0x01,
        0x00, 0x0c, 0x1c, 0x1f, 0xe0, 0x0f, 0x00, 0x7e, 0xe0, 0x0f, 0xff, 0xff,
        0x07, 0x00, 0xf8, 0x01, 0x0c, 0x3f, 0x0c, 0x0f, 0xe0, 0x0f, 0x00, 0x7e,
        0xe0, 0x0f, 0xff, 0xff, 0x03, 0x80, 0xff, 0x01, 0x8c, 0x7f, 0x0c, 0x03,
        0xe0, 0x0f, 0x00, 0x7f, 0xc0, 0x0f, 0xff, 0xff, 0x03, 0xfc, 0xff, 0x01,
        0x8c, 0x61, 0x0c, 0x03, 0xe0, 0x0f, 0x00, 0x7f, 0xc0, 0x0f, 0x3f, 0xf8,
        0x03, 0xff, 0xff, 0x01, 0x8c, 0x61, 0xfc, 0x7f, 0xe0, 0x0f, 0x00, 0x7e,
        0xc0, 0x0f, 0x3f, 0xf0, 0x87, 0xff, 0xf8, 0x01, 0x8c, 0x61, 0xfc, 0x7f,
        0xe0, 0x0f, 0x00, 0x7e, 0xe0, 0x0f, 0x3f, 0xf0, 0x87, 0x3f, 0xf8, 0x01,
        0xfc, 0x71, 0xfc, 0x7f, 0xe0, 0x0f, 0x00, 0x7e, 0xe0, 0x0f, 0x3f, 0xf0,
        0x8f, 0x1f, 0xf8, 0x01, 0xfc, 0x39, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x7e,
        0xe0, 0x07, 0x3f, 0xe0, 0xcf, 0x1f, 0xf8, 0x01, 0xc0, 0x19, 0x00, 0x00,
        0xe0, 0xff, 0x7f, 0xfc, 0xf0, 0x07, 0x3f, 0xe0, 0xcf, 0x1f, 0xfc, 0x01,
        0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x7f, 0xfc, 0xff, 0x03, 0x3f, 0xc0,
        0x9f, 0x3f, 0xff, 0x01, 0x0c, 0x60, 0x00, 0x00, 0xe0, 0xff, 0x7f, 0xf8,
        0xff, 0x01, 0x3f, 0xc0, 0x9f, 0xff, 0xff, 0x01, 0x8c, 0x61, 0xfc, 0x7f,
        0xe0, 0xff, 0x7f, 0xf0, 0xff, 0x00, 0x3f, 0xc0, 0x3f, 0xff, 0xfb, 0x01,
        0x8c, 0x61, 0xf8, 0x7f, 0xe0, 0xff, 0x7f, 0xc0, 0x3f, 0x00, 0x3f, 0x80,
        0x3f, 0xfe, 0xf1, 0x03, 0x8c, 0x61, 0xf8, 0x7f, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8c, 0x61, 0x38, 0x00,
        0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x8c, 0x61, 0x30, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x03, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0x30, 0x00, 0x00, 0x00, 0x00, 0xfc,
        0xf1, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xf0, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x03, 0x00, 0x00, 0x00, 0x03,
        0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0x0c, 0x03,
        0x00, 0x00, 0x80, 0x0f, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xf0, 0x1f, 0x0c, 0x03, 0x00, 0x00, 0x80, 0xff, 0xe0, 0x3f, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f, 0x0c, 0x03, 0x00, 0x00, 0x00, 0xfe,
        0xff, 0x0f, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x38, 0x70, 0x0c, 0x03,
        0x00, 0x00, 0x20, 0xf0, 0xff, 0x83, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x1c, 0x60, 0xfc, 0x7f, 0x00, 0x00, 0xf0, 0x80, 0x3f, 0xe0, 0xf8, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x0c, 0x60, 0xfc, 0x7f, 0x00, 0x00, 0xf0, 0x03,
        0x00, 0xf8, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0c, 0x60, 0xfc, 0x7f,
        0x00, 0x00, 0xf8, 0x3f, 0x80, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x1c, 0x70, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0xf8, 0xff,
        0xff, 0xff, 0xff, 0xff, 0x78, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff,
        0xff, 0x3f, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x3f, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xfc, 0xff, 0x07, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xe0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x00, 0xf8, 0xff,
        0xff, 0xff, 0xff, 0xff
};


Display::Display() :
m_disp(0),
m_statusFrame(0),
m_displaySaveMode(false)
{
}

Display::~Display()
{
}

void Display::setup(BoardConfig const *const boardConfig, bool rotate)
{
    if (boardConfig->OledReset != 0)
    {
        pinMode(boardConfig->OledReset, OUTPUT);
        digitalWrite(boardConfig->OledReset, HIGH);
        delay(1);
        digitalWrite(boardConfig->OledReset, LOW);
        delay(10);
        digitalWrite(boardConfig->OledReset, HIGH);
    }

    Wire.begin(boardConfig->OledSda, boardConfig->OledScl);
    m_disp = new SSD1306(&Wire, boardConfig->OledAddr);

    if (rotate)
    {
        m_disp->rotate180();
    }

    Bitmap bitmap(m_disp->getWidth(), m_disp->getHeight());
    m_disp->display(&bitmap);

    m_displayFrameRate.setTimeout(500);
    m_displayFrameRate.start();

    m_frameTimeout.setTimeout(15 * 1000);
    m_displaySaveModeTimer.setTimeout(10 * 1000);
}

void Display::activateDisplaySaveMode()
{
    m_displaySaveMode = true;
}

void Display::setDisplaySaveTimeout(uint32_t timeout)
{
    m_displaySaveModeTimer.setTimeout(timeout * 1000);
}

void Display::activateDisplay()
{
    m_disp->displayOn();
}

void Display::deactivateDisplay()
{
    m_disp->displayOff();
}

void Display::update()
{
    if (m_displayFrameRate.hasExpired())
    {
        if (m_frames.size() > 0)
        {
            std::shared_ptr<DisplayFrame> frame = *m_frames.begin();
            Bitmap                        bitmap(m_disp);
            frame->drawStatusPage(bitmap);
            m_disp->display(&bitmap);

            if (m_frameTimeout.isRunning() == false)
            {
                m_frameTimeout.start();
                m_displaySaveModeTimer.stop();
            }
            else if (m_frameTimeout.hasExpired())
            {
                m_frames.pop_front();
                m_frameTimeout.stop();
            }
        }
        else
        {
            if (m_disp->isDisplayOn())
            {
                Bitmap bitmap(m_disp);
                m_statusFrame->drawStatusPage(bitmap);
                m_disp->display(&bitmap);

                if (m_displaySaveMode)
                {
                    if (m_displaySaveModeTimer.isRunning() && m_displaySaveModeTimer.hasExpired())
                    {
                        m_disp->displayOff();
                        m_displaySaveModeTimer.stop();
                    }
                    else if (m_displaySaveModeTimer.isRunning() == false)
                    {
                        m_displaySaveModeTimer.start();
                    }
                }
            }
        }

        m_displayFrameRate.start();
    }
}

void Display::addFrame(std::shared_ptr<DisplayFrame> frame)
{
    m_frames.push_back(frame);
}

void Display::setStatusFrame(std::shared_ptr<StatusFrame> frame)
{
    m_statusFrame = frame;
}

void Display::showSpashScreen(String firmwareTitle, String version)
{
    Bitmap bitmap(m_disp);
    bitmap.drawString(0, 10, false, firmwareTitle);
    bitmap.drawString(0, 20, false, version);
    bitmap.drawString(0, 35, false, "by Peter Buchegger");
    bitmap.drawString(30, 45, false, "OE5BPA");
    m_disp->display(&bitmap);
}

void Display::showSpashScreen(String version)
{
    Bitmap bitmap(m_disp->getWidth(), m_disp->getHeight(), bootscreen);
    bitmap.drawString(86, 56, true, version);
    m_disp->display(&bitmap);
}

void Display::showStatusScreen(String header, String text)
{
    Bitmap bitmap(m_disp);
    bitmap.drawString(0, 0, false, header);
    bitmap.drawStringLF(0, 10, false, text);
    m_disp->display(&bitmap);
}

uint16_t Display::getWidth()
{
    return m_disp->getWidth();
}

uint16_t Display::getHeight()
{
    return m_disp->getHeight();
}

void TextFrame::drawStatusPage(Bitmap &bitmap)
{
    bitmap.drawString(0, 0, false, m_header);
    bitmap.drawStringLF(0, 10, false, m_text);
}
