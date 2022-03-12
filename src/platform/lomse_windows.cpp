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
#include <cstdlib>          //getenv()
using namespace std;

//other
#include <io.h>
#define access   _access_s


namespace lomse
{

//=======================================================================================
// Logger implementation
//=======================================================================================
std::string Logger::get_default_log_path()
{
    string logpath = std::getenv("HOMEPATH");
    logpath += "\\lomse-log.txt";
    return logpath;
}

//=======================================================================================
// FontSelector::find_font implementation for Windows
//  https://docs.microsoft.com/en-us/typography/font-list/tahoma
//=======================================================================================
std::string FontSelector::find_font(const std::string& language,
                                    const std::string& UNUSED(fontFile),
                                    const std::string& name,
                                    bool fBold, bool fItalic)
{
    //search in cache
    string key=language + name + (fBold ? "1" : "0") + (fItalic ? "1" : "0");
    map<string, string>::iterator it = m_cache.find(key);
    if (it != m_cache.end())
        return it->second;

    //get Windows fonts path
    string fontspath = std::getenv("WINDIR");
    string fullpath = fontspath;
    fullpath += "\\Fonts\\";

    //transform font name to capital letters for comparisons
    const locale& loc = locale();
    string fontname;
    for (string::value_type a : name)
        fontname += std::toupper(a, loc);

    //music font
    if (fontname == "BRAVURA")
    {
        fullpath = m_pLibScope->fonts_path();
        fullpath += "Bravura.otf";
        LOMSE_LOG_INFO("key=%s, Path=%s", key.c_str(), fullpath.c_str());
        m_cache.insert(make_pair(key, fullpath));
        return fullpath;
    }

    //Chinese fonts
    if (language == "zh_CN" || (language.empty() && name=="WenQuanYi Zen Hei"))
    {
        if (fBold)
            fullpath += "msjhbd.ttc";
        else
            fullpath += "msjh.ttc";
        LOMSE_LOG_INFO("key=%s, Path=%s", key.c_str(), fullpath.c_str());
        m_cache.insert(make_pair(key, fullpath));
        return fullpath;
    }
    //Check Microsoft YaHei
    //Microsoft YaHei: A Simplified Chinese font that provides excellent
    //reading experience particularly onscreen. The font is very legible
    //at small sizes.
    // msyh.ttc (normal), msyhbd.ttc (bold)

    //Microsoft JhengHei: default interface font
    // msjh.ttc (normal), msjhbd.ttc (bold)


    //substitutions for browser safe fonts
    //and generic fallback fonts:
    //‘serif’, ‘sans-serif’, ‘cursive’, ‘fantasy’, and ‘monospace’

    // serif --> Times New Roman   
    if (   fontname == "SERIF"
        //[Times New Roman], Times, serif
        || fontname == "TIMES"
        || fontname == "LIBERATION SERIF"
        //Georgia, serif
        || fontname == "GEORGIA"
        //Palatino Linotype, Book Antiqua, Palatino, serif
        || fontname == "PALATINO LINOTYPE"
        || fontname == "BOOK ANTIQUA"
        || fontname == "PALATINO"
        //MS Serif, New York, serif
        || fontname == "MS SERIF"
        || fontname == "NEW YORK"
       )
        fontname = "TIMES NEW ROMAN";

    // sans-serif --> Arial
    else if (   fontname == "SANS-SERIF"
             || fontname == "SANS"
             || fontname == "LIBERATION SANS"
             //[Arial], Helvetica, sans-serif
             || fontname == "HELVETICA"
             //Arial Black, Gadget, sans-serif
             || fontname == "ARIAL BLACK"
             || fontname == "GADGET"
             //Impact, Charcoal, sans-serif
             || fontname == "IMPACT"
             || fontname == "CHARCOAL"
             //Lucida Sans Unicode, Lucida Grande, sans-serif
             || fontname == "LUCIDA SANS UNICODE"
             || fontname == "LUCIDA GRANDE"
             //[Tahoma], Geneva, sans-serif
             || fontname == "GENEVA"
             //[Trebuchet MS], Geneva, sans-serif
             //Verdana, Geneva, sans-serif
             || fontname == "VERDANA"
             //MS Sans Serif, Geneva, sans-serif
             || fontname == "MS SANS SERIF"
       )
        fontname = "ARIAL";

    // cursive --> Monotype Corsiva
    else if (   fontname == "CURSIVE"
             //Comic Sans MS, cursive
             || fontname == "COMIC SANS MS"
            )
        fontname = "MONOTYPE CORSIVA";

    // monospace --> Courier New
    else if (   fontname == "MONOSPACE"
             //Courier New, monospace
             //Lucida Console, Monaco, monospace
             || fontname == "LUCIDA CONSOLE"
             || fontname == "MONACO"
            )
        fontname = "COURIER NEW";

    // fantasy -->  Courier New
    else if (   fontname == "FANTASY"
             || fontname == ""
            )
        fontname = "COURIER NEW";


    //search for specific fonts
    if (fontname == "ARIAL") //----------------------- Arial
    {
        if (fBold && fItalic)
            fullpath += "arialbi.ttf";
        else if (fBold)
            fullpath += "arialbd.ttf";
        else if (fItalic)
            fullpath += "ariali.ttf";
        else
            fullpath += "arial.ttf";
    }
    else if (fontname == "COURIER NEW") //------------ Courier New
    {
        if (fBold && fItalic)
            fullpath += "courbi.ttf";
        else if (fItalic)
            fullpath += "couri.ttf";
        else if (fBold)
            fullpath += "courbd.ttf";
        else
            fullpath += "cour.ttf";
    }
    else if (fontname == "MONOTYPE CORSIVA") //------- Monotype Corsiva
    {
        fullpath += "mtcorsva.ttf";
    }
    else if (fontname == "TAHOMA") //----------------- Tahoma
    {
        if (fBold)
            fullpath += "tahomabd.ttf";
        else
            fullpath += "tahoma.ttf";
    }
    else if (fontname == "TIMES NEW ROMAN") //-------- Times New Roman
    {
        if (fBold && fItalic)
            fullpath += "timesbi.ttf";
        else if (fBold)
            fullpath += "timesbd.ttf";
        else if (fItalic)
            fullpath += "timesi.ttf";
        else
            fullpath += "times.ttf";
    }
    else if (fontname == "TREBUCHET MS")    //-------- Trebuchet MS
    {
        if (fBold && fItalic)
            fullpath += "trebucbi.ttf";
        else if (fBold)
            fullpath += "trebucbd.ttf";
        else if (fItalic)
            fullpath += "trebucit.ttf";
        else
            fullpath += "trebuc.ttf";
    }
    else if (   fontname == "LUCIDA HANDWRITING"  //-- Lucida Handwritting    
             || fontname == "HANDWRITTEN"
            )
    {
        fullpath += "lhandw.ttf";
    }

    //Symbol fonts and other fonts
    else
    {
        if (fontname == "SYMBOL")
            fullpath += "symbol.ttf";
        else if (fontname == "WEBDINGS")
            fullpath += "webdings.ttf";
        else if (fontname == "WINGDINGS" || fontname == "ZAPF DINGBATS")
            fullpath += "wingding.ttf";

        //Other fonts: ask user program
        else
            fullpath = m_pLibScope->get_font(name, fBold, fItalic);
    }
    
    //check that file exists
    if (access(fullpath.c_str(), 0) != 0) 
    {
        LOMSE_LOG_ERROR("File font = %s does not exist. Trying Arial.", fullpath.c_str());
        fullpath = fontspath;
        fullpath += "arial.ttf";
        if (access(fullpath.c_str(), 0) != 0) 
            LOMSE_LOG_ERROR("Arial font not found. The program will probably crash!");
    }

    LOMSE_LOG_INFO("key=%s, Path=%s", key.c_str(), fullpath.c_str());
    m_cache.insert(make_pair(key, fullpath));
    return fullpath;
}


}   //namespace lomse
