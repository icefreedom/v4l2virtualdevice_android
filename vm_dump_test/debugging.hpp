/*
 * Copyright (C) 2007-2009 Skype Technologies S.A. Confidential and proprietary
 *
 * All intellectual property rights, including but not limited to copyrights,
 * trademarks and patents, as well as know how and trade secrets contained
 * in, relating to, or arising from the internet telephony software of Skype
 * Limited (including its affiliates, "Skype"), including without limitation
 * this source code, Skype API and related material of such software
 * proprietary to Skype and/or its licensors ("IP Rights") are and shall
 * remain the exclusive property of Skype and/or its licensors. The recipient
 * hereby acknowledges and agrees that any unauthorized use of the IP Rights
 * is a violation of intellectual property laws.
 *
 * Skype reserves all rights and may take legal action against infringers of
 * IP Rights.
 *
 * The recipient agrees not to remove, obscure, make illegible or alter any
 * notices or indications of the IP Rights and/or Skype's rights and ownership
 * thereof.
 */

/*
 * Following preprocessor variables affect the behavior:
 * ENABLE_DEBUG - nothing will be logged if not defined
 * DEBUG_LONG_FORMAT - appends file, line and function name as a prefix to each log message
 * LOG_PREFIX - a prefix that will be added to all logging messages
 * LOG_MASK - mask constant. Each bit enables certain logging level.
 * (0x01 - ERROR, 0x02 - WARN ... 0x10 -VDBG)
 * For example use LOG_MASK=0x3 to enable errors and warnings only
 *
 * Note that you can add your own logging levels in each file this way:
 * #define MY_DEBUG1(fmt, args...) LOG(0x20, fmt, ##args)
 * #define MY_DEBUG2(fmt, args...) LOG(0x40, fmt, ##args)
 * */

#ifndef DEBUGGING_HPP
#define DEBUGGING_HPP


//#undef EMBEDDED_API_SERVER
#ifdef EMBEDDED_API_SERVER

#include "misc.hpp"
#include <time.h>

#define __FATAL FatalError
#define __DBG DbgPrintf

#else

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

/// Fatal error
inline void __FATAL(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	vprintf(format, ap);

	abort();
}

/// printf-style debug print
inline void __DBG(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
}
#endif // !EMBEDDED_API_SERVER

#ifndef USE_TIMESTAMPS
#define USE_TIMESTAMPS 0
#endif

#ifndef LOG_PREFIX
#define LOG_PREFIX ""
#endif

#ifndef LOG_MASK
#define LOG_MASK 0x0000000f
#endif

/*
			time_t tim=time(NULL); \
			struct tm *t=localtime(&tim); \
			__DBG("%02d:%02d:%02d " fmt "\n", t->tm_hour, t->tm_min, t->tm_sec, ##args); \

*/
#include <sys/time.h>
#define _DBG(use_timestamps, fmt, args...) \
	do { \
		if (use_timestamps) { \
            struct timeval tv;\
            gettimeofday(&tv, NULL);\
            __DBG("%02d:%02d:%02d:%02d " fmt "\n", (tv.tv_sec % 14400)/ 3600, (tv.tv_sec % 3600) / 60, tv.tv_sec % 60, tv.tv_usec / 10000, ##args); \
		} else {\
			__DBG(fmt "\n", ##args); \
		} \
	} while (0)

#define FATAL(fmt, args...) __FATAL(LOG_PREFIX fmt "\n", ##args)

#ifdef ENABLE_DEBUG


#ifdef DEBUG_LONG_FORMAT
#define _LOG(use_timestamps, level, prefix, file, line, func, fmt, args...) \
	if (LOG_MASK & level) { \
		_DBG(use_timestamps, LOG_PREFIX prefix "%s:%d(%s): " fmt, file, line, func, ##args); \
	}
#else
#define _LOG(use_timestamps, level, prefix, file, line, func, fmt, args...) \
	if (LOG_MASK & level) { \
		_DBG(use_timestamps, LOG_PREFIX prefix fmt, ##args); \
	}
#endif

#else
#define _LOG(level, fmt, args...)
#endif

#define ERROR(fmt, args...) _LOG(USE_TIMESTAMPS, 0x1, "ERR: ", __FILE__, __LINE__, __func__, fmt, ##args)
#define WARN(fmt, args...) _LOG(USE_TIMESTAMPS, 0x2, "WRN: ", __FILE__, __LINE__, __func__, fmt, ##args)
#define INFO(fmt, args...) _LOG(USE_TIMESTAMPS, 0x4, "INF: ", __FILE__, __LINE__, __func__, fmt, ##args)
#define DBG(fmt, args...) _LOG(USE_TIMESTAMPS, 0x8, "DBG: ", __FILE__, __LINE__, __func__, fmt, ##args)
#define VDBG(fmt, args...) _LOG(USE_TIMESTAMPS, 0x10, "VDBG: ", __FILE__, __LINE__, __func__, fmt, ##args)
#define LOG(level, fmt, args...) _LOG(USE_TIMESTAMPS, level, "", __FILE__, __LINE__, __func__, fmt, ##args)

/// Function logger, used through FUNCLOG macro
struct FuncLog
{
	inline FuncLog (const char *func_ ) :
		func(func_),
		prefix("FLOG:"),
		logMask(LOG_MASK),
		logLevel(0x8),
		useTimestamps(USE_TIMESTAMPS)
	{
		if (logMask & logLevel)
			_DBG(useTimestamps, "%s%s", prefix, func);
	}
	inline FuncLog (const char *func_, const char *prefix_, unsigned int useTimestamps_, unsigned int logMask_, unsigned int logLevel_) :
		func(func_),
		prefix(prefix_),
		logMask(logMask_),
		logLevel(logLevel_),
		useTimestamps(useTimestamps_)
	{
		if (logMask & logLevel)
			_DBG(useTimestamps, "%s%s", prefix, func);
	}
	inline ~FuncLog() {
		if (logMask & logLevel)
			_DBG(useTimestamps, "%s%s --end", prefix, func);
	}
	private:
		const char *func, *prefix;
		unsigned int logMask, logLevel, useTimestamps;
};


#define FUNCLOG FuncLog __l(__func__, "FLOG:", USE_TIMESTAMPS, LOG_MASK, 0x08);

#endif

