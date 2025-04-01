#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_DBG_FLAG  = (1 << 2),
    LOG_INFO_FLAG = (1 << 1),
    LOG_ERR_FLAG  = (1 << 0),
} log_dbg_flag_t;

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/')) 

#define LOG_DBG(fmt, ...)  if (get_dbg_enable() & LOG_DBG_FLAG)  fprintf(stdout, "[%s:%s:%d] " fmt "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) if (get_dbg_enable() & LOG_INFO_FLAG) fprintf(stdout, "[%s:%s:%d] " fmt "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...)  if (get_dbg_enable() & LOG_ERR_FLAG)  fprintf(stderr, "[%s:%s:%d] " fmt "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)


void set_dbg_enable(log_dbg_flag_t flag);


log_dbg_flag_t get_dbg_enable();

#ifdef __cplusplus
}
#endif

#endif//__DEBUG_H__
