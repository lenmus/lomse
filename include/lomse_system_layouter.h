//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010 Lomse project
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

#ifndef __LOMSE_SYSTEM_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_SYSTEM_LAYOUTER_H__

#include "lomse_basic.h"
//#include "lomse_content_layouter.h"
//#include <sstream>
//#include <list>
#include <vector>
using namespace std;

namespace lomse
{

//forward declarations
class InternalModel;
class ImoInstrument;
//class GraphicModel;
//class ImoDocObj;
//class ImoScore;
//class GmoBoxScorePage;
//class GmoStubScore;
//class GmoShape;
class ImoStaffObj;
class GmoShape;
class GmoBoxSliceInstr;
//
//
//class lmStaff;
//class lmVStaff;
//class GmoBoxSlice;
//
class SystemLayouter;
class ColumnLayouter;
class ColumnStorage;
class LineTable;
class LineEntry;

class BreaksTable;
class BreakPoints;
class ColumnSplitter;
class ColumnResizer;
class LineResizer;
class LineSpacer;
class TimeGridLineExplorer;

//some helper definitions
#define LineEntryIterator		  std::vector<LineEntry*>::iterator
#define LinesIterator             std::vector<LineTable*>::iterator
#define LineSpacersIterator       std::vector<LineSpacer*>::iterator


enum ESpacingMethod
{
    k_proportional_spacing = 0,
};

//----------------------------------------------------------------------------------------
//BreaksTable definition: table to contain possible break points
//----------------------------------------------------------------------------------------

//an entry of the BreaksTable
typedef struct BreaksTimeEntry_Struct
{
    float   rTimepos;
    float   rPriority;
    LUnits  uxStart;
    LUnits  uxEnd;
    bool    fInBeam;
    LUnits  uxBeam;
}
BreaksTimeEntry;

//the breaks table
class BreaksTable
{
public:
    BreaksTable();
    ~BreaksTable();

//    void add_entry(float rTime, LUnits uxStart, LUnits uWidth, bool fInBeam,
//                  LUnits uxBeam, float rPriority = 0.8f);
//    void add_entry(BreaksTimeEntry* pBTE);
//    void change_priority(int iEntry, float rMultiplier);
//    std::string dump();
//    inline bool is_empty() { return m_BreaksTable.empty(); }
//
//    //traversing the table
//    BreaksTimeEntry* get_first();
//    BreaksTimeEntry* get_next();
//
//
//private:
//
//    std::list<BreaksTimeEntry*>               m_BreaksTable;      //the table
//    std::list<BreaksTimeEntry*>::iterator     m_it;               //for get_first(), get_next();

};



//----------------------------------------------------------------------------------------
//LineEntry: an entry in LineTable
//----------------------------------------------------------------------------------------
class LineEntry
{
public:
    // constructor and destructor
    LineEntry(ImoStaffObj* pSO, GmoShape* pShape, bool fProlog);
    ~LineEntry() {}

//    void reposition_at(LUnits uxNewXLeft);
//	void assign_fixed_and_variable_space(ColumnLayouter* pTT, float rFactor);
//    void move_shape();
//
//    //setters and getters
//    inline LUnits get_total_size() { return m_uSize + m_uFixedSpace + m_uVariableSpace; }
//    inline LUnits get_variable_space() { return m_uVariableSpace; }
//    inline void set_variable_space(LUnits uSpace) { m_uVariableSpace = uSpace; }
//    inline float get_duration() { return m_pSO->GetTimePosIncrement(); }
//    inline float get_timepos() { return m_rTimePos; }
//    inline LUnits get_position() { return m_xLeft; }
//    inline void set_position(LUnits uPos) { m_xLeft = uPos; }
//    inline LUnits get_shape_size() { return m_uSize; }
//    inline void mark_as_barline_entry() { m_fIsBarlineEntry = true; }
//    inline LUnits get_anchor() { return m_uxAnchor; }
//    LUnits get_shift_to_noterest_center();
//
//    //other
//    inline bool is_barline_entry() { return m_fIsBarlineEntry; }
//    inline bool is_note_rest() { return m_pSO && m_pSO->is_note_rest(); }
//    inline bool has_barline() { return m_pSO && m_pSO->IsBarline(); }
//
//    //debug
//    std::string dump(int iEntry);
//    static std::string dump_header();
//
//protected:
//    void set_note_rest_space(ColumnLayouter* pTT, float rFactor);
//    void assign_minimum_fixed_space(ColumnLayouter* pColFmt);
//    void assign_variable_space(LUnits uIdeal);
//    void assign_no_space();
//    LUnits compute_ideal_distance(ColumnLayouter* pColFmt, float rFactor);
//    LUnits compute_ideal_distance_fixed(ColumnLayouter* pColFmt);
//    LUnits compute_ideal_distance_proportional(ColumnLayouter* pColFmt, float rFactor);


public:
//protected:
    //member variables (one entry of the table)
    //----------------------------------------------------------------------------
    bool            m_fIsBarlineEntry;  //is last entry. Contains barline or nothing
    ImoStaffObj*    m_pSO;              //ptr to the StaffObj
    GmoShape*       m_pShape;           //ptr to the shape
	bool			m_fProlog;          //this shape is a prolog object (clef, KS, TS at start of system)
    float           m_rTimePos;         //timepos for this pSO or -1 if not anchored in time
    LUnits          m_xLeft;            //current position of the left border of the object
    LUnits          m_uxAnchor;         //position of the anchor line
    LUnits          m_xFinal;           //next position (right border position + trailing space)
    //to redistribute objects we need to know:
    LUnits          m_uSize;            //size of the shape (notehead, etc.)
    LUnits          m_uFixedSpace;      //fixed space added after shape
    LUnits          m_uVariableSpace;   //any variable added space we can adjust

};


//----------------------------------------------------------------------------------------
// LineTable: an object to encapsulate positioning data for a line
//----------------------------------------------------------------------------------------
class LineTable
{
protected:
	std::vector<LineEntry*>	m_LineEntries;	    //the entries that form this table
	int     m_nInstr;		    //instrument (0..n-1)
	int     m_nVoice;		    //voice (0=not yet defined)
    LUnits  m_uxLineStart;      //initial position      
    LUnits  m_uInitialSpace;    //space at beginning

public:
    LineTable(int nInstr, int nVoice, LUnits uxStart, LUnits uSpace);
    ~LineTable();

