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
        data["source"]      = msg->getSource();
        data["destination"] = msg->getDestination();
        data["path"]        = msg->getPath();
        data["type"]        = msg->getType().toString();
        String body         = msg->getBody()->encode();
        body.replace("\n", "");
        data["data"] = body;

        String r;
        serializeJson(data, r);

        String topic = String(system.getUserConfig()->mqtt.topic);
        if (!topic.endsWith("/"))
        {
            topic = topic + "/";
        }
        topic = topic + system.getUserConfig()->callsign;

        system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, getName(), "Send MQTT with topic: '%s', data: %s", topic.c_str(), r.c_str());

        m_MQTT.publish(topic.c_str(), r.c_str());
    }

    m_MQTT.loop();

    return true;
}

bool MQTTTask::connect(System &system)
{
    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "Connecting to MQTT broker: %s on port %d", system.getUserConfig()->mqtt.server.c_str(), system.getUserConfig()->mqtt.port);

    if (m_MQTT.connect(system.getUserConfig()->callsign.c_str(), system.getUserConfig()->mqtt.name.c_str(), system.getUserConfig()->mqtt.password.c_str()))
    {
        system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "Connected to MQTT broker as: %s", system.getUserConfig()->callsign.c_str());
        m_state = Okay;
        return true;
    }

    system.getLogger().log(logging::LoggerLevel::LOGGER_LEVEL_INFO, getName(), "Connecting to MQTT broker failed. Try again later.");
    m_state = Error;
    return false;
}
