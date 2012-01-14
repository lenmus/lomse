//--------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//-------------------------------------------------------------------------------------

#include "lomse_im_figured_bass.h"

#include <sstream>

namespace lomse
{


//-------------------------------------------------------------------------------------
// ImoFiguredBass implementation
//-------------------------------------------------------------------------------------



//typedef struct
//{
//    int             nBitmapID;
//    std::string        sFiguredBass;
//}
//lmCommonFBData;
//
//static const lmCommonFBData m_CommonFB[] =
//{
//    { 0,    "#" },        //5 #3
//    { 1,    "b" },        //5 b3
//    { 2,    "=" },        //5 =3
//    { 3,    "2" },        //6 4 2
//    { 4,    "#2" },        //6 4 #2
//    { 5,    "b2" },        //6 4 b2
//    { 6,    "=2" },        //6 4 =2
//    { 7,    "2+" },        //6 4 #2
//    { 8,    "2 3" },        //7 4 2
//    { 9,    "3" },        //3                 //better "5"
//    { 10,   "4" },        //5 4
//    //{ 11,   "4 3" },        //8 5 4 / 8 5 3
//    { 12,   "4 2" },        //6 4 2
//    { 13,   "4+ 2" },       //6 #4 2
//    { 14,   "4 3" },        //6 4 3
//    { 15,   "5" },        //5 3
//    { 16,   "5 #" },        //5 #3
//    { 17,   "5 b" },        //5 b3
//    { 18,   "5+" },        //#5 3
//    { 19,   "5/" },        //6 5 3
//    { 20,   "5 3" },        //5 3
//    { 21,   "5 4" },        //5 4
//    { 22,   "6" },        //6 3
//    { 23,   "6 #" },        //6 #3
//    { 24,   "6 b" },        //6 b3
//    { 25,   "6\\" },        //#6 3
//    { 26,   "6 3" },        //6 3
//    { 27,   "6 #3" },        //6 #3
//    { 28,   "6 b3" },        //6 b3
//    { 29,   "6 4" },        //6 4
//    { 30,   "6 4 2" },        //6 4 2
//    { 31,   "6 4 3" },        //6 4 3
//    { 32,   "6 5" },        //6 3 5
//    { 33,   "6 5 3" },        //6 5 3
//    { 34,   "7" },        //7 5 3
//    { 35,   "7 4 2" },        //7 4 2
//    { 36,   "8" },        //8 5 3     //?
//    { 37,   "9" },        //9 5 3
//    { 38,   "10" },        //10 5 3    //?
//};
//
//const int k_FB_NUM_COMMON = sizeof(m_CommonFB) / sizeof(lmCommonFBData);
//
//typedef struct
//{
//    std::string        sFigBass;
//    std::string        sSimpler;
//}
//lmSimplerFBData;
//
//static const lmSimplerFBData m_SimplerFB[] =
//{
//    { "5 3",    "5" },
//    { "#5 3",   "5+" },
//    { "5 #3",   "#" },
//    { "5 b3",   "b" },
//    { "5 =3",   "=" },
//    { "6 4 2",  "2" },
//    { "6 4 #2", "#2" },
//    { "6 4 b2", "b2" },
//    { "6 4 =2", "=2" },
//    { "6 4 #2", "2+" },
//    { "7 4 2",  "2 3" },
//    { "5 4",    "4" },
//    { "6 4 2",  "4 2" },
//    { "6 #4 2", "4+ 2" },
//    { "6 4 3",  "4 3" },
//    { "6 5 3",  "5/" },
//    { "6 3",    "6" },
//    { "6 #3",   "6 #" },
//    { "6 b3",   "6 b" },
//    { "#6 3",   "#6" },
//    { "=6 3",   "=6" },
//    { "b6 3",   "b6" },
//    { "6 3 5",  "6 5" },
//    { "7 5 3",  "7" },
//    { "b7 5 3", "7/" },
//    { "8 5 3",  "8" },
//    { "9 5 3",  "9" },
//    { "10 5 3", "10" },
//};

//-----------------------------------------------------------------------------------------
// ImoFiguredBassInfo implementation
//-----------------------------------------------------------------------------------------

ImoFiguredBassInfo::ImoFiguredBassInfo()
    : ImoSimpleObj(k_imo_figured_bass_info)
    , m_error("")
    , m_fAsPrevious(false)
{
    initialize_intervals();
}

void ImoFiguredBassInfo::initialize_intervals()
{
    for (int i=0; i <= lmFB_MAX_INTV; i++)
    {
        m_intervals[i].set_quality(k_interval_not_present);
        m_intervals[i].set_aspect(k_interval_draw_normal);
        m_intervals[i].set_source("");
        m_intervals[i].set_prefix("");
        m_intervals[i].set_suffix("");
        m_intervals[i].set_over("");
        m_intervals[i].set_must_sound(false);
    }
}

ImoFiguredBassInfo::ImoFiguredBassInfo(const std::string& data)
    : ImoSimpleObj(k_imo_figured_bass_info)
    , m_error("")
    , m_fAsPrevious(false)
{
    set_from_string(data);
}

//void ImoFiguredBassInfo::CopyDataFrom(ImoFiguredBassInfo* pFBData)
//{
//    for (int i=0; i <= k_FB_MAX_INTV; i++)
//    {
//        m_intervals[i].nQuality = pFBData->m_intervals[i].nQuality;
//        m_intervals[i].nAspect = pFBData->m_intervals[i].nAspect;
//        m_intervals[i].sSource = pFBData->m_intervals[i].sSource;
//        m_intervals[i].prefix = pFBData->m_intervals[i].prefix;
//        m_intervals[i].sSuffix = pFBData->m_intervals[i].sSuffix;
//        m_intervals[i].sOver = pFBData->m_intervals[i].sOver;
//        m_intervals[i].fSounds = pFBData->m_intervals[i].fSounds;
//    }
//
//    m_fAsPrevious = pFBData->m_fAsPrevious;
//}

std::string ImoFiguredBassInfo::get_figured_bass_string()
{
    std::string sFigBass = "";
    for (int i=lmFB_MAX_INTV; i >= lmFB_MIN_INTV; i--)
    {
        if (m_intervals[i].get_quality() != k_interval_not_present
            && m_intervals[i].get_aspect() != k_interval_draw_understood)
        {
            if (sFigBass != "")
                sFigBass += " ";
            sFigBass += m_intervals[i].get_source();
        }
    }

    return sFigBass;
}
//
//bool ImoFiguredBassInfo::is_equivalent_to(ImoFiguredBassInfo* pFBD)
//{
//    //Compares this figured bass with the received one. Returns true if both are
//    //equivalent, that is, if both encode the same chord
//
//    //TODO: code to take into account FB lines:
//    // line == line?   ==> always true?
//    // line == chrod?  ==> locate chord for line and compared chords?
//
//    bool fOK = true;
//    for (int i=k_FB_MAX_INTV; i >= k_FB_MIN_INTV && fOK; i--)
//    {
//        if (is_sounding(i) == pFBD->is_sounding(i))
//        {
//            if (get_quality(i) == k_interval_not_present)
//                fOK &= (pFBD->get_quality(i) == k_interval_not_present
//                        || pFBD->get_quality(i) == k_interval_as_implied);
//            else if (pFBD->get_quality(i) == k_interval_not_present)
//                fOK &= (get_quality(i) == k_interval_not_present
//                        || get_quality(i) == k_interval_as_implied);
//            else
//                fOK &= (get_quality(i) == pFBD->get_quality(i));
//        }
//        else
//            return false;
//    }
//    return fOK;
//}

void ImoFiguredBassInfo::set_from_string(const std::string& sData)
{
    //Creates intervals data from string.
    //Sets m_error with empty string if no error, or with error message

    initialize_intervals();

    //interval being parsed
    const char* pStart = NULL;          //pointer to first char of interval string
    std::string prefix, suffix, over;
    EIntervalQuality quality = k_interval_as_implied;
    bool fParenthesis = false;
    std::string interval;
    std::string fingerprint = "";       //explicit present intervals (i.e. "53", "642"

    //Finite automata to parse the string

    //posible automata states
    enum k_FBState
    {
        k_FB_START,
        k_FB_PFX01,
        k_FB_NUM01,
        k_FB_PAR01,
        k_FB_END01,
        k_FB_ERROR,
        k_FB_FINISH,
    };

    int nState = k_FB_START;            //automata current state
    const char* pDataStart = sData.c_str();     //points to first char of string
    const char* p = pDataStart;                 //p points to char being parsed
    bool fContinueParsing = true;
    while (fContinueParsing)
    {
        switch (nState)
        {
            //Starting a new interval
            case k_FB_START:

                //initialize interval data
                pStart = p;
                prefix = "";
                suffix = "";
                over = "";
                interval = "";
                quality = k_interval_as_implied;
                fParenthesis = false;

                if (*p == '(')
                {
                    fParenthesis = true;
                    ++p;    //GetNextChar()
                }
                nState = k_FB_PFX01;
                break;

            //New interval, without parenthesis
            case k_FB_PFX01:
                if (is_number(*p))
                {
                    interval = *p;
                    ++p;    //GetNextChar()
                    nState = k_FB_NUM01;
                }
                else if (*p == '#' || *p == '+')
                {
                    prefix = *p;
                    quality = k_interval_raised_half;
                    ++p;    //GetNextChar()
                    nState = k_FB_NUM01;
                }
                else if (*p == 'b' || *p == '-')
                {
                    prefix = *p;
                    quality = k_interval_lowered_half;
                    ++p;    //GetNextChar()
                    nState = k_FB_NUM01;
                }
                else if (*p == '=')
                {
                    prefix = *p;
                    quality = k_interval_force_natural;
                    ++p;    //GetNextChar()
                    nState = k_FB_NUM01;
                }
                else
                    nState = k_FB_ERROR;
                break;

            //interval number
            case k_FB_NUM01:
                if (is_number(*p))
                {
                    interval += *p;
                    ++p;    //GetNextChar()
                }
                else if (*p == '#' || *p == '+')
                {
                    suffix = *p;
                    quality = k_interval_raised_half;
                    ++p;    //GetNextChar()
                    nState = k_FB_PAR01;
                }
                else if (*p == 'b' || *p == '-')
                {
                    suffix = *p;
                    quality = k_interval_lowered_half;
                    ++p;    //GetNextChar()
                    nState = k_FB_PAR01;
                }
                else if (*p == '/')
                {
                    over = *p;
                    quality = k_interval_raised_half;
                    ++p;    //GetNextChar()
                    nState = k_FB_PAR01;
                }
                else if (*p == '\\')
                {
                    over = *p;
                    quality = k_interval_lowered_half;
                    ++p;    //GetNextChar()
                    nState = k_FB_PAR01;
                }
                else
                    nState = k_FB_PAR01;
                break;

            //close parenthesis
            case k_FB_PAR01:
                if (*p == ')')
                {
                    nState = (fParenthesis ? k_FB_END01 : k_FB_ERROR);
                    ++p;    //GetNextChar()
                }
                else
                    nState = k_FB_END01;
                break;

            //one interval finished
            case k_FB_END01:

                //one interval
                if (*p == ' ' || *p == '\0')
                {
                    //one interval completed. Get interval number. If number not present
                    //assume 3
                    int nIntv;
                    if (interval != "")
                        nIntv = to_int(interval);
                    else
                    {
                        nIntv = 3;
                        m_intervals[nIntv].set_aspect(k_interval_draw_understood);
                    }

                    //check interval. Greater than 1 and lower than 13
                    if (nIntv < 2 || nIntv > lmFB_MAX_INTV)
                    {
                        m_error = "Invalid interval '" + interval
                            + "' in figured bass string '" + sData
                            + "'. Figured bass ignored";
                        fContinueParsing = false;
                        break;
                    }
                    //transfer data to total variables
                    m_intervals[nIntv].set_quality(quality);
                    if (fParenthesis)
                        m_intervals[nIntv].set_aspect(k_interval_draw_parenthesis);
                    m_intervals[nIntv].set_source(sData.substr((size_t)(pStart-pDataStart), (size_t)(p-pStart) ));
                    m_intervals[nIntv].set_prefix(prefix);
                    m_intervals[nIntv].set_suffix(suffix);
                    m_intervals[nIntv].set_over(over);
                    m_intervals[nIntv].set_must_sound(true);

                    //add interval to finger print
                    stringstream value;
                    value << nIntv;
                    fingerprint += value.str();

                    //continue with next interval or finish parser
                    if (*p == ' ')
                    {
                        nState = k_FB_START;
                        ++p;    //GetNextChar()
                    }
                    else
                        fContinueParsing = false;
                }
                else
                    nState = k_FB_ERROR;
                break;

            //Error state
            case k_FB_ERROR:
                m_error = "Invalid char '%c', after '%s', in figured bass string %s. Figured bass ignored";
                    //*p, sData.Left(size_t(p-pDataStart-1)).c_str(), sData.c_str() );
                fContinueParsing = false;
                break;

            default:
                fContinueParsing = false;
        }
    }

    set_implicit_intervals(fingerprint);
}

bool ImoFiguredBassInfo::is_number(char ch)
{
    static const std::string numbers("0123456789");
    return (numbers.find(ch) != string::npos);
}

int ImoFiguredBassInfo::to_int(const std::string& value)
{
    int num = 0;
    std::istringstream iss(value);
    iss >> std::dec >> num;
    return num;
}

void ImoFiguredBassInfo::set_implicit_intervals(const std::string& fingerprint)
{
    //determine implicit intervals that exists although not present in
    //figured bass notation

    if (fingerprint == "3")        //5 3
    {
        m_intervals[5].set_must_sound(true);        //add 5th
    }
    else if (fingerprint == "4")   //5 4
    {
        m_intervals[5].set_must_sound(true);        //add 5th
    }
    else if (fingerprint == "5")   //5 3
    {
        m_intervals[3].set_must_sound(true);        //add 3rd
    }
    else if (fingerprint == "6")   //6 3
    {
        m_intervals[3].set_must_sound(true);        //add 3rd
    }
    else if (fingerprint == "2")   //6 4 2
    {
        m_intervals[6].set_must_sound(true);        //add 6th
        m_intervals[4].set_must_sound(true);        //add 4th
    }
    else if (fingerprint == "42")  //6 4 2
    {
        m_intervals[6].set_must_sound(true);        //add 6th
        m_intervals[4].set_must_sound(true);        //add 4th
    }
    else if (fingerprint == "43")  //6 4 3
    {
        m_intervals[6].set_must_sound(true);        //add 6th
    }
    else if (fingerprint == "65")  //6 5 3
    {
        m_intervals[3].set_must_sound(true);        //add 3rd
    }
    else if (fingerprint == "7")  //7 5 3
    {
        m_intervals[5].set_must_sound(true);        //add 5th
        m_intervals[3].set_must_sound(true);        //add 3rd
    }
    else if (fingerprint == "9")  //9 5 3
    {
        m_intervals[5].set_must_sound(true);        //add 5th
        m_intervals[3].set_must_sound(true);        //add 3rd
    }
    else if (fingerprint == "10")  //10 5 3
    {
        m_intervals[5].set_must_sound(true);        //add 5th
        m_intervals[3].set_must_sound(true);        //add 3rd
    }
}



//-----------------------------------------------------------------------------------------
// ImoFiguredBass implementation
//-----------------------------------------------------------------------------------------

ImoFiguredBass::ImoFiguredBass(ImoFiguredBassInfo& info)
    : ImoStaffObj(k_imo_figured_bass)
    , m_info(info)
    //, m_fStartOfLine(false)
    //, m_fEndOfLine(false)
    //, m_fParenthesis(false)
    //, m_pPrevFBLine((ImoFiguredBassLine*)NULL)
    //, m_pNextFBLine((ImoFiguredBassLine*)NULL)
{
    //SetLayer(lm_eLayerNotes);
    //CopyDataFrom(pFBData);
}

ImoFiguredBass::~ImoFiguredBass()
{
    ////delete figured bass lines starting or ending in this object
    //if (m_pPrevFBLine)
    //    delete m_pPrevFBLine;
    //if (m_pNextFBLine)
    //    delete m_pNextFBLine;
}

//ImoFiguredBassLine* ImoFiguredBass::create_line(long nID, ImoFiguredBass* pEndFB)
//{
//    //create a line between the two figured bass objects. The start of the
//    //line is this FB. The other is the end FB and can be NULL and in this
//    //case the line will continue until the last note/rest.
//
//
//    //check that there is no other line starting in this FB
//    if (m_pNextFBLine)
//        return (ImoFiguredBassLine*)NULL;
//
//    ImoFiguredBassLine* pFBL = (ImoFiguredBassLine*)NULL;
//    if (pEndFB)
//    {
//        pFBL = LOMSE_NEW ImoFiguredBassLine(this, nID, this, pEndFB);
//        this->SetAsStartOfFBLine(pFBL);
//        pEndFB->SetAsEndOfFBLine(pFBL);
//
//        this->AttachAuxObj(pFBL);
//        pEndFB->AttachAuxObj(pFBL);
//    }
//    //else if (!pStartFB)
//    //    SetTiePrev((ImoFiguredBassLine*)NULL);
//    //else if (!pEndFB)
//    //    SetTieNext((ImoFiguredBassLine*)NULL);
//    return pFBL;
//}
//
//ImoFiguredBassLine* ImoFiguredBass::create_line(ImoFiguredBass* pEndFB, long nID,
//                                               lmLocation tStartLine,
//                                               lmLocation tEndLine,
//                                               lmTenths ntWidth,
//                                               wxColour nColor)
//{
//    //This method is to be used only during score creation from LDP file.
//    //Creates a line between the two figured bass objects. The start of the
//    //line is this FB. The other is the received one. Line info is
//    //transferred to the created line.
//    //No checks so, before invoking this method you should have verified that:
//    //  - there is no next line already created
//    //
//
//    wxASSERT(!m_pNextFBLine);
//
//    ImoFiguredBassLine* pFBL = create_line(nID, pEndFB);
//    return pFBL;
//}
//
//void ImoFiguredBass::OnRemovedFromRelation(lmRelObX* pRel)
//{
//	//AWARE: this method is invoked only when the relation is being deleted and
//	//this deletion is not requested by this FB. If this FB would like
//	//to delete the relation it MUST invoke Remove(this) before deleting the
//	//relation object
//
//    SetDirty(true);
//
//	if (pRel->IsFiguredBassLine())
//	{
//        if (m_pPrevFBLine == pRel)
//        {
//            this->DetachAuxObj(m_pPrevFBLine);
//			m_pPrevFBLine = (ImoFiguredBassLine*)NULL;
//        }
//        else if (m_pNextFBLine == pRel)
//        {
//            this->DetachAuxObj(m_pNextFBLine);
//			m_pNextFBLine = (ImoFiguredBassLine*)NULL;
//        }
//        else
//        {
//            wxLogMessage("[ImoFiguredBass::OnRemovedFromRelation] Non existing relation!");
//            wxASSERT(false);
//        }
//    }
//    else
//    {
//        wxLogMessage("[ImoFiguredBass::OnRemovedFromRelation] Not expected relation %d !",
//                    pRel->GetScoreObjType() );
//        wxASSERT(false);
//    }
//}
//
//
//
////-------------------------------------------------------------------------------------------
//// ImoFiguredBassLine: an auxliary relation object to model the 'hold chord' line
////-------------------------------------------------------------------------------------------
//
//ImoFiguredBassLine::ImoFiguredBassLine(lmScoreObj* pOwner, long nID,
//                                     ImoFiguredBass* pStartFB, ImoFiguredBass* pEndFB,
//                                     wxColour nColor, lmTenths tWidth)
//    : lmBinaryRelObX(pOwner, nID, lm_eSO_FBLine, pStartFB, pEndFB, lmDRAGGABLE)
//    , m_tWidth(tWidth)
//	, m_nColor(nColor)
//{
//    DefineAsMultiShaped();      //define as a multi-shaped ScoreObj, because
//                                //the line can be broken into two lines at system break
//}
//
//ImoFiguredBassLine::~ImoFiguredBassLine()
//{
//}


}  //namespace lomse
