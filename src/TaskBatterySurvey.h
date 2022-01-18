#ifndef TASK_BATTERY_SURVEY_H_
#define TASK_BATTERY_SURVEY_H_

#include <TaskManager.h>

template<typename T>
class ValueAveraging
{
    protected:
        static const uint16_t ARRAY_SIZE_MAX = 30;

    public:
        ValueAveraging() :
            m_average(ARRAY_SIZE_MAX)
        {
            ResetValues();
        }
        ~ValueAveraging()
        {
        }

        void StackValue(T value)
        {
            if (value > 0)
            {
                m_offset = (m_offset + 1) % m_average;
                m_values[m_offset] = value;
            }
        }

        T GetValue()
        {
            uint16_t n = 0;
            double   sum = 0.0;

            for (uint16_t i = 0; i < m_average; i++)
            {
                if (isnan(m_values[i]))
                {
                    break;
                }

                if (m_values[i] > 0)
                {
                    sum += m_values[i];
                    n++;
                }
            }

            // No usable value found.
            if (n == 0)
            {
                return 0;
            }

            return static_cast<T>((sum / double(n)) + 0.5); // ceil
        }

        bool SetAverageCount(uint16_t v)
        {
            bool ret = false;

            if ((v >= 0) && (v <= ARRAY_SIZE_MAX))
            {
                // Zeroing the array
                ResetValues();
                m_average = v;
                ret = true;
            }

            return ret;
        }

        uint16_t GetAverageCount()
        {
            return m_average;
        }

        uint16_t GetMaxAverageCount()
        {
            return ARRAY_SIZE_MAX;
        }

        void ResetValues()
        {
            for (uint16_t i = 0; i < ARRAY_SIZE_MAX; i++)
            {
                m_values[i] = NAN;
            }

            m_offset = ARRAY_SIZE_MAX - 1;
        }

    private:
        double               m_values[ARRAY_SIZE_MAX]; ///< values array storage
        uint16_t             m_offset; ///< offset in m_values[]
        uint16_t             m_average; ///< max values used from m_values[] to build average value
};

class BatterySurveyTask : public Task
{
    public:
        BatterySurveyTask();
        virtual ~BatterySurveyTask();

        virtual bool setup(System &system) override;
        virtual bool loop(System &system) override;

        float        getBatteryVoltage();
        //bool         getBatteryIsConnected();
        //bool         getBatteryIsCharging();

    private:
        void         updateVoltageReading(System &system);

    private:
        static const uint16_t FULL_BATTERY_VOLTAGE = 4200;
        static const uint16_t FLAT_BATTERY_VOLTAGE = 3270;
        static const uint16_t CHARGING_BATTERY_VOLTAGE = 4210;
        static const uint16_t NO_BATTERY_VOLTAGE = 2230;

    private:
        BoardConfig const        *m_board;
        uint8_t                   m_batteryPin;
        //bool                      m_batteryIsConnected;
        //bool                      m_batteryIsCharging;
        unsigned long             m_lastBatteryReading;
        ValueAveraging<uint16_t>  m_average;
};

#endif
