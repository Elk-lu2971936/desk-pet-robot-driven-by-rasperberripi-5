#include "debug.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "voice_record.h"
#include "command.h"


#define VOICE_RECORD_CMD "arecord -r 16000 -f S16_LE -t wav -V stereo -c 1 -d "
#define SPLICING_AUDIO_CMD "sox "


int splicing_audio(const char* file_prefix, size_t voice_count, const char* out)
{
    std::string splicing = "";
    for (size_t i = 0; i < voice_count; ++i) {
        splicing += file_prefix + std::to_string(i) + ".wav ";
    }
    std::string output;
    std::string cmd = SPLICING_AUDIO_CMD;
    cmd += splicing;
    cmd += out;
    cmd += " 2>&1";

    return execute_command(cmd.c_str(), output);
}


int voice_record(const char* save_file, size_t record_time)
{
    std::string output;
    std::string cmd = VOICE_RECORD_CMD;
    cmd += std::to_string(record_time) + " ";
    cmd += save_file;
    cmd += " 2>&1";

    return execute_command(cmd.c_str(), output);
}

// date 3h26min
