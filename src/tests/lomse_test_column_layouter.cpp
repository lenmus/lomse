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
#include "lomse_build_options.h"

//classes related to these tests
#include "lomse_system_layouter.h"
#include "lomse_injectors.h"
#include "lomse_gm_basic.h"
#include "lomse_internal_model.h"
#include "lomse_score_layouter.h"
#include "lomse_score_meter.h"
#include "lomse_document.h"
#include "lomse_im_factory.h"

#include <cmath>

using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
// helper, mock class
class MyMusicLine : public MusicLine
{
private:

public:
    MyMusicLine(LUnits xStart, LUnits fixedSpace, LUnits xs, LUnits xa,
                LUnits xr, LUnits xv, LUnits xf, bool fEndsInBarline=false)
         : MusicLine(0, 0, xStart, fixedSpace)
    {
        float rTime = 0.0f;
        LUnits xAnchor = xs - xa;       //0.0f; + xStart+fixedSpace
        LUnits xLeft = xa + xAnchor;
        LUnits uSize = xr - xLeft;
        LUnits uFixedSpace = xv - xr;;
        LUnits uVarSpace = xf - xv;
        LUnits xFinal = xf;
        my_add_entry(fEndsInBarline, false, rTime, xAnchor, xLeft,
                     uSize, uFixedSpace, uVarSpace, xFinal);
        m_barlineType = (fEndsInBarline ? k_visible_barline : k_no_barline);

        m_uxRightEdge = xr;
        m_uxStartOfEndVarSpace = xv;

    }
    virtual ~MyMusicLine() {}

    //inline void my_do_measurements() { do_measurements(); }

    void my_add_entry(bool fIsBarline, bool fProlog, float rTime, LUnits xAnchor,
                      LUnits xLeft, LUnits uSize, LUnits uFixedSpace,
                      LUnits uVarSpace, LUnits xFinal)
    {
        LineEntry* pEntry = LOMSE_NEW LineEntry(fIsBarline, fProlog, rTime, xAnchor, xLeft,
                                          uSize, uFixedSpace, uVarSpace, xFinal);
        push_back(pEntry);
    }

};

//---------------------------------------------------------------------------------------
// helper, mock class
class MyColumnStorage : public ColumnStorage
{
private:

public:
    MyColumnStorage(LUnits xStart, LUnits fixedSpace)
        : ColumnStorage()
    {
        set_start_position(xStart);
        set_initial_space(fixedSpace);
    }
    virtual ~MyColumnStorage() {}

    inline bool my_are_there_lines() { return get_last_line() != end(); }

    void my_add_line(LUnits xs, LUnits xa, LUnits xr, LUnits xv, LUnits xf,
                     bool fEndsInBarline=false)
    {
        MyMusicLine* pLine = LOMSE_NEW MyMusicLine(m_uxStart, m_uStartFixedSpace,
                                             xs, xa, xr, xv, xf, fEndsInBarline);
        m_Lines.push_back(pLine);
    }

    void my_add_line(MusicLine* pLine)
    {
        m_Lines.push_back(pLine);
    }

};


//---------------------------------------------------------------------------------------
class ColumnStorageTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    ColumnStorageTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~ColumnStorageTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }
};


