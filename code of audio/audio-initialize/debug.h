#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_DBG(fmt, ...) if (g_dbg_enable & 0x2) fprintf(stdout, "[%s:%s:%d] " fmt "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) if (g_dbg_enable & 0x1) fprintf(stderr, "[%s:%s:%d] " fmt "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)


extern char g_dbg_enable;

#ifdef __cplusplus
}
#endif

#endif//__DEBUG_H__