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

//other
//For FontSelector
#include <fontconfig.h>     //to use fontconfig
#include <unistd.h>         //for access() function
//For Logger
#include <pwd.h>            //for the passwd structure


//std
#include <sstream>
#include <string>
using namespace std;


namespace lomse
{

//=======================================================================================
// Logger implementation
//=======================================================================================
std::string Logger::get_default_log_path()
{
    struct passwd* pw = getpwuid(getuid());
    const char* homedir = pw->pw_dir;
    string logPath(homedir);
    logPath += "/lomse-log.txt";
    return logPath;
}


//=======================================================================================
// FontSelector::find_font implementation for Linux
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

    string fullpath("");
    FcConfig* config = FcInitLoadConfigAndFonts();      //= FcConfigGetCurrent();

    // configure the search pattern
    FcPattern* pattern = 0;
        //font language
    if (!language.empty() && language != "any")
        pattern =FcPatternBuild(pattern, FC_LANG, FcTypeString, language.c_str(), (char *) 0);
        //font family
    if(language == "zh_CN" || (language.empty() && name=="WenQuanYi Zen Hei"))
    {
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "Noto Sans CJK SC", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "WenQuanYi Zen Hei", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "文泉驿正黑", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "WenQuanYi Micro Hei", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "文泉驿微米黑", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "Microsoft YaHei", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "AR PL ShanHeiSun Uni", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "AR PL New Sung", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "ZYSong18030", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "Adobe Source Han Sans", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "思源黑體", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "Source Han Sans", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "Fandol Hei", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "MHei", (char *) 0);
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "sans", (char *) 0);  //fallback
    }
    else if (!name.empty())
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, name.c_str(), (char *) 0);
    else
        pattern =FcPatternBuild(pattern, FC_FAMILY, FcTypeString, "sans", (char *) 0);
        //style, weight
    if (fBold)
        pattern =FcPatternBuild(pattern, FC_WEIGHT, FcTypeInteger, FC_WEIGHT_BOLD, (char *) 0);
    else if (fItalic)
        pattern =FcPatternBuild(pattern, FC_SLANT, FcTypeInteger, FC_SLANT_ITALIC, (char *) 0);
    else
        pattern =FcPatternBuild(pattern, FC_STYLE, FcTypeString, "Regular", (char *) 0);

    //FcPatternPrint(pattern);
    FcConfigSubstitute(config, pattern, FcMatchPattern);

    //Supply default values for underspecified font patterns:
    //  - Patterns without a specified style or weight are set to Medium
    //  - Patterns without a specified style or slant are set to Roman
    //  - Patterns without a specified pixel size are given one computed from any
    //    specified point size (default 12), dpi (default 75) and scale (default 1).
    FcDefaultSubstitute(pattern);

    //find the best matching font
    FcResult result = FcResultNoMatch;
    FcPattern* font = FcFontMatch(config, pattern, &result);
    if (font)
    {
        FcChar8* file = nullptr;
        if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
        {
            string fullFileName((char*)file);
            fullpath = fullFileName;
        }
        FcPatternDestroy(font);
    }
    else
    {
        stringstream msg;
        msg << "key=" << key;
        switch(result)
        {
            case FcResultNoMatch:
                msg << "Object doesn't exist at all";
                break;
            case FcResultTypeMismatch:
                msg << "Object exists, but the type doesn't match";
                break;
            case FcResultNoId:
                msg << "Object exists, but has fewer values than specified";
                break;
            case FcResultOutOfMemory:
                msg << "malloc failed";
                break;
            default:
                msg << "Unknown reason. Result=" << result;
        }
        LOMSE_LOG_ERROR(msg.str());
        fullpath = "/usr/share/fonts/truetype/LiberationSerif-Regular.ttf";
    }
    FcPatternDestroy(pattern);

    //if Bravura font is requested but it is not installed in the system (e.g.
    //missing dependency in installation page, local installation from sources, etc)
    //try to use local copy at lenmus/res/fonts
    //
    //AWARE: Looking for "ravura.otf" instead of "Bravura.otf" just in case in some
    //       distro they decide to use lower case for the font name.
    if (name=="Bravura" && fullpath.find("ravura.otf") == std::string::npos)
    {
        string localfont = m_pLibScope->fonts_path();
        localfont += "Bravura.otf";
        LOMSE_LOG_INFO("Bravura font not found. Trying path=%s", localfont.c_str());
        if (access(localfont.c_str(), F_OK) == 0)
        {
            fullpath = localfont;
        }
        else
        {
            // do nothing. Program will not crash but music symbols will not be displayed.
            LOMSE_LOG_ERROR("Bravura font not found. Music symbols cannot be dispayed.");
        }
    }

    LOMSE_LOG_INFO("key=%s, Path=%s", key.c_str(), fullpath.c_str());
    m_cache.insert(make_pair(key, fullpath));

    FcConfigDestroy(config);

    return fullpath;
}


}   //namespace lomse
