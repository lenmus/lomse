//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_IMPORT_OPTIONS_H__        //to avoid nested includes
#define __LOMSE_IMPORT_OPTIONS_H__

///@cond INTERNAL
namespace lomse
{
///@endcond

//---------------------------------------------------------------------------------------
// MusicXmlOptions
/** Class MusicXmlOptions stores and manages the settings for dealing and fixing
    malformed imported files.
	The only instance of this class is stored in the LibraryScope object. You can
	access it and modify its settings either from LomseDoorway object or from the
	LibraryScope object:

	@code
    LomseDoorway* pLomse = ...
    MusicXmlOptions* opt = pLomse->get_musicxml_options();
	@endcode

	@code
    LibraryScope& lomse = ...
    MusicXmlOptions* opt = lomse.get_musicxml_options();
	@endcode

	Example of use:

	@code
    LomseDoorway* pLomse = ...
    MusicXmlOptions* opt = pLomse->get_musicxml_options();
    opt->fix_beams(true);
    Presenter* pPresenter = pLomse->open_document(k_view_vertical_book,
                                                  "my_score.xml");
    ...
    opt->fix_beams(false);
    Presenter* pPresenter = pLomse->open_document(k_view_vertical_book,
                                                  "other_score.xml");
	@endcode


	Settings description and default values for Lomse library are described in the
	following table:

	<table>
	<tr><th>Setting</th>        <th>Default value</th>  <th>Description</th>
	<tr><td>fix_beams</td>		<td>true</td>
		<td>When %true, if beam information is not congruent with note type, the
            importer will fix the beam.</td></tr>
	<tr><td>use_default_clefs</td>		<td>true</td>
		<td>When %true, if an score part has pitched notes but the clef is missing,
            the importer will assume a G or an F4 clef, depending on notes pitch
            range.</td></tr>
	</table>

	@see fix_beams(), use_default_clefs()
*/
class MusicXmlOptions
{
private:

    class MusicXmlOptionsSettings
    {
        private:
            friend class MusicXmlOptions;

            MusicXmlOptionsSettings()
                : m_fFixBeams(true)
                , m_fDefaultClef(true)
            {
            }

            bool m_fFixBeams;
            bool m_fDefaultClef;

    };

    MusicXmlOptionsSettings m_settings;

    //constructors
    friend class LibraryScope;
    MusicXmlOptions(MusicXmlOptionsSettings& settings) : m_settings(settings) {}
    MusicXmlOptions() {}

public:

    //getters
	/** Returns current setting for the 'fix_beams' option.    */
    inline bool fix_beams() { return m_settings.m_fFixBeams; }

	/** Returns current setting for the 'use_default_clefs' option.    */
    inline bool use_default_clefs() { return m_settings.m_fDefaultClef; }

    //setters (only for options that can be changed without rebuilding the object)
    /** Sets the value for 'fix_beams' option. When %true, if beam information is not
        congruent with note type, the importer will fix the beam.    */
    inline void fix_beams(bool value) { m_settings.m_fFixBeams = value; }

    /** Sets the value for 'use_default_clefs' option. When %true, if an score part
        has pitched notes but the clef is missing, the importer will assume a G or
        an F4 clef, depending on notes pitch range.    */
    inline void use_default_clefs(bool value) { m_settings.m_fDefaultClef = value; }

};


}   //namespace lomse

#endif    // __LOMSE_IMPORT_OPTIONS_H__

