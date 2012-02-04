//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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

#include <UnitTest++.h>
#include <sstream>
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_document_layouter.h"
#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_gm_basic.h"
#include "lomse_box_system.h"
#include "lomse_shape_staff.h"
#include "lomse_instrument_engraver.h"
#include "lomse_internal_model.h"
#include "lomse_calligrapher.h"
#include "lomse_score_meter.h"
#include "lomse_internal_model.h"
#include "lomse_box_content_layouter.h"
#include "lomse_im_factory.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
//helper, to access protected members
class MyDocLayouter : public DocLayouter
{
public:
    MyDocLayouter(InternalModel* pIModel, LibraryScope& libraryScope)
        : DocLayouter(pIModel, libraryScope) {}
    ~MyDocLayouter() {}

    void my_layout_content() { layout_content(); }
    GmoBox* my_get_current_box() { return m_pItemMainBox; }
};


//---------------------------------------------------------------------------------------
class DocLayouterTestFixture
{
public:
    LibraryScope m_libraryScope;
    FontStorage* m_pFonts;
    std::string m_scores_path;

    DocLayouterTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
        m_pFonts = m_libraryScope.font_storage();
        m_scores_path = LOMSE_TEST_SCORES_PATH;
    }

    ~DocLayouterTestFixture()
    {
    }
};

//---------------------------------------------------------------------------------------
SUITE(DocLayouterTest)
{

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_ReturnsGModel)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        //pGModel->dump_page(0, cout);
        CHECK( pGModel != NULL );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_CreatesModelAndFirstPage)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        CHECK( pGModel != NULL );
        CHECK( pGModel->get_num_pages() == 1 );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_DocPageHasPaperSize)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage->is_box_doc_page() == true );
        CHECK( pPage->get_width() == 24000.0f );
        CHECK( pPage->get_height() == 35700.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_DocPageHasBoxContent)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBox = pPage->get_child_box(0);
        CHECK( pBox->is_box_doc_page_content() == true );
        CHECK( pBox->get_width() == 16000.0f );
        CHECK( pBox->get_height() == 2735.0f );
        CHECK( pBox->get_left() == 1000.0f );
        CHECK( pBox->get_top() == 1500.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_DocPageHasBoxScore)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
//        pGModel->dump_page(0, cout);
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);     //DocPageContent
        GmoBox* pBox = pBDPC->get_child_box(0);      //ScorePage
        CHECK( pBox != NULL );
        CHECK( pBox->is_box_score_page() == true );
        CHECK( pBox->get_width() == 16000.0f );
        //cout << pBox->get_height() << endl;
        CHECK( pBox->get_height() == 2735.0f );     //system height
        CHECK( pBox->get_left() == 1000.0f );
        CHECK( pBox->get_top() == 1500.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxScoreHasBoxSystem)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
