#include "notification.h"
#include "config.h"
#include <iostream>
#include <gtkmm.h>
#include <thread>
#include <future>

//TODO: add custom window widget
namespace core
{
    void show_window(std::string title, std::string body)
    {
    }

    void send_notify(std::string title, std::string message, NotificationType type)
    {
        if((int)Config::get_notification_level() < (int)type)
            return;

        //Show window here
    }
}
