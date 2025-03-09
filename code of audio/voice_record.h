#ifndef __VOICE_RECORD_H__
#define __VOICE_RECORD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"


int voice_record(const char* save_file, size_t record_time);


int splicing_audio(const char* file_prefix, size_t voice_count, const char* out);

#ifdef __cplusplus
}
#endif

#endif //__VOICE_RECORD_H__
