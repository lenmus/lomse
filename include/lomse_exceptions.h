//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_EXCEPTIONS_H__
#define __LOMSE_EXCEPTIONS_H__


#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


namespace lomse
{

//-------------------------------------------------------------
class ldp_format_error : public std::exception
{
public:
    ldp_format_error() {}
    virtual const char *what() const throw() {
        return "LDP file format error: "
               "format generic failure";
    }
};


//-------------------------------------------------------------
class exception
{
private:
	char* m_msg;

public:
	~exception()
	{
		delete [] m_msg;
	}

	exception() : m_msg(0) {}

	exception(const char* fmt, ...) :
		m_msg(0)
	{
		if(fmt)
		{
			m_msg = new char [4096];
			va_list arg;
			va_start(arg, fmt);
			vsprintf(m_msg, fmt, arg);
			va_end(arg);
		}
	}

	exception(const exception& exc) :
		m_msg(exc.m_msg ? new char[strlen(exc.m_msg) + 1] : 0)
	{
		if(m_msg) strcpy(m_msg, exc.m_msg);
	}

	const char* msg() const { return m_msg; }

};



}   //namespace lomse

#endif      //__LOMSE_EXCEPTIONS_H__
