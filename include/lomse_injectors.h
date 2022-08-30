//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_INJECTORS_H__
#define __LOMSE_INJECTORS_H__

#include "lomse_ldp_factory.h"
#include "lomse_build_options.h"
#include "lomse_events.h"
#include "lomse_events_dispatcher.h"
#include "lomse_import_options.h"


#include <iostream>

namespace lomse
{

//forward declarations
class LdpParser;
class LdpAnalyser;
class LdpCompiler;
class XmlParser;
class LmdAnalyser;
class LmdCompiler;
class MxlAnalyser;
class MxlCompiler;
class CompressedMxlCompiler;
class MnxAnalyser;
class MnxCompiler;
class ModelBuilder;
class Document;
class LdpFactory;
class FontStorage;
class FontSelector;
class MusicGlyphs;
class View;
class SimpleView;
class VerticalBookView;
class HorizontalBookView;
class SingleSystemView;
class SinglePageView;
class FreeFlowView;
class HalfPageView;
class Interactor;
class Presenter;
class LomseDoorway;
class Drawer;
class BitmapDrawer;
class Task;
class Request;
class ScorePlayer;
class MidiServerBase;
class Metronome;
class IdAssigner;
class DocCursor;
class DocCommandExecuter;
class CaretPositioner;
class MusicGlyphs;

//---------------------------------------------------------------------------------------
// Trace levels for lines breaker algorithm
enum ETraceLevelLinesBreaker
{
    k_trace_breaks_off          = 0x0001,
    k_trace_breaks_table        = 0x0002,   //dump of final breaks table
    k_trace_breaks_computation  = 0x0004,   //trace computation of breaks
    k_trace_breaks_penalties    = 0x0008,   //trace penalties computation
};

//---------------------------------------------------------------------------------------
class LOMSE_EXPORT LibraryScope
{
protected:
    ostream& m_reporter;
    LomseDoorway* m_pDoorway;
    LomseDoorway* m_pNullDoorway;
    LdpFactory* m_pLdpFactory;
    FontStorage* m_pFontStorage;
    FontSelector* m_pFontSelector;
    Metronome* m_pGlobalMetronome;
    EventsDispatcher* m_pDispatcher;
    std::string m_sMusicFontFile;
    std::string m_sMusicFontName;
    std::string m_sMusicFontPath;
    std::string m_sFontsPath;
    MusicGlyphs* m_pMusicGlyphs;

    //options
    bool m_fReplaceLocalMetronome;
    MusicXmlOptions m_importOptions;

    //debug options
    bool m_fJustifySystems;         //if false, prevents systems justification
    bool m_fDumpColumnTables;       //dump columns and slices data
    bool m_fDrawAnchorObjects;      //draw anchor objects (e.g., invisible shapes)
    bool m_fDrawAnchorLines;        //draw a line at anchor positions (spacing algorithm)
    bool m_fShowShapeBounds;        //draw a box around each shape
    bool m_fDrawSlurCtrolPoints;    //draw ctrol.points in slurs and ties
    bool m_fDrawVerticalProfile;    //draw vertical profile
    bool m_fDrawChordsColoured = false; //draw flag, link and start chord notes in colors
    bool m_fUnitTests;              //library is running for Unit Tests
    int m_traceLinesBreaker;        //trace level for lines breaker algorithm

    //spacing algorithm
    bool m_fUseDbgValues;           //use values defined here for spacing params.
    float m_spacingOptForce;
    float m_spacingAlpha;
    float m_spacingDmin;
    Tenths m_spacingSmin;
    int m_renderSpacingOpts;        //options for spacing and lines breaker algorithm

public:
    LibraryScope(ostream& reporter=std::cout, LomseDoorway* pDoorway=nullptr);
    ~LibraryScope();

