#ifndef TASK_MQTT_H_
#define TASK_MQTT_H_

#include <APRSMessage.h>
#include <PubSubClient.h>
#include <TaskManager.h>
#include <WiFi.h>

class MQTTTask : public Task
{
    public:
        MQTTTask(TaskQueue<std::shared_ptr<APRSMessage>> &toMQTT);
        virtual ~MQTTTask();

        virtual bool setup(System &system) override;
        virtual bool loop(System &system) override;

    private:
        bool connect(System &system);

    private:
        TaskQueue<std::shared_ptr<APRSMessage>> &m_toMQTT;
        WiFiClient                               m_client;
        PubSubClient                             m_MQTT;

};

#endif
