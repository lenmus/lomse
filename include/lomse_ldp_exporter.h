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

///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class ImoObj;
class LdpGenerator;


//----------------------------------------------------------------------------------
/** %LdpExporter is responsible for generating the LDP source format for an
    score. Exporting a document is a simple operation. It is just instantiating the
    exporter object, specifying the desired generation options and request to generate
    the source code. Example:

    @code
        ofstream file1(path + "musicxml_export.xml", ios::out);
        if (file1.good())
        {
            LdpExporter exporter;
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
class LdpExporter
{
protected:
    std::string m_version;
    int m_nIndent = 0;                  //current indent level
    int m_nIndentSpaces = 3;            //number of spaces per indent step
    bool m_fAddId = false;              //True= export object Id
    bool m_fRemoveNewlines = false;     //True= do not generate newlines nor indent spaces

    //temporary
    ImoScore* m_pCurrScore = nullptr;     //current score being exported
    bool m_fProcessingChord = false;

public:
    /** Constructor */
    LdpExporter();
    /** Destructor */
    virtual ~LdpExporter() {}

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

    /** This method controls generation of objects Id. When set to @TRUE the object
        Id will be added after each tag name, e.g "(clef#39 G)" instead of
        "(clef G)".
        @param value  @TRUE for generating Ids.
    */
    inline void set_add_id(bool value) { m_fAddId = value; }

    //@}    //settings


    //getters for settings
    /** @name Getters for current options    */
    //@{

    /** Returns current setting for the number of spaces that will be generated for each
        indentation step.
    */
    inline int get_indent_spaces() { return m_nIndentSpaces; }

    /** Returns current setting: @TRUE if neither newlines nor indentation spaces will be
        generated.
    */
    inline bool get_remove_newlines() { return m_fRemoveNewlines; }

    /** Returns current setting: @TRUE if Ids will be generated.
    */
    inline bool get_add_id() { return m_fAddId; }

    //@}    //getters for settings



//excluded from public API. Only for internal use.
///@cond INTERNALS
public:

    std::string get_source(ImoObj* pImo, ImoObj* pParent);

    //settings
    inline void set_indent_level(int value) { m_nIndent = value; }
    inline void increment_indent() { ++m_nIndent; }
    inline void decrement_indent() { --m_nIndent; }

    //getters
    inline int get_indent_level() { return m_nIndent; }

    //static methods for ldp names to types conversion
    static std::string clef_type_to_ldp(int clefType);
    static std::string key_type_to_ldp(int keyType);
    static std::string barline_type_to_ldp(int barlineType);
    static std::string color_to_ldp(Color color);
    static std::string float_to_string(float num);
    static std::string accidentals_to_string(int acc);
    static std::string notetype_to_string(int noteType, int dots=0);

    //other methods
    std::string& get_library_version() { return m_version; }
    inline void set_current_score(ImoScore* pScore) { m_pCurrScore = pScore; }
    inline ImoScore* get_current_score() { return m_pCurrScore; }
    inline void set_processing_chord(bool value) { m_fProcessingChord = value; }
    inline bool is_processing_chord() { return m_fProcessingChord; }

protected:
    LdpGenerator* new_generator(ImoObj* pImo);

///@endcond
};


}   //namespace lomse

#endif    // __LOMSE__LDP_EXPORTER_H__

