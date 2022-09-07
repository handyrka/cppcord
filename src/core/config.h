#pragma once
#include <string>

namespace core
{
    enum class NotificationLevel
    {
        None,
        Mentions = 1 << 2,
        All = 1 << 4
    };

    /**
     * @brief Config storage
     */

    class Config
    {
    public:
        static void load();

        //Setters
        static void set_notification_level(NotificationLevel level);
        static void set_token(std::string token);
        static void set_status(std::string status);

        //Getters
        static NotificationLevel get_notification_level();
        static std::string get_token();
        static std::string get_status();
        static bool should_authorize();

    private:
        static inline NotificationLevel s_Level = NotificationLevel::Mentions;
        static inline std::string s_Token, s_Status = "online";

        static void write();
    };
}