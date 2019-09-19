//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#ifndef __LOMSE_GM_MEASURES_TABLE_H__
#define __LOMSE_GM_MEASURES_TABLE_H__

#include "lomse_basic.h"

//using namespace std;

namespace lomse
{

//forward declarations
class ImoScore;
class GmoShapeBarline;
class GmoBoxSystem;

//---------------------------------------------------------------------------------------
/** %GmMeasuresTable object is responsible for storing and managing a table with
    the measures graphical information.
*/
class GmMeasuresTable
{
protected:
    typedef vector<GmoShapeBarline*> BarlinesVector;    //barlines for one instrument

    vector<BarlinesVector*> m_instrument;       //pointers to each instrument barlines
    vector<int> m_numBarlines;                  //num.added barlines in each instrument

public:
    GmMeasuresTable(ImoScore* pScore);
    ///Destructor
    ~GmMeasuresTable();

    //creation
    //invoked when a non-middle barline is found
    void finish_measure(int iInstr, GmoShapeBarline* pBarlineShape);

    //info
    int get_num_measures(int iInstr);
    inline int get_num_instruments() { return int(m_instrument.size()); }

    /** Returns the measure end barline left border position.

        When measure has no end barline (e.g. last measure in the score, when ended
        without barline), the end barline position will be the system left border.
    */
    LUnits get_end_barline_left(int iInstr, int iMeasure, GmoBoxSystem* pBox);

    /** Returns the measure start barline right border position.

        For first measure in the system, the start barline is the first available
        position after system prolog.
    */
    LUnits get_start_barline_right(int iInstr, int iMeasure, GmoBoxSystem* pBox);

    //debug
    void dump_gm_measures();

protected:
    void initialize_vectors(ImoScore* pScore);

};



}   //namespace lomse

#endif      //__LOMSE_GM_MEASURES_TABLE_H__