//        pGModel->dump_page(0, cout);
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);     //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);      //ScorePage
        GmoBox* pBox = pBSP->get_child_box(0);       //System
        CHECK( pBox != NULL );
        CHECK( pBox->is_box_system() == true );
        //CHECK( pBox->get_width() == 16000.0f );
        //CHECK( pBox->get_height() == 735.0f );
        //CHECK( pBox->get_left() == 1000.0f );
        //CHECK( pBox->get_top() == 1500.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxSystemHasBoxSlice)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);     //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);      //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);      //System
        GmoBox* pBox = pBSys->get_child_box(0);      //Slice
        CHECK( pBox != NULL );
        CHECK( pBox->is_box_slice() == true );
        //CHECK( pBox->get_width() == 16000.0f );
        //CHECK( pBox->get_height() == 735.0f );
        //CHECK( pBox->get_left() == 1000.0f );
        //CHECK( pBox->get_top() == 1500.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxSliceHasBoxSliceInstr)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);     //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);      //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);      //System
        GmoBox* pBSlice = pBSys->get_child_box(0);   //Slice
        GmoBox* pBox = pBSlice->get_child_box(0);    //SliceInstr
        CHECK( pBox != NULL );
        CHECK( pBox->is_box_slice_instr() == true );
        //CHECK( pBox->get_width() == 16000.0f );
        //CHECK( pBox->get_height() == 735.0f );
        //CHECK( pBox->get_left() == 1000.0f );
        //CHECK( pBox->get_top() == 1500.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxSystemHasStaffShape)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);     //System
        GmoShape* pShape = pBSys->get_shape(0);     //ShapeStaff
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_staff() == true );
        //CHECK( pShape->get_width() == 16000.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxSystemGetStaffShape)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        GmoShape* pShape = pBSys->get_staff_shape(0);
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_staff() == true );
        GmoShapeStaff* pSS = dynamic_cast<GmoShapeStaff*>(pShape);
        CHECK( pSS->get_num_staff() == 0 );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_EmptyScore_OneStaff)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 1.6) "
            "(instrument (musicData )))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        CHECK( pGModel->get_num_pages() == 1 );
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage->get_num_boxes() == 1 );
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        CHECK( pBDPC->get_num_boxes() == 1 );
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        CHECK( pBSP->get_num_boxes() == 1 );
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys->get_num_shapes() == 1 );
        GmoShape* pShape = pBSys->get_staff_shape(0);
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_staff() == true );
        GmoShapeStaff* pSS = dynamic_cast<GmoShapeStaff*>(pShape);
        CHECK( pSS->get_num_staff() == 0 );
        CHECK( pBSys->get_num_boxes() == 0 );

        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxesPositionAndMargins)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G) )) )))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        CHECK( pGModel->get_num_pages() == 1 );
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage->get_num_boxes() == 1 );
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        CHECK( pBDPC->get_num_boxes() == 1 );
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        CHECK( pBSP->get_num_boxes() == 1 );
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys->get_top_margin() == 0.0f );
        CHECK( pBSys->get_bottom_margin() == 0.0f );

        CHECK( pBSys->get_num_boxes() == 1 );
        GmoBox* pBSlice = pBSys->get_child_box(0);     //Slice
        CHECK( pBSlice->get_top_margin() == 0.0f );
        CHECK( pBSlice->get_bottom_margin() == 0.0f );
        CHECK( pBSlice->get_left_margin() == 0.0f );
        CHECK( pBSlice->get_right_margin() == 0.0f );
        CHECK( pBSlice->get_top() == pBSys->get_top() );

        CHECK( pBSlice->get_num_boxes() == 1 );
        GmoBox* pBSliceInstr = pBSlice->get_child_box(0);     //SliceInsr
        CHECK( pBSliceInstr->get_num_boxes() == 0 );
        CHECK( pBSliceInstr->get_top_margin() == 0.0f );
        CHECK( pBSliceInstr->get_bottom_margin() == 0.0f );
        CHECK( pBSliceInstr->get_left_margin() == 0.0f );
        CHECK( pBSliceInstr->get_right_margin() == 0.0f );
        CHECK( pBSliceInstr->get_top() == pBSlice->get_top() );

        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_EmptyScore_StaffTop)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 1.6) "
            "(instrument (musicData )) )))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        CHECK( pGModel->get_num_pages() == 1 );
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage->get_num_boxes() == 1 );
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        CHECK( pBDPC->get_num_boxes() == 1 );
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        CHECK( pBSP->get_num_boxes() == 1 );
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys->get_num_shapes() == 1 );
        GmoShape* pShape = pBSys->get_staff_shape(0);
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_staff() == true );
        GmoShapeStaff* pSS = dynamic_cast<GmoShapeStaff*>(pShape);
        CHECK( pSS->get_num_staff() == 0 );
        CHECK( pSS->get_top() > 0.0f );

        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_EmptyScore_TwoStaves)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 1.6) "
            "(instrument (staves 2)(musicData )))))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        CHECK( pGModel->get_num_pages() == 1 );
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage->get_num_boxes() == 1 );
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        CHECK( pBDPC->get_num_boxes() == 1 );
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        CHECK( pBSP->get_num_boxes() == 1 );
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys->get_num_shapes() == 3 );  // two staves + bracket
        GmoShape* pShape = pBSys->get_staff_shape(0);
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_staff() == true );
        GmoShapeStaff* pSS = dynamic_cast<GmoShapeStaff*>(pShape);
        CHECK( pSS->get_num_staff() == 0 );
        pShape = pBSys->get_staff_shape(1);
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_staff() == true );
        pSS = dynamic_cast<GmoShapeStaff*>(pShape);
        CHECK( pSS->get_num_staff() == 1 );
        CHECK( pBSys->get_num_boxes() == 0 );

        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_EmptyScore_TwoInstruments)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData )) (instrument (musicData )) )))" );
        DocLayouter dl( doc.get_im_model(), m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_gm_model();
        CHECK( pGModel->get_num_pages() == 1 );
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage->get_num_boxes() == 1 );
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        CHECK( pBDPC->get_num_boxes() == 1 );
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        CHECK( pBSP->get_num_boxes() == 1 );
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys->get_num_shapes() == 2 );
        GmoShape* pShape = pBSys->get_staff_shape(0);
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_staff() == true );
        GmoShapeStaff* pSS = dynamic_cast<GmoShapeStaff*>(pShape);
        CHECK( pSS->get_num_staff() == 0 );
        pShape = pBSys->get_staff_shape(1);
        CHECK( pShape != NULL );
        CHECK( pShape->is_shape_staff() == true );
        pSS = dynamic_cast<GmoShapeStaff*>(pShape);
        CHECK( pSS->get_num_staff() == 0 );
        CHECK( pBSys->get_num_boxes() == 0 );

        delete pGModel;
    }

    //TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_HasMainSizer)
    //{
    //    Document doc(m_libraryScope);
    //    doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
    //        "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
    //    MyDocLayouter dl( doc.get_im_model() );
    //    CHECK( dl.get_main_sizer() != NULL );
    //}


};


