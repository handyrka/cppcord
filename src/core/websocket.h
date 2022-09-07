#pragma once
#include <ixwebsocket/IXWebSocket.h>
#include <functional>
#include <string>
#include <json/json.h>
#include "../json/user.h"
#include "../json/channel.h"
#include "../json/message.h"

namespace core
{
    class Websocket
    {
    public:
        void start();
        void stop();
        void set_token(std::string token);
        
        // Signals
        std::function<void(uint16_t, std::string)> error;
        std::function<void()> ready;

        //Events
        std::function<void(json::Message&)> message_create;
        std::function<void(json::Message&)> message_update;
        std::function<void(json::MessageDelete&)> message_delete;
        std::function<void(json::TypingStart&)> typing_start;

        // Data
        std::string m_AnalyticsToken, m_AuthSessionHash;
        std::vector<std::string> m_GeoRegions;
        std::string m_SessionId = "";
        json::User m_SelfUser;
        std::vector<json::Channel> m_PrivateChannels;

    private:
        ix::WebSocket m_Sock;
        std::unique_ptr<std::thread> m_HeartbeatThread;
        bool m_Running = false;
        bool m_Resuming = false;
        std::string m_Token = "";

        void on_message(const ix::WebSocketMessagePtr& msg);
        void heartbeat();

        // Data
        int m_HeartbeatDelay = 4000;
        int m_Seq = 0;
    };
}
