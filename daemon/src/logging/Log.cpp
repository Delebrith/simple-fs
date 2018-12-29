#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>


using namespace simplefs::log;


static bool infoLoggingOn = true;
static bool warningLoggingOn = true;
static bool errorLoggingOn = true;


void simplefs::log::allowLoggingInfo(bool allowed)
{
	infoLoggingOn = allowed;
}

void simplefs::log::allowLoggingWarnings(bool allowed)
{
	warningLoggingOn = allowed;
}

void simplefs::log::allowLoggingErrors(bool allowed)
{
	errorLoggingOn = allowed;
}

void print_message(const char* type, const char* module, const char* message, va_list args)
{
	char msgbuf[256];
	vsnprintf(msgbuf, 256, message, args);

	time_t rawtime = time(NULL);
	//is not thread safe, however as long as it is only used for local current time it will not be a problem
	struct tm* now = localtime(&rawtime);

	printf("%04d.%02d.%02d %02d:%02d:%02d [%s][%s] %s\n",
		now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
		now->tm_hour, now->tm_min, now->tm_sec,
		type,
		module,
		msgbuf);
}

void simplefs::log::logInfo(const char* module, const char* message, ...)
{
	if (!infoLoggingOn)
		return;

	va_list args;
	va_start(args, message);

	print_message("INFO", module, message, args);

	va_end(args);
}

void simplefs::log::logWarning(const char* module, const char* message, ...)
{
	if (!warningLoggingOn)
		return;

	va_list args;
	va_start(args, message);

	print_message("WARNING", module, message, args);

	va_end(args);
}

void simplefs::log::logError(const char* module, const char* message, ...)
{
	if (!errorLoggingOn)
		return;

	va_list args;
	va_start(args, message);

	print_message("ERROR", module, message, args);

	va_end(args);
}