//---------------------------------------------------------------------------------------
// InstrumentEngraver
//---------------------------------------------------------------------------------------
class InstrEngraverTestFixture
{
public:
    LibraryScope m_libraryScope;

    InstrEngraverTestFixture()     //SetUp fixture
        : m_libraryScope(cout)
    {
    }

    ~InstrEngraverTestFixture()
    {
    }
};

//---------------------------------------------------------------------------------------
SUITE(InstrEngraverTest)
{

    TEST_FIXTURE(InstrEngraverTestFixture, InstrEngraver_MeasureIndents_NoBracket)
    {
        Document doc(m_libraryScope);
        ImoInstrument* pInstr = static_cast<ImoInstrument*>(
                                    ImFactory::inject(k_imo_instrument, &doc));
        ScoreMeter meter(1, 1, 180.0f);
        InstrumentEngraver engraver(m_libraryScope, &meter, pInstr, NULL);
        engraver.measure_indents();
        CHECK( engraver.get_indent_first() == 0.0f );
        CHECK( engraver.get_indent_other() == 0.0f );

        delete pInstr;
    }

    TEST_FIXTURE(InstrEngraverTestFixture, InstrEngraver_MeasureIndents_Bracket)
    {
        Document doc(m_libraryScope);
        ImoInstrument* pInstr = static_cast<ImoInstrument*>(
                                    ImFactory::inject(k_imo_instrument, &doc));
        pInstr->add_staff();
        ScoreMeter meter(1, 1, 180.0f);
        InstrumentEngraver engraver(m_libraryScope, &meter, pInstr, NULL);
        engraver.measure_indents();
        CHECK( engraver.get_indent_first() > 0.0f );
        CHECK( engraver.get_indent_other() > 0.0f );

        delete pInstr;
    }

};
