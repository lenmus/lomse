//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
    , m_iFirstSystem(-1)
    , m_iLastSystem(-1)
    , m_iPage(0)
    , m_maxSystemHeight(0.0f)
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
    if (m_iFirstSystem == -1)
        m_iFirstSystem = iSystem;
    m_iLastSystem = iSystem;

    add_child_box(pSystem);
    pSystem->set_system_number(iSystem);

    m_maxSystemHeight = max(m_maxSystemHeight, pSystem->get_height());
}

//---------------------------------------------------------------------------------------
GmoBoxSystem* GmoBoxScorePage::get_system(int iSystem)
{
	//returns pointer to GmoBoxSystem for system iSystem (0..n-1)

	int i = iSystem - m_iFirstSystem;
	if (i < 0 || i >= get_num_systems())
		return nullptr;		//the system is not in this page
	else
		return static_cast<GmoBoxSystem*>(m_childBoxes[i]);
}

//---------------------------------------------------------------------------------------
int GmoBoxScorePage::nearest_system_to_point(LUnits y)
{
    //returns sytem absolute number or -1 if not found

    for (int i = m_iFirstSystem; i <= m_iLastSystem; ++i)
    {
        GmoBoxSystem* pBox = static_cast<GmoBoxSystem*>(m_childBoxes[i]);
        if (y <= pBox->get_bottom() && y >= pBox->get_top())
            return i;
    }
    return -1;
}

//---------------------------------------------------------------------------------------
TimeUnits GmoBoxScorePage::end_time()
{
//    if (m_childBoxes.empty())
//        return 0.0;
    GmoBoxSystem* pLastSystem = static_cast<GmoBoxSystem*>(m_childBoxes.back());
    return pLastSystem->end_time();
}

//---------------------------------------------------------------------------------------
TimeUnits GmoBoxScorePage::start_time()
{
//    if (m_childBoxes.empty())
//        return 0.0;
    GmoBoxSystem* pFirstSystem = static_cast<GmoBoxSystem*>(m_childBoxes.front());
    return pFirstSystem->start_time();
}


}  //namespace lomse