SUITE(ColumnStorageTest)
{

    TEST_FIXTURE(ColumnStorageTestFixture, IncludeObjectCreatesLine)
    {
        Document doc(m_libraryScope);
        MyColumnStorage storage(30.0f, 60.0f);
        CHECK( storage.my_are_there_lines() == false );
        ImoClef* pClef = static_cast<ImoClef*>(ImFactory::inject(k_imo_clef, &doc));
        pClef->set_clef_type(k_clef_G2);
        GmoShape* pShape = NULL;
        int iStaff = 0;
        int iLine = 0;
        int iInstr = 0;
        float rTime = 0.0f;

        storage.include_object(iLine, iInstr, pClef, rTime, iStaff, pShape, true);

        CHECK( storage.get_fixed_space_at_start() == 60.0f );
        CHECK( storage.my_are_there_lines() == true );

        delete pClef;
    }

    TEST_FIXTURE(ColumnStorageTestFixture, FixedSpaceForNext)
    {
        Document doc(m_libraryScope);
        MyColumnStorage storage(30.0f, 60.0f);
        ImoBarline* pBar = static_cast<ImoBarline*>(
                                ImFactory::inject(k_imo_barline, &doc) );
        pBar->set_type(ImoBarline::k_end);
        GmoShape* pShape = NULL;
        int iStaff = 0;
        int iLine = 0;
        int iInstr = 0;
        float rTime = 64.0f;
        storage.include_object(iLine, iInstr, pBar, rTime, iStaff, pShape, false);

        storage.determine_sizes();

        //cout << "var at end = " << storage.get_end_hook_width() << endl;
        //cout << "fixed for next = " << storage.get_fixed_space_for_next_column() << endl;
        CHECK( is_equal(storage.get_end_hook_width(), 0.0f) );
        CHECK( is_equal(storage.get_fixed_space_for_next_column(), 0.0f) );

        delete pBar;
    }

    TEST_FIXTURE(ColumnStorageTestFixture, Measurements_NoBarline)
    {
        MyColumnStorage storage(30.0f, 60.0f);
        //                  xs       xa       xr       xv       xf       Barline
        storage.my_add_line(  90.0f,   90.0f, 2100.0f, 2300.0f, 2400.0f, false);
        storage.my_add_line(  90.0f,   90.0f, 2000.0f, 2200.0f, 2300.0f, false);
        storage.my_add_line(  90.0f,   90.0f, 2050.0f, 2080.0f, 2080.0f, false);

        storage.determine_sizes();

        //cout << "X0 = " << storage.get_start_of_column() << endl;
        //cout << "XS = " << storage.get_start_of_first_symbol() << endl;
        //cout << "XA = " << storage.get_first_anchor_position() << endl;
        //cout << "XR = " << storage.get_right_most_edge() << endl;
        //cout << "XV = " << storage.get_start_of_final_var_space() << endl;
        //cout << "XF = " << storage.get_end_of_column() << endl;
        //cout << "main = " << storage.get_main_width() << endl;
        //cout << "StartHook width = " << storage.get_start_hook_width() << endl;
        //cout << "End Hook width = " << storage.get_end_hook_width() << endl;

        CHECK( is_equal(storage.get_start_of_column(), 30.0f) );              //X0
        CHECK( is_equal(storage.get_start_of_first_symbol(), 90.0f) );        //XS
        CHECK( is_equal(storage.get_first_anchor_position(), 90.0f) );        //XA
        CHECK( is_equal(storage.get_right_most_edge(), 2100.0f) );            //XR
        CHECK( is_equal(storage.get_start_of_final_var_space(), 2300.0f) );   //XV
        CHECK( is_equal(storage.get_end_of_column(), 2400.0f) );              //XF
        CHECK( is_equal(storage.get_main_width(), 2270.0f) );                 //main width
        CHECK( is_equal(storage.get_start_hook_width(), 0.0f) );
        CHECK( is_equal(storage.get_end_hook_width(), 100.0f) );
    }

    TEST_FIXTURE(ColumnStorageTestFixture, Measurements_Barline)
    {
        MyColumnStorage storage(30.0f, 60.0f);
        //                  xs       xa       xr       xv       xf       Barline
        storage.my_add_line(  90.0f,   90.0f, 2100.0f, 2300.0f, 2400.0f, true);
        storage.my_add_line(  90.0f,   90.0f, 2000.0f, 2200.0f, 2300.0f, false);
        storage.my_add_line(  90.0f,   90.0f, 2050.0f, 2080.0f, 2080.0f, false);

        storage.determine_sizes();

        //cout << "X0 = " << storage.get_start_of_column() << endl;
        //cout << "XS = " << storage.get_start_of_first_symbol() << endl;
        //cout << "XA = " << storage.get_first_anchor_position() << endl;
        //cout << "XR = " << storage.get_right_most_edge() << endl;
        //cout << "XV = " << storage.get_start_of_final_var_space() << endl;
        //cout << "XF = " << storage.get_end_of_column() << endl;
        //cout << "main = " << storage.get_main_width() << endl;
        //cout << "StartHook width = " << storage.get_start_hook_width() << endl;
        //cout << "End Hook width = " << storage.get_end_hook_width() << endl;

        CHECK( is_equal(storage.get_start_of_column(), 30.0f) );              //X0
        CHECK( is_equal(storage.get_start_of_first_symbol(), 90.0f) );        //XS
        CHECK( is_equal(storage.get_first_anchor_position(), 90.0f) );        //XA
        CHECK( is_equal(storage.get_right_most_edge(), 2100.0f) );            //XR
        CHECK( is_equal(storage.get_start_of_final_var_space(), 2300.0f) );   //XV
        CHECK( is_equal(storage.get_end_of_column(), 2400.0f) );              //XF
        CHECK( is_equal(storage.get_main_width(), 2070.0f) );                 //main width
        CHECK( is_equal(storage.get_start_hook_width(), 0.0f) );
        CHECK( is_equal(storage.get_end_hook_width(), 100.0f) );
    }

    TEST_FIXTURE(ColumnStorageTestFixture, Measurements_NegativeAnchor)
    {
        MyColumnStorage storage(0.0f, 0.0f);
        //                  xs       xa       xr       xv       xf       Barline
        storage.my_add_line(2100.0f, 2100.0f, 2100.0f, 2300.0f, 2400.0f, false);
        storage.my_add_line(-500.0f, 2100.0f, 2000.0f, 2200.0f, 2300.0f, false);

        storage.determine_sizes();

        //cout << "X0 = " << storage.get_start_of_column() << endl;
        //cout << "XS = " << storage.get_start_of_first_symbol() << endl;
        //cout << "XA = " << storage.get_first_anchor_position() << endl;
        //cout << "XR = " << storage.get_right_most_edge() << endl;
        //cout << "XV = " << storage.get_start_of_final_var_space() << endl;
        //cout << "XF = " << storage.get_end_of_column() << endl;
        //cout << "main = " << storage.get_main_width() << endl;
        //cout << "StartHook width = " << storage.get_start_hook_width() << endl;
        //cout << "End Hook width = " << storage.get_end_hook_width() << endl;

        CHECK( is_equal(storage.get_start_of_column(), 0.0f) );               //X0
        CHECK( is_equal(storage.get_start_of_first_symbol(), -500.0f) );      //XS
        CHECK( is_equal(storage.get_first_anchor_position(), 2100.0f) );      //XA
        CHECK( is_equal(storage.get_right_most_edge(), 2100.0f) );            //XR
        CHECK( is_equal(storage.get_start_of_final_var_space(), 2300.0f) );   //XV
        CHECK( is_equal(storage.get_end_of_column(), 2400.0f) );              //XF
        CHECK( is_equal(storage.get_main_width(), 2800.0f) );                 //main width
        CHECK( is_equal(storage.get_start_hook_width(), 2600.0f) );
        CHECK( is_equal(storage.get_end_hook_width(), 100.0f) );
    }

    TEST_FIXTURE(ColumnStorageTestFixture, Measurements_NegativeAnchor2)
    {
        MyColumnStorage storage(0.0f, 0.0f);
        //                  xs       xa       xr       xv       xf       Barline
        storage.my_add_line(0.0f,    0.0f,    2100.0f, 2300.0f, 2400.0f, false);
        storage.my_add_line(-500.0f, 0.0f,    2000.0f, 2200.0f, 2300.0f, false);

        storage.determine_sizes();

        //cout << "X0 = " << storage.get_start_of_column() << endl;
        //cout << "XS = " << storage.get_start_of_first_symbol() << endl;
        //cout << "XA = " << storage.get_first_anchor_position() << endl;
        //cout << "XR = " << storage.get_right_most_edge() << endl;
        //cout << "XV = " << storage.get_start_of_final_var_space() << endl;
        //cout << "XF = " << storage.get_end_of_column() << endl;
        //cout << "main = " << storage.get_main_width() << endl;
        //cout << "StartHook width = " << storage.get_start_hook_width() << endl;
        //cout << "End Hook width = " << storage.get_end_hook_width() << endl;

        CHECK( is_equal(storage.get_start_of_column(), 0.0f) );               //X0
        CHECK( is_equal(storage.get_start_of_first_symbol(), -500.0f) );      //XS
        CHECK( is_equal(storage.get_first_anchor_position(), 0.0f) );         //XA
        CHECK( is_equal(storage.get_right_most_edge(), 2100.0f) );            //XR
        CHECK( is_equal(storage.get_start_of_final_var_space(), 2300.0f) );   //XV
        CHECK( is_equal(storage.get_end_of_column(), 2400.0f) );              //XF
        CHECK( is_equal(storage.get_main_width(), 2800.0f) );                 //main width
        CHECK( is_equal(storage.get_start_hook_width(), 500.0f) );
        CHECK( is_equal(storage.get_end_hook_width(), 100.0f) );
    }

};



