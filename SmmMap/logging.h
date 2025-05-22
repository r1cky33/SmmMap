#ifndef __smminfect_logging_h__
#define __smminfect_logging_h__

#include "log_lvl.h"
#include "serial.h"

// color code definition: https://xdevs.com/guide/color_serial/

#define SERIAL_DEBUG_LEVEL 2 

#define LOG_ERROR(fmt, ...)  do { if (SERIAL_DEBUG_LEVEL >= LOG_LEVEL_ERROR)  SerialPrintf("\033[0;31m(E) " fmt "\033[0;39;49m", ##__VA_ARGS__); } while (0)
#define LOG_INFO(fmt, ...)   do { if (SERIAL_DEBUG_LEVEL >= LOG_LEVEL_INFO)   SerialPrintf("\033[1;33m(I) " fmt "\033[0;39;49m", ##__VA_ARGS__); } while (0)
#define LOG_VERB(fmt, ...)  do { if (SERIAL_DEBUG_LEVEL >= LOG_LEVEL_VERB)  SerialPrintf("\033[0;32m(V) " fmt "\033[0;39;49m", ##__VA_ARGS__); } while (0)
#define LOG_DBG(fmt, ...)  do { if (SERIAL_DEBUG_LEVEL >= LOG_LEVEL_DBG)  SerialPrintf("\033[0;37m(D) " fmt "\033[0;39;49m", ##__VA_ARGS__); } while (0)

#endif
