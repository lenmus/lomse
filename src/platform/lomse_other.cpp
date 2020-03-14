//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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

#include "lomse_font_storage.h"

#include "lomse_build_options.h"
#include "lomse_logger.h"

#include <locale>   //to upper conversion

namespace lomse
{

//=======================================================================================
// Logger implementation
//=======================================================================================
Logger::Logger(int mode)
    : m_mode(mode)
    , m_areas(0xffffffff)       //all areas enabled
{
    dbgLogger.open("lomse-log.txt");
}

//=======================================================================================
// FontSelector::find_font implementation for other Operating Systems
//=======================================================================================
std::string FontSelector::find_font(const std::string& language,
                                    const std::string& fontFile,
                                    const std::string& name,
                                    bool fBold, bool fItalic)
{
    //Priority is given to font file.
    //For generic families (i.e.: sans, serif, monospace, ...) priority is given to
    //language

    LOMSE_LOG_INFO("lang=%s, name=%s", language.c_str(), name.c_str());

    string fullpath = m_pLibScope->fonts_path();

    if (!fontFile.empty())
        return fullpath + fontFile;

    //transform name to capital letters for comparisons
    const locale& loc = locale();
    string fontname;
    for (string::value_type a : name)
        fontname += std::toupper(a, loc);

    //music font
    if (fontname == "BRAVURA")
    {
        fullpath += "Bravura.otf";
        return fullpath;
    }

    //Chinese fonts
    if (language == "zh_CN")
    {
        fullpath += "wqy-zenhei.ttc";
        return fullpath;
    }


    if (fontname == "LIBERATION SERIF" || fontname == "SERIF"
             || fontname == "TIMES NEW ROMAN")
    {
        if (fBold && fItalic)
            fullpath += "LiberationSerif-BoldItalic.ttf";
        else if (fBold)
            fullpath += "LiberationSerif-Bold.ttf";
        else if (fItalic)
            fullpath += "LiberationSerif-Italic.ttf";
        else
            fullpath += "LiberationSerif-Regular.ttf";
        LOMSE_LOG_INFO("--- Path=%s", fullpath.c_str());
        return fullpath;
    }

    else if (fontname == "LIBERATION SANS" || fontname == "SANS-SERIF" || fontname == "SANS"
             || fontname == "HELVETICA")
    {
        if (fBold && fItalic)
            fullpath += "LiberationSans-BoldItalic.ttf";
        else if (fBold)
            fullpath += "LiberationSans-Bold.ttf";
        else if (fItalic)
            fullpath += "LiberationSans-Italic.ttf";
        else
            fullpath += "LiberationSans-Regular.ttf";
        LOMSE_LOG_INFO("--- Path=%s", fullpath.c_str());
        return fullpath;
    }

    else
        return m_pLibScope->get_font(name, fBold, fItalic);
}


}   //namespace lomse
