#pragma once
#include <string>

namespace core
{
    enum class NotificationType
    {
        // Always log errors
        System,

        // Mentions, DMs & GCs
        Important = 1 << 2,

        // Guild messages
        General = 1 << 4
    };

    void send_notify(std::string title, std::string message, NotificationType type);
}