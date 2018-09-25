//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#ifndef __LOMSE_IM_FIGURED_BASS_H__
#define __LOMSE_IM_FIGURED_BASS_H__

#include "lomse_internal_model.h"

namespace lomse
{


//interval quality: how the interval is interpreted
enum EIntervalQuality
{
    k_interval_not_present = 0,     //the interval is not present, that is, it doesn't sound
    k_interval_as_implied,          //the interval is as implied by key signature
    k_interval_raised_half,         //the interval is raised by half step, relative to the key signature
    k_interval_lowered_half,        //the interval is lowered by half step, relative to the key signature
    k_interval_force_natural,       //the interval is natural, regardless of the key signature
    k_interval_force_diminished,    //the interval is diminished, regardless of the key signature
};

//interval aspect: how the interval is drawn
enum EIntervalAspect
{
    k_interval_draw_normal = 0,      //no modifiers. Draw number as it is stored
    k_interval_draw_parenthesis,     //draw interval number enclosed in parenthesis
    k_interval_draw_understood,      //interval number is understood. Do not draw it
};


// ImoFigBassIntervalInfo: defines one interval
//-------------------------------------------------------------------------------------------
class ImoFigBassIntervalInfo
{
protected:
    EIntervalQuality  m_quality;        //quality modifier
    EIntervalAspect   m_aspect;         //aspect modifier
    std::string       m_source;         //source string, without parenthesis
    std::string       m_prefix;         //string with all prefix chars.
    std::string       m_suffix;         //string with all suffix chars.
    std::string       m_over;           //string with all overlayed chars
    bool              m_fSounds;        //the interval exists and must sound

public:
    ImoFigBassIntervalInfo()
        : m_quality(k_interval_not_present)
        , m_aspect(k_interval_draw_normal)
        , m_fSounds(false)
    {
    }

    //getters
    inline EIntervalQuality get_quality() { return m_quality; }
    inline EIntervalAspect get_aspect() { return m_aspect; }
    inline const std::string& get_source() { return m_source; }
    inline const std::string& get_prefix() { return m_prefix; }
    inline const std::string& get_suffix() { return m_suffix; }
    inline const std::string& get_over() { return m_over; }
    inline bool get_must_sound() { return m_fSounds; }

    //getters
    inline void set_quality(EIntervalQuality value) { m_quality = value; }
    inline void set_aspect(EIntervalAspect value) { m_aspect = value; }
    inline void set_source(const std::string& value) { m_source = value; }
    inline void set_prefix(const std::string& value) { m_prefix = value; }
    inline void set_suffix(const std::string& value) { m_suffix = value; }
    inline void set_over(const std::string& value) { m_over = value; }
    inline void set_must_sound(bool value) { m_fSounds = value; }
};

#define lmFB_MIN_INTV   2
#define lmFB_MAX_INTV   13


// ImoFiguredBassInfo: helper class with information about a figured bass annotation
//-------------------------------------------------------------------------------------------
class ImoFiguredBassInfo : public ImoSimpleObj
{
protected:
    std::string m_error;    //error msg for constructor from string
    ImoFigBassIntervalInfo m_intervals[lmFB_MAX_INTV+1]; //i=0..13 --> intervals 2nd..13th. 0&1 not used
    bool m_fAsPrevious;     //this FB is the same than previous FB (line FB)
                            //If this flag is true m_intervals is not valid
public:
    ImoFiguredBassInfo();
    ImoFiguredBassInfo(const std::string& data);

    void set_from_string(const std::string& data);
    //void CopyDataFrom(ImoFiguredBassInfo* pFBData);

    //general information
    std::string get_figured_bass_string();
    inline const std::string& get_error_msg() { return m_error; }
    bool is_equivalent_to(ImoFiguredBassInfo* pFBD);

    //information about intervals. Interval number = 2..13
    inline EIntervalQuality get_quality(int nIntv) { return m_intervals[nIntv].get_quality(); }
    inline EIntervalAspect get_aspect(int nIntv) { return m_intervals[nIntv].get_aspect(); }
    inline const std::string& get_source(int nIntv) { return m_intervals[nIntv].get_source(); }
    inline const std::string& get_prefix(int nIntv) { return m_intervals[nIntv].get_prefix(); }
    inline const std::string& get_suffix(int nIntv) { return m_intervals[nIntv].get_suffix(); }
    inline const std::string& get_over(int nIntv) { return m_intervals[nIntv].get_over(); }
    inline bool is_sounding(int nIntv) { return m_intervals[nIntv].get_must_sound(); }

