//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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
#include "private/lomse_document_p.h"
#include "lomse_graphical_model.h"
#include "lomse_gm_basic.h"
#include "lomse_box_system.h"
#include "lomse_shape_staff.h"
#include "lomse_instrument_engraver.h"
#include "lomse_internal_model.h"
#include "lomse_calligrapher.h"
#include "lomse_score_meter.h"
#include "lomse_internal_model.h"
#include "lomse_inlines_container_layouter.h"
#include "lomse_im_factory.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
//helper, to access protected members
class MyDocLayouter : public DocLayouter
{
public:
    MyDocLayouter(Document* pDoc, LibraryScope& libraryScope)
        : DocLayouter(pDoc, libraryScope) {}
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
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
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
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        //pGModel->dump_page(0, cout);
        CHECK( pGModel != nullptr );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_CreatesModelAndFirstPage)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        CHECK( pGModel != nullptr );
        CHECK( pGModel && pGModel->get_num_pages() == 1 );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_DocPageHasPaperSize)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage && pPage->is_box_doc_page() == true );
        CHECK( pPage && pPage->get_width() == 24000.0f );
        CHECK( pPage && pPage->get_height() == 35700.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_DocPageHasBoxContent)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBox = pPage->get_child_box(0);
        CHECK( pBox && pBox->is_box_doc_page_content() == true );
        CHECK( pBox && pBox->get_width() == 16000.0f );
        CHECK( pBox && pBox->get_height() == 2735.0f );
        CHECK( pBox && pBox->get_left() == 1000.0f );
        CHECK( pBox && pBox->get_top() == 1500.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_DocPageHasBoxScore)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
//        pGModel->dump_page(0, cout);
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);     //DocPageContent
        GmoBox* pBox = pBDPC->get_child_box(0);      //ScorePage
        CHECK( pBox != nullptr );
        CHECK( pBox && pBox->is_box_score_page() == true );
        CHECK( pBox && pBox->get_width() == 16000.0f );
        //cout << pBox->get_height() << endl;
        CHECK( pBox && pBox->get_height() == 2735.0f );     //system height
        CHECK( pBox && pBox->get_left() == 1000.0f );
        CHECK( pBox && pBox->get_top() == 1500.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxScoreHasBoxSystem)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
