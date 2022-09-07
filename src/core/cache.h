#pragma once
#include "http.h"
#include <vector>
#include <mutex>
#include <functional>
#include <gdkmm.h>
#include <variant>
#include "../json/user.h"

namespace core
{
    using CacheManagerCallback = std::function<void(Glib::RefPtr<Gdk::PixbufAnimation>&)>;

    using CacheManagerUserCallback = std::function<void(json::User&)>;

    class CacheManager
    {
    public:
        void cache(std::string url, CacheManagerCallback cb, bool isIcon = false, bool recache = false);

        void cacheUser(std::string id, CacheManagerUserCallback callback);
        void recacheUser(std::string id);

    private:
        std::map<std::string, json::User> m_Users;

        int m_CacheCounter = 0; //To deal with 429 Too Many Requests
        std::mutex m_CacheMutex;
    };
}
