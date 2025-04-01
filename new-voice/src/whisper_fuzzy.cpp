#include "whisper.h"
#include "debug.h"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include "whisper_stream.h"

using json = nlohmann::json;



typedef struct whisper_fuzzy_t {
    whisper_params_t *params;                           
    whisper_callback_t callback;                       
    void *userdata;                                     
    std::unordered_map<std::string, std::string>* map;  
} whisper_fuzzy_t;


whisper_params_t *whisper_fuzzy_get_params(whisper_fuzzy_t *w)
{
    return w ? w->params : nullptr;
}


static bool whisper_fuzzy_params_parse(int argc, char const* argv[], whisper_params_t & params) {
    params.program_name = argv[0];
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            whisper_print_usage(params);
            exit(0);
        }
        else if (arg == "-t"    || arg == "--threads")       { params.n_threads     = std::stoi(argv[++i]); }
        else if (                  arg == "--step")          { params.step_ms       = std::stoi(argv[++i]); }
        else if (                  arg == "--length")        { params.length_ms     = std::stoi(argv[++i]); }
        else if (                  arg == "--keep")          { params.keep_ms       = std::stoi(argv[++i]); }
        else if (arg == "-c"    || arg == "--capture")       { params.capture_id    = std::stoi(argv[++i]); }
        else if (arg == "-d"    || arg == "--debug")         { set_dbg_enable(log_dbg_flag_t(std::stoi(argv[++i]))); }
        else if (arg == "-mt"   || arg == "--max-tokens")    { params.max_tokens    = std::stoi(argv[++i]); }
        else if (arg == "-ac"   || arg == "--audio-ctx")     { params.audio_ctx     = std::stoi(argv[++i]); }
        else if (arg == "-vth"  || arg == "--vad-thold")     { params.vad_thold     = std::stof(argv[++i]); }
        else if (arg == "-fth"  || arg == "--freq-thold")    { params.freq_thold    = std::stof(argv[++i]); }
        else if (arg == "-tr"   || arg == "--translate")     { params.translate     = true; }
        else if (arg == "-nf"   || arg == "--no-fallback")   { params.no_fallback   = true; }
        else if (arg == "-ps"   || arg == "--print-special") { params.print_special = true; }
        else if (arg == "-kc"   || arg == "--keep-context")  { params.no_context    = false; }
        else if (arg == "-l"    || arg == "--language")      { params.language      = argv[++i]; }
        else if (arg == "-m"    || arg == "--model")         { params.model         = argv[++i]; }
        else if (arg == "-f"    || arg == "--file")          { params.fname_out     = argv[++i]; }
        else if (arg == "-u"    || arg == "--user")          { params.user          = argv[++i]; }
        else if (arg == "-tdrz" || arg == "--tinydiarize")   { params.tinydiarize   = true; }
        else if (arg == "-sa"   || arg == "--save-audio")    { params.save_audio    = true; }
        else if (arg == "-ng"   || arg == "--no-gpu")        { params.use_gpu       = false; }
        else if (arg == "-fa"   || arg == "--flash-attn")    { params.flash_attn    = true; }

        else {
            fprintf(stderr, "error: unknown argument: %s\n", arg.c_str());
            whisper_print_usage(params);
            exit(0);
        }
    }

    return true;
}


void to_lower(std::string& str)
{
    for (char& c : str) {
        c = std::tolower(c);
    }
}


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


std::string str_trim(const std::string& str)
{

    size_t start = str.find_first_not_of(" \t\n\r\f\v");
   
    size_t end = str.find_last_not_of(" \t\n\r\f\v");

    if (start == std::string::npos || end == std::string::npos) {
        return ""; 
    }

    
    return str.substr(start, end - start + 1);
}


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


whisper_fuzzy_t* whisper_fuzzy_init(int argc, char const* argv[])
{
    int ret = 0;
    whisper_fuzzy_t* w = (whisper_fuzzy_t*)malloc(sizeof(whisper_fuzzy_t));
    if (!w) {
        LOG_ERR("fail to malloc whisper");
        return nullptr;
    }
    memset(w, 0, sizeof(whisper_fuzzy_t));
    
    w->params = new whisper_params_t;
    if (!w->params) {
        LOG_ERR("fail to new whisper_params_t");
        goto _exit;
    }
    
    ret = whisper_fuzzy_params_parse(argc, argv, *w->params);
    if (!ret) {
        LOG_ERR("fail to parse params");
        goto _exit;
    }

    if (w->params->user.empty()) {
        whisper_print_usage(*w->params);
        goto _exit;
    }

    w->map = new std::unordered_map<std::string, std::string>;
    if (!w->map) {
        LOG_ERR("fail to new map");
        goto _exit;
    }

    ret = read_config(w->params->user.c_str(), *w->map);
    if (ret < 0) {
        LOG_DBG("fail to read_config");
        goto _exit;
    }
    return w;

_exit:
    whisper_fuzzy_exit(w);
    return nullptr;
}


void whisper_fuzzy_exit(whisper_fuzzy_t* w)
{
    if (!w)
        return;
    
    if (w->params) {
        delete w->params;
        w->params = nullptr;
    }

    if (w->map) {
        delete w->map;
        w->map = nullptr;
    }
    free(w);
}


int whisper_fuzzy_match(whisper_fuzzy_t* w, size_t leat_count, const char *text)
{
    if (!text || !w || !w->callback) {
        LOG_ERR("args fail!  text(%p), w(%p), callback(%p)", 
            text, w, w ? w->callback : nullptr);
        return -1;
    }
    std::string trim_text = str_trim(text);
    const char *code = text_to_code(*w->map, trim_text.c_str());

    return w->callback(leat_count, text, code, w->userdata);
}


int whisper_fuzzy(whisper_fuzzy_t* w, whisper_callback_t callback, void* userdata)
{
    if (!w) {
        LOG_ERR("whisper_fuzzy is nullptr!");
        return -1;
    }
    w->callback = callback;
    w->userdata = userdata;

    return whisper_stream_main(w);
}
