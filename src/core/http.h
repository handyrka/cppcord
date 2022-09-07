#pragma once
#include <string>
#include <curl/curl.h>
#include <functional>
#include <thread>
#include <vector>

namespace core
{
    inline const std::string DISCORD_API_URL = "https://discord.com/api/v9";
    inline const std::string DISCORD_DEFAULT_AVATAR = "https://media.discordapp.net/attachments/978319897564160013/1016480041758105710/13247924177349302721.png";
    inline const std::string DISCORD_DEFAULT_GC_ICON = "https://media.discordapp.net/attachments/978319897564160013/1016480041498066964/2750201884356207296.png";

    class HttpResponse
    {
    public:
        CURLcode code() const;
        const char* data() const;
        size_t size() const;

        std::string to_string() const;

    private:
        std::vector<char> m_Data;
        size_t m_Size;
        CURLcode m_Code;

        friend class HttpClient;
    };

    class HttpClient
    {
    public:
        static HttpResponse get(std::string url, bool auth = true);
        static HttpResponse post(std::string url, std::string data, std::string contentType = "application/json", bool auth = true);

        static void get_async(std::string url, std::function<void(HttpResponse)> callback, bool auth = true);
        static void post_async(std::string url, std::string data, std::function<void(HttpResponse)> callback, std::string contentType = "application/json", bool auth = true);
    };
}
