//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE__MXL_EXPORTER_H__        //to avoid nested includes
#define __LOMSE__MXL_EXPORTER_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_internal_model.h"

#include <sstream>
#include <array>

namespace lomse
{

//forward declarations
struct BarlineData;
class ImoInstrument;
class ImoObj;
class MxlGenerator;


//---------------------------------------------------------------------------------------
class MxlExporter
{
protected:
    LibraryScope& m_libraryScope;
    int m_nIndent = 0;
    bool m_fAddId = false;
    bool m_fRemoveNewlines = false;
    bool m_fRemoveSeparators = false;
    std::string m_lomseVersion;
    std::string m_exportTime;
    int m_divisions = 480;
    int m_curTimepos = 0;   //in divisions

    //temporary
    bool m_fProcessingChord;
    ImoInstrument* m_pCurInstrument = nullptr;
    ImoScore* m_pCurScore = nullptr;
    BarlineData* m_pLeftBarline = nullptr;

    //controlling open tags
    std::stack<std::string> m_openTags;

    //controlling open slurs
    static const int k_max_slur_number = 16;
    std::array<ImoId, k_max_slur_number> m_slurs;

    //controlling open tuplets
    static const int k_max_tuplet_number = 16;
    std::array<ImoId, k_max_tuplet_number> m_tuplets;



public:
    MxlExporter(LibraryScope& libScope);
    virtual ~MxlExporter();

    //settings
    inline void set_indent(int value) { m_nIndent = value; }
    inline void increment_indent() { ++m_nIndent; }
    inline void decrement_indent() { --m_nIndent; }
    inline void set_add_id(bool value) { m_fAddId = value; }
    inline void set_remove_newlines(bool value) { m_fRemoveNewlines = value; }
    inline void set_remove_separator_lines(bool value) { m_fRemoveSeparators = value; }
    inline void save_divisions(int value) { m_divisions = value; }


    //getters for settings
    inline int get_indent() const { return m_nIndent; }
    inline bool get_add_id() const { return m_fAddId; }
    inline bool get_remove_newlines() const { return m_fRemoveNewlines; }
    inline bool get_remove_separator_lines() const { return m_fRemoveSeparators; }
    inline int get_divisions() const { return m_divisions; }

    //current objects
    inline ImoInstrument* get_current_instrument() const { return m_pCurInstrument; }
    inline void set_current_instrument(ImoInstrument* pInstr) { m_pCurInstrument = pInstr; }
    inline ImoScore* get_current_score() const { return m_pCurScore; }
    inline void set_current_score(ImoScore* pScore) { m_pCurScore = pScore; }
    inline void set_current_timepos(int timepos) { m_curTimepos = timepos; }
    inline void increment_current_timepos(int timepos) { m_curTimepos += timepos; }
    inline int get_current_timepos() const { return m_curTimepos; }

    //barlines
    void save_data_for_left_barline(BarlineData& data);
    void clear_data_for_left_barline();
    const BarlineData& get_data_for_left_barline() const;
    bool left_barline_required();

    //tuplets
    int start_tuplet_and_get_number(ImoId tupletId);
    int close_tuplet_and_get_number(ImoId tupletId);

    //slurs
    int start_slur_and_get_number(ImoId slurId);
    int close_slur_and_get_number(ImoId slurId);

    //the main method
    std::string get_source(ImoObj* pImo);
    std::string get_source(AScore score);

    //auxiliary
    std::string get_version_and_time_string() const;
    std::string get_lomse_version() const { return m_lomseVersion; }
    std::string get_export_time() const { return m_exportTime; }

    //static methods for types mnx names to conversion
    static std::string barline_type_to_mnx(int barType);
    static std::string note_type_to_mxl_name(int noteType);
    static std::string accidentals_to_mxl_name(int acc);
    static std::string color_to_mnx(Color color);
    static std::string float_to_string(float num);

    //other methods
    inline void set_processing_chord(bool value) { m_fProcessingChord = value; }
    inline bool is_processing_chord() { return m_fProcessingChord; }

    //helper for controlling open tags
    inline bool are_there_open_tags() { return m_openTags.size() > 0; }
    inline std::string current_open_tag() {
        if (are_there_open_tags())
            return m_openTags.top();
        else
            return "";
    }
    inline void push_tag(const std::string& tag) { m_openTags.push(tag); }
    inline void pop_tag() { m_openTags.pop(); }

protected:
    MxlGenerator* new_generator(ImoObj* pImo);

};


}   //namespace lomse

#endif    // __LOMSE__MXL_EXPORTER_H__

