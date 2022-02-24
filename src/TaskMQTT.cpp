#include <logger.h>

#include "Tasks.h"
#include "TaskMQTT.h"
#include "ProjectConfiguration.h"

#include <ArduinoJson.h>

MQTTTask::MQTTTask(TaskQueue<std::shared_ptr<APRSMessage>> &toMQTT) :
Task(TASK_MQTT, TaskMQTT),
m_toMQTT(toMQTT),
m_MQTT(m_client)
{
    m_visible = false; // Do not display its status on screen (displayed in RouterTask)
    m_state = Error;
}

MQTTTask::~MQTTTask()
{
}

bool MQTTTask::setup(System &system)
{
    m_MQTT.setServer(system.getUserConfig()->mqtt.server.c_str(), system.getUserConfig()->mqtt.port);
    return true;
}

bool MQTTTask::loop(System &system)
{
    if (!system.isWifiEthConnected())
    {
        return false;
    }

    if (!m_MQTT.connected())
    {
        connect(system);
    }

    if (!m_toMQTT.empty())
    {
        std::shared_ptr<APRSMessage> msg = m_toMQTT.getElement();

        DynamicJsonDocument data(1024);
        data["Source"]      = msg->getSource();
        data["Destination"] = msg->getDestination();
        data["Path"]        = msg->getPath();
        data["Type"]        = msg->getType().toString();
        String body         = msg->getBody()->encode();
        body.replace("\n", "");
        data["Data"] = body;

        String r;
        serializeJson(data, r);

        String topic = String(system.getUserConfig()->mqtt.topic);
        if (!topic.endsWith("/"))
        {
            topic = topic + "/";
        }
        topic = topic + system.getUserConfig()->callsign;

        logPrintD("Send MQTT with topic: \"");
        logPrintD(topic);
        logPrintD("\", data: ");
        logPrintlnD(r);

        m_MQTT.publish(topic.c_str(), r.c_str());
    }

    m_MQTT.loop();

    return true;
}

bool MQTTTask::connect(const System &system)
{
    logPrintI("Connecting to MQTT broker: ");
    logPrintI(system.getUserConfig()->mqtt.server);
    logPrintI(" on port ");
    logPrintlnI(String(system.getUserConfig()->mqtt.port));

    if (m_MQTT.connect(system.getUserConfig()->callsign.c_str(), system.getUserConfig()->mqtt.name.c_str(), system.getUserConfig()->mqtt.password.c_str()))
    {
        logPrintI("Connected to MQTT broker as: ");
        logPrintlnI(system.getUserConfig()->callsign);
        m_state = Okay;
        return true;
    }

    logPrintlnI("Connecting to MQTT broker failed. Try again later.");
    m_state = Error;
    return false;
}
