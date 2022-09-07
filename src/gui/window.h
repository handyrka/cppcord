#pragma once
#include <gtkmm.h>
#include <map>
#include "../core/websocket.h"
#include "../core/cache.h"
#include "../json/channel.h"

namespace gui
{
    class Window : public Gtk::ApplicationWindow
    {
    public:
        Window();
        virtual ~Window();

        void try_login();

    private:
        void widget_setup();
        void login();

        // GUI construction methods
        void construct_loading();
        void construct_login();
        void construct_ready();
        void construct_error();
        void construct_lists();
        void append_message(json::Message msg);

        // Signals (?)
        void channel_switched(int index);
        void update_typing();

        // Websocket callbacks
        void ready();
        void error(uint16_t code, std::string message);

        void message_create(json::Message& message);
        void message_update(json::Message& message);
        void message_delete(json::MessageDelete& message);
        void typing_start(json::TypingStart& typing);

        Gtk::Stack* m_Stack, *m_ListStack;
        Gtk::Entry* m_LoginEntry;
        Gtk::Label* m_ErrorLabel, *m_TypingLabel;
        Gtk::Label* m_ChannelNameLabel, *m_ChannelTopicLabel;
        Gtk::ListBox* m_PrivateList, *m_ServerList, *m_MessageList;
        Gtk::Entry* m_Entry;
        Gtk::Button* m_PrivateButton, *m_ServerButton;
        Gtk::Button* m_UploadButton, *m_SendButton;
        Gtk::ScrolledWindow* m_MessageScrolled;

        std::shared_ptr<core::Websocket> m_Sock;
        std::vector<std::string> m_ChannelNames;
        core::CacheManager cacheMgr;

        // Current channel related
        json::Channel m_CurrentChannel;
        std::map<std::string, Gtk::Box*> m_CurrentChannelMessages;
        std::vector<std::string> m_CurrentChannelTyping;

        Glib::RefPtr<Glib::MainContext> m_MainContext;
    };
}
