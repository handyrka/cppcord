#pragma once
#include "serializable.h"
#include "message.h"
#include "../core/http.h"
#include <algorithm>
#include <iostream>

namespace json
{
    class TypingStart : public Serializable
    {
    public:
        inline void from_json(Json::Value& value) override
        {
            m_UserID = value["user_id"].asString();
            m_Timestamp = value["timestamp"].asUInt64();
            m_ChannelID = value["channel_id"].asString();
        }

        // Members
        std::string m_UserID;
        u_long m_Timestamp;
        std::string m_ChannelID;
    };

    class Channel : public Serializable
    {
    public:        
        inline void from_json(Json::Value& value) override
        {
            m_Bitrate = value["bitrate"].asInt();
            m_RtcRegion = value["rtc-region"].asString();
            m_UserLimit = value["user_limit"].asInt();
            m_Flags = value["flags"].asInt();
            m_ID = value["id"].asString();
            m_Name = value["name"].asString();
            m_ParentID = value["parent_id"].asString();
            m_OwnerID = value["owner_id"].asString();
            m_IconHash = value["icon"].asString();
            m_LastMessageID = value["last_message_id"].asString();

            for(auto a : value["permission_overwrites"])
                m_PermissionOverwrites.push_back(a.asString());

            for(auto a : value["recipient_ids"])
                m_RecipientIDs.push_back(a.asString());

            m_Position = value["position"].asInt();
            m_RateLimit = value["rate_limit_per_user"].asInt();
            m_Topic = value["topic"].asString();
            m_Type = value["type"].asInt();
        }
        
        // Members
        int m_Bitrate = -1;
        std::string m_RtcRegion = "";
        int m_UserLimit;
        int m_Flags = 0;

        std::string m_ID;
        std::string m_Name;
        std::string m_ParentID;
        std::string m_LastMessageID;
        std::string m_OwnerID;
        std::vector<std::string> m_RecipientIDs;
        std::string m_Topic;
        std::string m_IconHash;
        std::vector<std::string> m_PermissionOverwrites;

        int m_Position = 0;
        int m_RateLimit;
        int m_Type = 0; 

        inline void send_typing() const 
        { 
            core::HttpClient::post_async(core::DISCORD_API_URL + "/channels/" + m_ID + "/typing", "", [](core::HttpResponse resp){ resp.dispose(); });
        }

        inline void fetch_messages_async(std::function<void(std::vector<Message>)> callback, int max = 50)
        {
            core::HttpClient::get_async(core::DISCORD_API_URL + "/channels/" + m_ID + "/messages?limit=" + std::to_string(max), [=](core::HttpResponse resp)
            {
                std::vector<Message> mgs;

                Json::Value value;
                Json::Reader reader;

                reader.parse(resp.to_string(), value);
                resp.dispose();

                for(auto& msg : value)
                {
                    Message m;
                    m.from_json(msg);
                    mgs.push_back(m);
                }

                std::reverse(mgs.begin(), mgs.end());

                callback(mgs);
            });
        }

        inline void fetch_message_async(std::string id, std::function<void(Message)> callback)
        {
            core::HttpClient::get_async(core::DISCORD_API_URL + "/channels/" + m_ID + "/messages/" + id, [=](core::HttpResponse resp)
            {
                Message msg;
                Json::Value value;
                Json::Reader reader;

                reader.parse(resp.to_string(), value);
                resp.dispose();

                msg.from_json(value);

                callback(msg);
            });
        }

        inline void send_message_async(MessagePost message)
        {
            core::HttpClient::post_async(core::DISCORD_API_URL + "/channels/" + m_ID + "/messages", message.to_json(), [&](core::HttpResponse resp)
            {
                std::cout << "[Channel::send_message_async()] Response from the server: " << resp.to_string() << std::endl;
                resp.dispose();
            });
        }

        inline bool is_private() const { return !m_RecipientIDs.empty(); }
        inline bool has_icon() const { return !m_IconHash.empty(); }

        inline std::string get_icon_url() { return "https://cdn.discordapp.com/channel-icons/" + m_ID + "/" + m_IconHash + ".png?size=32"; }
    };
}
