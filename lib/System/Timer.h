#ifndef TIMER_H_
#define TIMER_H_

class Timer
{
    public:
        Timer();

        void          setTimeout(const unsigned long timeout_ms);
        unsigned long getRemainingInSecs() const;

        bool          isRunning() const;
        bool          hasExpired();
        void          start();
        void          stop();

    private:
        unsigned long m_timeout_ms;
        unsigned long m_start;
        bool m_hasExpired;
        bool m_isRunning;

};

#endif
