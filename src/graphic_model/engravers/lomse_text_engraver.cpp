//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_text_engraver.h"

#include "lomse_internal_model.h"
#include "lomse_calligrapher.h"
#include "lomse_gm_basic.h"
#include "lomse_shape_text.h"
#include "lomse_score_meter.h"
#include "lomse_vertical_profile.h"


namespace lomse
{

//=======================================================================================
// TextEngraver implementation
//=======================================================================================
TextEngraver::TextEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           const string& text, const string& language, ImoStyle* pStyle,
                           int classid)
    : Engraver(libraryScope, pScoreMeter)
    , m_text(text)
    , m_pStyle(pStyle)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_language(language)
    , m_idxStaff(-1)
    , m_classid(classid)
    , m_pVProfile(nullptr)
{
}

//---------------------------------------------------------------------------------------
TextEngraver::~TextEngraver()
{
}

//---------------------------------------------------------------------------------------
LUnits TextEngraver::measure_width()
{
    TextMeter meter(m_libraryScope);
    meter.select_font(m_language,
                      m_pStyle->font_file(),
                      m_pStyle->font_name(),
                      m_pStyle->font_size() );
    return meter.measure_width(m_text);
}

//---------------------------------------------------------------------------------------
LUnits TextEngraver::measure_height()
{
    TextMeter meter(m_libraryScope);
    meter.select_font(m_language,
                      m_pStyle->font_file(),
                      m_pStyle->font_name(),
                      m_pStyle->font_size() );
    return meter.get_font_height();
}

//---------------------------------------------------------------------------------------
GmoShapeText* TextEngraver::create_shape(ImoObj* pCreatorImo, ShapeId idx,
                                         LUnits xLeft, LUnits yTop)
{
    UPoint pos(xLeft, yTop);
    if (pCreatorImo && pCreatorImo->is_contentobj())
    {
        //TODO: This is a temporal fix for dealing with <words> "default-x, default-y"
        ImoContentObj* pObj = static_cast<ImoContentObj*>(pCreatorImo);
        pos.x += tenths_to_logical(pObj->get_user_ref_point_x());
        pos.y += tenths_to_logical(pObj->get_user_ref_point_y());

        add_user_shift(static_cast<ImoContentObj*>(pCreatorImo), &pos);
    }

    //TODO-LOG
    //if (valign == k_center)
    {
        TextMeter meter(m_libraryScope);
        yTop -= meter.get_descender();
    }

    GmoShapeText* pShape = LOMSE_NEW GmoShapeText(pCreatorImo, idx, m_text, m_pStyle,
                                                  m_language, m_classid, pos.x, pos.y,
                                                  m_libraryScope);
//    stringstream msg;
//    msg << "idxStaff=" << m_idxStaff;
//    if (m_idxStaff >=0 && m_pVProfile)
//    {
//        LUnits yMin = m_pVProfile->get_min_for(pShape->get_left(), pShape->get_right(),
//                                               m_idxStaff);
//        msg << ", yMinProfile=" << yMin << ", shape.y=" << pos.y - pShape->get_height();
//        //pShape->set_top( min(pos.y - pShape->get_height(), yMin) );
//    }
//    LOMSE_LOG_INFO(msg.str());
    return pShape;
}



//=======================================================================================
// TextBoxEngraver implementation
//=======================================================================================
TextBoxEngraver::TextBoxEngraver(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                           const string& text, const string& language, ImoStyle* pStyle)
    : Engraver(libraryScope, pScoreMeter)
    , m_text(text)
    , m_pStyle(pStyle)
    , m_pFontStorage( libraryScope.font_storage() )
    , m_language(language)
{
}

//---------------------------------------------------------------------------------------
TextBoxEngraver::~TextBoxEngraver()
{
}

//---------------------------------------------------------------------------------------
LUnits TextBoxEngraver::measure_width()
{
    TextMeter meter(m_libraryScope);
    meter.select_font(m_language,
                      m_pStyle->font_file(),
                      m_pStyle->font_name(),
                      m_pStyle->font_size() );
    return meter.measure_width(m_text);
}

//---------------------------------------------------------------------------------------
LUnits TextBoxEngraver::measure_height()
{
    TextMeter meter(m_libraryScope);
    meter.select_font(m_language,
                      m_pStyle->font_file(),
                      m_pStyle->font_name(),
                      m_pStyle->font_size() );
    return meter.get_font_height();
}

//---------------------------------------------------------------------------------------
GmoShapeTextBox* TextBoxEngraver::create_shape(ImoObj* pCreatorImo, LUnits xLeft, LUnits yTop)
{
    UPoint pos(xLeft, yTop);
    if (pCreatorImo && pCreatorImo->is_contentobj())
    {
        //TODO: This is a temporal fix for dealing with <words> "default-x, default-y"
        ImoContentObj* pObj = static_cast<ImoContentObj*>(pCreatorImo);
        pos.x += tenths_to_logical(pObj->get_user_ref_point_x());
        pos.y += tenths_to_logical(pObj->get_user_ref_point_y());

        add_user_shift(static_cast<ImoContentObj*>(pCreatorImo), &pos);
    }

    //TODO-LOG
    //if (valign == k_center)
    {
        TextMeter meter(m_libraryScope);
        yTop -= meter.get_descender();
    }

    ShapeId idx = 0;
    return LOMSE_NEW GmoShapeTextBox(pCreatorImo, idx, m_text, m_language,
                                     m_pStyle, m_libraryScope, pos,
                                     USize(1000.0f, 1000.0f),     //rectangle size
                                     50.0f            //radius for rounded corners
                                    );
}



//=======================================================================================
// MeasureNumberEngraver implementation
//=======================================================================================
MeasureNumberEngraver::MeasureNumberEngraver(LibraryScope& libraryScope,
                                             ScoreMeter* pScoreMeter, const string& text)
    : Engraver(libraryScope, pScoreMeter)
    , m_text(text)
    , m_pFontStorage( libraryScope.font_storage() )
{
    m_pStyle = m_pMeter->get_style_info("Measure numbers");
}

//---------------------------------------------------------------------------------------
MeasureNumberEngraver::~MeasureNumberEngraver()
{
}

//---------------------------------------------------------------------------------------
LUnits MeasureNumberEngraver::measure_width()
{
    TextMeter meter(m_libraryScope);
    meter.select_font("en",
                      m_pStyle->font_file(),
                      m_pStyle->font_name(),
                      m_pStyle->font_size() );
    return meter.measure_width(m_text);
}

//---------------------------------------------------------------------------------------
LUnits MeasureNumberEngraver::measure_height()
{
    TextMeter meter(m_libraryScope);
    meter.select_font("en",
                      m_pStyle->font_file(),
                      m_pStyle->font_name(),
                      m_pStyle->font_size() );
    return meter.get_font_height();
}

//---------------------------------------------------------------------------------------
GmoShapeText* MeasureNumberEngraver::create_shape(ImoObj* pCreator, ShapeId idx,
                                                  LUnits xLeft, LUnits yTop)
{
    return LOMSE_NEW GmoShapeText(pCreator, idx, m_text, m_pStyle, "en",
                                  TextEngraver::k_class_measure_number,
                                  xLeft, yTop, m_libraryScope);
}


}  //namespace lomse
