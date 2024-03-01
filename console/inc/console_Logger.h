#ifndef console_Logger_h__
#define console_Logger_h__

#include "console_defs.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void LOG_INFO2(const char* FunctionName, char* format,...);

#define LOG_INFO(x, ...) LOG_INFO2(__FUNCTION__,x, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // console_Logger_h__