//=======================================================================================
// MusicLine tests
//=======================================================================================

//---------------------------------------------------------------------------------------
// helper, mock class
class MyMusicLine2 : public MusicLine
{
private:

public:
    MyMusicLine2(int line, int nInstr, LUnits uxStart, LUnits fixedSpace)
         : MusicLine(line, nInstr, uxStart, fixedSpace)
    {
    }
    virtual ~MyMusicLine2() {}

    inline void my_do_measurements() { do_measurements(); }

    void my_add_entry(bool fIsBarline, bool fProlog, float rTime, LUnits xAnchor,
                      LUnits xLeft, LUnits uSize, LUnits uFixedSpace,
                      LUnits uVarSpace, LUnits xFinal)
    {
        LineEntry* pEntry = LOMSE_NEW LineEntry(fIsBarline, fProlog, rTime, xAnchor, xLeft,
                                          uSize, uFixedSpace, uVarSpace, xFinal);
        push_back(pEntry);
    }

};


//---------------------------------------------------------------------------------------
class MusicLineTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    MusicLineTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~MusicLineTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }
};


SUITE(MusicLineTest)
{

    TEST_FIXTURE(MusicLineTestFixture, Measurements_OnlyProlog)
    {
        MyMusicLine2 line(0, 0, 10.0f, 135.0f);
        //              Barline, Prolog, rTime, xAnchor, xLeft,     uSize,      FixedSp,    VarSp,      xFinal
        line.my_add_entry(false, true,   -1.0f,  0.0f,   135.0f,    200.0f,     50.0f,      150.0f,     535.0f);

        line.my_do_measurements();

        CHECK( is_equal(line.get_line_start_position(), 10.0f) );
        CHECK( is_equal(line.get_fixed_space_at_start(), 135.0f) );
        CHECK( is_equal(line.get_start_of_first_symbol(), 135.0f) );
        CHECK( is_equal(line.get_first_anchor_position(), 135.0f) );
        CHECK( is_equal(line.get_right_most_edge(), 335.0f) );
        CHECK( is_equal(line.get_start_of_final_var_space(), 385.0f) );
        CHECK( is_equal(line.get_final_pos(), 535.0f) );
    }

    TEST_FIXTURE(MusicLineTestFixture, Measurements_SimpleTimed)
    {
        MyMusicLine2 line(0, 0, 10.0f, 135.0f);
        //              Barline, Prolog, rTime, xAnchor, xLeft,     uSize,      FixedSp,    VarSp,      xFinal
        line.my_add_entry(false, true,   0.0f,  0.0f,   135.0f,    220.0f,     50.0f,      130.0f,     535.0f);
        line.my_add_entry(false, true,   0.0f,  0.0f,   135.0f,    200.0f,     50.0f,      150.0f,     535.0f);

        line.my_do_measurements();

        CHECK( is_equal(line.get_line_start_position(), 10.0f) );
        CHECK( is_equal(line.get_fixed_space_at_start(), 135.0f) );
        CHECK( is_equal(line.get_start_of_first_symbol(), 135.0f) );
        CHECK( is_equal(line.get_first_anchor_position(), 135.0f) );
        CHECK( is_equal(line.get_right_most_edge(), 355.0f) );
        CHECK( is_equal(line.get_start_of_final_var_space(), 405.0f) );
        CHECK( is_equal(line.get_final_pos(), 535.0f) );
    }

    TEST_FIXTURE(MusicLineTestFixture, Measurements_NegativeStart)
    {
        MyMusicLine2 line(0, 0, 0.0f, 0.0f);
        //              Barline, Prolog,   rTime,  xAnchor,   xLeft,     uSize,  FixedSp,     VarSp,     xFinal
        line.my_add_entry(false, false,  256.00f, -220.00f,   0.00f,   480.00f,   40.00f,   700.00f,   1220.00f);

        line.my_do_measurements();

        //cout << "line start = " << line.get_line_start_position() << endl;
        //cout << "fixed space at start = " << line.get_fixed_space_at_start() << endl;
        //cout << "start of first symbol = " << line.get_start_of_first_symbol() << endl;
        //cout << "first anchor pos = " << line.get_first_anchor_position() << endl;
        //cout << "right most edge = " << line.get_right_most_edge() << endl;
        //cout << "start of final var space = " << line.get_start_of_final_var_space() << endl;
        //cout << "final pos = " << line.get_final_pos() << endl;

        CHECK( is_equal(line.get_line_start_position(), 0.0f) );
        CHECK( is_equal(line.get_fixed_space_at_start(), 0.0f) );
        CHECK( is_equal(line.get_start_of_first_symbol(), 0.0f) );
        CHECK( is_equal(line.get_first_anchor_position(), 220.0f) );
        CHECK( is_equal(line.get_right_most_edge(), 480.0f) );
        CHECK( is_equal(line.get_start_of_final_var_space(), 520.0f) );
        CHECK( is_equal(line.get_final_pos(), 1220.0f) );
    }

    TEST_FIXTURE(MusicLineTestFixture, Measurements_00031)
    {
        MyMusicLine2 line(0, 0, 10.0f, 135.0f);
        //              Barline, Prolog, rTime, xAnchor, xLeft,     uSize,      FixedSp,    VarSp,      xFinal
        line.my_add_entry(false, true,  -1.0f,   0.0f,  135.0f,    450.0f,    180.0f,     630.00f,    765.00f);
        line.my_add_entry(false, true,  -1.0f,   0.0f,  765.0f,      0.0f,    180.0f,     180.00f,    945.00f);
        line.my_add_entry(false, false,  0.0f,   0.0f, 1215.0f,    232.0f,     45.0f,     860.14f,   2075.14f);
        line.my_add_entry(false, false,  0.0f, 210.4f, 1425.4f,    232.0f,     45.0f,     649.74f,   2075.14f);
        line.my_add_entry(false, false,  0.0f,   0.0f, 1215.0f,    232.0f,     45.0f,     860.14f,   2075.14f);

        line.my_do_measurements();

        //cout << "line start = " << line.get_line_start_position() << endl;
        //cout << "fixed space at start = " << line.get_fixed_space_at_start() << endl;
        //cout << "start of first symbol = " << line.get_start_of_first_symbol() << endl;
        //cout << "first anchor pos = " << line.get_first_anchor_position() << endl;
        //cout << "right most edge = " << line.get_right_most_edge() << endl;
        //cout << "start of final var space = " << line.get_start_of_final_var_space() << endl;
        //cout << "final pos = " << line.get_final_pos() << endl;

        CHECK( is_equal(line.get_line_start_position(), 10.0f) );
        CHECK( is_equal(line.get_fixed_space_at_start(), 135.0f) );
        CHECK( is_equal(line.get_start_of_first_symbol(), 1215.0f) );
        CHECK( is_equal(line.get_first_anchor_position(), 1215.0f) );
        CHECK( is_equal(line.get_right_most_edge(), 1657.4f) );
        CHECK( is_equal(line.get_start_of_final_var_space(), 1702.4f) );
        CHECK( is_equal(line.get_final_pos(), 2075.14f) );
        CHECK( is_equal(line.get_line_start_position(), 10.0f) );
    }

};



