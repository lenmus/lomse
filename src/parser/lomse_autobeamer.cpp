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

#include "lomse_autobeamer.h"

#include "lomse_internal_model.h"
#include "lomse_im_note.h"

#include <list>
using namespace std;

namespace lomse
{

//=======================================================================================
// AutoBeamer implementation
//=======================================================================================
void AutoBeamer::do_autobeam()
{
    extract_notes();
    process_notes();
}

//---------------------------------------------------------------------------------------
void AutoBeamer::extract_notes()
{
    m_notes.clear();
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >& noteRests
        = m_pBeam->get_related_objects();
    std::list< pair<ImoStaffObj*, ImoRelDataObj*> >::iterator it;
    for (it = noteRests.begin(); it != noteRests.end(); ++it)
    {
        if ((*it).first->is_note())
            m_notes.push_back( static_cast<ImoNote*>( (*it).first ) );
    }
    //cout << "Num. note/rests in beam: " << noteRests.size() << endl;
    //cout << "NUm. notes in beam: " << m_notes.size() << endl;
}

//---------------------------------------------------------------------------------------
void AutoBeamer::get_triad(int iNote)
{
    if (iNote == 0)
    {
        m_curNotePos = k_first_note;
        m_pPrevNote = nullptr;
        m_pCurNote = m_notes[0];
        m_pNextNote = m_notes[1];
    }
    else if (iNote == (int)m_notes.size() - 1)
    {
        m_curNotePos = k_last_note;
        m_pPrevNote = m_pCurNote;
        m_pCurNote = m_notes[iNote];
        m_pNextNote = nullptr;
    }
    else
    {
        m_curNotePos = k_middle_note;
        m_pPrevNote = m_pCurNote;
        m_pCurNote = m_notes[iNote];
        m_pNextNote = m_notes[iNote+1];
    }
}

//---------------------------------------------------------------------------------------
void AutoBeamer::determine_maximum_beam_level_for_current_triad()
{
    m_nLevelPrev = (m_curNotePos == k_first_note ? -1 : m_nLevelCur);
    m_nLevelCur = get_beaming_level(m_pCurNote);
    m_nLevelNext = (m_pNextNote ? get_beaming_level(m_pNextNote) : -1);
}

//---------------------------------------------------------------------------------------
void AutoBeamer::process_notes()
{
    for (int iNote=0; iNote < (int)m_notes.size(); iNote++)
    {
        get_triad(iNote);
        determine_maximum_beam_level_for_current_triad();
        compute_beam_types_for_current_note();
    }
}

//---------------------------------------------------------------------------------------
void AutoBeamer::compute_beam_types_for_current_note()
{
    for (int level=0; level < 6; level++)
    {
        compute_beam_type_for_current_note_at_level(level);
    }
}

//---------------------------------------------------------------------------------------
void AutoBeamer::compute_beam_type_for_current_note_at_level(int level)
{
    if (level > m_nLevelCur)
        m_pCurNote->set_beam_type(level, ImoBeam::k_none);

    else if (m_curNotePos == k_first_note)
    {
        //a) Case First note:
	    // 2.1) CurLevel > Level(i+1)   -->		Forward hook
	    // 2.2) other cases             -->		Begin

        if (level > m_nLevelNext)
            m_pCurNote->set_beam_type(level, ImoBeam::k_forward);    //2.1
        else
            m_pCurNote->set_beam_type(level, ImoBeam::k_begin);      //2.2
    }

    else if (m_curNotePos == k_middle_note)
    {
        //b) Case Intermediate note:
	    //   2.1) CurLevel < Level(i)
	    //     2.1a) CurLevel > Level(i+1)		-->		End
	    //     2.1b) else						-->		Continue
        //
	    //   2.2) CurLevel > Level(i-1)
		//     2.2a) CurLevel > Level(i+1)		-->		Hook (fwd or bwd, depending on beat)
		//     2.2b) else						-->		Begin
        //
	    //   2.3) else [CurLevel <= Level(i-1)]
		//     2.3a) CurLevel > Level(i+1)		-->		End
		//     2.3b) else						-->		Continue

        if (level > m_nLevelCur)     //2.1) CurLevel < Level(i)
        {
            if (level < m_nLevelNext)
                m_pCurNote->set_beam_type(level, ImoBeam::k_end);        //2.1a
            else
                m_pCurNote->set_beam_type(level, ImoBeam::k_continue);   //2.1b
        }
        else if (level > m_nLevelPrev)       //2.2) CurLevel > Level(i-1)
        {
            if (level > m_nLevelNext)        //2.2a
            {
                //hook. Backward/Forward, depends on position in beat or on values
                //of previous beams
                int i;
                for (i=0; i < level; i++)
                {
                    if (m_pCurNote->get_beam_type(i) == ImoBeam::k_begin ||
                        m_pCurNote->get_beam_type(i) == ImoBeam::k_forward)
                    {
                        m_pCurNote->set_beam_type(level, ImoBeam::k_forward);
                        break;
                    }
                    else if (m_pCurNote->get_beam_type(i) == ImoBeam::k_end ||
                                m_pCurNote->get_beam_type(i) == ImoBeam::k_backward)
                    {
                        m_pCurNote->set_beam_type(level, ImoBeam::k_backward);
                        break;
                    }
                }
                if (i == level)
                {
                    //no possible to take decision based on higher level beam values
                    //Determine it based on position in beat

                    //int nPos = m_pCurNote->GetPositionInBeat();
                    //if (nPos == lmUNKNOWN_BEAT)
                        //Unknownn time signature. Cannot determine type of hook. Use backward
                        m_pCurNote->set_beam_type(level, ImoBeam::k_backward);
                    //else if (nPos >= 0)
                    //    //on-beat note
                    //    m_pCurNote->set_beam_type(level, ImoBeam::k_forward);
                    //else
                    //    //off-beat note
                    //    m_pCurNote->set_beam_type(level, ImoBeam::k_backward);
                }
            }
            else
                m_pCurNote->set_beam_type(level, ImoBeam::k_begin);      //2.2b
        }

        else   //   2.3) else [CurLevel <= Level(i-1)]
        {
            if (level > m_nLevelNext)
                m_pCurNote->set_beam_type(level, ImoBeam::k_end);        //2.3a
            else
                m_pCurNote->set_beam_type(level, ImoBeam::k_continue);   //2.3b
        }
    }

    else
    {
        //c) Case Final note:
	    //   2.1) CurLevel <= Level(i-1)    -->		End
	    //   2.2) else						-->		Backward hook
        if (level <= m_nLevelPrev)
            m_pCurNote->set_beam_type(level, ImoBeam::k_end);        //2.1
        else
            m_pCurNote->set_beam_type(level, ImoBeam::k_backward);   //2.2
    }
}

//---------------------------------------------------------------------------------------
int AutoBeamer::get_beaming_level(ImoNote* pNote)
{
    switch(pNote->get_note_type())
    {
        case k_eighth:
            return 0;
        case k_16th:
            return 1;
        case k_32nd:
            return 2;
        case k_64th:
            return 3;
        case k_128th:
            return 4;
        case k_256th:
            return 5;
        default:
            return -1; //Error: Requesting beaming a note longer than eight
    }
}


}   //namespace lomse
