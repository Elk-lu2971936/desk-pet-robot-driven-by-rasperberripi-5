#include "debug.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#define LINE_LEN (1024)


/**
 *
 * @param command 
 * @param output  
 * @return 
 */
int execute_command(const char* command, std::string& output)
{
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        LOG_ERR("Failed to execute command: %s", command);
        return -1;
    }

    char buffer[LINE_LEN];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        LOG_DBG("%s", buffer); 
        output += buffer;
        output += "\n";
    }

    int status = pclose(pipe);
    if (status == -1) {
        LOG_ERR("Error closing pipe.");
        return -1;
    }

    int code = WEXITSTATUS(status); 
    if (code != 0) {
        LOG_ERR("run cmd: %s, code: %d", command, code);
        return -1;
    }
    return 0;
}