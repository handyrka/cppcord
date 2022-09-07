#include "cache.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <giomm.h>

namespace core
{
    static std::string make_filename(std::string url)
    {
        return std::string("cache/") + std::to_string(std::hash<std::string>{}(url));
    }
    
    void CacheManager::cache(std::string url, CacheManagerCallback cb, bool isIcon, bool recache)
    {
        if(!std::filesystem::exists("cache"))
            std::filesystem::create_directory("cache");
            
        std::thread([=]
        {
            //Lock mutex so that other tasks will wait in queue
            std::lock_guard lock(m_CacheMutex);

            std::string fname = make_filename(url);

            //Return already cached ones
            if(!recache)
            {
                if(std::filesystem::exists(fname))
                {
                    char* contents = nullptr;
                    gsize length = 0;

                    auto file = Gio::File::create_for_path(fname);
                    file->load_contents(contents, length);

                    auto loader = Gdk::PixbufLoader::create();
                    loader->write((const guint8*)contents, length);
                    loader->close();
                    g_free(contents);
                    
                    auto pixbuf = loader->get_animation();
                    cb(pixbuf);
                    return;
                }
            }

            //Check for value in counter
            if(++m_CacheCounter >= 5)
            {
                m_CacheCounter = 0;
                std::cout << "[CacheManager::cache()] Reached rate limit, waiting..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500)); //Short nap
            }

            auto resp = HttpClient::get(url);
            
            if(resp.code() != CURLcode::CURLE_OK)
                throw std::runtime_error("[CacheManager::cache()] Failed to GET " + url);
    
            std::cout << "[CacheManager::cache()] GET " << url << " success." << std::endl;

            //Write data to a file
            std::ofstream str (fname);
            str.write(resp.data(), resp.size());
            str.close();

            // Load pixbuf
            auto loader = Gdk::PixbufLoader::create();
            loader->write((guint8*)resp.data(), resp.size());
            loader->close();
            
            auto pixbuf = loader->get_animation();
            resp.dispose();

            //Callback
            cb(pixbuf);

        }).detach();
    }

    void CacheManager::cacheUser(std::string id, CacheManagerUserCallback callback)
    {
        if(m_Users.count(id))
        {
            callback(m_Users[id]);
            return;
        }

        json::User::from_id_async(id, [=](json::User user)
        {
            m_Users[id] = user;
            callback(user);
        });
    }

    void CacheManager::recacheUser(std::string id)
    {
        json::User::from_id_async(id, [=](json::User user)
        {
            m_Users[id] = user;
        });
    }
}
