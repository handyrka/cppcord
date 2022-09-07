#include "helper.h"
#include <chrono>

#define DISCORD_EPOCH 1420070400000;

namespace helper
{
    std::string make_snowflake()
    {
        using namespace std::chrono;
        static uint counter = 0;
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

        std::stringstream ss { k };
        tm dt;
        ss >> std::get_time(&dt, "%Y-%m-%dT%H:%M:%Sz");
        time_t t = std::mktime(&dt);
        m_Time = *std::localtime(&t);
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
