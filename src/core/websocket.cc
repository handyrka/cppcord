#include "websocket.h"
#include "notification.h"
#include "../json/heartbeat.h"
#include "../json/identify.h"
#include "../json/resume.h"
#include <iostream>

#define GATEWAY_URL "wss://gateway.discord.gg/?v=9&encoding=json"

namespace core
{
    void Websocket::start()
    {
        m_Sock.stop();
        m_Sock.setUrl(GATEWAY_URL);
        m_Sock.setOnMessageCallback([&](const ix::WebSocketMessagePtr& msg) { this->on_message(msg); });
        m_Sock.disableAutomaticReconnection();
        m_Sock.start();

        std::cout << "[Websocket::start()] Establishing connection to gateway..." << std::endl;
    }

    void Websocket::stop()
    {
        m_Sock.stop();
        m_Running = false;

        std::cout << "[Websocket::stop()] Gateway connection closed." << std::endl;
    }

    void Websocket::set_token(std::string token)
    {
        m_Token = token;
    }

    void Websocket::heartbeat()
    {
        std::cout << "[Websocket::heartbeat()] Sending heartbeat..." << std::endl;

        json::payloads::Heartbeat hb;
        hb.m_Seq = m_Seq;
        auto js = hb.to_json();
        m_Sock.send(js);
    }

    void Websocket::on_message(const ix::WebSocketMessagePtr& msg)
    {
        switch(msg->type)
        {
            case ix::WebSocketMessageType::Open:
            {
                std::cout << "[Websocket::on_message()] Gateway connection established!" << std::endl;
                break;
            }

            case ix::WebSocketMessageType::Error:
            {
                m_Running = false;
                std::cout << "[Websocket::on_message()] Gateway connection error: " << msg->errorInfo.reason << std::endl;
                
                error(msg->errorInfo.http_status, msg->errorInfo.reason);
                break;
            }

            case ix::WebSocketMessageType::Close:
            {
                m_Running = false;
                std::cout << "[Websocket::on_message()] Gateway connection closed with code " << msg->closeInfo.code << "." << std::endl;

                if(msg->closeInfo.code != 1000)
                {
                    error(msg->closeInfo.code, msg->closeInfo.reason);
                    return;
                }
                
                if(!m_SessionId.empty()) // Resume
                {
                    m_Resuming = true;
                    m_Sock.start();
                }
                
                break;
            }

            case ix::WebSocketMessageType::Message:
            {                
                Json::Value value;
                Json::Reader reader;
                reader.parse(msg->str, value);

                switch(value["op"].asInt())
                {
                    case 0: // Dispatch (Event)
                    {
                        std::string ev = value["t"].asString();
                        m_Seq = value["s"].asInt();

                        std::cout << "[Websocket::on_message()] Event " << ev << std::endl;
                        
                        if(ev == "READY")
                        {
                            m_AnalyticsToken = value["d"]["analytics_token"].asString();
                            m_AuthSessionHash = value["d"]["auth_session_id_hash"].asString();
                            
                            for(auto region : value["d"]["geo_ordered_rtc_regions"])
                            {
                                m_GeoRegions.push_back(region.asString());
                            }

                            for(auto channel : value["d"]["private_channels"])
                            {
                                json::Channel chn;
                                chn.from_json(channel);
                                m_PrivateChannels.push_back(chn);
                            }

                            m_SelfUser.from_json(value["d"]["user"]);

                            m_SessionId = value["d"]["session_id"].asString();
                            ready();
                        }
                        else if(ev == "MESSAGE_CREATE")
                        {
                            json::Message msg;
                            msg.from_json(value["d"]);
                            message_create(msg);
                        }
                        else if(ev == "MESSAGE_UPDATE")
                        {
                            json::Message msg;
                            msg.from_json(value["d"]);
                            message_update(msg);
                        }
                        else if(ev == "MESSAGE_DELETE")
                        {
                            json::MessageDelete msg;
                            msg.from_json(value["d"]);
                            message_delete(msg);
                        }
                        else if(ev == "TYPING_START")
                        {
                            json::TypingStart start;
                            start.from_json(value["d"]);
                            typing_start(start);
                        }

                        break;
                    }

                    case 1: // Heartbeat
                    {
                        std::cout << "[Websocket::on_message()] Heartbeat requested from server" << std::endl;
                        heartbeat();
                        break;
                    }

                    case 7: // Reconnect
                    {
                        std::cout << "[Websocket::on_message()] Reconnect..." << std::endl;
                        
                        m_Running = false;
                        m_HeartbeatThread->join();
                        m_Sock.start();
                        break;
                    }

                    case 9: // Invalid Session
                    {
                        std::cout << "[Websocket::on_message()] Invaild session!" << std::endl;
                        
                        m_Resuming = false;
                        m_Running = false;
                        m_HeartbeatThread->join();
                        error(-1, "Invalid session");
                        break;
                    }

                    case 10: // Hello
                    {
                        m_HeartbeatDelay = value["d"]["heartbeat_interval"].asInt() - 4000; //Just in case

                        if(!m_Resuming)
                        {
                            json::payloads::Identify id;
                            id.m_Token = m_Token;
                            m_Sock.send(id.to_json());
                        }
                        else
                        {
                            std::cout << "[Websocket::on_message()] Resuming..." << std::endl;
                            
                            json::payloads::Resume rs;
                            rs.m_Token = m_Token;
                            rs.m_SessionId = m_SessionId;
                            rs.m_Seq = m_Seq;
                            m_Sock.send(rs.to_json());
                        }

                        if(m_HeartbeatThread && m_HeartbeatThread->joinable())
                            m_HeartbeatThread->join();

                        m_Running = true;
                        m_HeartbeatThread = std::make_unique<std::thread>([&]
                        { 
                            while(true)
                            {
                                for(int i = 0; i < m_HeartbeatDelay; i += 5)
                                {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                                    
                                    if(!m_Running) // Dirty fix so that it will stop if not running
                                        return;
                                }
                                heartbeat();   
                            }
                        });

                        break;
                    }

                    case 11: // Heartbeat ack
                    {
                        std::cout << "[Websocket::on_message()] Heartbeat confirmed!" << std::endl;
                        break;
                    }
                }

                break;
            }

            default: break;
        }
    }
}
