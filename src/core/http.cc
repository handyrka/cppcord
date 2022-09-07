#include "http.h"
#include "config.h"
#include <string.h>
#include <algorithm>

namespace core
{
    static size_t curl_write_function(void *contents, size_t size, size_t nmemb, void *userp)
    {
        std::vector<char>* vec = (std::vector<char>*)userp;

        for(size_t i = 0; i < size * nmemb; i++)
            vec->push_back(((char*)contents)[i]);

        return size * nmemb;
    }

    CURLcode HttpResponse::code() const
    {
        return m_Code;
    }

    const char* HttpResponse::data() const
    {
        return (const char*)&m_Data[0];
    }

    size_t HttpResponse::size() const
    {
        return m_Size;
    }

    std::string HttpResponse::to_string() const
    {
        return std::string(&m_Data[0], m_Size);
    }

    HttpResponse HttpClient::get(std::string url, bool auth)
    {
        auto curl = curl_easy_init();

        if(!curl) 
            throw "Failed to open curl_easy";
        
        struct curl_slist *headers = NULL;
        std::vector<char> buffer;
        HttpResponse response;
        
        if(auth)
            headers = curl_slist_append(headers, ("Authorization: " + core::Config::get_token()).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:104.0) Gecko/20100101 Firefox/104.0");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_function);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        response.m_Code = curl_easy_perform(curl);
        response.m_Data = buffer;
        response.m_Size = buffer.size();

        curl_easy_cleanup(curl);

        return response;
    }

    HttpResponse HttpClient::post(std::string url, std::string data, std::string contentType, bool auth)
    {
        auto curl = curl_easy_init();

        if(!curl) 
            throw "Failed to open curl_easy";
        
        struct curl_slist *headers = NULL;
        std::vector<char> buffer;
        HttpResponse response;
        
        if(auth)
            headers = curl_slist_append(headers, ("Authorization: " + core::Config::get_token()).c_str());

        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, true);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:104.0) Gecko/20100101 Firefox/104.0");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_function);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        response.m_Code = curl_easy_perform(curl);
        response.m_Data = buffer;
        response.m_Size = buffer.size();

        curl_easy_cleanup(curl);

        return response;
    }

    void HttpClient::get_async(std::string url, std::function<void(HttpResponse)> callback, bool auth)
    { 
        std::thread([=]
        {
            auto reps = get(url, auth);
            callback(reps);
        }).detach();
    }

    void HttpClient::post_async(std::string url, std::string data, std::function<void(HttpResponse)> callback, std::string contentType, bool auth)
    {
        std::thread([=]
        {
            auto reps = post(url, data, contentType, auth);
            callback(reps);
        }).detach();
    }
}
