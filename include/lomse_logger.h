//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2015 Cecilio Salmeron. All rights reserved.
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

#ifndef __LOMSE_LOGGER_H__
#define __LOMSE_LOGGER_H__

#include "lomse_config.h"
#include "lomse_basic.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

//other
#include <boost/format.hpp>

namespace lomse
{

extern ofstream dbgLogger;

#if (LOMSE_COMPILER_MSVC == 1)
    #define LOMSE_LOG_ERROR(msg)        logger.log_error(__FILE__,__LINE__,__FUNCTION__, msg);
    #define LOMSE_LOG_WARN(msg)         logger.log_warn(__FILE__,__LINE__,__FUNCTION__, msg);
    #define LOMSE_LOG_INFO(msg)         logger.log_info(__FILE__,__LINE__,__FUNCTION__, msg);
    #if (LOMSE_ENABLE_DEBUG_LOGS == 1)
        #define LOMSE_LOG_DEBUG(area, msg)  logger.log_debug(__FILE__,__LINE__,__FUNCTION__, area, msg);
        #define LOMSE_LOG_TRACE(area, msg)  logger.log_trace(__FILE__,__LINE__,__FUNCTION__, area, msg);
    #else
        #define LOMSE_LOG_DEBUG(area, msg)
        #define LOMSE_LOG_TRACE(area, msg)
    #endif

#elif (LOMSE_COMPILER_GCC == 1)
    #define LOMSE_LOG_ERROR(msg)        logger.log_error(__FILE__,__LINE__,__PRETTY_FUNCTION__, msg);
    #define LOMSE_LOG_WARN(msg)         logger.log_warn(__FILE__,__LINE__,__PRETTY_FUNCTION__, msg);
    #define LOMSE_LOG_INFO(msg)         logger.log_info(__FILE__,__LINE__,__PRETTY_FUNCTION__, msg);
    #if (LOMSE_ENABLE_DEBUG_LOGS == 1)
        #define LOMSE_LOG_DEBUG(area, msg)  logger.log_debug(__FILE__,__LINE__,__PRETTY_FUNCTION__, area, msg);
        #define LOMSE_LOG_TRACE(area, msg)  logger.log_trace(__FILE__,__LINE__,__PRETTY_FUNCTION__, area, msg);
    #else
        #define LOMSE_LOG_DEBUG(area, msg)
        #define LOMSE_LOG_TRACE(area, msg)
    #endif

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
    int m_mode;
    uint_least32_t m_areas;

public:
    Logger(int mode=k_normal_mode);
    ~Logger();

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
                   const string& msg);
    void log_warn(const string& file, int line, const string& prettyFunction,
                  const string& msg);
    void log_info(const string& file, int line, const string& prettyFunction,
                  const string& msg);
    void log_debug(const string& file, int line, const string& prettyFunction,
                   uint_least32_t area, const string& msg);
    void log_trace(const string& file, int line, const string& prettyFunction,
                   uint_least32_t area, const string& msg);

protected:
    void log_message(const string& file, int line, const string& prettyFunction,
                     const string& prefix, const string& msg);

};

extern Logger logger;


}   //namespace lomse

#endif      //__LOMSE_LOGGER_H__
