//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#include "lomse_logger.h"

#include <algorithm> // min
#include <stdarg.h> // va_start, va_end
using namespace std;


namespace lomse
{

static ofstream defaultLoggerStream;
static const char* forensicLogPath = "forensic_log.txt";
StreamLogger dbgLogger;
ofstream nullLogger;
Logger logger;

//=======================================================================================
// Logger implementation.
//=======================================================================================

Logger::Logger(int mode)
    : m_logStream(&defaultLoggerStream)
    , m_customForensicLogStream(nullptr)
    , m_mode(mode)
    , m_areas(0xffffffff)       //all areas enabled
{
}

//---------------------------------------------------------------------------------------
Logger::~Logger()
{
    deinit();
}

//---------------------------------------------------------------------------------------
void Logger::init(std::ostream* logStream, std::ostream* forensicLogStream)
{
    if (m_initialized)
        return;

    if (logStream)
    {
        m_logStream = logStream;
    }
    else
    {
        m_logStream = &defaultLoggerStream;

        // get_default_log_path() is platform dependent.
        // The code is in file platform/lomse_<platform>.cpp
        // platform/lomse_<platform>.cpp
        const std::string logPath = get_default_log_path();
        defaultLoggerStream.open(logPath);
        LOMSE_LOG_INFO("lomse log path=%s", logPath.c_str());
    }

    if (forensicLogStream)
        m_customForensicLogStream = forensicLogStream;
    else
        clear_forensic_log();

    m_initialized = true;
}

//---------------------------------------------------------------------------------------
void Logger::deinit()
{
    if (m_logStream == &defaultLoggerStream)
        defaultLoggerStream.close();

    m_logStream = &defaultLoggerStream;
    m_initialized = false;
}

//---------------------------------------------------------------------------------------
void Logger::clear_forensic_log()
{
    if (m_customForensicLogStream)
        return;

    m_forensicLogStream.open(forensicLogPath);
    m_forensicLogStream.close();
}

//---------------------------------------------------------------------------------------
std::ostream& Logger::get_forensic_log_stream()
{
    if (m_customForensicLogStream)
        return *m_customForensicLogStream;

    m_forensicLogStream.open(forensicLogPath, std::ofstream::out | std::ofstream::app);
    return m_forensicLogStream;
}

//---------------------------------------------------------------------------------------
void Logger::close_forensic_log()
{
    if (m_customForensicLogStream)
        return;

    m_forensicLogStream.close();
}

//---------------------------------------------------------------------------------------
void Logger::log_message(const string& file, int line, const string& prettyFunction,
                         const string& prefix, const char* fmtstr, va_list args)
{
    log_message(file, line, prettyFunction, prefix, format(fmtstr, args));
}

//---------------------------------------------------------------------------------------
void Logger::log_message(const string& file, int line, const string& prettyFunction,
                         const string& prefix, const string& msg)
{
    size_t end = prettyFunction.rfind("(");
    size_t begin = prettyFunction.substr(0,end).rfind(" ") + 1;
    end -= begin;

    size_t fileStartLinux = file.rfind("/") + 1;
    size_t fileStartWindows = file.rfind("\\") + 1;
    size_t fileStart = max(fileStartLinux, fileStartWindows);

    (*m_logStream) << file.substr(fileStart) << ", line " << line << ". " << prefix << "["
            << prettyFunction.substr(begin,end) << "] " << msg << endl;
}

//---------------------------------------------------------------------------------------
void Logger::log_error(const string& file, int line, const string& prettyFunction,
                       const char* fmtstr, ...)
{
    va_list args;
    va_start(args, fmtstr);
    log_message(file, line, prettyFunction, "ERROR: ", fmtstr, args);
    va_end(args);
}

//---------------------------------------------------------------------------------------
void Logger::log_error(const string& file, int line, const string& prettyFunction,
                       const string& msg)
{
    log_message(file, line, prettyFunction, "ERROR: ", msg);
}

//---------------------------------------------------------------------------------------
void Logger::log_warn(const string& file, int line, const string& prettyFunction,
                      const char* fmtstr, ...)
{
    va_list args;
    va_start(args, fmtstr);
    log_message(file, line, prettyFunction, "WARNING: ", fmtstr, args);
    va_end(args);
}

//---------------------------------------------------------------------------------------
void Logger::log_warn(const string& file, int line, const string& prettyFunction,
                      const string& msg)
{
    log_message(file, line, prettyFunction, "WARNING: ", msg);
}

//---------------------------------------------------------------------------------------
void Logger::log_info(const string& file, int line, const string& prettyFunction,
                      const char* fmtstr, ...)
{
    va_list args;
    va_start(args, fmtstr);
    log_message(file, line, prettyFunction, "INFO: ", fmtstr, args);
    va_end(args);
}

//---------------------------------------------------------------------------------------
void Logger::log_info(const string& file, int line, const string& prettyFunction,
                      const string& msg)
{
    log_message(file, line, prettyFunction, "INFO: ", msg);
}

//---------------------------------------------------------------------------------------
void Logger::log_debug(const string& file, int line, const string& prettyFunction,
                       uint_least32_t area, const char* fmtstr, ...)
{
    if ((m_mode == k_debug_mode || m_mode == k_trace_mode) && ((m_areas & area) != 0))
    {
        va_list args;
        va_start(args, fmtstr);
        log_message(file, line, prettyFunction, "DEBUG: ", fmtstr, args);
        va_end(args);
    }
}

//---------------------------------------------------------------------------------------
void Logger::log_debug(const string& file, int line, const string& prettyFunction,
                       uint_least32_t area, const string& msg)
{
    if ((m_mode == k_debug_mode || m_mode == k_trace_mode) && ((m_areas & area) != 0))
    {
        log_message(file, line, prettyFunction, "DEBUG: ", msg);
    }
}

//---------------------------------------------------------------------------------------
void Logger::log_trace(const string& file, int line, const string& prettyFunction,
                       uint_least32_t area, const char* fmtstr, ...)
{
    if (m_mode == k_trace_mode && ((m_areas & area) != 0))
    {
        va_list args;
        va_start(args, fmtstr);
        log_message(file, line, prettyFunction, "TRACE: ", fmtstr, args);
        va_end(args);
    }
}

//---------------------------------------------------------------------------------------
void Logger::log_trace(const string& file, int line, const string& prettyFunction,
                       uint_least32_t area, const string& msg)
{
    if (m_mode == k_trace_mode && ((m_areas & area) != 0))
    {
        log_message(file, line, prettyFunction, "TRACE: ", msg);
    }
}

//---------------------------------------------------------------------------------------
string Logger::format(const char* fmtstr, va_list args)
{
    va_list args2;
    va_copy(args2, args);

    int len = vsnprintf(nullptr, 0, fmtstr, args);
    if (len < 0)
    {
        (*m_logStream) << endl << "*** ERROR. Logger::format() error: Invalid argument to "
            "format function" << endl;
        return string(fmtstr);
        //throw std::invalid_argument("Invalid argument to format-function");
    }

    vector<char> data(len + 1);
    vsnprintf(data.data(), len + 1, fmtstr, args2);

    va_end(args2);

    return string(data.data());
}

}   //namespace lomse
