#ifndef __WHISPER_STREAM_H__
#define __WHISPER_STREAM_H__

#include <thread>
#include <string>

#include "stdint.h"
#include "whisper_fuzzy.h"



struct whisper_params_t {
    int32_t n_threads  = std::min(4, (int32_t) std::thread::hardware_concurrency()); 
    
    int32_t step_ms    = 1000;  
    int32_t length_ms  = 3000;  
    int32_t keep_ms    = 100;   
    int32_t capture_id = -1;   
    int32_t max_tokens = 8;     
    int32_t audio_ctx  = 0;    

    float vad_thold    = 0.6f;  
    float freq_thold   = 100.0f;

    bool translate     = false; 
    bool no_fallback   = false; 
    bool print_special = false;
    bool no_context    = true;  
    bool no_timestamps = false; 
    bool tinydiarize   = false; 
    bool save_audio    = false;
    bool use_gpu       = true;  
    bool flash_attn    = false; 

    std::string language  = "en"; 
    std::string model     = "models/ggml-base.en.bin"; 
    std::string user      = ""; 
    std::string fname_out;      
    const char *program_name;  
};


void whisper_print_usage(const whisper_params_t & params);

int whisper_stream_main(whisper_fuzzy_t *whisper_fuzzy_ctx);

#endif //__WHISPER_STREAM_H__
