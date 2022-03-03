#ifndef TASK_WIFI_H_
#define TASK_WIFI_H_

#include <TaskManager.h>
#include <WiFiMulti.h>

class WifiTask : public Task
{
    public:
        WifiTask();
        virtual ~WifiTask();

        virtual void enable(bool enabled) override;
        virtual bool setup(System &system) override;
        virtual bool loop(System &system) override;

    private:
        WiFiMulti m_wiFiMulti;
        uint8_t   m_oldWifiStatus;
};

#endif