    //access to an item
    inline LineEntry* front() { return m_LineEntries.front(); }
    inline LineEntry* back() { return m_LineEntries.back(); }
    inline LineEntry* item(int i) { return m_LineEntries[i]; }
    inline LineEntry* get_last_entry() { return m_LineEntries.back(); }

    //iterator to an item
    inline LineEntryIterator begin() { return m_LineEntries.begin(); }
    inline LineEntryIterator end() { return m_LineEntries.end(); }

//    //table manipulation
//    inline void clear() { m_LineEntries.clear(); }
//    inline void push_back(LineEntry* pEntry) { m_LineEntries.push_back(pEntry); }
//	LineEntry* add_entry(ImoStaffObj* pSO, GmoShape* pShape, bool fProlog);
//	LineEntry* add_final_entry(ImoStaffObj* pSO, GmoShape* pShape);
//
//
//    //properties
//    inline LUnits GetLineStartPosition() { return m_uxLineStart; }
//    inline LUnits GetSpaceAtBeginning() { return m_uInitialSpace; }
//    LUnits GetLineWidth();
//    inline size_t Size() { return m_LineEntries.size(); }
//    inline bool IsLineForInstrument(int nInstr) { return m_nInstr == nInstr; }
//    inline bool IsLineForVoice(int nVoice) { return m_nVoice == 0 || m_nVoice == nVoice; }
//    inline bool IsVoiceNotYetDefined() { return m_nVoice == 0; }
//    inline void SetVoice(int nVoice) { m_nVoice = nVoice; }
//    inline int GetInstrument() { return m_nInstr; }
//    inline int GetVoice() { return m_nVoice; }
//    bool ContainsBarline();
//
//    //other
//    void ClearDirtyFlags();
//
//    //Debug and Unit Tests
//    inline int GetNumObjectsInLine() { return (int)m_LineEntries.size(); }
//    std::string DumpMainTable();

};


//class DirtyFlagsCleaner
//{
//protected:
//    ColumnStorage*    m_pColStorage;
//
//public: 
//    DirtyFlagsCleaner(ColumnStorage* pColStorage);
//    
//    void ClearDirtyFlags();
//};


//----------------------------------------------------------------------------------------
//ColumnStorage: encapsulates the lines for a column
//----------------------------------------------------------------------------------------
class ColumnStorage
{
protected:
	std::vector<LineTable*> m_Lines;	    //lines that form this column
	//lmStaff* m_pStaff[lmMAX_STAFF];         //staves (nedeed to compute spacing)

public:
    ColumnStorage();
    ~ColumnStorage();

//    void Initialize();
//
//    //access to an item
//    LineTable* front() { return m_Lines.front(); }
//
//    //iterator
//    inline LinesIterator begin() { return m_Lines.begin(); }
//    inline LinesIterator end() { return m_Lines.end(); }
//    inline LinesIterator GetLastLine() {
//	    LinesIterator it = m_Lines.end();
//	    return --it;
//    }
//    LinesIterator FindLineForInstrAndVoice(int nInstr, int nVoice);
//
//
//    //storage manipulation
//    LineTable* OpenNewLine(int nInstr, int nVoice, LUnits uxStart, LUnits uSpace);
//    inline void SaveStaffPointer(int iStaff, lmStaff* pStaff) { m_pStaff[iStaff] = pStaff; }
//
//    //properties
//    inline size_t Size() { return m_Lines.size(); }
//    inline bool IsEndOfTable(LinesIterator it) { return it == m_Lines.end(); }
//
//    //access to column measurements
//    LUnits GetColumnWitdh();
//    LUnits GetStartOfBarPosition();
//
//    //units conversion
//    LUnits TenthsToLogical(Tenths rTenths, int nStaff);
//
//    //debug
//    std::string DumpColumnStorage();
//
//    //Public methods coded only for Unit Tests
//    inline int GetNumObjectsInLine(int iLine) { return (int)m_Lines[iLine]->Size(); }
//
//    //other methods
//    void ClearDirtyFlags();
//
//protected:
//    void DeleteLines();

};


//----------------------------------------------------------------------------------------
//LinesBuilder: receives information about objects in a column, organizes this
//info into lines and stores them in the received column storage
//----------------------------------------------------------------------------------------
class LinesBuilder
{
protected:
    ColumnStorage*  m_pColStorage;              //music lines for this column
    //int             m_nStaffVoice[LOMSE_MAX_STAFF];	//voice assigned to each staff
	LineEntry*      m_pCurEntry;				//ptr to last added entry
	LinesIterator   m_itCurLine;				//point to the pos table for current line

public:
    LinesBuilder(ColumnStorage* pStorage);
    ~LinesBuilder();

//    void Initialize();
//
    //methods to build the lines
    void start_measurements_for_instrument(int iInstr, LUnits uxStart, 
                                           ImoInstrument* pInstr, LUnits uSpace);
    void close_line(ImoStaffObj* pSO, LUnits xStart);     //GmoShape* pShape, 
    void include_object(int iInstr, ImoStaffObj* pSO, bool fProlog, int nStaff);
//    void EndOfData();        //inform that all data has been suplied
//
//private:
//    void ResetDefaultStaffVoices();
//    int DecideVoiceToUse(ImoStaffObj* pSO, int nStaff);
//    void StartLine(int nInstr, int nVoice=0, LUnits uxStart = -1.0f, LUnits uSpace = 0.0f);
//    void StartLineInheritInitialPostionAndSpace(int nInstr, int nVoice);
//    void CreateLinesForEachStaff(int nInstr, LUnits uxStart, ImoInstrument* pInstr,
//                                 LUnits uSpace);
};


//----------------------------------------------------------------------------------------
//ColumnLayouter: column layout algorithm
//  - explores all lines, by time pos, aligning objects
//----------------------------------------------------------------------------------------
class ColumnLayouter
{
protected:
    ColumnStorage*            m_pColStorage;  //music lines for this column
    std::vector<LineSpacer*>  m_LineSpacers;  //one spacer for each line

