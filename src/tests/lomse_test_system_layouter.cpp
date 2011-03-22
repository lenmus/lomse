//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#include <UnitTest++.h>
#include <sstream>
#include "lomse_config.h"

//classes related to these tests
#include "lomse_system_layouter.h"
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class SystemLayouterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    SystemLayouterTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~SystemLayouterTestFixture()  // tearDown()
    {
    }

};


SUITE(SystemLayouterTest)
{

    //TEST_FIXTURE(SystemLayouterTestFixture, SystemLayouter_Create)
    //{
    //    SystemLayouter sys(0.66f, ESpacingMethod::k_spacing_proportional, 30.0f);

    //    CHECK( sys.. != NULL );
    //}

};



//---------------------------------------------------------------------------------------
// LinesBuilder
//---------------------------------------------------------------------------------------
class MyLinesBuilder : public LinesBuilder      //To have access to private members
{
public:
    MyLinesBuilder(ColumnStorage* pStorage) : LinesBuilder(pStorage) {}

        //access to protected methods / variables

    inline bool ut_is_there_current_line() { return is_there_current_line(); }
    inline LUnits ut_iinitial_space() { return m_uInitialSpace; }
};


class LinesBuilderTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    LinesBuilderTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~LinesBuilderTestFixture()  // tearDown()
    {
    }

};


SUITE(LinesBuilderTest)
{

    TEST_FIXTURE(LinesBuilderTestFixture, LinesBuilder_IncludeObjectCreatesLine)
    {
        ColumnStorage storage;
        MyLinesBuilder builder(&storage);
        builder.set_start_position(30.0f);
        builder.set_initial_space(60.0f);
        CHECK( builder.ut_is_there_current_line() == false );
        CHECK( builder.ut_iinitial_space() == 60.0f );
        ImoInstrument instr;
        ImoMusicData* pMD = new ImoMusicData();
        instr.append_child(pMD);
        ImoClef* pClef = new ImoClef(ImoClef::k_G2);
        pMD->append_child(pClef);
        GmoShape* pShape = NULL;
        int iStaff = 0;
        int iLine = 0;
        int iInstr = 0;
        float rTime = 0.0f;

        builder.include_object(iLine, iInstr, &instr, pClef, rTime, iStaff, pShape);

        CHECK( builder.ut_is_there_current_line() == true );
    }

};




////---------------------------------------------------------------------------------------
//// LinesBuilder
////---------------------------------------------------------------------------------------
//class LinesBuilderTestFixture
//{
//public:
//    LibraryScope m_libraryScope;
//    std::string m_scores_path;
//
//    LinesBuilderTestFixture()   // setUp()
//        : m_libraryScope(cout)
//        , m_scores_path(LOMSE_TEST_SCORES_PATH)
//    {
//    }
//
//    ~LinesBuilderTestFixture()  // tearDown()
//    {
//    }
//
//};
//
//
//SUITE(LinesBuilderTest)
//{
//
//    //TEST_FIXTURE(LinesBuilderTestFixture, LinesBuilder_Create)
//    //{
//    //    ColumnStorage storage;
//    //    LinesBuilder builder(&storage);
//
//    //    CHECK( builder.is_there_current_line() == false );
//    //}
//
//};
//
