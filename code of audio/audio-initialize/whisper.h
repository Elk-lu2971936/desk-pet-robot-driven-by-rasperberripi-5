#ifndef __WHISPER_H__
#define __WHISPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RECODE_TIMEOUT (5) 
#define SILENCE_THRESHOLD (-25) 
#define WHISPER_SERVER_PORT (8090) 

/**
 * Whisper 
 */
struct whisper_t;

/**
 * Whisper 
 *
 * @param text 
 * @param code 
 * @param userdata 
 */
typedef void (*whisper_callback_t)(const char *text, const char* code, void* userdata);

/**
 *
 * @param config 
 * @return 
 */
whisper_t* whisper_init(const char* config);

/**
 *
 * @param w 
 */
void whisper_exit(whisper_t* w);

/**
 *
 * @param w 
 * @param callback 
 * @param userdata 
 * @return 
 */
int whisper(whisper_t* w, whisper_callback_t callback, void* userdata);

#ifdef __cplusplus
}
#endif

#endif //__WHISPER_H__