//=======================================================================================
// LineSpacer tests
//=======================================================================================

//helper, to access protected members
class MyLineSpacer : public LineSpacer
{
public:
    MyLineSpacer(MusicLine* pLine, ScoreMeter* pMeter)
        : LineSpacer(pLine, pMeter)
    {
    }
    virtual ~MyLineSpacer() {};

    //overrides
    void assign_fixed_and_variable_space(LineEntry* pEntry) { pEntry->update_x_final(); }

    //accessors
    void my_prepare_for_traversing() { prepare_for_traversing(); }
    float my_get_current_time() { return m_rCurTime; }
    LUnits my_get_current_x_pos() { return m_uxCurPos; }
    LUnits my_get_romvable_space() { return m_uxRemovable; }
    void my_process_non_timed_at_prolog(LUnits uSpace) { process_non_timed_at_prolog(uSpace); }
    void my_process_non_timed_at_current_timepos(LUnits uxPos) { process_non_timed_at_current_timepos(uxPos); }
    void my_process_timed_at_current_timepos(LUnits uxPos) { process_timed_at_current_timepos(uxPos); }
    float my_get_next_available_time() { return get_next_available_time(); }
    LUnits my_get_next_position() { return get_next_position(); }

};

//---------------------------------------------------------------------------------------
class LineSpacerTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    LineSpacerTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~LineSpacerTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }
};


