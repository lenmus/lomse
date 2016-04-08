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

using namespace std;


namespace lomse
{

ofstream dbgLogger;
Logger logger;

//=======================================================================================
// Logger implementation
//=======================================================================================
Logger::Logger(int mode)
    : m_mode(mode)
    , m_areas(0xffffffff)       //all areas enabled
{
    dbgLogger.open("lomse-log.txt");
}

//---------------------------------------------------------------------------------------
Logger::~Logger()
{
    dbgLogger.close();
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

    dbgLogger << file.substr(fileStart) << ", line " << line << ". " << prefix << "["
            << prettyFunction.substr(begin,end) << "] " << msg << endl;
}

//---------------------------------------------------------------------------------------
void Logger::log_error(const string& file, int line, const string& prettyFunction,
                       const string& msg)
{
    log_message(file, line, prettyFunction, "ERROR: ", msg);
}

//---------------------------------------------------------------------------------------
void Logger::log_warn(const string& file, int line, const string& prettyFunction,
                      const string& msg)
{
    log_message(file, line, prettyFunction, "WARNING: ", msg);
}

//---------------------------------------------------------------------------------------
void Logger::log_info(const string& file, int line, const string& prettyFunction,
                      const string& msg)
{
    log_message(file, line, prettyFunction, "INFO: ", msg);
}

//---------------------------------------------------------------------------------------
void Logger::log_debug(const string& file, int line, const string& prettyFunction,
                       uint_least32_t area, const string& msg)
{
    if ((m_mode == k_debug_mode || m_mode == k_trace_mode) && ((m_areas & area) != 0))
        log_message(file, line, prettyFunction, "DEBUG: ", msg);
}

//---------------------------------------------------------------------------------------
void Logger::log_trace(const string& file, int line, const string& prettyFunction,
                       uint_least32_t area, const string& msg)
{
    if (m_mode == k_trace_mode && ((m_areas & area) != 0))
        log_message(file, line, prettyFunction, "TRACE: ", msg);
}


}   //namespace lomse
