#ifndef _CONSOLE_H
#define _CONSOLE_H


#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined __linux__
#include <unistd.h>
#endif

#include "cdlib.h"
#include "cgi.h"

#include "console_typedefs.h"
#include "console_CommandFunc.h"
#include "console_Helpers.h"
#include "console_PacketHandling.h"
#include "console_Logger.h"

#endif