SUITE(LineSpacerTest)
{
//******************* Before spacing
//Line table dump. Instr=0, voice=1, xStart=0.00, FixedSpace=0.00
//=============================================================================================================
//
//item    Type      ID Prolog   Timepos  xAnchor    xLeft     size  SpFixed    SpVar    Space   xFinal ShpIdx
//-------------------------------------------------------------------------------------------------------------
//   0:   pSO   45  13   S        -1.00     0.00     0.00   526.00     0.00     0.00   526.00     0.00    --
//   1:   pSO   48  14           512.00     0.00     0.00   366.00     0.00     0.00   366.00     0.00    --
//=============================================================================================================
//
//******************* After spacing
//Line table dump. Instr=0, voice=1, xStart=0.00, FixedSpace=0.00
//=============================================================================================================
//
//item    Type      ID Prolog   Timepos  xAnchor    xLeft     size  SpFixed    SpVar    Space   xFinal ShpIdx
//-------------------------------------------------------------------------------------------------------------
//   0:   pSO   45  13   S        -1.00     0.00     0.00   526.00   180.00     0.00   706.00   706.00    --
//   1:   pSO   48  14           512.00     0.00   976.00   366.00    45.00   489.00   900.00  1876.00    --
//=============================================================================================================

    // clef (not in prolog) before note

    TEST_FIXTURE(LineSpacerTestFixture, PrepareForTraversing)
    {
        MyMusicLine2 line(0, 0, 10.0f, 135.0f);
        //              Barline, Prolog,   rTime,  xAnchor, xLeft,   uSize,      FixedSp,    VarSp,      xFinal
        line.my_add_entry(false, false,   -1.00f,  0.00f,   0.00f,   526.00f,    0.00f,     0.00f,      0.00f);
        line.my_add_entry(false, false,  512.00f,  0.00f,   0.00f,   366.00f,    0.00f,     0.00f,      0.00f);

        ScoreMeter meter(1, 1, 150.0f);
        MyLineSpacer spacer(&line, &meter);

        spacer.my_prepare_for_traversing();

        CHECK( is_equal(spacer.my_get_current_time(), 512.0f) );
        CHECK( is_equal(spacer.my_get_current_x_pos(), 145.0f) );
    }

    TEST_FIXTURE(LineSpacerTestFixture, ProcessNonTimedAtProlog_InProlog)
    {
        MyMusicLine2 line(0, 0, 10.0f, 135.0f);     //145.0
        //              Barline, Prolog,   rTime,  xAnchor, xLeft,   uSize,      FixedSp,    VarSp,      xFinal
        line.my_add_entry(false, true,    -1.00f,  0.00f,   0.00f,   526.00f,  180.00f,     0.00f,      0.00f);
        line.my_add_entry(false, false,  512.00f,  0.00f,   0.00f,   366.00f,   45.00f,   489.00f,      0.00f);

        ScoreMeter meter(1, 1, 150.0f);
        MyLineSpacer spacer(&line, &meter);
        spacer.my_prepare_for_traversing();

        LUnits uSpaceAfterProlog = 270.0f;
        spacer.my_process_non_timed_at_prolog(uSpaceAfterProlog);

        //cout << "next time = " << spacer.get_next_available_time() << endl;
        //cout << "next pos = " << spacer.get_next_position() << endl;

        CHECK( is_equal(spacer.get_next_available_time(), 512.0f) );
        CHECK( is_equal(spacer.get_next_position(), 1121.0f) );     //145+526+180+270
    }

    TEST_FIXTURE(LineSpacerTestFixture, ProcessNonTimedAtProlog_NotInProlog)
    {
        MyMusicLine2 line(0, 0, 10.0f, 135.0f);     //145.0
        //              Barline, Prolog,   rTime,  xAnchor, xLeft,   uSize,      FixedSp,    VarSp,      xFinal
        line.my_add_entry(false, false,   -1.00f,  0.00f,   0.00f,   526.00f,  180.00f,     0.00f,      0.00f);
        line.my_add_entry(false, false,  512.00f,  0.00f,   0.00f,   366.00f,   45.00f,   489.00f,      0.00f);

        ScoreMeter meter(1, 1, 150.0f);
        MyLineSpacer spacer(&line, &meter);
        spacer.my_prepare_for_traversing();

        LUnits uSpaceAfterProlog = 270.0f;
        spacer.my_process_non_timed_at_prolog(uSpaceAfterProlog);

        //cout << "next time = " << spacer.get_next_available_time() << endl;
        //cout << "next pos = " << spacer.get_next_position() << endl;

        CHECK( is_equal(spacer.get_next_available_time(), 512.0f) );
        CHECK( is_equal(spacer.get_next_position(), 851.0f) );     //145+526+180
    }

    TEST_FIXTURE(LineSpacerTestFixture, DetermineNextFeasiblePosition)
    {
        MyMusicLine2 line(0, 0, 0.0f, .0f);
        //              Barline, Prolog,   rTime,  xAnchor,   xLeft,   uSize,    FixedSp,   VarSp,      xFinal
        line.my_add_entry(false, false,  512.00f,  -200.0f,   0.00f,   500.0f,     45.0f,   700.0f,      0.00f);

        ScoreMeter meter(1, 1, 150.0f);
        MyLineSpacer spacer(&line, &meter);
        spacer.my_prepare_for_traversing();
        LUnits uSpaceAfterProlog = 270.0f;
        spacer.my_process_non_timed_at_prolog(uSpaceAfterProlog);

        LUnits xAssigned = spacer.determine_next_feasible_position_after(0.0f);

        //cout << "assigned pos = " << xAssigned << endl;

        CHECK( is_equal(xAssigned, 200.0f) );
    }

    TEST_FIXTURE(LineSpacerTestFixture, ProcessTimedAtCurrentTimepos_anchor)
    {
        MyMusicLine2 line(0, 0, 0.0f, .0f);
        //              Barline, Prolog,   rTime,  xAnchor,   xLeft,   uSize,    FixedSp,   VarSp,      xFinal
        line.my_add_entry(false, false,  512.00f,  -200.0f,   0.00f,   500.0f,     45.0f,   700.0f,      0.00f);

        ScoreMeter meter(1, 1, 150.0f);
        MyLineSpacer spacer(&line, &meter);
        spacer.my_prepare_for_traversing();
        LUnits uSpaceAfterProlog = 270.0f;
        spacer.my_process_non_timed_at_prolog(uSpaceAfterProlog);

        spacer.my_process_timed_at_current_timepos(200.0f);

        //cout << "next time = " << spacer.get_next_available_time() << endl;
        //cout << "next pos = " << spacer.get_next_position() << endl;

        CHECK( spacer.get_next_available_time() > 10000.0f );
        CHECK( is_equal(spacer.get_next_position(), 1445.0f) );     //200+500+45+700
    }

};



