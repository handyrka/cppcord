#pragma once
#include "serializable.h"
#include "../core/http.h"

namespace json
{
    class User : public Serializable
    {
    public:
        inline void from_json(Json::Value& value) override
        {
            m_AccentColor = value["accent_color"].asString();
            m_AvatarHash = value["avatar"].asString();
            m_AvatarDecoration = value["avatar_decoration"].asString();
            m_BannerHash = value["banner"].asString();
            m_BannerColor = value["banner_color"].asString();
            m_Bio = value["bio"].asString();
            m_Desktop = value["desktop"].asBool();
            m_Bot = value["bot"].asBool();
            m_Discriminator = value["discriminator"].asString();
            m_Email = value["email"].asString();
            m_Flags = value["flags"].asInt();
            m_PublicFlags = value["public_flags"].asInt();
            m_ID = value["id"].asString();
            m_MfaEnabled = value["mfa_enabled"].asBool();
            m_Mobile = value["mobile"].asBool();
            m_NsfwAllowed = value["nsfw_allowed"].asBool();
            m_Phone = value["phone"].asString();
            m_Premium = value["premium"].asBool();
            m_PurchasedFlags = value["purchased_flags"].asInt();
            m_Username = value["username"].asString();
            m_Verified = value["verified"].asBool();
        }
        
        // Members
        std::string m_AccentColor;
        std::string m_AvatarHash;
        std::string m_AvatarDecoration;
        std::string m_BannerHash;
        std::string m_BannerColor;
        std::string m_Bio;
        bool m_Desktop;
        bool m_Bot;
        std::string m_Discriminator;
        std::string m_Email;
        int m_Flags;
        int m_PublicFlags;
        std::string m_ID;
        bool m_MfaEnabled;
        bool m_Mobile;
        bool m_NsfwAllowed;
        std::string m_Phone;
        bool m_Premium;
        int m_PurchasedFlags = 0;
        std::string m_Username;
        bool m_Verified;

        // Functions
        inline static void from_id_async(std::string id, std::function<void(User)> callback)
        {
            core::HttpClient::get_async("https://discord.com/api/v9/users/" + id, [callback](core::HttpResponse resp)
            {
                Json::Value value;   
                Json::Reader reader;
                reader.parse(resp.to_string(), value);

                User user;
                user.from_json(value);
                callback(user);
            });
        }

        inline std::string get_as_tag() const { return m_Username + "#" + m_Discriminator; }

        inline bool has_avatar() const { return !m_AvatarHash.empty(); }

        inline std::string get_avatar_url() const 
        { 
            std::string result_url = "https://cdn.discordapp.com/avatars/" + m_ID + "/" + m_AvatarHash;
            
            if(m_AvatarHash.find("a_") != std::string::npos)
                return result_url + ".gif";
            else
                return result_url + ".png?size=32";
        }

        inline bool has_banner() const { return !m_BannerHash.empty(); }

        inline std::string get_banner_url() const 
        { 
            std::string result_url = "https://cdn.discordapp.com/banners/" + m_ID + "/" + m_BannerHash;
            
            if(m_AvatarHash.find("a_") != std::string::npos)
                return result_url + ".gif";
            else
                return result_url + ".png?size=32"; 
        }
    };
}
