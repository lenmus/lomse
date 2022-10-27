//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
    the graphical information about the measures in the score.
*/
class GmMeasuresTable
{
protected:
    typedef std::vector<GmoShapeBarline*> BarlinesVector;    //barlines for one instrument

    std::vector<BarlinesVector*> m_instrument;       //pointers to each instrument barlines
    std::vector<int> m_numBarlines;                  //num.added barlines in each instrument

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
