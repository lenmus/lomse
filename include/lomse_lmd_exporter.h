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

#ifndef __LOMSE__LMD_EXPORTER_H__        //to avoid nested includes
#define __LOMSE__LMD_EXPORTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"

#include <sstream>
using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class LmdGenerator;


//---------------------------------------------------------------------------------------
class LmdExporter
{
protected:
    LibraryScope& m_libraryScope;
    int m_nIndent;
    bool m_fAddId;
    int m_scoreFormat;
    bool m_fRemoveNewlines;
    string m_lomseVersion;
    string m_exportTime;

public:
    LmdExporter(LibraryScope& libScope);
    virtual ~LmdExporter();

    //formats for scores
    enum {
        k_format_ldp = 0, k_format_lmd, k_format_musicxml,
    };

    //settings
    inline void set_indent(int value) { m_nIndent = value; }
    inline void increment_indent() { ++m_nIndent; }
    inline void decrement_indent() { --m_nIndent; }
    inline void set_add_id(bool value) { m_fAddId = value; }
    inline void set_remove_newlines(bool value) { m_fRemoveNewlines = value; }
    inline void set_score_format(int value) { m_scoreFormat = value; }

    //getters for settings
    inline int get_indent() { return m_nIndent; }
    inline bool get_add_id() { return m_fAddId; }
    inline int get_score_format() { return m_scoreFormat; }
    inline bool get_remove_newlines() { return m_fRemoveNewlines; }

    //the main method
    string get_source(ImoObj* pImo);

    //auxiliary
    string get_version_and_time_string();

    //static methods for types ldp names to conversion
    static string clef_type_to_ldp(int clefType);
    static string barline_type_to_ldp(int barType);
    static string color_to_ldp(Color color);
    static string float_to_string(float num);

protected:
    LmdGenerator* new_generator(ImoObj* pImo);

};


}   //namespace lomse

#endif    // __LOMSE__LMD_EXPORTER_H__