    ////modify data
    //inline void SetQuality(int nIntv, EIntervalQuality nQuality)
    //                { m_intervals[nIntv].nQuality = nQuality; }
    //inline void SetAspect(int nIntv, EIntervalAspect nAspect)
    //                { m_intervals[nIntv].nAspect = nAspect; }
    //inline void SetSource(int nIntv, std::string sSource)
    //                { m_intervals[nIntv].sSource = sSource; }
    //inline void SetPrefix(int nIntv, std::string prefix)
    //                { m_intervals[nIntv].prefix = prefix; }
    //inline void SetSuffix(int nIntv, std::string sSuffix)
    //                { m_intervals[nIntv].sSuffix = sSuffix; }
    //inline void SetOver(int nIntv, std::string sOver)
    //                { m_intervals[nIntv].sOver = sOver; }
    //inline void SetSounds(int nIntv, bool fValue)
    //                { m_intervals[nIntv].fSounds = fValue; }

protected:
    void initialize_intervals();
    void set_implicit_intervals(const std::string& fingerprint);
    bool is_number(char ch);
    int to_int(const std::string& string);


};

//----------------------------------------------------------------------------------
class ImoFiguredBass : public ImoStaffObj
{
protected:
    ImoFiguredBassInfo m_info;
//    bool                m_fStartOfLine;         //start of line (hold chord)
//    bool                m_fEndOfLine;           //change of chord
//    bool                m_fParenthesis;         //enclose all figured bass in parenthesis
//    ImoFiguredBassLine*  m_pPrevFBLine;
//    ImoFiguredBassLine*  m_pNextFBLine;

public:
    ImoFiguredBass() : ImoStaffObj(k_imo_figured_bass) {}
    ImoFiguredBass(ImoFiguredBassInfo& info);
    ~ImoFiguredBass();

    inline std::string get_figured_bass_string() { return m_info.get_figured_bass_string(); }

//    ImoFiguredBassLine* create_line(ImoId nID, ImoFiguredBass* pEndFB);
//    inline void SetAsStartOfFBLine(ImoFiguredBassLine* pFBLine) { m_pNextFBLine = pFBLine; }
//    inline void SetAsEndOfFBLine(ImoFiguredBassLine* pFBLine) { m_pPrevFBLine = pFBLine; }
//    ImoFiguredBassLine* create_line(ImoFiguredBass* pEndFB, ImoId nID,
//                                    lmLocation tStartLine, lmLocation tEndLine,
//                                    lmTenths ntWidth, wxColour nColor);
//
//	//overrides to deal with relations (figured bass lines)
//    void OnRemovedFromRelation(lmRelObX* pRel);
};



//// ImoFiguredBassLine: an auxliary relation object to model the 'hold chord' line
////-------------------------------------------------------------------------------------------
//
//class ImoFiguredBassLine : public lmBinaryRelObX
//{
//public:
//    ImoFiguredBassLine(lmScoreObj* pOwner, ImoId nID, ImoFiguredBass* pStartFB,
//                      ImoFiguredBass* pEndFB, wxColour nColor = *wxBLACK,
//                      lmTenths tWidth = 1.0f);
//    ~ImoFiguredBassLine();
//
//    //implementation of pure virtual methods of base class
//    lmLUnits LayoutObject(lmBox* pBox, lmPaper* pPaper, lmUPoint uPos, wxColour colorC);
//	lmUPoint ComputeBestLocation(lmUPoint& uOrg, lmPaper* pPaper);
//    int GetNumPoints() { return 2; }
//
//    //overrides
//    void OnParentComputedPositionShifted(lmLUnits uxShift, lmLUnits uyShift) {}
//    void OnParentMoved(lmLUnits uxShift, lmLUnits uyShift) {}
//
//    //source code related methods. Implementation of needed virtual methods in lmRelObj
//    std::string SourceLDP_First(int nIndent, bool fUndoData, lmStaffObj* pSO);
//    std::string SourceLDP_Last(int nIndent, bool fUndoData, lmStaffObj* pSO);
//    std::string SourceXML_First(int nIndent, lmStaffObj* pSO);
//    std::string SourceXML_Last(int nIndent, lmStaffObj* pSO);
//
//    // debug methods
//    std::string Dump();
//
//    //undoable edition commands
//    void MoveObjectPoints(int nNumPoints, int nShapeIdx, lmUPoint* pShifts, bool fAddShifts);
//
//protected:
//    lmTenths        m_tWidth;
//    wxColour        m_nColor;
//
//    //user displacements on computed line points. Four points: two for start & end
//    //of first line (points 0, 1) and two for start & end of second line (points 2, 3)
//    lmTPoint    m_tPoint[4];
//
//};


}   //namespace lomse

#endif      //__LOMSE_IM_FIGURED_BASS_H__

