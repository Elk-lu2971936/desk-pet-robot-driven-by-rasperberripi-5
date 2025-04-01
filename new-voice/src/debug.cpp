#include "debug.h"

log_dbg_flag_t g_dbg_enable = log_dbg_flag_t(LOG_ERR_FLAG | LOG_INFO_FLAG | LOG_DBG_FLAG);

void set_dbg_enable(log_dbg_flag_t flag)
{
    g_dbg_enable = flag;
}


log_dbg_flag_t get_dbg_enable()
{
    return g_dbg_enable;
}
