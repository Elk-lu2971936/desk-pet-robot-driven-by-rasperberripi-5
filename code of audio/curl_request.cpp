#include <curl/curl.h>
#include <iostream>
#include <string>



size_t write_callback(void* ptr, size_t size, size_t nmemb, std::string* data)
{
    size_t totalSize = size * nmemb;
    data->append(static_cast<char*>(ptr), totalSize); 
    return totalSize;
}


 
std::string curl_request(const std::string& url, const std::string& filePath, const std::string& responseFormat)
{
    CURL* curl;
    CURLcode res;
    std::string response; 

    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        
        struct curl_httppost* form = NULL;
        struct curl_httppost* lastptr = NULL;

       
        curl_formadd(&form, &lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filePath.c_str(), CURLFORM_END);

        
        curl_formadd(&form, &lastptr, CURLFORM_COPYNAME, "response_format", CURLFORM_COPYCONTENTS, responseFormat.c_str(), CURLFORM_END);

        
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

      
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_formfree(form);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return ""; 
        }

       
        curl_formfree(form);

       
        curl_easy_cleanup(curl);
    }

    
    curl_global_cleanup();

    return response; 
}

#ifdef _XTEST

#include "json.hpp"

using json = nlohmann::json;

int main()
{
    
    std::string url = "http://127.0.0.1:8090/inference";
    std::string filePath = "./whisper.cpp-1.7.4/samples/jfk.wav";
    std::string responseFormat = "json";

   
    std::string response = curl_request(url, filePath, responseFormat);

    
    if (!response.empty()) {
        std::cout << "Server response: " << std::endl;
        std::cout << response << std::endl;
        
        try {
            
            json parsed_json = json::parse(response);

            
            std::string text = parsed_json["text"];

            
            std::cout << "Text: " << text << std::endl;
        } catch (const json::parse_error& e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        }
    } else {
        std::cout << "Request failed." << std::endl;
    }

    return 0;
}

#endif //_XTEST
