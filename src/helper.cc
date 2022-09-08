#include "helper.h"

#define DISCORD_EPOCH 1420070400000;

namespace helper
{
    std::string make_snowflake()
    {
        using namespace std::chrono;
        static uint64_t counter = 0;
        uint64_t mil = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        uint64_t epoch = mil - DISCORD_EPOCH;

        uint64_t snowflake = epoch << 22;
        return std::to_string(snowflake | counter++ % 4096);
    }

    Timestamp::Timestamp()
    {
    }

    Timestamp::Timestamp(std::string k)
    {
        // Dicsord timestamps are in UTC format (+00:00), so we have to convert it to local time

        struct tm tt = {0};
        double seconds;
        sscanf(k.c_str(), "%04d-%02d-%02dT%02d:%02d:%lf", &tt.tm_year, &tt.tm_mon, &tt.tm_mday, &tt.tm_hour, &tt.tm_min, &seconds);
        tt.tm_sec = (int) seconds;
        tt.tm_mon -= 1;
        tt.tm_year -= 1900;

        auto tts = mktime(&tt) - timezone;
        auto time = localtime(&tts);

        if(time == nullptr) return;

        m_Time = *time;
        m_Time.tm_isdst = !daylight;
    }

    std::string Timestamp::to_full_date() const
    {
        std::stringstream ss;
        ss << std::put_time(&m_Time, "%b%e %Y %r");
        return ss.str();
    }

    std::string Timestamp::to_short_date() const
    {
        std::stringstream ss;
        ss << std::put_time(&m_Time, "%D %r");
        return ss.str();
    }

}
