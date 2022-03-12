//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_AUTOBEAMER_H__
#define __LOMSE_AUTOBEAMER_H__

#include <vector>
using namespace std;

namespace lomse
{

//forward declarations
class ImoNote;
class ImoBeam;

//---------------------------------------------------------------------------------------
//Helper, to determine beam types automatically
class AutoBeamer
{
protected:
    ImoBeam* m_pBeam;

public:
    AutoBeamer(ImoBeam* pBeam)
        : m_pBeam(pBeam)
        , m_curNotePos(k_first_note)
        , m_pPrevNote(nullptr)
        , m_pCurNote(nullptr)
        , m_pNextNote(nullptr)
        , m_nLevelPrev(0)
        , m_nLevelCur(0)
        , m_nLevelNext(0)
    {
    }
    ~AutoBeamer() {}

    void do_autobeam();

protected:


    int get_beaming_level(ImoNote* pNote);
    void extract_notes();
    void determine_maximum_beam_level_for_current_triad();
    void process_notes();
    void compute_beam_types_for_current_note();
    void get_triad(int iNote);
    void compute_beam_type_for_current_note_at_level(int level);

    //notes in the beam, after removing rests
    std::vector<ImoNote*> m_notes;

    //notes will be processed in triads. The triad is the current
    //note being processed and the previous and next ones
    enum ENotePos { k_first_note=0, k_middle_note, k_last_note, };
    ENotePos m_curNotePos;
    ImoNote* m_pPrevNote;
    ImoNote* m_pCurNote;
    ImoNote* m_pNextNote;

    //maximum beam level for each triad note
    int m_nLevelPrev;
    int m_nLevelCur;
    int m_nLevelNext;

};


}   //namespace lomse

#endif      //__LOMSE_AUTOBEAMER_H__
