#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>

#define SEVENZYYEARS 2208988800UL
#define NTP_PACKET_SIZE 48
#define NTP_DEFAULT_LOCAL_PORT 1337

class NTPClient
{
    public:
        NTPClient();
        explicit NTPClient(long timeOffset);
        explicit NTPClient(const char* poolServerName);
        NTPClient(const char* poolServerName, long timeOffset);
        NTPClient(const char* poolServerName, long timeOffset, unsigned long updateInterval);
        explicit NTPClient(IPAddress poolServerIP);
        NTPClient(IPAddress poolServerIP, long timeOffset);
        NTPClient(IPAddress poolServerIP, long timeOffset, unsigned long updateInterval);

        /**
         * Set time server name
         *
         * @param poolServerName
         */
        void setPoolServerName(const char* poolServerName);

        /**
         * Set random local port
         */
        void setRandomPort(unsigned int minValue = 49152, unsigned int maxValue = 65535);

        /**
         * Starts the underlying UDP client with the default local port
         */
        void begin();

        /**
         * Starts the underlying UDP client with the specified local port
         */
        void begin(unsigned int port);

        /**
         * This should be called in the main loop of your application. By default an update from the NTP Server is only
         * made every 60 seconds. This can be configured in the NTPClient constructor.
         *
         * @return true on success, false on failure
         */
        bool update();

        /**
         * This will force the update from the NTP Server.
         *
         * @return true on success, false on failure
         */
        bool forceUpdate();

        int getDay() const;
        int getHours() const;
        int getMinutes() const;
        int getSeconds() const;

        /**
         * Changes the time offset. Useful for changing timezones dynamically
         */
        void setTimeOffset(int timeOffset);

        /**
         * Set the update interval to another frequency. E.g. useful when the
         * timeOffset should not be set in the constructor
         */
        void setUpdateInterval(unsigned long updateInterval);

        /**
         * @return time formatted like `hh:mm:ss`
         */
        String getFormattedTime() const;

        /**
         * @return time in seconds since Jan. 1, 1970
         */
        unsigned long getEpochTime() const;

        /**
         * Stops the underlying UDP client
         */
        void end();

    private:
        void          sendNTPPacket();

    private:
        WiFiUDP       m_udp;
        bool          m_udpSetup;
        const char*   m_poolServerName; // Default time server: pool.ntp.org
        IPAddress     m_poolServerIP;
        unsigned int  m_port;
        long          m_timeOffset;
        unsigned long m_updateInterval; // In ms
        unsigned long m_currentEpoc;    // In s
        unsigned long m_lastUpdate;     // In ms
        byte          m_packetBuffer[NTP_PACKET_SIZE];


};
