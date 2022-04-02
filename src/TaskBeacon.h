#ifndef TASK_BEACON_H_
#define TASK_BEACON_H_

#include <OneButton.h>
#include <TinyGPS++.h>

#include <APRSMessage.h>
#include <TaskMQTT.h>
#include <TaskManager.h>

class BeaconTask : public Task
{
    public:
        BeaconTask(TaskQueue<std::shared_ptr<APRSMessage>> &toModem, TaskQueue<std::shared_ptr<APRSMessage>> &toAprsIs);
        virtual ~BeaconTask();

        virtual bool setup(System &system) override;
        virtual bool loop(System &system) override;
        bool         sendBeacon(System &system);
        bool         updatePosition(System &system, double latitude, double longitude);

    private:
        static void  pushButton();

    private:
        TaskQueue<std::shared_ptr<APRSMessage>> &m_toModem;
        TaskQueue<std::shared_ptr<APRSMessage>> &m_toAprsIs;

        std::shared_ptr<APRSMessage>             m_beaconMsg;
        Timer                                    m_beacon_timer;
        bool                                     m_forceBeaconing;

        HardwareSerial                           m_ss;
        TinyGPSPlus                              m_gps;
        bool                                     m_useGps;

        static uint                              m_instances;
        static OneButton                         m_userButton;
        static bool                              m_send_update;
};

#endif
