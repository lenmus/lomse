//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE__LDP_EXPORTER_H__        //to avoid nested includes
#define __LOMSE__LDP_EXPORTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"

#include <sstream>
using namespace std;

namespace lomse
{

//forward declarations
class ImoObj;
class LdpGenerator;


// LdpExporter: Generates LDP source code for a basic model object
//----------------------------------------------------------------------------------
class LdpExporter
{
protected:
    LibraryScope* m_pLibraryScope;
    string m_version;
    int m_nIndent;
    bool m_fAddId;
    bool m_fRemoveNewlines;

    //temporary
    ImoScore* m_pCurrScore;     //current score being exported
    bool m_fProcessingChord;

public:
    LdpExporter();
    LdpExporter(LibraryScope* pLibraryScope);
    virtual ~LdpExporter();

    //settings
    inline void set_indent(int value) { m_nIndent = value; }
    inline void increment_indent() { ++m_nIndent; }
    inline void decrement_indent() { --m_nIndent; }
    inline void set_add_id(bool value) { m_fAddId = value; }
    inline void set_remove_newlines(bool value) { m_fRemoveNewlines = value; }

    //getters for settings
    inline int get_indent() { return m_nIndent; }
    inline bool get_add_id() { return m_fAddId; }
    inline bool get_remove_newlines() { return m_fRemoveNewlines; }

    //the main method
    string get_source(ImoObj* pImo, ImoObj* pParent=nullptr);

    //static methods for ldp names to types conversion
    static string clef_type_to_ldp(int clefType);
    static string key_type_to_ldp(int keyType);
    static string barline_type_to_ldp(int barlineType);
    static string color_to_ldp(Color color);
    static string float_to_string(float num);
    static string accidentals_to_string(int acc);
    static string notetype_to_string(int noteType, int dots=0);

    //other methods
    string& get_library_version() { return m_version; }
    inline void set_current_score(ImoScore* pScore) { m_pCurrScore = pScore; }
    inline ImoScore* get_current_score() { return m_pCurrScore; }
    inline void set_processing_chord(bool value) { m_fProcessingChord = value; }
    inline bool is_processing_chord() { return m_fProcessingChord; }

protected:
    LdpGenerator* new_generator(ImoObj* pImo);

};


}   //namespace lomse

#endif    // __LOMSE__LDP_EXPORTER_H__