    inline ostream& default_reporter() { return m_reporter; }
    inline LomseDoorway* platform_interface() { return m_pDoorway; }
    LdpFactory* ldp_factory();
    FontStorage* font_storage();
    inline std::string& fonts_path() { return m_sFontsPath; }
    EventsDispatcher* get_events_dispatcher();
    FontSelector* get_font_selector();

    //callbacks
    void post_event(SpEventInfo pEvent);
    void post_request(Request* pRequest);
    std::string get_font(const std::string& name, bool fBold, bool fItalic);

    double get_screen_ppi() const;
    int get_pixel_format() const;

    //library info
    static std::string get_version_string();
    static std::string get_version_long_string();
    static int get_version_major();
    static int get_version_minor();
    static int get_version_patch();
    static std::string get_build_date();

    //fonts
    MusicGlyphs* get_glyphs_table();
    inline void set_default_fonts_path(const std::string& fontsPath) {
        m_sFontsPath = fontsPath;
    }
    void set_music_font(const std::string& fontFile, const std::string& fontName,
                        const std::string& path="");
    inline const std::string& get_music_font_name() { return m_sMusicFontName; }
    inline const std::string& get_music_font_file() { return m_sMusicFontFile; }
    const std::string& get_music_font_path();
    bool is_music_font_smufl_compliant();

    //global options
    inline void set_global_metronome_and_replace_local(Metronome* pMtr) {
        m_pGlobalMetronome = pMtr;
        m_fReplaceLocalMetronome = true;
    }
    inline Metronome* get_global_metronome() { return m_pGlobalMetronome; }
    inline bool global_metronome_replaces_local() { return m_fReplaceLocalMetronome; }
    inline MusicXmlOptions* get_musicxml_options() { return &m_importOptions; }

    //spacing and lines breaker algorithm parameters
    inline bool use_debug_values() { return m_fUseDbgValues; }
    inline float get_optimum_force() { return m_spacingOptForce; }
    inline void set_optimum_force(float force) {
        m_spacingOptForce = force;
        m_fUseDbgValues = true;
    }
    inline float get_spacing_alpha() {return m_spacingAlpha; }
    inline void set_spacing_alpha(float alpha) {
        m_spacingAlpha = alpha;
        m_fUseDbgValues = true;
    }
    inline float get_spacing_dmin() { return m_spacingDmin; }
    inline void set_spacing_dmin(float dmin) {
        m_spacingDmin = dmin;
        m_fUseDbgValues = true;
    }
    inline Tenths get_spacing_smin() { return m_spacingSmin; }
    inline void set_spacing_smin(Tenths smin) {
        m_spacingSmin = smin;
        m_fUseDbgValues = true;
    }
    inline int get_render_spacing_opts() { return m_renderSpacingOpts; }
    inline void set_render_spacing_opts(int opts) {
        m_renderSpacingOpts = opts;
        m_fUseDbgValues = true;
    }

    //global options, for debug and tests
    inline void set_justify_systems(bool value) { m_fJustifySystems = value; }
    inline bool justify_systems() { return m_fJustifySystems; }
    inline void set_dump_column_tables(bool value) { m_fDumpColumnTables = value; }
    inline bool dump_column_tables() { return m_fDumpColumnTables; }
    inline void set_draw_anchor_objecs(bool value) { m_fDrawAnchorObjects = value; }
    inline bool draw_anchor_objects() { return m_fDrawAnchorObjects; }
    inline void set_draw_anchor_lines(bool value) { m_fDrawAnchorLines = value; }
    inline bool draw_anchor_lines() { return m_fDrawAnchorLines; }
    inline void set_draw_shape_bounds(bool value) { m_fShowShapeBounds = value; }
    inline bool draw_shape_bounds() { return m_fShowShapeBounds; }
    inline void set_draw_slur_ctrol_points(bool value) { m_fDrawSlurCtrolPoints = value; }
    inline bool draw_slur_ctrol_points() { return m_fDrawSlurCtrolPoints; }
    inline void set_draw_vertical_profile(bool value) { m_fDrawVerticalProfile = value; }
    inline bool draw_vertical_profile() { return m_fDrawVerticalProfile; }
    inline void set_draw_chords_coloured(bool value) { m_fDrawChordsColoured = value; }
    inline bool draw_chords_coloured() { return m_fDrawChordsColoured; }
    inline void set_unit_test(bool value) { m_fUnitTests = value; }
    inline bool is_unit_test() { return m_fUnitTests; }
    inline void set_trace_level_for_lines_breaker(int level) {
        m_traceLinesBreaker = level;
    }
    inline int get_trace_level_for_lines_breaker() { return m_traceLinesBreaker; }

};

//---------------------------------------------------------------------------------------
class DocumentScope
{
protected:
    ostream& m_reporter;

public:
    DocumentScope(ostream& reporter=std::cout) : m_reporter(reporter) {}