//        pGModel->dump_page(0, cout);
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);     //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);      //ScorePage
        GmoBox* pBox = pBSP->get_child_box(0);       //System
        CHECK( pBox != nullptr );
        CHECK( pBox && pBox->is_box_system() == true );
        //CHECK( pBox && pBox->get_width() == 16000.0f );
        //CHECK( pBox && pBox->get_height() == 735.0f );
        //CHECK( pBox && pBox->get_left() == 1000.0f );
        //CHECK( pBox && pBox->get_top() == 1500.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxSystemHasBoxSlice)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);     //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);      //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);      //System
        GmoBox* pBox = pBSys->get_child_box(0);      //Slice
        CHECK( pBox != nullptr );
        CHECK( pBox && pBox->is_box_slice() == true );
        //CHECK( pBox && pBox->get_width() == 16000.0f );
        //CHECK( pBox && pBox->get_height() == 735.0f );
        //CHECK( pBox && pBox->get_left() == 1000.0f );
        //CHECK( pBox && pBox->get_top() == 1500.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxSliceHasBoxSliceInstr)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);     //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);      //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);      //System
        GmoBox* pBSlice = pBSys->get_child_box(0);   //Slice
        GmoBox* pBox = pBSlice->get_child_box(0);    //SliceInstr
        CHECK( pBox != nullptr );
        CHECK( pBox && pBox->is_box_slice_instr() == true );
        //CHECK( pBox && pBox->get_width() == 16000.0f );
        //CHECK( pBox && pBox->get_height() == 735.0f );
        //CHECK( pBox && pBox->get_left() == 1000.0f );
        //CHECK( pBox && pBox->get_top() == 1500.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxSystemHasStaffShape)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        GmoBox* pBSys = pBSP->get_child_box(0);     //System
        int iMax = pBSys->get_num_shapes();
        GmoShape* pShape = nullptr;
        bool fStaffShapeFound = false;
        for (int i=0; i < iMax; ++i)
        {
            pShape = pBSys->get_shape(i);
            if (pShape && pShape->is_shape_staff())
            {
                fStaffShapeFound = true;
                break;
            }
        }
        CHECK( fStaffShapeFound == true );
        //CHECK( pShape && pShape->get_width() == 16000.0f );
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxSystemGetStaffShape)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(pageLayout (pageSize 24000 35700)(pageMargins 1000 1500 3000 2500 4000) landscape) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G)(key e)(n c4 q)(r q)(barline simple))))))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys != nullptr );
        if (pBSys)
        {
            GmoShape* pShape = pBSys->get_staff_shape(0);
            CHECK( pShape != nullptr );
            CHECK( pShape && pShape->is_shape_staff() == true );
            GmoShapeStaff* pSS = dynamic_cast<GmoShapeStaff*>(pShape);
            CHECK( pSS && pSS->get_num_staff() == 0 );
        }
        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_EmptyScore_OneStaff)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 1.6) "
            "(instrument (musicData)))))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        CHECK( pGModel && pGModel->get_num_pages() == 1 );
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage && pPage->get_num_boxes() == 1 );
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        CHECK( pBDPC && pBDPC->get_num_boxes() == 1 );
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        CHECK( pBSP->get_num_boxes() == 1 );
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys != nullptr );
        if (pBSys)
        {
            CHECK( pBSys->get_num_shapes() == 1 );
            GmoShape* pShape = pBSys->get_staff_shape(0);
            CHECK( pShape != nullptr );
            CHECK( pShape && pShape->is_shape_staff() == true );
            GmoShapeStaff* pSS = dynamic_cast<GmoShapeStaff*>(pShape);
            CHECK( pSS && pSS->get_num_staff() == 0 );
            CHECK( pBSys->get_num_boxes() == 0 );
        }

        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_BoxesPositionAndMargins)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 1.6) "
            "(instrument (musicData (clef G) )) )))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        CHECK( pGModel && pGModel->get_num_pages() == 1 );
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage && pPage->get_num_boxes() == 1 );
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        CHECK( pBDPC && pBDPC->get_num_boxes() == 1 );
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        CHECK( pBSP->get_num_boxes() == 1 );
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys != nullptr );
        if (pBSys)
        {
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
            CHECK( pBSliceInstr->get_num_boxes() == 1 );
            CHECK( pBSliceInstr->get_top_margin() == 0.0f );
            CHECK( pBSliceInstr->get_bottom_margin() == 0.0f );
            CHECK( pBSliceInstr->get_left_margin() == 0.0f );
            CHECK( pBSliceInstr->get_right_margin() == 0.0f );
            CHECK( pBSliceInstr->get_top() == pBSlice->get_top() );
        }

        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_EmptyScore_StaffTop)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 1.6) "
            "(instrument (musicData)) )))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        CHECK( pGModel && pGModel->get_num_pages() == 1 );
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage && pPage->get_num_boxes() == 1 );
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        CHECK( pBDPC && pBDPC->get_num_boxes() == 1 );
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        CHECK( pBSP->get_num_boxes() == 1 );
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys != nullptr );
        if (pBSys)
        {
            CHECK( pBSys->get_num_shapes() == 1 );
            GmoShape* pShape = pBSys->get_staff_shape(0);
            CHECK( pShape != nullptr );
            CHECK( pShape && pShape->is_shape_staff() == true );
            GmoShapeStaff* pSS = dynamic_cast<GmoShapeStaff*>(pShape);
            CHECK( pSS && pSS->get_num_staff() == 0 );
            CHECK( pSS && pSS->get_top() > 0.0f );
        }

        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_EmptyScore_TwoStaves)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 1.6) "
            "(instrument (staves 2)(musicData)))))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        CHECK( pGModel && pGModel->get_num_pages() == 1 );
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage && pPage->get_num_boxes() == 1 );
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        CHECK( pBDPC && pBDPC->get_num_boxes() == 1 );
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        CHECK( pBSP->get_num_boxes() == 1 );
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys != nullptr );
        if (pBSys)
        {
            CHECK( pBSys->get_num_shapes() == 4 );  // two staves + bracket + left barline
            GmoShape* pShape = pBSys->get_staff_shape(0);
            CHECK( pShape != nullptr );
            CHECK( pShape && pShape->is_shape_staff() == true );
            GmoShapeStaff* pSS = dynamic_cast<GmoShapeStaff*>(pShape);
            CHECK( pSS && pSS->get_num_staff() == 0 );
            pShape = pBSys->get_staff_shape(1);
            CHECK( pShape != nullptr );
            CHECK( pShape && pShape->is_shape_staff() == true );
            pSS = dynamic_cast<GmoShapeStaff*>(pShape);
            CHECK( pSS && pSS->get_num_staff() == 1 );
            CHECK( pBSys->get_num_boxes() == 0 );
        }

        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_EmptyScore_TwoInstruments)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) (content (score (vers 1.6) "
            "(instrument (musicData)) (instrument (musicData)) )))" );
        DocLayouter dl(&doc, m_libraryScope);
        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        CHECK( pGModel && pGModel->get_num_pages() == 1 );
        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage && pPage->get_num_boxes() == 1 );
        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        CHECK( pBDPC && pBDPC->get_num_boxes() == 1 );
        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        CHECK( pBSP->get_num_boxes() == 1 );
        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys != nullptr );
        if (pBSys)
        {
            CHECK(  pBSys->get_num_shapes() == 3 );
            GmoShape* pShape = pBSys->get_staff_shape(0);
            CHECK( pShape != nullptr );
            CHECK( pShape && pShape->is_shape_staff() == true );
            GmoShapeStaff* pSS = dynamic_cast<GmoShapeStaff*>(pShape);
            CHECK( pSS && pSS->get_num_staff() == 0 );
            pShape = pBSys->get_staff_shape(1);
            CHECK( pShape != nullptr );
            CHECK( pShape && pShape->is_shape_staff() == true );
            pSS = dynamic_cast<GmoShapeStaff*>(pShape);
            CHECK( pSS && pSS->get_num_staff() == 0 );
            CHECK( pBSys->get_num_boxes() == 0 );
        }

        delete pGModel;
    }

    TEST_FIXTURE(DocLayouterTestFixture, DocLayouter_fix_infinite_width)
    {
        Document doc(m_libraryScope);
        doc.from_string("(lenmusdoc (vers 0.0) "
            "(content (score (vers 2.0) "
            "(instrument (musicData (clef G) )) )))" );
        DocLayouter dl(&doc, m_libraryScope, k_infinite_width);

        dl.layout_document();
        GraphicModel* pGModel = dl.get_graphic_model();
        CHECK( pGModel && pGModel->get_num_pages() == 1 );

        GmoBoxDocPage* pPage = pGModel->get_page(0);
        CHECK( pPage && pPage->get_size().width != LOMSE_INFINITE_LENGTH );
        CHECK( pPage && pPage->get_num_boxes() == 1 );

        GmoBox* pBDPC = pPage->get_child_box(0);    //DocPageContent
        Tenths height = pBDPC->get_size().height;
        CHECK( pBDPC && pBDPC->get_num_boxes() == 1 );

        GmoBox* pBSP = pBDPC->get_child_box(0);     //ScorePage
        CHECK( pBSP->get_size().height == height );
        CHECK( pBSP->get_num_boxes() == 1 );

        GmoBoxSystem* pBSys = dynamic_cast<GmoBoxSystem*>( pBSP->get_child_box(0) );
        CHECK( pBSys && pBSys->get_size().height == height );

        delete pGModel;
    }


};
