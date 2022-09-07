#include "window.h"
#include "../core/config.h"
#include "../core/http.h"
#include "../json/message.h"
#include <iostream>

namespace gui
{
    Window::Window()
    {
        m_MainContext = Glib::MainContext::get_default();

        // Init window
        auto css = Gtk::CssProvider::create();
        css->load_from_resource("/res/style.css");
        get_style_context()->add_provider_for_screen(Gdk::Screen::get_default(), css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        auto theme = Gtk::IconTheme::get_for_screen(Gdk::Screen::get_default());
        theme->add_resource_path("/res/");

        set_default_icon_name("icon");
        set_title("cppcord");
        set_default_size(1280, 720);

        m_Sock = std::make_shared<core::Websocket>();
        m_Sock->ready = [&]{ ready(); };
        m_Sock->error = [&](uint16_t code, std::string msg){ error(code, msg); };
        m_Sock->message_create = [&](auto& msg) { message_create(msg); };
        m_Sock->message_update = [&](auto& msg) { message_update(msg); };
        m_Sock->message_delete = [&](auto& msg) { message_delete(msg); };
        m_Sock->typing_start = [&](auto& tp) { typing_start(tp); };

        // Add widgets
        widget_setup();
    }

    Window::~Window(){}

    void Window::widget_setup()
    {
        m_Stack = Gtk::make_managed<Gtk::Stack>();
        m_Stack->set_hexpand();
        m_Stack->set_vexpand();
        m_Stack->set_transition_type(Gtk::STACK_TRANSITION_TYPE_CROSSFADE);
        m_Stack->set_transition_duration(250);
        add(*m_Stack);
        m_Stack->show_all();

        // Stack pages construction
        construct_loading();
        construct_login();
        construct_error();
        construct_ready();

        if (core::Config::should_authorize())
            m_Stack->set_visible_child("login");
        else
            login();
    }

    void Window::construct_loading()
    {
        auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 4);
        box->set_hexpand();
        box->set_vexpand();

        auto spinner = Gtk::make_managed<Gtk::Spinner>();
        spinner->start();
        spinner->set_halign(Gtk::ALIGN_END);
        spinner->set_hexpand();
        box->add(*spinner);

        auto label = Gtk::make_managed<Gtk::Label>();
        label->set_halign(Gtk::ALIGN_START);
        label->set_hexpand();
        label->set_text("Connecting...");
        box->add(*label);

        box->show_all();
        m_Stack->add(*box, "loading", "Loading");
    }

    static void login_callback(gpointer self, Window *window)
    {
        window->try_login();
    }

    void Window::login()
    {
        auto token = core::Config::get_token();
        m_Sock->set_token(token);
        m_Sock->start();

        m_Stack->set_visible_child("loading");
    }

    void Window::try_login()
    {
        auto token = m_LoginEntry->get_text();
        core::Config::set_token(token);

        login();
    }

    void Window::construct_login()
    {
        auto vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 12);
        vbox->set_hexpand();
        vbox->set_vexpand();

        auto label = Gtk::make_managed<Gtk::Label>();
        label->set_text("Login");
        label->set_vexpand();
        label->set_valign(Gtk::ALIGN_END);
        vbox->add(*label);