    LUnits            m_uMinColumnSize;           //minimum size for this column

    //formatter parameters
    float               m_rSpacingFactor;           //for proportional spacing of notes
    ESpacingMethod    m_nSpacingMethod;           //fixed, proportional, etc.
    Tenths            m_rSpacingValue;            //spacing for 'fixed' method

public:
    ColumnLayouter(ColumnStorage* pStorage, float rSpacingFactor,
                   ESpacingMethod nSpacingMethod, Tenths nSpacingValue);
    ~ColumnLayouter();

//    inline void Initialize() {}
//
//    //methods to compute results
//    void DoSpacing(bool fTrace = false);
//    inline void IncrementColumnSize(LUnits uIncr) { m_uMinColumnSize += uIncr; }
//
//    //access to info
//    bool IsThereBarline();
//    inline LUnits GetMinimumSize() { return m_uMinColumnSize; }
//
//    //methods for spacing
//	LUnits TenthsToLogical(Tenths rTenths, int nStaff); 
//    inline bool IsProportionalSpacing() { return m_nSpacingMethod == esm_PropConstantFixed; }
//    inline bool IsFixedSpacing() { return m_nSpacingMethod == esm_Fixed; }
//    inline Tenths GetFixedSpacingValue() const { return m_rSpacingValue; }
//
//    //Public methods coded only for Unit Tests
//     inline int GetNumLines() { return (int)m_pColStorage->Size(); }
//
//private:
//    LUnits ComputeSpacing();
//    void DeleteLineSpacers();
//    inline bool ThereAreObjects() { return m_fThereAreObjects; }
//
//    //variables and methods for column traversal ---------------------------------
//    bool    m_fThereAreObjects;
//    float   m_rCurrentTime;         //current tiempos being aligned
//    LUnits  m_rCurrentPos;          //xPos to start placing objects
//
//    void CreateLineSpacers();
//    void ProcessNonTimedAtProlog();
//    void ProcessTimedAtCurrentTimepos();
//    void ProcessNonTimedAtCurrentTimepos();
//    //----------------------------------------------------------------------------

};


//---------------------------------------------------------------------------------------
//BreakPoints: 
//  encloses the algorithm to determine optimum break points to split a column
//---------------------------------------------------------------------------------------
class BreakPoints
{
protected:
    ColumnStorage* m_pColStorage;      //the column to split
    BreaksTable* m_pPossibleBreaks;
    BreaksTimeEntry* m_pOptimumEntry;

public:
    BreakPoints(ColumnStorage* pColStorage);
    ~BreakPoints();

//    bool FindOptimunBreakPointForSpace(LUnits uAvailable);
//    float GetOptimumTimeForFoundBreakPoint();
//    LUnits GetOptimumPosForFoundBreakPoint();
//
//protected:
//    void ComputeBreaksTable();
//    void DeleteBreaksTable();

};



//---------------------------------------------------------------------------------------
// SystemLayouter: orchestrates the layout of a system
class SystemLayouter
{
protected:
    std::vector<ColumnLayouter*> m_ColLayouters;    //layouter for each column
    std::vector<ColumnStorage*> m_ColStorage;       //data for each column
    std::vector<LinesBuilder*> m_LinesBuilder;      //lines builder for each column

