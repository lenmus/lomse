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

#include "lomse_autoclef.h"

#include "lomse_staffobjs_cursor.h"
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_im_algorithms.h"

namespace lomse
{

//=======================================================================================
// AutoClef implementation
//=======================================================================================
AutoClef::AutoClef(ImoScore* pScore)
    : m_pScore(pScore)
    , m_pCursor( LOMSE_NEW StaffObjsCursor(pScore) )
{
}

//---------------------------------------------------------------------------------------
AutoClef::~AutoClef()
{
    delete m_pCursor;
}

//---------------------------------------------------------------------------------------
void AutoClef::do_autoclef()
{
    find_staves_needing_clef();
    add_clefs();
}

//---------------------------------------------------------------------------------------
void AutoClef::find_staves_needing_clef()
{
    //An staff needs clef if it has pitched notes before finding a clef.
    //This method saves data for each staff in vectors m_fNeedsClef, m_pAt,
    //m_maxPitch, m_minPitch

    int staves = m_pCursor->get_num_staves();
    int stavesWithNotes = 0;

    m_fNeedsClef.assign(staves, false);
    m_pAt.assign(staves, (ImoStaffObj*)nullptr);
    m_maxPitch.assign(staves, k_undefined_fpitch);
    m_minPitch.assign(staves, k_undefined_fpitch);
    m_numNotes.assign(staves, 0);

    vector<bool> fHasNotes;         //true if staff i has pitched notes
    fHasNotes.assign(staves, false);

    while(!m_pCursor->is_end())
    {
        ImoStaffObj* pSO = m_pCursor->get_staffobj();
        int iStaff = m_pCursor->staff_index();

        if (m_pAt[iStaff] == nullptr)
            m_pAt[iStaff] = pSO;

        if (pSO->is_note())
        {
            ImoNote* pN = static_cast<ImoNote*>(pSO);
            if (!m_fNeedsClef[iStaff] && !fHasNotes[iStaff])
            {
                if (pN->is_pitch_defined())
                {
                    int clefType = m_pCursor->get_applicable_clef_type();
                    if (clefType == k_clef_undefined)
                        m_fNeedsClef[iStaff] = true;
                    fHasNotes[iStaff] = true;
                }
            }

            if (m_fNeedsClef[iStaff])
            {
                if (pN->is_pitch_defined())
                {
                    FPitch fp = pN->get_fpitch();
                    if (m_maxPitch[iStaff] == k_undefined_fpitch || m_maxPitch[iStaff] < fp)
                        m_maxPitch[iStaff] = fp;
                    if (m_minPitch[iStaff] == k_undefined_fpitch || m_minPitch[iStaff] > fp)
                        m_minPitch[iStaff] = fp;
                    ++m_numNotes[iStaff];

                     if (m_numNotes[iStaff] == 10)
                        ++stavesWithNotes;
                     if (stavesWithNotes == staves)
                        break;
               }
            }
        }
        m_pCursor->move_next();
    }
}

//---------------------------------------------------------------------------------------
void AutoClef::add_clefs()
{
    int idx = 0;
    vector<bool>::iterator it;
    for (it=m_fNeedsClef.begin(); it != m_fNeedsClef.end(); ++it, ++idx)
    {
        if (*it==true)
            add_clef(idx);
    }
}

//---------------------------------------------------------------------------------------
void AutoClef::add_clef(int idx)
{
    int iInstr;
    int iStaff;
    m_pCursor->staff_index_to_instr_staff(idx, &iInstr, &iStaff);

    stringstream clef;
    clef << "(clef " << decide_clef(idx, iStaff) << " p" << (iStaff+1) << ")";
//    cout << "clef: " << clef.str() << ", idx=" << idx << ", iInstr=" << iInstr
//         << ", iStaff=" << iStaff << endl;
    ImoInstrument* pInstr = m_pScore->get_instrument(iInstr);
    ImoTreeAlgoritms::insert_staffobjs(pInstr, m_pAt[idx], clef.str());
}

//---------------------------------------------------------------------------------------
string AutoClef::decide_clef(int idx, int iStaff)
{
    //TODO: improve algorithm for deciding which clef to use

    if (m_minPitch[idx] < FPitch(4,3,0))  //min note < f3 --> use F4
        return "F4";
    if (m_maxPitch[idx] < FPitch(0,4,0))  //max note < c4 --> use F4
        return "F4";
//    if (m_maxPitch[idx] < FPitch(5,4,0))  //max note > g4 --> use G
//        return "G";
//    if (m_minPitch[idx] < FPitch(5,3,0))  //min note > g3 --> use G
//        return "G";

    if (iStaff == 1)
        return "F4";

    return "G";
}


}   //namespace lomse
