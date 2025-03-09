#ifndef __CURL_REQUEST_H__
#define __CURL_REQUEST_H__

#include <string>


std::string curl_request(const std::string& url, const std::string& filePath, const std::string& responseFormat = "json");

#endif//__CURL_REQUEST_H__
