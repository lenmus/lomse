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

#ifndef __LOMSE_AUTOBEAMER_H__
#define __LOMSE_AUTOBEAMER_H__

#include <vector>
//#include "lomse_ldp_elements.h"
//#include "lomse_relation_builder.h"
//#include "lomse_internal_model.h"       //required to define BeamsBuilder, SlursBuilder
//#include "lomse_im_note.h"              //required for enum EAccidentals
//#include "lomse_analyser.h"              //required for enum EAccidentals

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
    AutoBeamer(ImoBeam* pBeam) : m_pBeam(pBeam) {}
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