        auto hbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 4);
        hbox->set_hexpand();
        hbox->set_vexpand();
        vbox->add(*hbox);

        m_LoginEntry = Gtk::make_managed<Gtk::Entry>();
        m_LoginEntry->set_halign(Gtk::ALIGN_END);
        m_LoginEntry->set_valign(Gtk::ALIGN_START);
        m_LoginEntry->set_hexpand();
        m_LoginEntry->set_placeholder_text("Enter token...");

        auto button = Gtk::make_managed<Gtk::Button>();
        button->set_halign(Gtk::ALIGN_START);
        button->set_valign(Gtk::ALIGN_START);
        button->set_hexpand();
        button->set_label("Login");

        // Login button or Enter
        g_signal_connect(G_OBJECT(m_LoginEntry->gobj()), "activate", G_CALLBACK(login_callback), this);
        g_signal_connect(G_OBJECT(button->gobj()), "clicked", G_CALLBACK(login_callback), this);

        hbox->add(*m_LoginEntry);
        hbox->add(*button);

        vbox->show_all();
        m_Stack->add(*vbox, "login", "Login");
    }

    void Window::construct_ready()
    {
        auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 0);
        box->set_hexpand();
        box->set_vexpand();

        // Lists

        auto sWin = Gtk::make_managed<Gtk::ScrolledWindow>();
        sWin->set_vexpand();
        sWin->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
        box->add(*sWin);

        auto view = Gtk::make_managed<Gtk::Viewport>(sWin->get_hadjustment(), sWin->get_vadjustment());
        sWin->add(*view);
        view->set_vexpand();

        m_ListStack = Gtk::make_managed<Gtk::Stack>();
        m_ListStack->set_vexpand();
        m_ListStack->set_transition_type(Gtk::STACK_TRANSITION_TYPE_CROSSFADE);
        m_ListStack->set_transition_duration(250);

        // Private channel list

        m_PrivateList = Gtk::make_managed<Gtk::ListBox>();
        m_PrivateList->set_vexpand();
        m_PrivateList->signal_row_selected().connect([&](Gtk::ListBoxRow* row)
        {
            auto ind = row->get_index() - 1;

            if(ind < 0)
                return;

            m_CurrentChannelMessages.clear();
            m_CurrentChannel = m_Sock->m_PrivateChannels[ind];
            m_PrivateList->set_sensitive(false);
            m_ServerList->set_sensitive(false);
            channel_switched(ind);
        });

        // This button on click shows server list
        auto pRow = Gtk::make_managed<Gtk::ListBoxRow>();
        pRow->set_selectable(false);
        pRow->set_activatable(false);

        m_PrivateButton = Gtk::make_managed<Gtk::Button>();
        m_PrivateButton->set_label("Private");
        m_PrivateButton->signal_clicked().connect([&]{ m_ListStack->set_visible_child("server"); });

        pRow->add(*m_PrivateButton);
        m_PrivateList->append(*pRow);

        m_ListStack->add(*m_PrivateList, "private", "Private Channels");

        // Server list

        m_ServerList = Gtk::make_managed<Gtk::ListBox>();
        m_ServerList->set_vexpand();

        // This button does the same thing except reverse
        auto sRow = Gtk::make_managed<Gtk::ListBoxRow>();
        sRow->set_selectable(false);
        sRow->set_activatable(false);

        m_ServerButton = Gtk::make_managed<Gtk::Button>();
        m_ServerButton->set_label("Servers");
        m_ServerButton->signal_clicked().connect([&]{ m_ListStack->set_visible_child("private"); });
        sRow->add(*m_ServerButton);

        m_ServerList->append(*sRow);

        m_ListStack->add(*m_ServerList, "server", "Servers");
        view->add(*m_ListStack);

        // Main box

        auto mainBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 2);
        mainBox->get_style_context()->add_class("main_box");
        mainBox->set_hexpand();
        mainBox->set_vexpand();
        box->add(*mainBox);

        // Channel box (name, topic, etc..)
        //TODO: add self user thing on the right

        auto channelBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 8);
        channelBox->get_style_context()->add_class("darkend");

        m_ChannelNameLabel = Gtk::make_managed<Gtk::Label>();
        m_ChannelNameLabel->set_size_request(-1, 40);

        m_ChannelTopicLabel = Gtk::make_managed<Gtk::Label>();
        m_ChannelTopicLabel->set_size_request(-1, 40);
        m_ChannelTopicLabel->set_max_width_chars(96);
        m_ChannelTopicLabel->set_ellipsize(Pango::ELLIPSIZE_END);
        m_ChannelTopicLabel->get_style_context()->add_class("topic");

        channelBox->add(*m_ChannelNameLabel);
        channelBox->add(*m_ChannelTopicLabel);

        mainBox->add(*channelBox);

        // Message list
        m_MessageScrolled = Gtk::make_managed<Gtk::ScrolledWindow>();
        m_MessageScrolled->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
        m_MessageScrolled->set_hexpand();
        m_MessageScrolled->set_vexpand();

        auto viewport = Gtk::make_managed<Gtk::Viewport>(m_MessageScrolled->get_hadjustment(), m_MessageScrolled->get_vadjustment());
        viewport->set_hexpand();
        viewport->set_vexpand();

        m_MessageList = Gtk::make_managed<Gtk::ListBox>();
        m_MessageList->set_hexpand();
        m_MessageList->set_vexpand();
        m_MessageList->get_style_context()->add_class("message-list");

        viewport->add(*m_MessageList);
        m_MessageScrolled->add(*viewport);
        mainBox->add(*m_MessageScrolled);

        // Upload button, Text View, send button
        auto hBox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 2);

        m_UploadButton = Gtk::make_managed<Gtk::Button>();
        m_UploadButton->set_label("+");
        hBox->add(*m_UploadButton);

        m_Entry = Gtk::make_managed<Gtk::Entry>();
        m_Entry->set_hexpand();
        m_Entry->set_property("show-emoji-icon", true);
        m_Entry->set_placeholder_text("Enter message...");
        m_Entry->set_input_hints(Gtk::INPUT_HINT_EMOJI | Gtk::INPUT_HINT_SPELLCHECK);

        static int prev_diff = 0;

        m_Entry->signal_changed().connect([&]
        {
            if(m_Entry->get_text().length() > prev_diff)
            {
                prev_diff = m_Entry->get_text().length() + 25;
                m_CurrentChannel.send_typing();
            }
        });

        auto send = [&]
        {
            json::MessagePost post;
            post.set_content(m_Entry->get_text());
            m_CurrentChannel.send_message_async(post);

            prev_diff = 0;
            m_Entry->set_text("");
        };

        m_Entry->signal_activate().connect(send);
        hBox->add(*m_Entry);

        m_SendButton = Gtk::make_managed<Gtk::Button>();
        m_SendButton->set_label(">");
        m_SendButton->signal_clicked().connect(send);
        hBox->add(*m_SendButton);
        mainBox->add(*hBox);

        m_TypingLabel = Gtk::make_managed<Gtk::Label>();
        m_TypingLabel->get_style_context()->add_class("typing");
        m_TypingLabel->set_halign(Gtk::ALIGN_START);
        m_TypingLabel->set_valign(Gtk::ALIGN_START);
        m_TypingLabel->set_xalign(0);
        m_TypingLabel->set_justify(Gtk::JUSTIFY_LEFT);
        mainBox->add(*m_TypingLabel);

        box->show_all();
        m_Stack->add(*box, "ready", "Ready");
    }

    void Window::construct_error()
    {
        auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 4);
        box->set_hexpand();
        box->set_vexpand();

        m_ErrorLabel = Gtk::make_managed<Gtk::Label>();
        m_ErrorLabel->set_valign(Gtk::ALIGN_END);
        m_ErrorLabel->set_vexpand();
        m_ErrorLabel->set_text("Connecting...");
        box->add(*m_ErrorLabel);

        auto button = Gtk::make_managed<Gtk::Button>();

        button->signal_clicked().connect([&]
        {
            login();
        });

        button->set_label("Retry");
        button->set_valign(Gtk::ALIGN_START);
        button->set_halign(Gtk::ALIGN_CENTER);
        button->set_vexpand();
        box->add(*button);

        box->show_all();
        m_Stack->add(*box, "error", "Error");
    }

    void Window::construct_lists()
    {
        // User information

        // Init private channels

        for(size_t i = 0; i < m_Sock->m_PrivateChannels.size(); i++)
        {
            auto chn = m_Sock->m_PrivateChannels[i];
            m_ChannelNames.push_back("");

            auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 4);
            auto vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 2);
            auto img = Gtk::make_managed<Gtk::Image>();
            img->set_size_request(32, 32);
            box->add(*img);

            auto name = Gtk::make_managed<Gtk::Label>();
            name->set_halign(Gtk::ALIGN_START);
            name->get_style_context()->add_class("name");
            vbox->add(*name);

            auto subname = Gtk::make_managed<Gtk::Label>();
            subname->set_halign(Gtk::ALIGN_START);
            subname->get_style_context()->add_class("subname");
            vbox->add(*subname);

            box->add(*vbox);
            m_PrivateList->append(*box);
            box->show_all();

            if(chn.m_RecipientIDs.size() > 1)
            {
                auto _name = chn.m_Name.empty() ? "Group chat" : chn.m_Name;
                m_ChannelNames[i] = _name;
                name->set_text(_name);
                subname->set_text(std::to_string(chn.m_RecipientIDs.size() + 1) + " members");

                m_PrivateList->show_all();

                auto url = chn.has_icon() ? chn.get_icon_url() : core::DISCORD_DEFAULT_GC_ICON;
                cacheMgr.cache(url, [this, img](Glib::RefPtr<Gdk::PixbufAnimation>& pixbuf)
                {
                    m_MainContext->invoke([img, pixbuf]
                    {
                        img->property_pixbuf_animation() = pixbuf;
                        img->show();
                        return false;
                    });
                }, true);
            }
            else
            {
                cacheMgr.cacheUser(chn.m_RecipientIDs[0], [this, i, name, img](json::User& user)
                {
                    m_MainContext->invoke([this, i, name, user]
                    {
                        name->set_text(user.get_as_tag());
                        m_ChannelNames[i] = user.get_as_tag();
                        m_PrivateList->show_all();
                        return false;
                    });

                    auto url = user.has_avatar() ? user.get_avatar_url() : core::DISCORD_DEFAULT_AVATAR;
                    cacheMgr.cache(url, [this, img](Glib::RefPtr<Gdk::PixbufAnimation>& pixbuf)
                    {
                        m_MainContext->invoke([img, pixbuf]
                        {
                            img->property_pixbuf_animation() = pixbuf;
                            img->show();
                            return false;
                        });
                    }, true);
                });
            }
        }
    }

    void Window::append_message(json::Message msg)
    {
        auto box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 2);
        box->set_vexpand();
        box->set_margin_bottom(6);

        auto vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 2);
        vbox->set_vexpand();

        // User icon
        auto img = Gtk::make_managed<Gtk::Image>();
        img->set_size_request(32, 32);
        img->set_halign(Gtk::ALIGN_START);
        img->set_valign(Gtk::ALIGN_START);
        box->add(*img);
        box->add(*vbox);

        // Load user icon
        auto user = msg.m_Author;

        auto url = user.has_avatar() ? user.get_avatar_url() : core::DISCORD_DEFAULT_AVATAR;
        cacheMgr.cache(url, [this, img](Glib::RefPtr<Gdk::PixbufAnimation>& pixbuf)
        {
            if(!img) return;
            m_MainContext->invoke([img, pixbuf]
            {
                img->property_pixbuf_animation() = pixbuf;
                img->show();
                return false;
            });
        });

        // TODO: fix message look, add message embeds and attachements, store messages in list so that they can be edited. also message sending, typing indicator, deleting..

        // Username
        auto name = Gtk::make_managed<Gtk::Label>(user.get_as_tag());
        name->set_justify(Gtk::JUSTIFY_LEFT);
        name->get_style_context()->add_class("username");
        name->set_halign(Gtk::ALIGN_START);
        name->set_valign(Gtk::ALIGN_START);
        vbox->add(*name);

        // Content
        auto content = Gtk::make_managed<Gtk::Label>(msg.m_Content);
        content->set_xalign(0);
        content->set_justify(Gtk::JUSTIFY_LEFT);
        content->get_style_context()->add_class("content");
        content->set_halign(Gtk::ALIGN_START);
        content->set_valign(Gtk::ALIGN_START);
        content->set_line_wrap();
        vbox->add(*content);

        // Timestamp
        auto timestamp = Gtk::make_managed<Gtk::Label>(msg.m_Timestamp.to_short_date());
        timestamp->set_hexpand();
        timestamp->set_justify(Gtk::JUSTIFY_RIGHT);
        timestamp->get_style_context()->add_class("timestamp");
        timestamp->set_halign(Gtk::ALIGN_END);
        timestamp->set_valign(Gtk::ALIGN_START);
        box->add(*timestamp);

        m_CurrentChannelMessages[msg.m_ID] = box;

        box->show_all();
        m_MessageList->add(*box);
    }

    void Window::ready()
    {
        m_MainContext->invoke([this]
        {
            construct_lists();
            m_Stack->set_visible_child("ready");
            return false;
        });
    }

    void Window::error(uint16_t code, std::string message)
    {
        m_MainContext->invoke([code, message, this]
        {
            if(code == 4004) //Authorization error
                m_Stack->set_visible_child("login");
            else
            {
                m_ErrorLabel->set_text(message);
                m_Stack->set_visible_child("error");
            }
            return false;
        });
    }

    void Window::message_create(json::Message& message)
    {
        if(message.m_ChannelID != m_CurrentChannel.m_ID)
            return;

        m_MainContext->invoke([this, message]
        {
            m_CurrentChannelTyping.erase(std::remove(m_CurrentChannelTyping.begin(), m_CurrentChannelTyping.end(), message.m_Author.m_ID), m_CurrentChannelTyping.end());
            update_typing();

            append_message(message);
            m_MessageList->show_all();

            auto adj = m_MessageScrolled->get_vadjustment();
            adj->set_value(adj->get_upper());

            return false;
        });
    }

    void Window::message_update(json::Message& message)
    {

    }

    void Window::message_delete(json::MessageDelete& message)
    {

    }

    void Window::typing_start(json::TypingStart& typing)
    {
        if(typing.m_ChannelID != m_CurrentChannel.m_ID)
            return;

        if(std::count(m_CurrentChannelTyping.begin(), m_CurrentChannelTyping.end(), typing.m_UserID))
            return;

        m_CurrentChannelTyping.push_back(typing.m_UserID);

        m_MainContext->invoke([this, typing]
        {
            m_TypingLabel->set_text("");
            return false;
        });

        Glib::signal_timeout().connect_once([this, typing]
        {
            m_CurrentChannelTyping.erase(std::remove(m_CurrentChannelTyping.begin(), m_CurrentChannelTyping.end(), typing.m_UserID), m_CurrentChannelTyping.end());
            update_typing();
        }, 8000);

        update_typing();
    }

    void Window::channel_switched(int index)
    {
        for(auto& el : m_MessageList->get_children())
            delete el;

        m_ChannelNameLabel->set_text(m_ChannelNames[index]);

        m_CurrentChannel.fetch_messages_async([&](std::vector<json::Message> messages)
        {
            m_MainContext->invoke([this, messages]
            {
                for(auto& msg : messages)
                    append_message(msg);

                m_MessageList->show_all();
                m_PrivateList->set_sensitive(true);
                m_ServerList->set_sensitive(true);

                return false;
            });
        });
    }

    void Window::update_typing()
    {
        if(m_CurrentChannelTyping.size() <= 0)
        {
            m_MainContext->invoke([this]
            {
                m_TypingLabel->set_text("");
                return false;
            });
            return;
        }

        for(size_t i = 0; i < m_CurrentChannelTyping.size(); i++)
        {
            std::string id = m_CurrentChannelTyping[i];
            cacheMgr.cacheUser(id, [=](json::User& user)
            {
                m_MainContext->invoke([this, user, i]
                {
                    std::string text = m_TypingLabel->get_text();

                    text += user.m_Username;

                    if(i < m_CurrentChannelTyping.size() - 1)
                        text += ", ";
                    else
                        text += " typing...";

                    m_TypingLabel->set_text(text);
                    return false;
                });
            });
        }
    }
}
