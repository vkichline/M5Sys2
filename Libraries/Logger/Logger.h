#pragma once

#include <Arduino.h>
#include <SD.h>


// To disable logging from even being compiled into the code, define M5SYS_MAX_LOGEVEL_NONE.
// To compile in only error-level logging, define M5SYS_MAX_LOGLEVEL_ERROR
// To compile in error and debugging-level logging, define M5SYS_MAX_LOGLEVEL_INFO
// To compile in everything but verbose logging, define M5SYS_MAX_LOGLEVEL_DEBUG
// With no special define, all logging is compiled in.
// To ACTIVATE logging, call set_logging (or rely on preference settings.)
// Note: logging uses printf: including a newline is your responsibility.
// Note: each call is prepended with a qualifier, like '[D] ', so compose statements instead of appending calls.
// To completely disable these macros, define M5SYS_OMIT_LOGGING_MACROS
//
#if defined(M5SYS_OMIT_LOGGING_MACROS)
  // define no macros
#elif defined(M5SYS_MAX_LOGEVEL_NONE)
  #define  ERROR(args...)   ""
  #define  INFO(args...)    ""
  #define  DEBUG(args...)   ""
  #define  VERBOSE(args...) ""
#elif defined(M5SYS_MAX_LOGEVEL_ERROR)
  #define  ERROR(args...)   sys.log.error(args)
  #define  INFO(args...)    ""
  #define  DEBUG(args...)   ""
  #define  VERBOSE(args...) ""
#elif defined(M5SYS_MAX_LOGLEVEL_INFO)
  #define  ERROR(args...)   sys.log.error(args)
  #define  INFO(args...)    sys.log.info(args)
  #define  DEBUG(args...)   ""
  #define  VERBOSE(args...) ""
#elif defined(M5SYS_MAX_LOGLEVEL_DEBUG)
  #define  ERROR(args...)   sys.log.error(args)
  #define  INFO(args...)    sys.log.info(args)
  #define  DEBUG(args...)   sys.log.debug(args)
  #define  VERBOSE(args...) ""
#else    // nothing specified compiles everything in.
  #define  ERROR(args...)   sys.log.error(args)
  #define  INFO(args...)    sys.log.info(args)
  #define  DEBUG(args...)   sys.log.debug(args)
  #define  VERBOSE(args...) sys.log.verbose(args)
#endif


typedef enum {
  LOG_OUTPUT_SERIAL,
  LOG_OUTPUT_SERIAL2,
  LOG_OUTPUT_SD_FILE
} SysLogOutputKind_t;

typedef enum {
  LOG_NONE,
  LOG_ERROR,
  LOG_INFO,
  LOG_DEBUG,
  LOG_VERBOSE
} SysLogLevel_t;


class Logger {
  public:
    static void           begin(const char* appName, long millis_offset = 0);
    static void           menu();
    static SysLogLevel_t  get_level();
    static void           set(const SysLogLevel_t level);
    static void           set(Print &device);
    static void           set(const SysLogLevel_t level, Print &device);
    static void           error(const char* format, ...);
    static void           info(const char* format, ...);
    static void           debug(const char* format, ...);
    static void           verbose(const char* format, ...);
  protected:
    static Print*         log_device;
    static SysLogLevel_t  log_level;
    static File           log_file;
    static String         program_name;
    static long           millis_offset;
    static bool           include_time;
    static void           menu_logging_level();
    static void           menu_logging_target();
    static void           set_from_prefs();     // Read prefs and set logging level and device
    static void           render_time(char* buffer);
    static void           output_log(const char* tag, const char* format, va_list argp);
};
