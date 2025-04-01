#ifndef __WHISPER_FUZZY_H__
#define __WHISPER_FUZZY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>
#include <cstddef>


struct whisper_fuzzy_t;
struct whisper_params_t;


typedef int (*whisper_callback_t)(size_t leat_count, const char *text, const char* code, void* userdata);


whisper_params_t *whisper_fuzzy_get_params(whisper_fuzzy_t *w);


whisper_fuzzy_t* whisper_fuzzy_init(int argc, char const* argv[]);


void whisper_fuzzy_exit(whisper_fuzzy_t* w);


int whisper_fuzzy(whisper_fuzzy_t* w, whisper_callback_t callback, void* userdata);


int whisper_fuzzy_match(whisper_fuzzy_t* w, size_t leat_count, const char *text);


#ifdef __cplusplus
}
#endif

#endif //__WHISPER_FUZZY_H__
