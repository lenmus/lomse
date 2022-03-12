//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_font_storage.h"

#include "lomse_build_options.h"
#include "lomse_logger.h"

//std
#include <locale>           //to upper conversion
using namespace std;


namespace lomse
{

//=======================================================================================
// Logger implementation
//=======================================================================================
std::string Logger::get_default_log_path()
{
    return "lomse-log.txt";
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

    //search in cache
    string key=language + name + (fBold ? "1" : "0") + (fItalic ? "1" : "0");
    map<string, string>::iterator it = m_cache.find(key);
    if (it != m_cache.end())
        return it->second;

    string fullpath = m_pLibScope->fonts_path();

    if (!fontFile.empty())
    {
        fullpath += fontFile;
        LOMSE_LOG_INFO("key=%s, Path=%s", key.c_str(), fullpath.c_str());
        m_cache.insert(make_pair(key, fullpath));
        return fullpath;
    }

    //transform name to capital letters for comparisons
    const locale& loc = locale();
    string fontname;
    for (string::value_type a : name)
        fontname += std::toupper(a, loc);

    //music font
    if (fontname == "BRAVURA")
    {
        fullpath += "Bravura.otf";
    }

    //Chinese fonts
    else if (language == "zh_CN")
    {
        fullpath += "wqy-zenhei.ttc";
    }

    // Liberation Serif, serif or Times New Roman  -->  Liberation Serif
    else if (fontname == "LIBERATION SERIF" || fontname == "SERIF"
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
    }

    // Liberation Sans, sans-serif or Helvetica  -->  Liberation Sans
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
    }

    //Other fonts: ask user program
    else
        fullpath = m_pLibScope->get_font(name, fBold, fItalic);

    
    LOMSE_LOG_INFO("key=%s, Path=%s", key.c_str(), fullpath.c_str());
    m_cache.insert(make_pair(key, fullpath));
    return fullpath;
}


}   //namespace lomse