    //layout options
    float               m_rSpacingFactor;   //for proportional spacing of notes
    ESpacingMethod    m_nSpacingMethod;   //fixed, proportional, etc.
    Tenths              m_rSpacingValue;    //space for 'fixed' method


public:
    SystemLayouter();
        //float rSpacingFactor, ESpacingMethod nSpacingMethod, Tenths rSpacingValue);
    ~SystemLayouter();

//        //Collecting measurements
//
//    //caller informs that all data for this system has been suplied
//    void EndOfSystemMeasurements();             

    //caller ask to prepare for receiving data about column iCol [0..n-1] for
    //the given instrument
    void start_bar_measurements(int iCol, LUnits uxStart, LUnits uSpace);

    //caller sends data about one staffobj in column iCol [0..n-1]
    void include_object(int iCol, int iInstr, ImoStaffObj* pSO, bool fProlog,
                        int nStaff, GmoShape* pShape);
    //caller sends lasts object to store in column iCol [0..n-1]. 
    void include_barline_and_terminate_bar_measurements(int iCol, ImoStaffObj* pSO,
                                                        LUnits xStart);     //GmoShape* pShape, 

    //caller informs that there are no barline and no more objects in column iCol [0..n-1]. 
    void terminate_bar_measurements_without_barline(int iCol, LUnits xStart);

//    //caller request to ignore measurements for column iCol [0..n-1]
//    void DiscardMeasurementsForColumn(int iCol);
//
//        // Processing
//    void DoColumnSpacing(int iCol, bool fTrace = false);
//    LUnits RedistributeSpace(int iCol, LUnits uNewStart);
//    void AddTimeGridToBoxSlice(int iCol, GmoBoxSlice* pBSlice);
//
//        //Operations
//
//    void IncrementColumnSize(int iCol, LUnits uIncr);

