#ifndef TASK_OTA_H_
#define TASK_OTA_H_

#include <ArduinoOTA.h>
#include <TaskManager.h>

class OTATask : public Task
{
    public:
        OTATask();
        virtual ~OTATask();

        virtual bool setup(System &system) override;
        virtual bool loop(System &system) override;

    private:
        void onStart(System &system);
        void onEnd(System &system);
        void onProgress(System &system, unsigned int progress, unsigned int total);
        void onError(System &system, ota_error_t error);

    private:
        ArduinoOTAClass m_ota;
        bool            m_beginCalled;
};

#endif
