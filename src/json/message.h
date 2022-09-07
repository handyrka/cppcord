#pragma once
#include "serializable.h"
#include "user.h"
#include "../helper.h"
#include "../core/http.h"

namespace json
{
    class MessageDelete : public Serializable
    {
    public:
        inline void from_json(Json::Value& value) override
        {
            m_ID = value["id"].asString();
            m_ChannelID = value["channel_id"].asString();
        }

        std::string m_ID;
        std::string m_ChannelID;
    };

    class MessagePost : public Serializable
    {
    public:
        inline std::string to_json() const override
        {
            Json::Value value;

            value["content"] = m_Content;
            value["nonce"] = helper::make_snowflake();
            value["tts"] = tts;

            return value.toStyledString();
        }

        inline void set_content(std::string content)
        {
            m_Content = content;
        }

        //TODO: add file uploading methods

    private:
        std::string m_Content;
        bool tts = false;
    };

    class Message : public Serializable
    {
    public:
        inline void from_json(Json::Value& value) override
        {
            m_ID = value["id"].asString();
            m_ChannelID = value["channel_id"].asString();
            m_Author.from_json(value["author"]);
            m_Pinned = value["pinned"].asBool();
            m_MentionsEveryone = value["mention_everyone"].asBool();
            m_TTS = value["tts"].asBool();
            m_Type = value["type"].asInt();
            m_Flags = value["flags"].asInt();
            m_Content = value["content"].asString();
            m_Timestamp = { value["timestamp"].asString() };
            m_EditedTimestamp = { value["edited_timestamp"].asString() };
        }

        // Members
        std::string m_ID;
        std::string m_ChannelID;
        User m_Author;
        bool m_Pinned;
        bool m_MentionsEveryone;
        bool m_TTS;
        int m_Type;
        int m_Flags;
        std::string m_Content;
        helper::Timestamp m_Timestamp;
        helper::Timestamp m_EditedTimestamp;
    };
}
