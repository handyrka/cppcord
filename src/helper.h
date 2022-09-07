#pragma once
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <time.h>

namespace helper
{
    std::string make_snowflake();

     /**
     * @brief Handles ISO 8601 timestamp
     */
    class Timestamp
    {
    public:
        Timestamp();
        Timestamp(std::string k);

        std::string to_full_date() const;
        std::string to_short_date() const;

    private:
        tm m_Time;
    };
}
