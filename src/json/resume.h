#pragma once
#include "serializable.h"

namespace json::payloads
{
    class Resume : public Serializable
    {
    public:
        inline std::string to_json() const override
        {
            Json::Value json;
            json["op"] = 6; // Resume
            json["d"]["seq"] = m_Seq == 0 ? Json::Value::null : m_Seq;
            json["d"]["session_id"] = m_SessionId;
            json["d"]["token"] = m_Token;

            return json.toStyledString();
        }
        
        // Members
        std::string m_SessionId;
        std::string m_Token;
        int m_Seq = 0;
    };
}