//=======================================================================================
// ColumnLayouter tests
//=======================================================================================

class MyColumnLayouter2 : public ColumnLayouter
{
public:
    MyColumnLayouter2(LibraryScope& libraryScope, ScoreMeter* pMeter,
                      ColumnStorage* pStorage)
        : ColumnLayouter(libraryScope, pMeter, pStorage)
    {
    }
    virtual ~MyColumnLayouter2() {};

    void my_create_line_spacers()
    {
        const LinesIterator itEnd = m_pColStorage->end();
        for (LinesIterator it=m_pColStorage->begin(); it != itEnd; ++it)
	    {
            MyLineSpacer* pLinSpacer = LOMSE_NEW MyLineSpacer(*it, m_pScoreMeter);
            m_LineSpacers.push_back(pLinSpacer);
        }
    }
    void my_process_non_timed_at_prolog() { process_non_timed_at_prolog(); }
    void my_process_timed_at_current_timepos() { process_timed_at_current_timepos(); }

    float my_get_current_time() { return m_rCurrentTime; }
    LUnits my_get_current_pos() { return m_uCurrentPos; }


};


//---------------------------------------------------------------------------------------
class ColumnLayouterTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    ColumnLayouterTestFixture()   // setUp()
        : m_libraryScope(cout)
        , m_scores_path(LOMSE_TEST_SCORES_PATH)
    {
    }

    ~ColumnLayouterTestFixture()  // tearDown()
    {
    }

    bool is_equal(float x, float y)
    {
        return (fabs(x - y) < 0.1f);
    }

    //SystemLayouter* create_system_layouter()
    //{
    //    m_libraryScope, &meter, colLayouters, instrEngravers
    //}

};


