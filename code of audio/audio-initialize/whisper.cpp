#include "whisper.h"
#include "curl_request.h"
#include "debug.h"
#include "json.hpp"
#include "voice_max_loudness.h"
#include "voice_record.h"
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

using json = nlohmann::json;


/**
 * Whisper 
 */
typedef struct whisper_t {
    size_t voice_count;                                 
    std::unordered_map<std::string, std::string>* map;  
} whisper_t;


/**
 *
 * @param str 
 */
void to_lower(std::string& str)
{
    for (char& c : str) {
        c = std::tolower(c);
    }
}

/**
 *
 * @param filename 
 * @param map 
 * @return 
 */
int read_config(const std::string& filename, std::unordered_map<std::string, std::string>& map)
{
    std::ifstream file(filename);
    if (!file) {
        LOG_ERR("fail to open %s", filename.c_str());
        return -1;
    }

    json config;
    file >> config;

    for (const auto& item : config) {
        for (const auto& text : item["text"]) {
            std::string text_lower = text.get<std::string>();
            to_lower(text_lower);

            std::string code = item["code"].get<std::string>(); 

            map[text_lower] = code;
            LOG_DBG("read %s -> %s", text_lower.c_str(), code.c_str());
        }
    }
    return 0;
}

/**
 *
 * @param str 
 * @return 
 */
std::string trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    size_t end = str.find_last_not_of(" \t\n\r\f\v");

    if (start == std::string::npos || end == std::string::npos) {
        return ""; 
    }

    return str.substr(start, end - start + 1);
}

/**
 *
 * @param voice_file 
 * @param text 
 * @return 
 */
int voice_to_text(const char* voice_file, std::string& text)
{
    std::string url = "http://127.0.0.1:";
    url += std::to_string(WHISPER_SERVER_PORT);
    url += "/inference";

    std::string response = curl_request(url, voice_file);
    if (response.empty()) {
        LOG_ERR("server no response");
        return -1;
    }
    try {
        json parsed_json = json::parse(response);

        if (parsed_json.contains("text") && !parsed_json["text"].is_null()) {
            text = parsed_json["text"];
            text = trim(text);
            LOG_DBG("Text: %s", text.c_str());
        } else {
            std::cout << "text 字段为空或不存在, response: " << response << std::endl;
        }
    } catch (const json::parse_error& e) {
        std::string err = e.what();
        LOG_ERR("JSON Parse Error: %s", err.c_str());
    }

    return 0;
}

/**
 *
 * @param map 
 * @param text 
 * @return 
 */
const char* text_to_code(std::unordered_map<std::string, std::string>& map, const char* text)
{
    std::string text_lower = text;
    to_lower(text_lower);

    if (map.find(text_lower) == map.end()) {
        LOG_ERR("unknow %s ", text_lower.c_str());
        return "0x00";
    }
    return map[text_lower].c_str();
}

/**
 *
 * @param config
 * @return 
 */
whisper_t* whisper_init(const char* config)
{
    whisper_t* w = (whisper_t*)malloc(sizeof(whisper_t));
    if (!w) {
        LOG_ERR("fail to malloc whisper");
        return nullptr;
    }
    memset(w, 0, sizeof(whisper_t));

    w->map = new std::unordered_map<std::string, std::string>;
    if (!w->map) {
        free(w);
        return nullptr;
    }

    int ret = read_config(config, *w->map);
    if (ret < 0) {
        LOG_DBG("fail to read_config");
        whisper_exit(w);
        return nullptr;
    }
    return w;
}

/**
 *
 * @param w 
 */
void whisper_exit(whisper_t* w)
{
    if (!w)
        return;

    if (w->map) {
        delete w->map;
        w->map = nullptr;
    }
    free(w);
}

/**
 *
 * @param w 
 * @param callback 
 * @param userdata 
 * @return 
 */
int whisper(whisper_t* w, whisper_callback_t callback, void* userdata)
{
    int ret = 0;
    const char* record_prefix = "/tmp/record_";

    std::string save_file = record_prefix;
    save_file += std::to_string(w->voice_count) + ".wav";

    ret = voice_record(save_file.c_str(), 1);
    if (ret < 0) {
        LOG_ERR("fail to voice_record");
        w->voice_count = 0;
        return -1;
    }

    double max_loudness = get_max_loudness(save_file.c_str());
    if (max_loudness <= SILENCE_THRESHOLD && w->voice_count == 0) {
        LOG_DBG("max_loudness(%f) <= SILENCE_THRESHOLD(%d), no voice",
            max_loudness, SILENCE_THRESHOLD);
        return -2;
    }

    w->voice_count += 1;
    if (max_loudness > SILENCE_THRESHOLD
        && w->voice_count <= RECODE_TIMEOUT) {
        LOG_DBG("max_loudness(%f) > SILENCE_THRESHOLD(%d), continue record",
            max_loudness, SILENCE_THRESHOLD);
        return 1;
    }


    std::string save_record = record_prefix;
    save_record += "all.wav";

    ret = splicing_audio(record_prefix, w->voice_count, save_record.c_str());
    if (ret < 0) {
        LOG_ERR("fail to splicing_audio");
        w->voice_count = 0;
        return -1;
    }

    std::string text = "";
    ret = voice_to_text(save_record.c_str(), text);
    if (ret < 0) {
        LOG_ERR("fail to voice_to_text");
        w->voice_count = 0;
        return -1;
    }
    const char *code = text_to_code(*w->map, text.c_str());

    callback(text.c_str(), code, userdata);
    w->voice_count = 0;

    return 0;
}
