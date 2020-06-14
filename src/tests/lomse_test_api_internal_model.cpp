//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_time.h"
#include "private/lomse_document_p.h"
#include "lomse_document.h"
#include "lomse_presenter.h"
#include "lomse_graphic_view.h"


using namespace UnitTest;
using namespace std;
using namespace lomse;


//---------------------------------------------------------------------------------------
class InternalModelApiTestFixture
{
public:
    LibraryScope m_libraryScope;
    std::string m_scores_path;

    InternalModelApiTestFixture()
        : m_libraryScope(cout)
    {
        m_scores_path = TESTLIB_SCORES_PATH;
        m_libraryScope.set_default_fonts_path(TESTLIB_FONTS_PATH);
    }

    ~InternalModelApiTestFixture()
    {
    }

    inline const char* test_name()
    {
        return UnitTest::CurrentTest::Details()->testName;
    }

};

//---------------------------------------------------------------------------------------
SUITE(InternalModelApiTest)
{

    //@ IDocument -----------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0100)
    {
        //@0100. Document returns valid IDocument
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        ImoDocument* pImoDoc = theDoc.get_im_root();

        IDocument doc = theDoc.get_document_api();

        CHECK( doc.lmd_version() == pImoDoc->get_version() );
        CHECK( doc.lmd_version() == "0.0" );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0101)
    {
        //@0101. Presenter returns valid IDocument
        PresenterBuilder builder(m_libraryScope);
        Presenter* pPresenter = builder.new_document(k_view_simple,
            "(lenmusdoc (vers 0.0)(content "
            "(para (txt \"Hello world\"))"
            "(score#505 (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
            "))", cout, Document::k_format_ldp
        );
        IDocument doc = pPresenter->get_document();

        IScore score = doc.first_score();
        CHECK( score.object_id() == 505 );

        delete pPresenter;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0201)
    {
        //@0201. Access to first score. Not found
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content ))"
        );
        IDocument doc = theDoc.get_document_api();

        IScore score = doc.first_score();

        CHECK( !score.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0202)
    {
        //@0202. Access to first score. Success

        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        IDocument doc = theDoc.get_document_api();

        IScore score = doc.first_score();

        CHECK( score.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0203)
    {
        //@0203. Access to first score. More content. Success
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
            "(para (txt \"Hello world\"))"
            "(score (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();

        IScore score = doc.first_score();

        CHECK( score.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0204)
    {
        //@0204. num_children(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
                "(para (txt \"Hello world\"))"
                "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();

        CHECK( doc.num_children() == 2 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0205)
    {
        //@0205. num_children(). Empty document
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content))"
        );
        IDocument doc = theDoc.get_document_api();

        CHECK( doc.num_children() == 0 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0206)
    {
        //@0206. first_child(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
                "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
                "(para (txt \"Hello world\"))"
                "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();

        //child found
        CHECK( doc.num_children() == 3 );
        IObject content = doc.first_child();
        CHECK( content.is_valid() );

        //and can be downcasted
        CHECK( content.is_score() == true );
        IScore score2 = content.downcast_to_score();
        CHECK( score2.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0207)
    {
        //@0207. first_child(). Empty document.
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content))"
        );
        IDocument doc = theDoc.get_document_api();

        //child found
        CHECK( doc.num_children() == 0 );
        IObject content = doc.first_child();
        CHECK( !content.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0208)
    {
        //@0208. last_child(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
                "(heading 1 (txt \"Hello world\"))"
                "(para (txt \"This is a score\"))"
                "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();

        //child found
        CHECK( doc.num_children() == 3 );
        IObject content = doc.last_child();
        CHECK( content.is_valid() );

        //and can be downcasted
        CHECK( content.is_score() == true );
        IScore score2 = content.downcast_to_score();
        CHECK( score2.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0209)
    {
        //@0209. last_child(). Empty document.
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content))"
        );
        IDocument doc = theDoc.get_document_api();

        //child found
        CHECK( doc.num_children() == 0 );
        IObject content = doc.last_child();
        CHECK( !content.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0210)
    {
        //@0210. child_at(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
                "(heading 1 (txt \"Hello world\"))"
                "(para (txt \"This is a score\"))"
                "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();

        //child found
        CHECK( doc.num_children() == 3 );
        IObject content = doc.child_at(1);
        CHECK( content.is_valid() );
        CHECK( content.is_paragraph() == true );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0211)
    {
        //@0211. child_at(). Invalid parameter
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
                "(heading 1 (txt \"Hello world\"))"
                "(para (txt \"This is a score\"))"
                "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();

        IObject content = doc.child_at(4);

        CHECK( !content.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idocument_0212)
    {
        //@0212. child_at(). Empty document
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content))"
        );
        IDocument doc = theDoc.get_document_api();

        IObject content = doc.child_at(0);

        CHECK( !content.is_valid() );
    }


    //@ IDynamic ---------------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, idynamic_0100)
    {
        //@0100. IObject properties and casting. OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "<lenmusdoc vers='0.0'><content>"
                "<dynamic classid=\"test\" />"
            "<content/></lenmusdoc>"
            , Document::k_format_lmd
        );
        IDocument doc = theDoc.get_document_api();
        IObject obj = doc.first_child();

        CHECK( obj.is_valid() );
        CHECK( obj.object_id() != 0 );
        CHECK( obj.object_name() == "dynamic" );
        IDocument doc1 = obj.owner_document();
        CHECK( doc.object_id() == doc1.object_id() );

        //can be downcasted
        CHECK( obj.is_dynamic() );
        IDynamic dyn = obj.downcast_to_dynamic();
        CHECK( dyn.is_valid() );

        //other properties
        CHECK( dyn.classid() == "test" );
        CHECK( dyn.num_children() == 0 );
        CHECK( !dyn.first_child().is_valid() );
        CHECK( !dyn.last_child().is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idynamic_0101)
    {
        //@0101. Factory, enum and downcast are ok
        Document theDoc(m_libraryScope);
        IDocument doc = theDoc.get_document_api();

        IObject obj = doc.create_object(k_obj_dynamic);

        CHECK( obj.is_valid() );
        CHECK( obj.object_name() == "dynamic" );
        CHECK( obj.is_dynamic() );
        IDynamic dyn = obj.downcast_to_dynamic();
        CHECK( dyn.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, idynamic_0200)
    {
        //@0200. get siblings
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "<lenmusdoc vers='0.0'><content>"
                "<dynamic classid=\"Foo\" />"
                "<dynamic classid=\"Bar\" />"
                "<para>This is a paragraph</para>"
            "<content/></lenmusdoc>"
            , Document::k_format_lmd
        );
        IDocument doc = theDoc.get_document_api();
        IObject child = doc.first_child();
        CHECK( child.is_dynamic() );
        IDynamic dyn1 = child.downcast_to_dynamic();
        CHECK( dyn1.classid() == "Foo" );

        IObject obj = dyn1.previous_sibling();
        CHECK( !obj.is_valid() );

        obj = dyn1.next_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_dynamic() );
        IDynamic dyn2 = obj.downcast_to_dynamic();
        CHECK( dyn2.classid() == "Bar" );

        obj = dyn2.next_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_paragraph() );

        obj = dyn2.previous_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_dynamic() );
        CHECK( obj.object_id() == dyn1.object_id() );
    }


    //@ IInstrument ---------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrument_0100)
    {
        //@0100. get and set name and abbreviation
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument (name \"Violin\")(abbrev \"V.\")(musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrument instr = score.instrument_at(0);
        CHECK( instr.name_string() == "Violin" );
        CHECK( instr.abbreviation_string() == "V." );

        instr.set_name_string("Flute");
        instr.set_abbreviation_string("F.");

        CHECK( instr.name_string() == "Flute" );
        CHECK( instr.abbreviation_string() == "F." );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrument_0101)
    {
        //@0101. Factory, enum and downcast are ok
        Document theDoc(m_libraryScope);
        IDocument doc = theDoc.get_document_api();

        IObject obj = doc.create_object(k_obj_instrument);

        CHECK( obj.is_valid() );
        CHECK( obj.object_name() == "instrument" );
        CHECK( obj.is_instrument() );
        IInstrument instr = obj.downcast_to_instrument();
        CHECK( instr.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrument_0102)
    {
        //@0102. get and set MIDI information
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument (name \"Violin\")(abbrev \"V.\")(musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrument instr = score.instrument_at(0);

        CHECK( instr.num_sounds() == 1 );
        ISoundInfo sound = instr.sound_info_at(0);
        IMidiInfo midi = sound.midi_info();
        CHECK( midi.channel() == 1 );
        CHECK( midi.program() == 1 );
        midi.set_channel(7);
        midi.set_program(2);

        IScore score2 = doc.first_score();
        IInstrument instr2 = score2.instrument_at(0);
        ISoundInfo sound2 = instr2.sound_info_at(0);
        IMidiInfo midi2 = sound2.midi_info();
        CHECK( midi2.channel() == 7 );
        CHECK( midi2.program() == 2 );
    }


    //@ IInstrGroup ---------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0100)
    {
        //@0100. get and set name and abbreviation
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P1 P2 (name \"Group1\")(symbol bracket)(joinBarlines no))"
                "(group P3 P4 (abbrev \"G2\")(symbol brace)(joinBarlines yes))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrGroup group1 = score.instruments_group_at(0);
        IInstrGroup group2 = score.instruments_group_at(1);
        CHECK( group1.name_string() == "Group1" );
        CHECK( group1.abbreviation_string() == "" );
        CHECK( group2.name_string() == "" );
        CHECK( group2.abbreviation_string() == "G2" );

        group1.set_abbreviation_string("G1");
        group2.set_name_string("Group2");

        CHECK( group1.name_string() == "Group1" );
        CHECK( group1.abbreviation_string() == "G1" );
        CHECK( group2.name_string() == "Group2" );
        CHECK( group2.abbreviation_string() == "G2" );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0101)
    {
        //@0101. Factory, enum and downcast are ok
        Document theDoc(m_libraryScope);
        IDocument doc = theDoc.get_document_api();

        IObject obj = doc.create_object(k_obj_instr_group);

        CHECK( obj.is_valid() );
        CHECK( obj.object_name() == "instr-group" );
        CHECK( obj.is_instr_group() );
        IInstrGroup group = obj.downcast_to_instr_group();
        CHECK( group.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0110)
    {
        //@0110. get and set symbol
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P1 P2 (name \"Group1\")(symbol bracket)(joinBarlines no))"
                "(group P3 P4 (abbrev \"G2\")(symbol brace)(joinBarlines yes))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrGroup group1 = score.instruments_group_at(0);
        IInstrGroup group2 = score.instruments_group_at(1);

        CHECK( group1.symbol() == EGroupSymbol::k_group_symbol_bracket );
        CHECK( group2.symbol() == EGroupSymbol::k_group_symbol_brace );

        group1.set_symbol(EGroupSymbol::k_group_symbol_none);
        group2.set_symbol(EGroupSymbol::k_group_symbol_line);

        CHECK( group1.symbol() == k_group_symbol_none );
        CHECK( group2.symbol() == k_group_symbol_line );

//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0120)
    {
        //@0120. get and set barlines_mode
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P1 P2 (name \"Group1\")(symbol bracket)(joinBarlines no))"
                "(group P3 P4 (abbrev \"G2\")(symbol brace)(joinBarlines yes))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrGroup group1 = score.instruments_group_at(0);
        IInstrGroup group2 = score.instruments_group_at(1);

        CHECK( group1.barlines_mode() == EJoinBarlines::k_non_joined_barlines );
        CHECK( group2.barlines_mode() == EJoinBarlines::k_joined_barlines );

        group1.set_barlines_mode(EJoinBarlines::k_mensurstrich_barlines);
        group2.set_barlines_mode(EJoinBarlines::k_non_joined_barlines);

        CHECK( group1.barlines_mode() == k_mensurstrich_barlines );
        CHECK( group2.barlines_mode() == k_non_joined_barlines );

//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0130)
    {
        //@0130. set_range()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P1 P2 (name \"Group1\")(symbol bracket)(joinBarlines no))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrGroup group = score.instruments_group_at(0);

        //out of range
        CHECK( group.set_range(0, 4) == false );

        //out of order
        CHECK( group.set_range(3, 2) == false );

        //only one instrument
        CHECK( group.set_range(1, 1) == false );

        //valid range
        CHECK( group.set_range(1, 3) == true );

        //check group
        IInstrGroup group1 = score.instruments_group_at(0);
        CHECK( group1.num_instruments() == 3 );
        CHECK( group1.first_instrument().object_id() == 400L );
        CHECK( group1.last_instrument().object_id() == 600L );

//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0140)
    {
        //@0140. instrument_at(). Invalid parameter
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P2 P4 (name \"Group1\")(symbol bracket)(joinBarlines no))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrGroup group = score.instruments_group_at(0);

        IInstrument instr = group.instrument_at(3);
        CHECK( !instr.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0141)
    {
        //@0141. instrument_at(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P2 P4 (name \"Group1\")(symbol bracket)(joinBarlines no))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrGroup group = score.instruments_group_at(0);

        IInstrument instr = group.instrument_at(2);
        CHECK( instr.is_valid() );
        CHECK( instr.name_string() == "P4" );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iinstrgroup_0142)
    {
        //@0142. get index to_first and last instruments
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)"
                "(group P2 P4 (name \"Group1\")(symbol bracket)(joinBarlines no))"
            ")"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrGroup group = score.instruments_group_at(0);

        CHECK( group.index_to_first_instrument() == 1 );
        CHECK( group.index_to_last_instrument() == 3 );
    }


    //@ ILink ---------------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, ilink_0100)
    {
        //@0100. IObject properties and casting. OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "<lenmusdoc vers='0.0'><content>"
                "<para>"
                    "<link url='This is 1st url'>This is the first link</link>"
                    "<txt>Some text</txt>"
                    "<link url='This is 2nd url'>This is the second link</link>"
                "</para>"
            "<content/></lenmusdoc>"
            , Document::k_format_lmd
        );
        IDocument doc = theDoc.get_document_api();
        IObject child = doc.first_child();
        CHECK( child.is_paragraph() );
        IParagraph para = child.downcast_to_paragraph();

        IObject obj = para.first_child();

        CHECK( obj.is_valid() );
        CHECK( obj.object_id() != 0 );
        CHECK( obj.object_name() == "link" );
        IDocument doc1 = obj.owner_document();
        CHECK( doc.object_id() == doc1.object_id() );

        //can be downcasted
        CHECK( obj.is_link() );
        ILink link = obj.downcast_to_link();
        CHECK( link.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, ilink_0101)
    {
        //@0101. Factory, enum and downcast are ok
        Document theDoc(m_libraryScope);
        IDocument doc = theDoc.get_document_api();

        IObject obj = doc.create_object(k_obj_link);

        CHECK( obj.is_valid() );
        CHECK( obj.object_name() == "link" );
        CHECK( obj.is_link() );
        ILink link = obj.downcast_to_link();
        CHECK( link.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, ilink_0200)
    {
        //@0200. get siblings
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "<lenmusdoc vers='0.0'><content>"
                "<para>"
                    "<link url='This is 1st url'>This is the first link</link>"
                    "<link url='This is 2nd url'>This is the second link</link>"
                    "<txt>Some text</txt>"
                "</para>"
            "<content/></lenmusdoc>"
            , Document::k_format_lmd
        );
        IDocument doc = theDoc.get_document_api();
        IObject child = doc.first_child();
        CHECK( child.is_paragraph() );
        IParagraph para = child.downcast_to_paragraph();
        IObject obj = para.first_child();
        CHECK( obj.is_link() );
        ILink link1 = obj.downcast_to_link();

        obj = link1.previous_sibling();
        CHECK( !obj.is_valid() );

        obj = link1.next_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_link() );
        ILink link2 = obj.downcast_to_link();

        obj = link2.next_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_text_item() );

        obj = link2.previous_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_link() );
        CHECK( obj.object_id() == link1.object_id() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, ilink_0201)
    {
        //@0201. get children
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "<lenmusdoc vers='0.0'><content>"
                "<para>"
                    "<link url='This is 1st url'>"
                        "This is the first chunk of text. "
                        "<txt>Now, the second one. </txt>"
                        "And let's finish."
                    "</link>"
                "</para>"
            "<content/></lenmusdoc>"
            , Document::k_format_lmd
        );
        IDocument doc = theDoc.get_document_api();
        IObject child = doc.first_child();
        CHECK( child.is_paragraph() );
        IParagraph para = child.downcast_to_paragraph();
        IObject obj = para.first_child();
        CHECK( obj.is_link() );
        ILink link = obj.downcast_to_link();

        CHECK( link.num_children() == 3 );

        obj = link.first_child();
        CHECK( obj.is_valid() );
        CHECK( obj.is_text_item() );
//        unique_ptr<ILinkItem> li = obj.downcast_to_text_item();
//        CHECK( li.is_valid() );
//        CHECK( li->text() == "This is the first item" );

        obj = link.last_child();
        CHECK( obj.is_valid() );
        CHECK( obj.is_text_item() );
//        li = obj.downcast_to_text_item();
//        CHECK( li.is_valid() );
//        CHECK( li->text() == "The second item" );

        obj = link.child_at(1);
        CHECK( obj.is_valid() );
        CHECK( obj.is_text_item() );
//        li = obj.downcast_to_text_item();
//        CHECK( li.is_valid() );
//        CHECK( li->text() == "The end of the list" );
    }


    //@ IList ---------------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, ilist_0100)
    {
        //@0100. IObject properties and casting. OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "<lenmusdoc vers='0.0'><content>"
                "<orderedlist>"
                    "<listitem>This is the first item</listitem>"
                    "<listitem>The second item</listitem>"
                    "<listitem>The end of the list</listitem>"
                "</orderedlist>"
            "<content/></lenmusdoc>"
            , Document::k_format_lmd
        );
        IDocument doc = theDoc.get_document_api();
        IObject obj = doc.first_child();

        CHECK( obj.is_valid() );
        CHECK( obj.object_id() != 0 );
        CHECK( obj.object_name() == "list" );
        IDocument doc1 = obj.owner_document();
        CHECK( doc.object_id() == doc1.object_id() );

        //can be downcasted
        CHECK( obj.is_list() );
        IList lst = obj.downcast_to_list();
        CHECK( lst.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, ilist_0101)
    {
        //@0101. Factory, enum and downcast are ok
        Document theDoc(m_libraryScope);
        IDocument doc = theDoc.get_document_api();

        IObject obj = doc.create_object(k_obj_list);

        CHECK( obj.is_valid() );
        CHECK( obj.object_name() == "list" );
        CHECK( obj.is_list() );
        IList lst = obj.downcast_to_list();
        CHECK( lst.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, ilist_0200)
    {
        //@0200. get siblings
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "<lenmusdoc vers='0.0'><content>"
                "<itemizedlist>"
                    "<listitem>This is the first item</listitem>"
                    "<listitem>The second item</listitem>"
                    "<listitem>The end of the list</listitem>"
                "</itemizedlist>"
                "<orderedlist>"
                    "<listitem>One</listitem>"
                    "<listitem>Two</listitem>"
                "</orderedlist>"
                "<para>This is a paragraph</para>"
            "<content/></lenmusdoc>"
            , Document::k_format_lmd
        );
        IDocument doc = theDoc.get_document_api();
        IObject child = doc.first_child();
        CHECK( child.is_list() );
        IList lst1 = child.downcast_to_list();

        IObject obj = lst1.previous_sibling();
        CHECK( !obj.is_valid() );

        obj = lst1.next_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_list() );
        IList lst2 = obj.downcast_to_list();

        obj = lst2.next_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_paragraph() );

        obj = lst2.previous_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_list() );
        CHECK( obj.object_id() == lst1.object_id() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, ilist_0201)
    {
        //@0201. get children
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "<lenmusdoc vers='0.0'><content>"
                "<itemizedlist>"
                    "<listitem>This is the first item</listitem>"
                    "<listitem>The second item</listitem>"
                    "<listitem>The end of the list</listitem>"
                "</itemizedlist>"
            "<content/></lenmusdoc>"
            , Document::k_format_lmd
        );
        IDocument doc = theDoc.get_document_api();
        IObject child = doc.first_child();
        CHECK( child.is_list() );
        IList lst = child.downcast_to_list();

        CHECK( lst.num_children() == 3 );

        IObject obj = lst.first_child();
        CHECK( obj.is_valid() );
        CHECK( obj.is_list_item() );
//        unique_ptr<IListItem> li = obj.downcast_to_list_item();
//        CHECK( li.is_valid() );
//        CHECK( li->text() == "This is the first item" );

        obj = lst.last_child();
        CHECK( obj.is_valid() );
        CHECK( obj.is_list_item() );
//        li = obj.downcast_to_list_item();
//        CHECK( li.is_valid() );
//        CHECK( li->text() == "The second item" );

        obj = lst.child_at(1);
        CHECK( obj.is_valid() );
        CHECK( obj.is_list_item() );
//        li = obj.downcast_to_list_item();
//        CHECK( li.is_valid() );
//        CHECK( li->text() == "The end of the list" );
    }


    //@ IParagraph ----------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, iparagraph_0100)
    {
        //@0100. IObject properties and casting. OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
            "(para#301 (txt \"Hello world\"))"
            "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();
        IObject obj = doc.first_child();

        CHECK( obj.is_valid() );
        CHECK( obj.object_id() == 301 );
        CHECK( obj.object_name() == "paragraph" );
        IDocument doc1 = obj.owner_document();
        CHECK( doc.object_id() == doc1.object_id() );

        //can be downcasted
        CHECK( obj.is_paragraph() );
        IParagraph para = obj.downcast_to_paragraph();
        CHECK( para.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iparagraph_0101)
    {
        //@0101. Factory, enum and downcast are ok
        Document theDoc(m_libraryScope);
        IDocument doc = theDoc.get_document_api();

        IObject obj = doc.create_object(k_obj_paragraph);

        CHECK( obj.is_valid() );
        CHECK( obj.object_name() == "paragraph" );
        CHECK( obj.is_paragraph() );
        IParagraph para = obj.downcast_to_paragraph();
        CHECK( para.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iparagraph_0200)
    {
        //@0200. get siblings
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
            "(para (txt \"Hello world\"))"
            "(para (txt \"Another paragraph\"))"
            "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();
        IObject child = doc.first_child();
        CHECK( child.is_paragraph() );
        IParagraph para1 = child.downcast_to_paragraph();

        IObject obj = para1.previous_sibling();
        CHECK( !obj.is_valid() );

        obj = para1.next_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_paragraph() );
        IParagraph para2 = obj.downcast_to_paragraph();

        obj = para2.next_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_score() );

        obj = para2.previous_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_paragraph() );
        CHECK( obj.object_id() == para1.object_id() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iparagraph_0201)
    {
        //@0201. get children
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "<lenmusdoc vers='0.0'><content>"
                "<para>"
                    "This is a paragraph "
                    "<txt>with three items.</txt>"     //<txt style='bold'>
                    " And the third one."
                "</para>"
            "<content/></lenmusdoc>"
            , Document::k_format_lmd
        );
        IDocument doc = theDoc.get_document_api();
        IObject child = doc.first_child();
        CHECK( child.is_paragraph() );
        IParagraph para = child.downcast_to_paragraph();

        CHECK( para.num_children() == 3 );

        IObject obj = para.first_child();
        CHECK( obj.is_valid() );
        CHECK( obj.is_text_item() );
        ITextItem txt = obj.downcast_to_text_item();
        CHECK( txt.is_valid() );
//        CHECK( txt->text() == "This is a paragraph " );

        obj = para.last_child();
        CHECK( obj.is_valid() );
        CHECK( obj.is_text_item() );
        txt = obj.downcast_to_text_item();
        CHECK( txt.is_valid() );
//        CHECK( txt->text() == " And the third one." );

        obj = para.child_at(1);
        CHECK( obj.is_valid() );
        CHECK( obj.is_text_item() );
        txt = obj.downcast_to_text_item();
        CHECK( txt.is_valid() );
//        CHECK( txt->text() == "with three items." );
    }


    //@ IScore --------------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0100)
    {
        //@0100. object_id()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score#301 (vers 2.0)"
            "(instrument (musicData (clef G)"
            "(n g4 q)"
            ")))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        CHECK( score.object_id() == 301 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0101)
    {
        //@0101. Factory, enum and downcast are ok
        Document theDoc(m_libraryScope);
        IDocument doc = theDoc.get_document_api();

        IObject obj = doc.create_object(k_obj_score);

        CHECK( obj.is_valid() );
        CHECK( obj.object_name() == "score" );
        CHECK( obj.is_score() );
        IScore score = obj.downcast_to_score();
        CHECK( score.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0201)
    {
        //@0201. num_instruments()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        CHECK( score.num_instruments() == 1 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0202)
    {
        //@0202. instrument_at()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)(instrument#402 (musicData (clef G)(n g4 q))))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        IInstrument instr = score.instrument_at(0);

        CHECK( instr.object_id() == 402 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0203)
    {
        //@0203. delete_instrument(). Not in group
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrument instr = score.instrument_at(0);

        score.delete_instrument(instr.object_id());

        CHECK( score.num_instruments() == 0 );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0204)
    {
        //@0204. delete instrument does not remove the group when enough instruments
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)(group P1 P3))"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C2)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrument instr = score.instrument_at(0);

        score.delete_instrument(instr.object_id());

        CHECK( score.num_instruments() == 3 );
        CHECK( score.num_instruments_groups() == 1 );

        //check group integrity
        IInstrGroup group = score.instruments_group_at(0);
        CHECK( group.num_instruments() == 3 );
        IInstrument instr1 = group.first_instrument();
        IInstrument instr2 = group.last_instrument();
        CHECK( instr1.object_id() == 400 );
        CHECK( instr2.object_id() == 600 );

//        cout << test_name() << ", num.instrs=" << group.num_instruments() << endl;
//        ImoInstrGroup* pGrp = group.internal_object();
//        int numInstrs = pGrp->num_instruments();
//        for (int i=0; i < numInstrs; ++i)
//        {
//            ImoInstrument* pInstr = pGrp->instrument(i);
//            cout << "        instr: index=" << i << ", id=" << pInstr->id() << endl;
//        }
//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0205)
    {
        //@0205. delete instrument reduces group size when less instruments
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P1 P3))"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef C2)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrument instr = score.instrument_at(0);

        score.delete_instrument(instr.object_id());

        CHECK( score.num_instruments() == 2 );
        CHECK( score.num_instruments_groups() == 1 );

        //check group integrity
        IInstrGroup group = score.instruments_group_at(0);
        CHECK( group.num_instruments() == 2 );
        IInstrument instr1 = group.first_instrument();
        IInstrument instr2 = group.last_instrument();
        CHECK( instr1.object_id() == 400 );
        CHECK( instr2.object_id() == 500 );

//        cout << test_name() << ", num.instrs=" << group.num_instruments() << endl;
//        ImoInstrGroup* pGrp = group.internal_object();
//        int numInstrs = pGrp->num_instruments();
//        for (int i=0; i < numInstrs; ++i)
//        {
//            ImoInstrument* pInstr = pGrp->instrument(i);
//            cout << "        instr: index=" << i << ", id=" << pInstr->id() << endl;
//        }
//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0206)
    {
        //@0206. delete instrument removes group when less than two instruments
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2)(group P1 P2))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrument instr = score.instrument_at(0);

        //Document* pDoc = doc.internal_object();
        //cout << test_name() << endl << pDoc->to_string(true) << endl;

        CHECK(instr.is_valid());
//        cout << test_name() << ". To execute: instr.object_id()" << endl;
//        cout << test_name() << ". instr.object_id() = " << instr.object_id() << endl;
//        cout << test_name() << ". To execute: delete_instrument()" << endl;
        score.delete_instrument(instr.object_id());

//        cout << test_name() << ". To execute: score.num_instruments()" << endl;
        CHECK( score.num_instruments() == 1 );
//        cout << test_name() << ". To execute: score.num_instruments_groups()" << endl;
        CHECK( score.num_instruments_groups() == 0 );
//        cout << test_name() << ". Test finished. Were is the crash?" << endl;

//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0210)
    {
        //@0210. move up instrument. When first instrument nothing changes
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P1 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrument instr = score.instrument_at(0);

        score.move_up_instrument(instr);

        CHECK( score.num_instruments() == 3 );
        CHECK( score.num_instruments_groups() == 1 );
        CHECK( score.instrument_at(0).object_id() == 300L );
        CHECK( score.instrument_at(1).object_id() == 400L );
        CHECK( score.instrument_at(2).object_id() == 500L );

        //check group
        IInstrGroup group = score.instruments_group_at(0);
        CHECK( group.num_instruments() == 3 );
        CHECK( group.first_instrument().object_id() == 300L );
        CHECK( group.last_instrument().object_id() == 500L );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0211)
    {
        //@0211. move up instrument. OK.
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)(group P2 P4))"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrument instr = score.instrument_at(1);

        score.move_up_instrument(instr);

        CHECK( score.num_instruments() == 4 );
        CHECK( score.num_instruments_groups() == 1 );
        CHECK( score.instrument_at(0).object_id() == 400L );
        CHECK( score.instrument_at(1).object_id() == 300L );
        CHECK( score.instrument_at(2).object_id() == 500L );
        CHECK( score.instrument_at(3).object_id() == 600L );

        //check group
        IInstrGroup group = score.instruments_group_at(0);
        CHECK( group.num_instruments() == 3 );
        CHECK( group.first_instrument().object_id() == 300L );
        CHECK( group.last_instrument().object_id() == 600L );

//        cout << test_name() << ", num.instrs=" << group.num_instruments() << endl;
//        ImoInstrGroup* pGrp = group.internal_object();
//        int numInstrs = pGrp->num_instruments();
//        for (int i=0; i < numInstrs; ++i)
//        {
//            ImoInstrument* pInstr = pGrp->instrument(i);
//            cout << "        instr: index=" << i << ", id=" << pInstr->id() << endl;
//        }
//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0220)
    {
        //@0220. move down instrument. When last instrument nothing changes
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P1 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrument instr = score.instrument_at(2);

        score.move_down_instrument(instr);

        CHECK( score.num_instruments() == 3 );
        CHECK( score.num_instruments_groups() == 1 );
        CHECK( score.instrument_at(0).object_id() == 300L );
        CHECK( score.instrument_at(1).object_id() == 400L );
        CHECK( score.instrument_at(2).object_id() == 500L );

        //check group
        IInstrGroup group = score.instruments_group_at(0);
        CHECK( group.num_instruments() == 3 );
        CHECK( group.first_instrument().object_id() == 300L );
        CHECK( group.last_instrument().object_id() == 500L );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0221)
    {
        //@0221. move down instrument. OK.
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3 P4)(group P2 P4))"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            "(instrument#600 P4 (name \"P4\")(musicData (clef C1)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrument instr = score.instrument_at(2);

        score.move_down_instrument(instr);

        CHECK( score.num_instruments() == 4 );
        CHECK( score.num_instruments_groups() == 1 );
        CHECK( score.instrument_at(0).object_id() == 300L );
        CHECK( score.instrument_at(1).object_id() == 400L );
        CHECK( score.instrument_at(2).object_id() == 600L );
        CHECK( score.instrument_at(3).object_id() == 500L );

        //check group
        IInstrGroup group = score.instruments_group_at(0);
        CHECK( group.num_instruments() == 3 );
        CHECK( group.first_instrument().object_id() == 400L );
        CHECK( group.last_instrument().object_id() == 500L );

//        cout << test_name() << ", num.instrs=" << group.num_instruments() << endl;
//        ImoInstrGroup* pGrp = group.internal_object();
//        int numInstrs = pGrp->num_instruments();
//        for (int i=0; i < numInstrs; ++i)
//        {
//            ImoInstrument* pInstr = pGrp->instrument(i);
//            cout << "        instr: index=" << i << ", id=" << pInstr->id() << endl;
//        }
//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0230)
    {
        //@0230. append_new_instrument()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P2 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        IInstrument instr = score.append_new_instrument();

        CHECK( score.num_instruments() == 4 );
        CHECK( score.num_instruments_groups() == 1 );
        CHECK( score.instrument_at(0).object_id() == 300L );
        CHECK( score.instrument_at(1).object_id() == 400L );
        CHECK( score.instrument_at(2).object_id() == 500L );
        CHECK( score.instrument_at(3).object_id() == instr.object_id() );
        ImoInstrument* pInstr = instr.internal_object();
        CHECK( pInstr->get_instr_id().empty() == false );

        //check group
        IInstrGroup group = score.instruments_group_at(0);
        CHECK( group.num_instruments() == 2 );
        CHECK( group.first_instrument().object_id() == 400L );
        CHECK( group.last_instrument().object_id() == 500L );

//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << ". Source= " << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0301)
    {
        //@0301. create_instruments_group()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        IInstrGroup group = score.create_instruments_group(1, 2);

        CHECK( score.num_instruments() == 3 );

        //check group
        CHECK( score.num_instruments_groups() == 1 );
        IInstrGroup group2 = score.instruments_group_at(0);
        CHECK( group.object_id() == group2.object_id() );
        CHECK( group.num_instruments() == 2 );
        CHECK( group.first_instrument().object_id() == 400L );
        CHECK( group.last_instrument().object_id() == 500L );

//        cout << test_name() << ", num.instrs=" << group.num_instruments() << endl;
//        ImoInstrGroup* pGrp = group.internal_object();
//        int numInstrs = pGrp->num_instruments();
//        for (int i=0; i < numInstrs; ++i)
//        {
//            ImoInstrument* pInstr = pGrp->instrument(i);
//            cout << "        instr: index=" << i << ", id=" << pInstr->id() << endl;
//        }
//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0302)
    {
        //@0302. create_instruments_group(). Invalid arguments
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(instrument#300 P1 (name \"P1\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"P2\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"P3\")(musicData (clef G)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        //out of range
        IInstrGroup group = score.create_instruments_group(2, 3);
        CHECK( !group.is_valid() );
        CHECK( score.num_instruments_groups() == 0 );

        //out of order
        IInstrGroup group2 = score.create_instruments_group(1, 0);
        CHECK( !group2.is_valid() );
        CHECK( score.num_instruments_groups() == 0 );

        //only one instrument
        IInstrGroup group3 = score.create_instruments_group(1, 1);
        CHECK( !group3.is_valid() );
        CHECK( score.num_instruments_groups() == 0 );

//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0310)
    {
        //@0310. delete_instruments_group_at(). Non-existing group
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P2 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        CHECK( score.delete_instruments_group_at(2) == false );

        CHECK( score.num_instruments_groups() == 1 );

//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0311)
    {
        //@0311. delete_instruments_group_at(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P2 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        score.delete_instruments_group_at(0);

        CHECK( score.num_instruments_groups() == 0 );

//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0312)
    {
        //@0312. delete_instruments_group()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P2 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();
        IInstrGroup group = score.instruments_group_at(0);

        score.delete_instruments_group(group);

        CHECK( score.num_instruments_groups() == 0 );

//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0313)
    {
        //@0313. delete_all_instruments_groups()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)"
            "(parts (instrIds P1 P2 P3)(group P1 P2)(group P2 P3))"
            "(instrument#300 P1 (name \"Soprano\")(musicData (clef G)) )"
            "(instrument#400 P2 (name \"Bass\")(musicData (clef F4)) )"
            "(instrument#500 P3 (name \"Flute\")(musicData (clef G)) )"
            ")"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        score.delete_all_instruments_groups();

        CHECK( score.num_instruments_groups() == 0 );

//        Document* pDoc = doc.internal_object();
//        pDoc->end_of_changes();
//        cout << test_name() << pDoc->to_string(true) << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0400)
    {
        //@0400. previous_sibling(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
            "(para (txt \"Hello world\"))"
            "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        IObject obj = score.previous_sibling();

        CHECK( obj.is_valid() );
        CHECK( obj.is_paragraph() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0401)
    {
        //@0401. previous_sibling(). No previous sibling
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
            "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
            "(para (txt \"Hello world\"))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        IObject obj = score.previous_sibling();

        CHECK( !obj.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0402)
    {
        //@0402. next_sibling(). OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
            "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
            "(para (txt \"Hello world\"))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        IObject obj = score.next_sibling();

        CHECK( obj.is_valid() );
        CHECK( obj.is_paragraph() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0403)
    {
        //@0403. next_sibling(). No more siblings
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(lenmusdoc (vers 0.0)(content "
            "(heading 1 (txt \"Hello world\"))"
            "(score (vers 2.0)(instrument (musicData (clef G)(n g4 q))))"
            "))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        IObject obj = score.next_sibling();

        CHECK( !obj.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0500)
    {
        //@500. algorithms. timepos_for(locator)
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        //measure 2, location 64 ==> 128+64 = 192
        MeasureLocator ml(0, 1, 64);
        TimeUnits timepos = score.timepos_for(ml);
//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 192.0) );

        //measure 1, location 0 ==> 0
        MeasureLocator ml2(0, 0, 0);
        timepos = score.timepos_for(ml2);
//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 0.0) );

        //measure 8, location 0 ==> 896
        MeasureLocator ml3(0, 7, 0);
        timepos = score.timepos_for(ml3);
//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 896.0) );

        //measure 4, location 64 ==> 384+64 = 448
        MeasureLocator ml4(0, 3, 64);
        timepos = score.timepos_for(ml4);
//        cout << test_name() << endl;
//        cout << "timepos=" << timepos << endl;
        CHECK( is_equal_time(timepos, 448.0) );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0501)
    {
        //@0501. algorithms. get timepos for(measure/beat)
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0) (instrument (musicData "
            "(clef G)(time 6 8)(n c4 e)(n e4 e)(n g4 e)(n c4 e)(n e4 e)(n g4 e)(barline)"
            "(n c4 e)(n e4 e)(n g4 e)(n c4 e)(n e4 e)(n g4 e)(barline)"
            "(n c4 e)(n e4 e)(n g4 e)(n c4 e)(n e4 e)(n g4 e)(barline)"
            "(n c4 e)(n e4 e)(n g4 e)(n c4 e)(n e4 e)(n g4 e)(barline)"
            "(n c4 e)(n e4 e)(n g4 e)"
            ")))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        //6/8 : measure duration = 32x6 = 192

        //6/8 = two beats, 3 eight notes per beat ==> beat duration = 3x32=96
        //measure 2, beat 3 ==> 1 measure (192) + two beats (2x96=192) = 384
        TimeUnits timepos = score.timepos_for(1, 2, 0);
        CHECK( is_equal_time(timepos, 384.0) );
        //cout << test_name() << ". timepos=" << timepos << endl;
    }

    TEST_FIXTURE(InternalModelApiTestFixture, iscore_0502)
    {
        //@0502. algorithms. locator_for()
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "(score (vers 2.0)(instrument (musicData "
            "(clef G)(time 2 4)(n c4 q)(n e4 q)(barline)"
            "(n e4 q)(n g4 q)(barline)"
            "(n c5 q)(n e5 q)(barline)"
            "(n c4 e)"
            ")))"
        );
        IDocument doc = theDoc.get_document_api();
        IScore score = doc.first_score();

        MeasureLocator ml = score.locator_for(160.0);

//        cout << test_name() << endl;
//        cout << "instr=" << ml.iInstr << ", meas=" << ml.iMeasure << ", loc=" << ml.location << endl;
        CHECK( ml.iInstr == 0 );
        CHECK( ml.iMeasure == 1 );      //measure 1 starts at 128
        CHECK( ml.location == 32.0 );   //160 - 128 = 32
    }


    //@ ITextItem ---------------------------------------------------------------------------

    TEST_FIXTURE(InternalModelApiTestFixture, itextitem_0100)
    {
        //@0100. IObject properties OK
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "<lenmusdoc vers='0.0'><content>"
                "<para>"
                    "This is the first chunk of text. "
                    "<txt>Now, the second one. </txt>"
                    "And let's finish."
                "</para>"
            "<content/></lenmusdoc>"
            , Document::k_format_lmd
        );
        IDocument doc = theDoc.get_document_api();
        IObject child = doc.first_child();
        CHECK( child.is_paragraph() );
        IParagraph para = child.downcast_to_paragraph();

        IObject obj = para.first_child();

        CHECK( obj.is_valid() );
        CHECK( obj.object_id() != 0 );
        CHECK( obj.object_name() == "text" );
        IDocument doc1 = obj.owner_document();
        CHECK( doc.object_id() == doc1.object_id() );

        //can be downcasted
        CHECK( obj.is_text_item() );
        ITextItem txt = obj.downcast_to_text_item();
        CHECK( txt.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, itextitem_0101)
    {
        //@0101. Factory, enum and downcast are ok
        Document theDoc(m_libraryScope);
        IDocument doc = theDoc.get_document_api();

        IObject obj = doc.create_object(k_obj_text_item);

        CHECK( obj.is_valid() );
        CHECK( obj.object_name() == "text" );
        CHECK( obj.is_text_item() );
        ITextItem txt = obj.downcast_to_text_item();
        CHECK( txt.is_valid() );
    }

    TEST_FIXTURE(InternalModelApiTestFixture, itextitem_0200)
    {
        //@0200. get siblings
        Document theDoc(m_libraryScope);
        theDoc.from_string(
            "<lenmusdoc vers='0.0'><content>"
                "<para>"
                    "This is the first chunk of text. "
                    "<txt>Now, the second one. </txt>"
                    "And let's finish."
                "</para>"
            "<content/></lenmusdoc>"
            , Document::k_format_lmd
        );
        IDocument doc = theDoc.get_document_api();
        IObject child = doc.first_child();
        CHECK( child.is_paragraph() );
        IParagraph para = child.downcast_to_paragraph();
        IObject obj = para.first_child();
        CHECK( obj.is_text_item() );
        ITextItem txt1 = obj.downcast_to_text_item();

        obj = txt1.previous_sibling();
        CHECK( !obj.is_valid() );

        obj = txt1.next_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_text_item() );
        ITextItem txt2 = obj.downcast_to_text_item();

        obj = txt2.next_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_text_item() );

        obj = txt2.previous_sibling();
        CHECK( obj.is_valid() );
        CHECK( obj.is_text_item() );
        CHECK( obj.object_id() == txt1.object_id() );
    }

}


