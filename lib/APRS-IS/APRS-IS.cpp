#include "APRS-IS.h"
#include <logger.h>

void APRS_IS::setup(const String &user, const String &passcode, const String &tool_name, const String &version)
{
    m_user      = user;
    m_passcode  = passcode;
    m_tool_name = tool_name;
    m_version   = version;
}

bool APRS_IS::connect(const String &server, const int port)
{
    const String login = "user " + m_user + " pass " + m_passcode + " vers " + m_tool_name + " " + m_version + "\n\r";
    return _connect(server, port, login);
}

bool APRS_IS::connect(const String &server, const int port, const String &filter)
{
    const String login = "user " + m_user + " pass " + m_passcode + " vers " + m_tool_name + " " + m_version + " filter " + filter + "\n\r";
    return _connect(server, port, login);
}

bool APRS_IS::_connect(const String &server, const int port, const String &login_line)
{
    if (!m_client.connect(server.c_str(), port))
    {
        logPrintlnE("Something went wrong on connecting! Is the server reachable?");
        return false;
    }

    sendMessage(login_line);
    while (true)
    {
        String line = m_client.readStringUntil('\n');
        if (line.indexOf("logresp") != -1)
        {
            if (line.indexOf("unverified") == -1)
            {
                return true;
            }
            else
            {
                logPrintlnE("User can not be verified with passcode!");
                return false;
            }
        }
    }

    return true;
}

bool APRS_IS::connected()
{
    return m_client.connected();
}

bool APRS_IS::sendMessage(const String &message)
{
    if (!connected())
    {
        return false;
    }

    m_client.println(message);
    return true;
}

bool APRS_IS::sendMessage(const std::shared_ptr<APRSMessage> message)
{
    if (!connected())
    {
        return false;
    }

    m_client.println(message->encode() + "\n");
    return true;
}

int APRS_IS::available()
{
    return m_client.available();
}

String APRS_IS::getMessage()
{
    String line;

    if (m_client.available() > 0)
    {
        line = m_client.readStringUntil('\n');
    }

    return line;
}

std::shared_ptr<APRSMessage> APRS_IS::getAPRSMessage()
{
    String line;

    if (m_client.available() > 0)
    {
        line = m_client.readStringUntil('\n');
    }

    if (line.startsWith("#"))
    {
        logPrintlnD(line);
        return 0;
    }

    if (line.length() == 0)
    {
        return 0;
    }

    std::shared_ptr<APRSMessage> msg = std::shared_ptr<APRSMessage>(new APRSMessage());
    msg->decode(line);
    return msg;
}
