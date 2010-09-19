//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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
//  
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#ifndef __LOMSE__EXCEPTIONS_H__
#define __LOMSE__EXCEPTIONS_H__


#include <stdexcept>


namespace lomse
{

class ldp_format_error : public std::exception
{
public:
    ldp_format_error() {}
    virtual const char *what() const throw() {
        return "LDP file format error: "
               "format generic failure";
    }
};


}   //namespace lomse

#endif      //__LOMSE__EXCEPTIONS_H__
