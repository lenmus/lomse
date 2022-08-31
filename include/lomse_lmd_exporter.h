//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE__LMD_EXPORTER_H__        //to avoid nested includes
#define __LOMSE__LMD_EXPORTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"

#include <sstream>
#include <stack>

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
    std::string m_lomseVersion;
    std::string m_exportTime;

    //controlling open tags
    std::stack<std::string> m_openTags;

public:
    LmdExporter(LibraryScope& libScope);
    virtual ~LmdExporter();

    //formats for scores
    enum {
        k_format_ldp = 0, k_format_lmd, k_format_musicxml, k_format_mnx,
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
    std::string get_source(ImoObj* pImo);

    //auxiliary
    std::string get_version_and_time_string();
    inline LibraryScope& get_library_scope() { return m_libraryScope; }

    //static methods for types ldp names to conversion
    static std::string clef_type_to_ldp(int clefType);
    static std::string barline_type_to_ldp(int barType);
    static std::string color_to_ldp(Color color);
    static std::string float_to_string(float num);

    //helper for controlling open tags
    inline bool are_there_open_tags() { return m_openTags.size() > 0; }
    inline std::string current_open_tag() { return m_openTags.top(); }
    inline void push_tag(const std::string& tag) { m_openTags.push(tag); }
    inline void pop_tag() { m_openTags.pop(); }

protected:
    LmdGenerator* new_generator(ImoObj* pImo);

};


}   //namespace lomse

#endif    // __LOMSE__LMD_EXPORTER_H__

