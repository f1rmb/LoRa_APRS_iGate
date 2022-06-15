/**
 * The MIT License (MIT)
 * Copyright (c) 2015 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "NTPClient.h"

NTPClient::NTPClient() :
m_udpSetup(false),
m_poolServerName("pool.ntp.org"),
m_port(NTP_DEFAULT_LOCAL_PORT),
m_timeOffset(0),
m_updateInterval(3600000),
m_currentEpoc(0),
m_lastUpdate(0)
{
}

NTPClient::NTPClient(long timeOffset) :
m_udpSetup(false),
m_poolServerName("pool.ntp.org"),
m_port(NTP_DEFAULT_LOCAL_PORT),
m_timeOffset(timeOffset),
m_updateInterval(60000),
m_currentEpoc(0),
m_lastUpdate(0)

{
}

NTPClient::NTPClient(const char* poolServerName) :
m_udpSetup(false),
m_poolServerName(poolServerName),
m_port(NTP_DEFAULT_LOCAL_PORT),
m_timeOffset(0),
m_updateInterval(60000),
m_currentEpoc(0),
m_lastUpdate(0)
{
}

NTPClient::NTPClient(IPAddress poolServerIP) :
m_udpSetup(false),
m_poolServerName(NULL),
m_poolServerIP(poolServerIP),
m_port(NTP_DEFAULT_LOCAL_PORT),
m_timeOffset(0),
m_updateInterval(60000),
m_currentEpoc(0),
m_lastUpdate(0)
{
}

NTPClient::NTPClient(const char* poolServerName, long timeOffset) :
m_udpSetup(false),
m_poolServerName(poolServerName),
m_port(NTP_DEFAULT_LOCAL_PORT),
m_timeOffset(timeOffset),
m_updateInterval(60000),
m_currentEpoc(0),
m_lastUpdate(0)
{
}

NTPClient::NTPClient(IPAddress poolServerIP, long timeOffset) :
m_udpSetup(false),
m_poolServerName(NULL),
m_poolServerIP(poolServerIP),
m_port(NTP_DEFAULT_LOCAL_PORT),
m_timeOffset(timeOffset),
m_updateInterval(60000),
m_currentEpoc(0),
m_lastUpdate(0)
{
}

NTPClient::NTPClient(const char* poolServerName, long timeOffset, unsigned long updateInterval) :
m_udpSetup(false),
m_poolServerName(poolServerName),
m_port(NTP_DEFAULT_LOCAL_PORT),
m_timeOffset(timeOffset),
m_updateInterval(updateInterval),
m_currentEpoc(0),
m_lastUpdate(0)
{
}

NTPClient::NTPClient(IPAddress poolServerIP, long timeOffset, unsigned long updateInterval) :
m_udpSetup(false),
m_poolServerName(NULL),
m_poolServerIP(poolServerIP),
m_port(NTP_DEFAULT_LOCAL_PORT),
m_timeOffset(timeOffset),
m_updateInterval(updateInterval),
m_currentEpoc(0),
m_lastUpdate(0)
{
}

void NTPClient::begin()
{
    this->begin(NTP_DEFAULT_LOCAL_PORT);
}

void NTPClient::begin(unsigned int port)
{
    this->m_port = port;

    this->m_udp.begin(this->m_port);

    this->m_udpSetup = true;
}

bool NTPClient::forceUpdate()
{
#ifdef DEBUG_NTPClient
    Serial.println("Update from NTP Server");
#endif

    // flush any existing packets
    while(this->m_udp.parsePacket() != 0)
    {
        this->m_udp.flush();
    }

    this->sendNTPPacket();

    // Wait till data is there or timeout...
    byte timeout = 0;
    int cb = 0;
    do
    {
        delay (10);
        cb = this->m_udp.parsePacket();
        if (timeout > 100)
        {
            return false; // timeout after 1000 ms
        }

        timeout++;
    } while (cb == 0);

    this->m_lastUpdate = millis() - (10 * (timeout + 1)); // Account for delay in reading the time

    this->m_udp.read(this->m_packetBuffer, NTP_PACKET_SIZE);

    unsigned long highWord = word(this->m_packetBuffer[40], this->m_packetBuffer[41]);
    unsigned long lowWord = word(this->m_packetBuffer[42], this->m_packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    this->m_currentEpoc = secsSince1900 - SEVENZYYEARS;

    return true;  // return true after successful update
}

bool NTPClient::update()
{
    if ((millis() - this->m_lastUpdate >= this->m_updateInterval)     // Update after _updateInterval
            || this->m_lastUpdate == 0)                              // Update if there was no update yet.
    {
        if (!this->m_udpSetup || this->m_port != NTP_DEFAULT_LOCAL_PORT)  // setup the UDP client if needed
        {
            this->begin(this->m_port);
        }

        return this->forceUpdate();
    }

    return false;   // return false if update does not occur
}

unsigned long NTPClient::getEpochTime() const
{
    return this->m_timeOffset + // User offset
            this->m_currentEpoc + // Epoc returned by the NTP server
            ((millis() - this->m_lastUpdate) / 1000); // Time since last update
}

int NTPClient::getDay() const
{
    return (((this->getEpochTime()  / 86400L) + 4 ) % 7); //0 is Sunday
}
int NTPClient::getHours() const
{
    return ((this->getEpochTime()  % 86400L) / 3600);
}
int NTPClient::getMinutes() const
{
    return ((this->getEpochTime() % 3600) / 60);
}
int NTPClient::getSeconds() const
{
    return (this->getEpochTime() % 60);
}

String NTPClient::getFormattedTime(unsigned long rawTime) const
{
    unsigned long hours = (rawTime % 86400L) / 3600;
    String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

    unsigned long minutes = (rawTime % 3600) / 60;
    String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

    unsigned long seconds = rawTime % 60;
    String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

    return hoursStr + ":" + minuteStr + ":" + secondStr;
}

String NTPClient::getFormattedTime() const
{
    return getFormattedTime(this->getEpochTime());
}

void NTPClient::end()
{
    this->m_udp.stop();

    this->m_udpSetup = false;
}

void NTPClient::setTimeOffset(int timeOffset)
{
    this->m_timeOffset = timeOffset;
}

void NTPClient::setUpdateInterval(unsigned long updateInterval)
{
    this->m_updateInterval = updateInterval;
}

void NTPClient::setPoolServerName(const char* poolServerName)
{
    this->m_poolServerName = poolServerName;
}

void NTPClient::sendNTPPacket()
{
    // set all bytes in the buffer to 0
    memset(this->m_packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)

    this->m_packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    this->m_packetBuffer[1] = 0;     // Stratum, or type of clock
    this->m_packetBuffer[2] = 6;     // Polling Interval
    this->m_packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    this->m_packetBuffer[12]  = 49;
    this->m_packetBuffer[13]  = 0x4E;
    this->m_packetBuffer[14]  = 49;
    this->m_packetBuffer[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    if  (this->m_poolServerName)
    {
        this->m_udp.beginPacket(this->m_poolServerName, 123);
    }
    else
    {
        this->m_udp.beginPacket(this->m_poolServerIP, 123);
    }

    this->m_udp.write(this->m_packetBuffer, NTP_PACKET_SIZE);
    this->m_udp.endPacket();
}

void NTPClient::setRandomPort(unsigned int minValue, unsigned int maxValue)
{
    randomSeed(analogRead(0));
    this->m_port = random(minValue, maxValue);
}
