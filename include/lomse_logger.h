//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_LOGGER_H__
#define __LOMSE_LOGGER_H__

#include "lomse_config.h"
#include "lomse_basic.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

namespace lomse
{

#if (LOMSE_COMPILER_MSVC == 1)
    #define __PRETTY_FUNCTION__       __FUNCTION__
#endif

#ifdef __GNUC__
// GCC and Clang support error detection of format arguments in compile time
#define PRINTF_SYNTAX(strindex) __attribute__((format(printf, strindex, strindex+1)))
#else
#define PRINTF_SYNTAX(strindex)
#endif

#define LOMSE_LOG_ERROR(...)            glogger.log_error(__FILE__,__LINE__,__PRETTY_FUNCTION__,__VA_ARGS__)
#define LOMSE_LOG_WARN(...)             glogger.log_warn(__FILE__,__LINE__,__PRETTY_FUNCTION__,__VA_ARGS__)
#define LOMSE_LOG_INFO(...)             glogger.log_info(__FILE__,__LINE__,__PRETTY_FUNCTION__,__VA_ARGS__)
#if (LOMSE_ENABLE_DEBUG_LOGS == 1)
    #define LOMSE_LOG_DEBUG(area, ...)  glogger.log_debug(__FILE__,__LINE__,__PRETTY_FUNCTION__,area,__VA_ARGS__)
    #define LOMSE_LOG_TRACE(area, ...)  glogger.log_trace(__FILE__,__LINE__,__PRETTY_FUNCTION__,area,__VA_ARGS__)
#else
    #define LOMSE_LOG_DEBUG(area, ...)  do {} while(0)
    #define LOMSE_LOG_TRACE(area, ...)  do {} while(0)
#endif

//---------------------------------------------------------------------------------------
//trace levels, for all spacing algorithms
enum {
    k_trace_off     = 0x0000,   //No trace
    k_trace_entries = 0x0001,   //data when an object is included in table
    k_trace_table   = 0x0002,   //table before and after spacing algorithm
    k_trace_spacing = 0x0004,   //table after each spacing step

    k_trace_all     = 0xFFFF
};


//---------------------------------------------------------------------------------------
class Logger
{
private:
    std::ostream* m_logStream;
    std::ostream* m_customForensicLogStream;
    std::ofstream m_forensicLogStream;
    int m_mode;
    uint_least32_t m_areas;
    bool m_initialized = false;

public:
    Logger(int mode=k_normal_mode);
    ~Logger();

    void init(std::ostream* logStream = nullptr, std::ostream* forensicLogStream = nullptr);
    void deinit();
    std::ostream& get_stream() { return *m_logStream; }

    std::ostream& get_forensic_log_stream();
    void close_forensic_log();

    inline bool debug_mode_enabled() { return m_mode == k_debug_mode; }
    inline bool trace_mode_enabled() { return m_mode == k_trace_mode; }

    //settings
    inline void set_logging_areas(uint_least32_t areas) { m_areas = areas; }
    inline void add_logging_areas(uint_least32_t areas) { m_areas |= areas; }
    inline void clear_logging_areas() { m_areas = 0; }
    inline void set_logging_mode(int mode) { m_mode = mode; }

    //logging modes
    enum
    {
        k_normal_mode = 0,  //log all error, warn and info level messages.
        k_debug_mode,       //as normal mode plus all debug level messages.
        k_trace_mode,       //as debug mode plus all trace level messages.
    };

    //logging areas
    enum
    {
            //16 areas reserved for lomse library
        k_events =      0x00000001,     //event
        k_mvc =         0x00000002,     //creation/destruction of MVC objects
        k_score_player= 0x00000004,     //ScorePlayer playback
        k_render =      0x00000008,     //rendering
        k_layout =      0x00000010,     //layout
        k_gmodel =      0x00000020,     //Graphical model creation and related

            //16 areas reserved for user application
        k_user1 =       0x00010000,
        k_user2 =       0x00020000,
        k_user3 =       0x00040000,
        k_user4 =       0x00080000,
        k_user5 =       0x00100000,
        k_user6 =       0x00200000,
        k_user7 =       0x00400000,
        k_user8 =       0x00800000,
        k_user9 =       0x01000000,
        k_user10 =      0x02000000,
        k_user11 =      0x04000000,
        k_user12 =      0x08000000,
        k_user13 =      0x10000000,
        k_user14 =      0x20000000,
        k_user15 =      0x40000000,
        k_user16 =      0x80000000,

        k_all =         0x0ffffffff,    //all areas
    };

    void log_error(const string& file, int line, const string& prettyFunction,
                   const char* fmtstr, ...) PRINTF_SYNTAX(5);
    void log_error(const string& file, int line, const string& prettyFunction,
                   const string& msg);

    void log_warn(const string& file, int line, const string& prettyFunction,
                  const char* fmtstr, ...) PRINTF_SYNTAX(5);
    void log_warn(const string& file, int line, const string& prettyFunction,
                  const string& msg);

    void log_info(const string& file, int line, const string& prettyFunction,
                  const char* fmtstr, ...) PRINTF_SYNTAX(5);
    void log_info(const string& file, int line, const string& prettyFunction,
                  const string& msg);

    void log_debug(const string& file, int line, const string& prettyFunction,
                   uint_least32_t area, const char* fmtstr, ...) PRINTF_SYNTAX(6);
    void log_debug(const string& file, int line, const string& prettyFunction,
                   uint_least32_t area, const string& msg);

    void log_trace(const string& file, int line, const string& prettyFunction,
                   uint_least32_t area, const char* fmtstr, ...) PRINTF_SYNTAX(6);
    void log_trace(const string& file, int line, const string& prettyFunction,
                   uint_least32_t area, const string& msg);

protected:
    void log_message(const string& file, int line, const string& prettyFunction,
                     const string& prefix, const char* fmtstr, va_list args);
    void log_message(const string& file, int line, const string& prettyFunction,
                     const string& prefix, const string& msg);
    string format(const char* fmtstr, va_list args);

    void clear_forensic_log();

    std::string get_default_log_path();
};

extern Logger glogger;      //logger instance (global)

inline Logger& get_global_logger() { return glogger; }

class StreamLogger
{
public:
    template<typename T>
    StreamLogger& operator<<(const T& t) {
        glogger.get_stream() << t;
        return *this;
    }

    // Overload for I/O manipulators (e.g. std::endl)
    StreamLogger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        glogger.get_stream() << manip;
        return *this;
    }
};

extern StreamLogger dbgLogger;
extern std::ofstream nullLogger;

}   //namespace lomse

#endif      //__LOMSE_LOGGER_H__
