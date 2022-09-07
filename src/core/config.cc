#include "config.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <glibmm.h>
#include <iostream>

namespace core
{
    void Config::load()
    {
        using namespace Json;
        using namespace std;

        Value root;
        Json::Reader reader;

        if(!filesystem::exists("config.json"))
            write();

        ifstream file("config.json");
        stringstream str;
        str << file.rdbuf();
        file.close();

        if(!reader.parse(str.str(), root))
        {
            std::cerr << "[Config::load()] Could not parse config file. Falling back to default options!\n" << std::endl;
            return;
        }

        //Reading data
        s_Level = (NotificationLevel)root["notification_level"].asInt();
        s_Token = root["token"].asString();
        s_Status = root["status"].asString();

        std::cout << "[Config::load()] Configuration loaded successfully.\n" << std::endl;
    }

    void Config::write()
    {
        using namespace Json;
        using namespace std;

        Value root;

        //Writing data
        root["notification_level"] = (int)s_Level;
        root["token"] = s_Token;
        root["status"] = s_Status;

        //Output to a file
        ofstream file("config.json");
        file << root.toStyledString();
        file.close();
    }

    void Config::set_notification_level(NotificationLevel level)
    {
        s_Level = level;
        write();
    }

    void Config::set_token(std::string token)
    {
        s_Token = token;
        write();
    }

    void Config::set_status(std::string status)
    {
        s_Status = status;
        write();
    }

    NotificationLevel Config::get_notification_level()
    {
        return s_Level;
    }

    std::string Config::get_token()
    {
        return s_Token;
    }

    std::string Config::get_status()
    {
        return s_Status;
    }

    bool Config::should_authorize()
    {
        return s_Token.empty();
    }
}