    void add_shapes(std::vector<GmoBoxSliceInstr*>& sliceInstrBoxes);

//
//        //Access to information
//    LUnits GetStartPositionForColumn(int iCol);
//
//    LUnits GetMinimumSize(int iCol);
//    bool GetOptimumBreakPoint(int iCol, LUnits uAvailable, float* prTime,
//                              LUnits* puWidth);
//    bool ColumnHasBarline(int iCol);
//
//    //other methods
//    void ClearDirtyFlags(int iCol);
//
//
//    //Public methods coded for Unit Tests and debugging
//    std::string DumpColumnData(int iCol);
//    inline int GetNumColumns() { return (int)m_ColLayouters.size(); }
//    inline int GetNumLinesInColumn(int iCol) { return m_ColLayouters[iCol]->GetNumLines(); }
//    int GetNumObjectsInColumnLine(int iCol, int iLine);     //iCol, iLine = [0..n-1]
//    inline ColumnStorage* GetColumnData(int iCol) { return m_ColStorage[iCol]; }

};


//---------------------------------------------------------------------------------------
//ColumnSplitter:
//  Algorithm to determine optimum break points to split a column 
//---------------------------------------------------------------------------------------
class ColumnSplitter
{
protected:
    LineTable* m_pLineTable;       //the line to process

public:
    ColumnSplitter(LineTable* pLineTable);
    ~ColumnSplitter();

//    void ComputeBreakPoints(BreaksTable* pBT);

};


//---------------------------------------------------------------------------------------
//LineResizer: encapsulates the methods to recompute shapes positions so that the line
//will have the desired width, and to move the shapes to those positions
//---------------------------------------------------------------------------------------
class LineResizer
{
protected:
    LineTable*    m_pTable;           //table for the line to resize
    LUnits        m_uOldBarSize;
    LUnits        m_uNewBarSize;
    LUnits        m_uNewStart;
    LineEntryIterator m_itCurrent;

public:
    LineResizer(LineTable* pTable, LUnits uOldBarSize, LUnits uNewBarSize,
                LUnits uNewStart);

//    float MovePrologShapes();
//    void ReassignPositionToAllOtherObjects(LUnits uFizedSizeAtStart);
//    LUnits GetTimeLinePositionIfTimeIs(float rFirstTime);
//
//protected:
//    void InformAttachedObjs();

};


//---------------------------------------------------------------------------------------
//LineSpacer:
//  encapsulates the algorithm to assign spaces and positions to a single line
//---------------------------------------------------------------------------------------
class LineSpacer
{
private:
    LineTable*          m_pTable;           //the line to assign space
    float               m_rFactor;          //spacing factor
    ColumnLayouter*     m_pColFmt;          //for tenths/logical conversion
    LineEntryIterator   m_itCur;            //current entry
    float               m_rCurTime;         //current time
	LUnits              m_uxCurPos;         //current xPos at start of current time
    LUnits              m_uxRemovable;      //space that can be removed if required
    LineEntryIterator   m_itNonTimedAtCurPos;
    LUnits              m_uxNotTimedFinalPos;

public:
    LineSpacer(LineTable* pLineTable, ColumnLayouter* pColFmt, float rFactor);

//    void ProcessNonTimedAtProlog(LUnits uSpaceAfterProlog);
//    void ProcessNonTimedAtCurrentTimepos(LUnits uxPos);
//    void ProcessTimedAtCurrentTimepos(LUnits uxPos);
//	inline bool CurrentTimeIs(float rTime) { return m_rCurTime == rTime; }
//    inline bool ThereAreTimedObjs() {
//        return (m_itCur != m_pTable->end() && IsEqualTime((*m_itCur)->get_timepos(), m_rCurTime));
//    }
//    inline bool ThereAreMoreObjects() { return (m_itCur != m_pTable->end()); }
//    float GetNextAvailableTime();
//    LUnits GetNextPosition();
//
//protected:
//    void InitializeForTraversing();
//    LUnits ComputeShiftToAvoidOverlapWithPrevious();
//    void DragAnyPreviousCleftToPlaceItNearThisNote();
//
//    inline bool IsNonTimedObject(LineEntryIterator it) {
//        return (it != m_pTable->end() && (*it)->get_timepos() < 0.0f);
//    }
//    inline bool IsTimedObject(LineEntryIterator it) {
//        return (it != m_pTable->end() && (*it)->get_timepos() >= 0.0f);
//    }
//    inline bool CurrentObjectIsNonTimed() { return IsNonTimedObject(m_itCur); }
//
//
//    //variables and methods used only to position non-timed objects ------
//    // create helper object?
//    LUnits m_uxMaxOcuppiedSpace;
//    LUnits m_uxMinOcuppiedSpace;
//
//    void ComputeMaxAndMinOcuppiedSpace();
//    void PositionNonTimed();
//    void ShiftNonTimed(LUnits uxShift);
//
//    void PositionUsingMaxSpaceWithShift(LUnits uShift);
//    void PositionUsingMinSpaceWithShift(LUnits uShift);
//    //--------------------------------------------------------------------

};


//---------------------------------------------------------------------------------------
//TimeGridTable:
//  A table with occupied times and durations, and connecting time with position
//---------------------------------------------------------------------------------------

//an item in the positions and times table
typedef struct
{
    float rTimepos;
    float rDuration;
    LUnits uxPos;
}
PosTimeItem;

//the table
class TimeGridTable
{
protected:
    ColumnStorage* m_pColStorage;
    std::vector<PosTimeItem> m_PosTimes;         //the table

public:
    TimeGridTable(ColumnStorage* pColStorage);
    ~TimeGridTable();

//    inline int GetSize() { return (int)m_PosTimes.size(); }
//
//    //access to an entry values
//    inline float get_timepos(int iItem) { return m_PosTimes[iItem].rTimepos; }
//    inline float get_duration(int iItem) { return m_PosTimes[iItem].rDuration; }
//    inline LUnits GetXPos(int iItem) { return m_PosTimes[iItem].uxPos; }
//
//    //access by position
//    float GetTimeForPosititon(LUnits uxPos);
//
//    //debug
//    std::string dump();
//
//protected:
//    //variables and methods for column traversal
//    std::vector<TimeGridLineExplorer*> m_LineExplorers;
//    float m_rCurrentTime;
//    float m_rMinDuration;
//    LUnits m_uCurPos;
//    bool m_fTimedObjectsFound;
//
//    inline bool TimedObjectsFound() { return m_fTimedObjectsFound; }
//    bool ThereAreObjects();
//    void CreateLineExplorers();
//    void DeleteLineExplorers();
//    void SkipNonTimedAtCurrentTimepos();
//    void FindShortestNoteRestAtCurrentTimepos();
//    void CreateTableEntry();
//    void GetCurrentTime();
//    void InterpolateMissingTimes();

};

// helper class to interpolate missing entries
//--------------------------------------------
class TimeInserter
{
protected:
    std::vector<PosTimeItem>& m_PosTimes;