SUITE(ColumnLayouterTest)
{
    // ColumnLayoutre. Column spacing tests ---------------------------------------------

    TEST_FIXTURE(ColumnLayouterTestFixture, NonTimedAtProlog_1)
    {
        ScoreMeter meter(1, 1, 150.0f);
        MyColumnStorage* pStorage = LOMSE_NEW MyColumnStorage(30.0f, 60.0f);

        MyMusicLine2* pLine0 = LOMSE_NEW MyMusicLine2(0, 0, 0.0f, 0.0f);
        //               Barline, Prolog,   rTime,  xAnchor,   xLeft,     uSize,  FixedSp,     VarSp,     xFinal
        pLine0->my_add_entry(false, false,  256.00f,  1.00f,     1.00f,   258.00f,   45.00f,   782.85f,   1086.85f);

        MyMusicLine2* pLine1 = LOMSE_NEW MyMusicLine2(0, 0, 0.0f, 0.0f);
        //               Barline, Prolog,   rTime,  xAnchor,   xLeft,     uSize,  FixedSp,     VarSp,     xFinal
        pLine1->my_add_entry(false, false,  256.00f, -218.00f,   0.00f,   477.00f,   45.00f,   782.85f,   1304.85f);

        pStorage->my_add_line(pLine0);
        pStorage->my_add_line(pLine1);
        MyColumnLayouter2 column(m_libraryScope, &meter, pStorage);

        column.my_create_line_spacers();
        column.my_process_non_timed_at_prolog();

//        cout << "current time = " << column.my_get_current_time() << endl;
//        cout << "current pos = " << column.my_get_current_pos() << endl;

        CHECK( is_equal(column.my_get_current_time(), 256.0f) );
        CHECK( is_equal(column.my_get_current_pos(), 0.0f) );
    }

    TEST_FIXTURE(ColumnLayouterTestFixture, TimedAtCurrentTimepos_1)
    {
        ScoreMeter meter(1, 1, 150.0f);
        MyColumnStorage* pStorage = LOMSE_NEW MyColumnStorage(30.0f, 60.0f);

        MyMusicLine2* pLine0 = LOMSE_NEW MyMusicLine2(0, 0, 0.0f, 0.0f);
        //               Barline, Prolog,   rTime,  xAnchor,     xLeft,     uSize,  FixedSp,     VarSp,     xFinal
        pLine0->my_add_entry(false, false,  256.00f,  0.00f,     0.00f,   260.00f,   40.00f,   700.00f,   1000.00f);

        MyMusicLine2* pLine1 = LOMSE_NEW MyMusicLine2(0, 0, 0.0f, 0.0f);
        //               Barline, Prolog,   rTime,    xAnchor,   xLeft,     uSize,  FixedSp,     VarSp,     xFinal
        pLine1->my_add_entry(false, false,  256.00f, -220.00f,   0.00f,   480.00f,   40.00f,   700.00f,   1220.00f);

        pStorage->my_add_line(pLine0);
        pStorage->my_add_line(pLine1);
        MyColumnLayouter2 column(m_libraryScope, &meter, pStorage);
        column.my_create_line_spacers();
        column.my_process_non_timed_at_prolog();

        column.my_process_timed_at_current_timepos();

        //cout << "current time = " << column.my_get_current_time() << endl;
        //cout << "current pos = " << column.my_get_current_pos() << endl;
        //cout << "xLeft=" << pLine0->front()->get_position() << endl;

        CHECK( column.my_get_current_time() > 100000.0f );          //LOMSE_NO_TIME
        CHECK( is_equal(column.my_get_current_pos(), 1440.0f) );    //220+480+40+700
        CHECK( is_equal(pLine0->front()->get_position(), 220.0f) );
    }

};



