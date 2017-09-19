//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2017. All rights reserved.
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

#ifndef __LOMSE_IMPORT_OPTIONS_H__        //to avoid nested includes
#define __LOMSE_IMPORT_OPTIONS_H__


namespace lomse
{

//---------------------------------------------------------------------------------------
// MusicXmlOptions: manages options for dealing and fixing malformed imported files
//---------------------------------------------------------------------------------------
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
    inline bool fix_beams() { return m_settings.m_fFixBeams; }
    inline bool use_default_clefs() { return m_settings.m_fDefaultClef; }

    //setters (only for options that can be changed without rebuilding the object)

    /// If beam information is not congruent with note type, fix the beam.
    inline void fix_beams(bool value) { m_settings.m_fFixBeams = value; }

    /// When an score part has pitched notes but clef is missing, assume G or
    /// F4 clef depending on notes pitch range.
    inline void use_default_clefs(bool value) { m_settings.m_fDefaultClef = value; }

};


}   //namespace lomse

#endif    // __LOMSE_IMPORT_OPTIONS_H__

