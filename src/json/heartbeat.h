#pragma once
#include "serializable.h"

namespace json::payloads
{
    class Heartbeat : public Serializable
    {
    public:
        inline std::string to_json() const override
        {
            Json::Value json;
            json["op"] = 1; // Heartbeat
            json["d"] = m_Seq == 0 ? Json::Value::null : m_Seq;

            return json.toStyledString();
        }
        
        // Members
        int m_Seq = 0;
    };
}