////=======================================================================================
//// LineResizer tests
////=======================================================================================
//
//class MyLineResizer : public LineResizer
//{
//public:
//    MyLineResizer(MusicLine* pLine, LUnits uOldWidth, LUnits uNewWidth,
//                  LUnits uNewStart, UPoint sliceOrg)
//        : LineResizer(pLine, uOldWidth, uNewWidth, uNewStart, sliceOrg)
//    {
//    }
//    virtual ~MyLineResizer() {};
//
//};
//
//
////---------------------------------------------------------------------------------------
//class LineResizerTestFixture
//{
//public:
//    LibraryScope m_libraryScope;
//    std::string m_scores_path;
//
//    LineResizerTestFixture()   // setUp()
//        : m_libraryScope(cout)
//        , m_scores_path(LOMSE_TEST_SCORES_PATH)
//    {
//    }
//
//    ~LineResizerTestFixture()  // tearDown()
//    {
//    }
//
//    bool is_equal(float x, float y)
//    {
//        return (fabs(x - y) < 0.1f);
//    }
//};
//
//
//SUITE(LineResizerTest)
//{
//
//    TEST_FIXTURE(LineResizerTestFixture, NonTimedAtProlog)
//    {
//  RPROBLEM: LineResizer moves shapes. Need to use shapes
//    }
//
//};
