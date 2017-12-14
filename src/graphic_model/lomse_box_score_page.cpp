//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#include "lomse_gm_basic.h"

#include "lomse_internal_model.h"
#include "lomse_box_system.h"

namespace lomse
{

//---------------------------------------------------------------------------------------
// GmoBoxScorePage implementation
//---------------------------------------------------------------------------------------
GmoBoxScorePage::GmoBoxScorePage(ImoScore* pScore)
    : GmoBox(GmoObj::k_box_score_page, pScore)
    , m_nFirstSystem(-1)
    , m_nLastSystem(-1)
    , m_iPage(0)
{
}

//---------------------------------------------------------------------------------------
GmoBoxScorePage::~GmoBoxScorePage()
{
}

//---------------------------------------------------------------------------------------
void GmoBoxScorePage::add_system(GmoBoxSystem* pSystem, int iSystem)
{
    //Update references
    if (m_nFirstSystem == -1)
        m_nFirstSystem = iSystem;
    m_nLastSystem = iSystem;

    add_child_box(pSystem);
}

//---------------------------------------------------------------------------------------
GmoBoxSystem* GmoBoxScorePage::get_system(int iSystem)
{
	//returns pointer to GmoBoxSystem for system nSystem (0..n-1)

	int i = iSystem - m_nFirstSystem;
	if (i < 0)
		return nullptr;		//the system is not in this page
	else
		return static_cast<GmoBoxSystem*>(m_childBoxes[i]);
}

//---------------------------------------------------------------------------------------
int GmoBoxScorePage::nearest_system_to_point(LUnits y)
{
    //returns sytem absolute number or -1 if not found

    for (int i = m_nFirstSystem; i <= m_nLastSystem; ++i)
    {
        GmoBoxSystem* pBox = static_cast<GmoBoxSystem*>(m_childBoxes[i]);
        if (y <= pBox->get_bottom() && y >= pBox->get_top())
            return i;
    }
    return -1;
}


}  //namespace lomse


