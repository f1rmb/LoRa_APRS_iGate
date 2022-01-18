#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <Arduino.h>
#include <BoardFinder.h>
#include <SSD1306.h>
#include <Timer.h>
#include <Wire.h>
#include <list>
#include <map>
#include <memory>

class Timer;
class StatusFrame;

class DisplayFrame
{
    public:
        DisplayFrame()
        {
        }
        virtual ~DisplayFrame()
        {
        }
        virtual void drawStatusPage(Bitmap &bitmap) = 0;
};

class Display
{
    public:
        Display();
        ~Display();

        void setup(BoardConfig const *const boardConfig, bool rotate);
        // setup functions
        void showSpashScreen(String firmwareTitle, String version);
        void showSpashScreen(String version);
        void setStatusFrame(std::shared_ptr<StatusFrame> frame);
        void showStatusScreen(String header, String text);

        void activateDisplaySaveMode();
        void setDisplaySaveTimeout(uint32_t timeout);

        void activateDisplay();
        void deactivateDisplay();

        // functions for update loop
        void update();
        void addFrame(std::shared_ptr<DisplayFrame> frame);

        uint16_t getWidth();
        uint16_t getHeight();

    private:
        OLEDDisplay                              *m_disp;
        Timer                                     m_displayFrameRate;
        std::shared_ptr<StatusFrame>              m_statusFrame;
        std::list<std::shared_ptr<DisplayFrame>>  m_frames;
        Timer                                     m_frameTimeout;
        bool                                      m_displaySaveMode;
        Timer                                     m_displaySaveModeTimer;
};

class TextFrame : public DisplayFrame
{
    public:
        TextFrame(String header, String text) :
            m_header(header),
            m_text(text)
        {
        }
        virtual ~TextFrame()
        {
        }
        void drawStatusPage(Bitmap &bitmap) override;

    private:
        String  m_header;
        String  m_text;
};

#endif