    std::vector<PosTimeItem>::iterator  m_itInsertionPoint;
    float m_rTimeBeforeInsertionPoint;
    LUnits m_uPositionBeforeInsertionPoint;

public:
    TimeInserter(std::vector<PosTimeItem>& oPosTimes);
//    void InterpolateMissingTimes();
//
//protected:
//    bool IsTimeInTable(float rTimepos);
//    void FindInsertionPoint(float rTimepos);
//    void InsertTimeInterpolatingPosition(float rTimepos);

};


// helper class to encapsulate the line traversal algorithm
// for creating the time-pos table
//----------------------------------------------------------
class TimeGridLineExplorer
{
private:
    LineTable* m_pTable;           //the line to assign space
    LineEntryIterator m_itCur;            //current entry
    float m_rCurTime;
	LUnits m_uCurPos;
    LUnits m_uShiftToNoteRestCenter;
    float m_rMinDuration;

public:
    TimeGridLineExplorer(LineTable* pLineTable);
    ~TimeGridLineExplorer();

//    bool SkipNonTimedAtCurrentTimepos();
//    bool FindShortestNoteRestAtCurrentTimepos();
//    inline bool ThereAreObjects() { return (m_itCur != m_pTable->end()); }
//    float GetCurrentTime();
//    float GetDurationForFoundEntry();
//    LUnits GetPositionForFoundEntry();
//
//protected:
//    inline bool CurrentObjectIsNonTimed() {
//        return (m_itCur != m_pTable->end() && (*m_itCur)->get_timepos() < 0.0f);
//    }
//
//    inline bool CurrentObjectIsTimed() {
//        return (m_itCur != m_pTable->end() && (*m_itCur)->get_timepos() >= 0.0f);
//    }
};



//---------------------------------------------------------------------------------------
//ColumnResizer: encapsulates the methods to recompute shapes positions so that the
//column will have the desired width, and to move the shapes to those positions
//---------------------------------------------------------------------------------------
class ColumnResizer
{
protected:
    ColumnStorage* m_pColStorage;      //column to resize
    LUnits m_uNewBarSize;
    LUnits m_uOldBarSize;
    LUnits m_uNewStart;
    float m_rFirstTime;
    LUnits m_uFixedPart;
    std::vector<LineResizer*> m_LineResizers;

public:
    ColumnResizer(ColumnStorage* pColStorage, LUnits uNewBarSize);
//    void RepositionShapes(LUnits uNewStart);
//
//protected:
//    void CreateLineResizers();
//    void MovePrologShapesAndGetInitialTime();
//    void RepositionAllOtherShapes();
//    void DetermineFixedSizeAtStartOfColumn();
//    void DeleteLineResizers();

};


}   //namespace lomse

#endif    // __LOMSE_SYSTEM_LAYOUTER_H__

