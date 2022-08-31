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
#include <stack>

///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
struct BarlineData;
class ImoInstrument;
class ImoObj;
class MxlGenerator;


//---------------------------------------------------------------------------------------
/** %MxlExporter is responsible for generating the MusicXML source format for an
    score. Exporting a document is a simple operation. It is just instantiating the
    exporter object, specifying the desired generation options and request to generate
    the source code. Example:

    @code
        ofstream file1(path + "musicxml_export.xml", ios::out);
        if (file1.good())
        {
            MxlExporter exporter(m_libraryScope);
            exporter.set_remove_separator_lines(true);
            ImoScore* pScore = ...
            std::string source = exporter.get_source(pScore);
            file1.write(source.c_str(), source.size());
            file1.close();
        }
        else
        {
            std::cout << "file error write" << endl;
            CHECK( false );
        }
    @endcode

*/
class MxlExporter
{
protected:
    LibraryScope& m_libraryScope;
    int m_nIndent = 0;                  //current indentation level
    int m_nIndentSpaces = 3;            //number of spaces per indent step
//    bool m_fAddId = false;
    bool m_fRemoveNewlines = false;     //true= maximum compactness: no newlines and no indentation spaces
    bool m_fRemoveSeparators = false;   //false= include separator lines between <measure> elements
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

    //for ensuring proper ordering of non-timed staffobjs
    std::list< std::pair<ImoStaffObj*, ImoStaffObj*> > m_pendingStaffObjs;

public:
    /** Constructor */
    MxlExporter(LibraryScope& libScope);
    /** Destructor */
    virtual ~MxlExporter();

    //main methods for generating the source code
    /** @name Main methods for generating the source code    */
    //@{

    /** This method generates the source code for the object passed as argument.
        The method is oriented to generate the full source code for an score but
        also can be used to generate partial source code for an element. This can
        be useful for tests or other purposes. PLEASE take into
        account that not all objects can directly generate source code, and that
        for others the code can be incomplete.
        @param pImo  The object whose source code is requested.
    */
    std::string get_source(ImoObj* pImo) { return get_source(pImo, nullptr); }

    /** This method generates the source code for the score passed as argument.
        @param score  The score whose source code is requested.
    */
    std::string get_source(AScore score);

    //@}    //main methods


    //settings
    /** @name Options for generating code    */
    //@{

    /** This method controls the number of spaces that will be generated for each
        indentation step. By default, the exporter uses three spaces per step.
        Indentation is supressed when remove newlines is set to @TRUE.
        See set_remove_newlines().
        @param spaces  The number of spaces to use for each indentation step.
    */
    inline void set_indent_spaces(int spaces) { m_nIndentSpaces = spaces; }

    /** This method controls generation of newlines. When set to @TRUE maximum
        compactness will be achieved: no newlines and no indentation spaces will be
        generated. Default value is @FALSE so newlines and indentation spaces will
        be generated. The number of indentation spaces can be controlled with
        method set_indent_spaces().
        @param value  @TRUE for not generating neither newlines nor indent spaces.
    */
    inline void set_remove_newlines(bool value) { m_fRemoveNewlines = value; }

    /** This method controls generation of separator lines between @<measure@> elements.
        By default, the exporter will generate separator lines.
        @param value  @TRUE for not generating separator lines.
    */
    inline void set_remove_separator_lines(bool value) { m_fRemoveSeparators = value; }

    //@}    //settings


    //getters for settings
    /** @name Getters for current options    */
    //@{

    /** Returns current setting for the number of spaces that will be generated for each
        indentation step.
    */
    inline int get_indent_spaces() const { return m_nIndentSpaces; }
//    inline bool get_add_id() const { return m_fAddId; }

    /** Returns current setting: @TRUE if neither newlines nor indentation spaces will be
        generated.
    */
    inline bool get_remove_newlines() const { return m_fRemoveNewlines; }

    /** Returns current setting: @TRUE for not generating separator lines.
    */
    inline bool get_remove_separator_lines() const { return m_fRemoveSeparators; }

    //@}    //getters for settings



//excluded from public API. Only for internal use.
///@cond INTERNALS
public:
    std::string get_source(ImoObj* pImo, ImoObj* pParent);

    //setters for options
    inline void set_indent_level(int value) { m_nIndent = value; }
    inline void increment_indent() { ++m_nIndent; }
    inline void decrement_indent() { --m_nIndent; }
    inline void save_divisions(int value) { m_divisions = value; }

    //getters for settings
    inline int get_divisions() const { return m_divisions; }
    inline int get_indent_level() const { return m_nIndent; }

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
    int get_number_for_slur(ImoId slurId);

    //auxiliary
    std::string get_version_and_time_string() const;
    std::string get_lomse_version() const { return m_lomseVersion; }
    std::string get_export_time() const { return m_exportTime; }

    //static methods for conversion of types to mxl names
    static std::string note_type_to_mxl_name(int noteType);
    static std::string accidentals_to_mxl_name(int acc);
    static std::string float_to_string(float num);

    //managing pending staffobjs
    void save_pending_staffobj(ImoStaffObj* pNext, ImoStaffObj* pSO);
    void export_pending_staffobjs(MxlGenerator* pRequester, ImoStaffObj* pOwner);

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
    MxlGenerator* new_generator(ImoObj* pImo, ImoObj* pParent);

///@endcond
};


}   //namespace lomse

#endif    // __LOMSE__MXL_EXPORTER_H__

