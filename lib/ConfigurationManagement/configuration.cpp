#include "configuration.h"
#include <SPIFFS.h>
#include <logger.h>

ConfigurationManagement::ConfigurationManagement(const String &FilePath, const String &defaultFilePath)
{
    bool userFileIsValid = false;
    bool success = true;

    if (SPIFFS.begin(true) == false)
    {
        logPrintlnI("Mounting SPIFFS was not possible. Trying to format SPIFFS...");
        SPIFFS.format();

        if (!SPIFFS.begin())
        {
            logPrintlnE("Formating SPIFFS was not okay!");
            success = false;
        }
    }

    if (success)
    {
        File f = SPIFFS.open(FilePath);

        if (f != 0)
        {
            if (f.isDirectory() == false)
            {
                userFileIsValid = true;
            }

            f.close();
        }
    }

    m_FilePath = (userFileIsValid ? FilePath : defaultFilePath);
}

ConfigurationManagement::~ConfigurationManagement()
{
}

void ConfigurationManagement::readConfiguration(Configuration &conf)
{
    File file = SPIFFS.open(m_FilePath);

    if (!file)
    {
        logPrintlnE("Failed to open file for reading, using default configuration.");
        return;
    }

    DynamicJsonDocument  data(2048);
    DeserializationError error = deserializeJson(data, file);
    if (error != DeserializationError::Ok)
    {
        logPrintlnW("Failed to read file, using default configuration.");
    }
    // serializeJson(data, Serial);
    // Serial.println();
    file.close();

    readProjectConfiguration(data, conf);

    // update config in memory to get the new fields:
    writeConfiguration(conf);
}

void ConfigurationManagement::writeConfiguration(Configuration &conf)
{
    File file = SPIFFS.open(m_FilePath, "w");

    if (!file)
    {
        logPrintlnE("Failed to open file for writing...");
        return;
    }

    DynamicJsonDocument data(2048);

    writeProjectConfiguration(conf, data);

    serializeJson(data, file);
    // serializeJson(data, Serial);
    // Serial.println();
    file.close();
}