    ostream& default_reporter() { return m_reporter; }

};

//---------------------------------------------------------------------------------------
class Injector
{
public:
    Injector() {}
    ~Injector() {}

    //LDP format
    static LdpParser* inject_LdpParser(LibraryScope& libraryScope, DocumentScope& documentScope);
    static LdpAnalyser* inject_LdpAnalyser(LibraryScope& libraryScope, Document* pDoc);
    static LdpCompiler* inject_LdpCompiler(LibraryScope& libraryScope, Document* pDoc);

    //LMD format
    static XmlParser* inject_XmlParser(LibraryScope& libraryScope, DocumentScope& documentScope);
    static LmdAnalyser* inject_LmdAnalyser(LibraryScope& libraryScope, Document* pDoc,
                                           XmlParser* pParser);
    static LmdCompiler* inject_LmdCompiler(LibraryScope& libraryScope, Document* pDoc);

    //MusicXML format
    static MxlAnalyser* inject_MxlAnalyser(LibraryScope& libraryScope, Document* pDoc,
                                           XmlParser* pParser);
    static MxlCompiler* inject_MxlCompiler(LibraryScope& libraryScope, Document* pDoc);
    static CompressedMxlCompiler* inject_CompressedMxlCompiler(LibraryScope& libraryScope,
                                                               Document* pDoc);

    //MNX format
    static MnxAnalyser* inject_MnxAnalyser(LibraryScope& libraryScope, Document* pDoc,
                                           XmlParser* pParser);
    static MnxCompiler* inject_MnxCompiler(LibraryScope& libraryScope, Document* pDoc);


    static ModelBuilder* inject_ModelBuilder(DocumentScope& documentScope);
    static Document* inject_Document(LibraryScope& libraryScope,
                                     ostream& reporter = std::cout);
    static BitmapDrawer* inject_BitmapDrawer(LibraryScope& libraryScope);

    static View* inject_View(LibraryScope& libraryScope, int viewType);
    static View* inject_View(LibraryScope& libraryScope, int viewType,
                             Drawer* screenDrawer, Drawer* printDrawer);

    static Interactor* inject_Interactor(LibraryScope& libraryScope,
                                         WpDocument wpDoc, View* pView,
                                         DocCommandExecuter* pExec);

    static Presenter* inject_Presenter(LibraryScope& libraryScope,
                                       int viewType, Document* pDoc);
    static Presenter* inject_Presenter(LibraryScope& libraryScope,
                                       int viewType, Document* pDoc,
                                       Drawer* screenDrawer, Drawer* printDrawer);

    static Task* inject_Task(int taskType, Interactor* pIntor);
#if (LOMSE_ENABLE_THREADS == 1)
    static ScorePlayer* inject_ScorePlayer(LibraryScope& libraryScope,
                                           MidiServerBase* pSoundServer);
#endif
    static DocCursor* inject_DocCursor(Document* pDoc);
    static SelectionSet* inject_SelectionSet(Document* pDoc);
    static DocCommandExecuter* inject_DocCommandExecuter(Document* pDoc);
};



}   //namespace lomse

#endif      //__LOMSE_INJECTORS_H__
