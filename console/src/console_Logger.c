
#include <stdio.h>
#include <stdarg.h>

#ifdef __linux__
#include <time.h>
#endif

#include "console_typedefs.h"
#include "console_Logger.h"

/********************************************************************************
								LOGGER
********************************************************************************/
void LOG_INFO2(const char* FunctionName, char* format,...)
{
	// 1024 is the maximum OutBuffer size
	char     szBuff[1024];
	char	Header[1024];
	va_list  arg_ptr;

#ifdef _WIN32
	SYSTEMTIME sTime;
#elif __linux__
	time_t						time_value;
	struct tm*					now = NULL;
#endif

	if (CONSOLE_ENABLE_LOGGER_INFO == 0)
	{
		return;
	}

	if (!format) return;

#ifdef _WIN32
	GetLocalTime(&sTime);
#elif __linux__
	time_value = time(NULL);
	now = localtime(&time_value);
#endif

	va_start(arg_ptr, format);
	vsnprintf(szBuff, sizeof(szBuff), format, arg_ptr);

#ifdef _WIN32
	sprintf(Header,"%04d/%02d/%02d | %02d:%02d:%02d.%03d | %s()",
						sTime.wYear,
						sTime.wMonth,
						sTime.wDay,
						sTime.wHour,
						sTime.wMinute,
						sTime.wSecond,
						sTime.wMilliseconds,
						FunctionName);
#elif __linux__
	sprintf(Header,"%04d/%02d/%02d | %02d:%02d:%02d | %s()",
						now->tm_year+1900,
						now->tm_mon+1,
						now->tm_mday,
						now->tm_hour,
						now->tm_min,
						now->tm_sec,
						FunctionName);
#endif

	fprintf(stdout, "%-60s | %s\n",Header, szBuff);
	va_end(arg_ptr); 

}
