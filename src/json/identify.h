#pragma once
#include "serializable.h"
#include "../core/config.h"
#include <fstream>

namespace json::payloads
{
    class Identify : public Serializable
    {
    public:
        inline std::string to_json() const override
        {
            Json::Value value;

            value["op"] = 2;
            value["d"]["token"] = m_Token;
            value["d"]["capabilities"] = 1021;

            value["d"]["properties"]["os"] = "Linux";
            value["d"]["properties"]["browser"] = "Firefox";
            value["d"]["properties"]["device"] = "";
            value["d"]["properties"]["system_locale"] = "en-US";
            value["d"]["properties"]["browser_user_agent"] = "Mozilla/5.0 (X11; Linux x86_64; rv:104.0) Gecko/20100101 Firefox/104.0";
            value["d"]["properties"]["browser_version"] = "104.0";
            value["d"]["properties"]["os_version"] = "";
            value["d"]["properties"]["referrer"] = "";
            value["d"]["properties"]["referring_domain"] = "";
            value["d"]["properties"]["referrer_current"] = "";
            value["d"]["properties"]["referring_domain_current"] = "";
            value["d"]["properties"]["release_channel"] = "stable";
            value["d"]["properties"]["client_build_number"] = 145429;
            value["d"]["properties"]["client_event_source"] = Json::Value::null;
            
            value["d"]["presence"]["status"] = core::Config::get_status();
            value["d"]["presence"]["since"] = 0;
            value["d"]["presence"]["activities"] = Json::arrayValue;
            value["d"]["presence"]["afk"] = false;

            value["d"]["compress"] = false;
            
            value["d"]["client_state"]["guild_hashes"] = Json::objectValue; 
            value["d"]["client_state"]["highest_last_message_id"] = "0";
            value["d"]["client_state"]["read_state_version"] = 0; 
            value["d"]["client_state"]["user_guild_settings_version"] = -1; 
            value["d"]["client_state"]["user_settings_version"] = -1; 

            return value.toStyledString();
        }
        
        // Members
        std::string m_Token;
    };
}