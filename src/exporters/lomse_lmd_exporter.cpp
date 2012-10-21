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

#include "lomse_lmd_exporter.h"

#include <iostream>
#include <iomanip>
#include "lomse_internal_model.h"
#include "lomse_im_note.h"
#include "lomse_ldp_exporter.h"


namespace lomse
{

#define lk_indent_step  3

//=======================================================================================
// LmdGenerator
//=======================================================================================
class LmdGenerator
{
protected:
    LmdExporter* m_pExporter;
    stringstream m_source;
    string m_elementName;
    bool m_fTagOpen;

public:
    LmdGenerator(LmdExporter* pExporter);
    virtual ~LmdGenerator() {}

    virtual string generate_source() = 0;

protected:
    void start_element_no_attribs(const string& name, ImoObj* pImo);
    void start_element_with_attribs(const string& name, ImoObj* pImo);
    void close_start_tag();
    void start_attrib(const string& name);
    void end_attrib();
    void add_attribute(const string& name, const string& value);
    void end_element(bool fInNewLine = true);
    void start_comment();
    void end_comment();
    void empty_line();
    void new_line_and_indent_spaces(bool fStartLine = true);
    void add_source_for(ImoObj* pImo);
    void source_for_base_staffobj(ImoObj* pImo);
    void source_for_base_scoreobj(ImoObj* pImo);
    void source_for_base_contentobj(ImoObj* pImo);
    void source_for_base_imobj(ImoObj* pImo);
    void source_for_auxobj(ImoObj* pImo);
    void increment_indent();
    void decrement_indent();

    void add_duration(stringstream& source, int noteType, int dots);


};

#define k_in_same_line    false
#define k_in_new_line     true

//=======================================================================================
// generators for specific elements
//=======================================================================================


//---------------------------------------------------------------------------------------
class xxxxxxLmdGenerator : public LmdGenerator
{
protected:
    //ImoXXXXX* m_pObj;

public:
    xxxxxxLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        //m_pObj = static_cast<ImoXXXXX*>(pImo);
    }

    string generate_source()
    {
        //start_element_no_attribs("xxxxx", m_pObj);
        end_element();
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class BarlineLmdGenerator : public LmdGenerator
{
protected:
    ImoBarline* m_pObj;

public:
    BarlineLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoBarline*>(pImo);
    }

    string generate_source()
    {
        start_element_no_attribs("barline", m_pObj);
//        add_barline_type();
        source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class ClefLmdGenerator : public LmdGenerator
{
protected:
    ImoClef* m_pObj;

public:
    ClefLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoClef*>(pImo);
    }

    string generate_source()
    {
        start_element_no_attribs("clef", m_pObj);
        add_type();
        source_for_base_staffobj(m_pObj);
        end_element();
        return m_source.str();
    }

protected:

    void add_type()
    {
        m_source << LmdExporter::clef_type_to_ldp( m_pObj->get_clef_type() );
    }

};


//---------------------------------------------------------------------------------------
class ContentObjLmdGenerator : public LmdGenerator
{
protected:
    ImoContentObj* m_pObj;

public:
    ContentObjLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoContentObj*>(pImo);
    }

    string generate_source()
    {
        add_user_location();
        add_attachments();
        source_for_base_imobj(m_pObj);
        return m_source.str();
    }

protected:

    void add_user_location()
    {
        Tenths ux = m_pObj->get_user_location_x();
        if (ux != 0.0f)
            m_source << " (dx " << LmdExporter::float_to_string(ux) << ")";

        Tenths uy = m_pObj->get_user_location_y();
        if (uy != 0.0f)
            m_source << " (dy " << LmdExporter::float_to_string(uy) << ")";
    }

    void add_attachments()
    {
        if (m_pObj->get_num_attachments() > 0)
        {
            increment_indent();
//            std::list<ImoAuxObj*>& attachments = m_pObj->get_attachments();
//            std::list<ImoAuxObj*>::iterator it;
//            for (it = attachments.begin(); it != attachments.end(); ++it)
//            {
//                ImoAuxObj* pAuxObj = *it;
//                if ( pAuxObj->is_relobj() )
//                {
//                    ImRelObj* pRO = dynamic_cast<ImRelObj*)>(pAuxObj);
//
//                    //exclude beams, as source code for them is generted in ImoNote.
//                    //AWARE. This is necessary because LMD parser needs to have beam
//                    //info to crete the note, before it can process any other attachment.
//                    //Therefore, it was decided to generate beam tag before generating
//                    //attachment tags.
//                    if (!pRO->IsBeam())
//                    {
//                        if ( pRO->GetStartNoteRest() == (lmNoteRest*)this )
//                            m_source += pRO->SourceLMD_First(nIndent, fUndoData, (lmNoteRest*)this);
//                        else if ( pRO->GetEndNoteRest() == (lmNoteRest*)this )
//                            m_source += pRO->SourceLMD_Last(nIndent, fUndoData, (lmNoteRest*)this);
//                        else
//                            m_source += pRO->SourceLMD_Middle(nIndent, fUndoData, (lmNoteRest*)this);
//                    }
//                }
//                else if ( pAuxObj->IsRelObX() )
//                {
//                    lmRelObX* pRO = (lmRelObX*)pAuxObj;
//
//                    //exclude beams, as source code for them is generted in ImoNote.
//                    //AWARE. This is necessary because LMD parser needs to have beam
//                    //info to crete the note, before it can process any other attachment.
//                    //Therefore, it was decided to generate beam tag before generating
//                    //attachment tags.
//                    if (!pRO->IsBeam())
//                    {
//                        if (pRO->GetStartSO() == this)
//                            m_source += pRO->SourceLMD_First(nIndent, fUndoData, this);
//                        else if (pRO->GetEndSO() == this)
//                            m_source += pRO->SourceLMD_Last(nIndent, fUndoData, this);
//                        else
//                            m_source += pRO->SourceLMD_Middle(nIndent, fUndoData, this);
//                    }
//                }
//                else
//                    source_for_auxobj(pAuxObj);
//            }
            decrement_indent();
        }
    }
};


//---------------------------------------------------------------------------------------
class ErrorLmdGenerator : public LmdGenerator
{
protected:
    ImoObj* m_pImo;

public:
    ErrorLmdGenerator(ImoObj* pImo, LmdExporter* pExporter)
        : LmdGenerator(pExporter)
        , m_pImo(pImo)
    {
    }

    string generate_source()
    {
        start_element_no_attribs("TODO: ", m_pImo);
        m_source << m_pImo->get_name() << "   type=" << m_pImo->get_obj_type()
                 << ", id=" << m_pImo->get_id();
        end_element(k_in_same_line);
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class ImObjLmdGenerator : public LmdGenerator
{
protected:
    ImoObj* m_pObj;

public:
    ImObjLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = pImo;
    }

    string generate_source()
    {
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class InstrumentLmdGenerator : public LmdGenerator
{
protected:
    ImoInstrument* m_pObj;

public:
    InstrumentLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoInstrument*>(pImo);
    }

    string generate_source()
    {
        start_element_no_attribs("instrument", m_pObj);

        add_num_staves();
        add_midi_info();
        add_name_abbreviation();
        add_music_data();
        end_element();
        return m_source.str();
    }

protected:

    void add_num_staves()
    {
	    //sSource.append(nIndent * lmLMD_INDENT_STEP, _T(' '));
	    //sSource += wxString::Format(_T("(staves %d)\n"), m_pVStaff->GetNumStaves());
     //   int nStaves = m_pVStaff->GetNumStaves();
     //   for (int i=0; i < nStaves; i++)
     //       sSource += m_pVStaff->GetStaff(i+1)->SourceLMD(nIndent, fUndoData);
    }

    void add_midi_info()
    {
	    //sSource.append(nIndent * lmLMD_INDENT_STEP, _T(' '));
	    //sSource += wxString::Format(_T("(infoMIDI %d %d)\n"), m_nMidiInstr, m_nMidiChannel);
    }

    void add_name_abbreviation()
    {
     //   if (m_pName)
     //   {
	    //    sSource.append(nIndent * lmLMD_INDENT_STEP, _T(' '));
     //       sSource += m_pName->SourceLMD(_T("name"), fUndoData);
     //       sSource += _T("\n");
     //   }
     //   if (m_pAbbreviation)
     //   {
	    //    sSource.append(nIndent * lmLMD_INDENT_STEP, _T(' '));
     //       sSource += m_pAbbreviation->SourceLMD(_T("abbrev"), fUndoData);
     //       sSource += _T("\n");
     //   }
    }

    void add_music_data()
    {
        add_source_for( m_pObj->get_musicdata() );
    }
};


//---------------------------------------------------------------------------------------
class KeySignatureLmdGenerator : public LmdGenerator
{
protected:
    ImoKeySignature* m_pObj;

public:
    KeySignatureLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoKeySignature*>(pImo);
    }

    string generate_source()
    {
        start_element_no_attribs("key", m_pObj);

        add_key_type();

        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_key_type()
    {
        switch(m_pObj->get_key_type())
        {
            case k_key_C:   m_source << "C";    break;
            case k_key_G:   m_source << "G";    break;
            case k_key_D:   m_source << "D";    break;
            case k_key_A:   m_source << "A";    break;
            case k_key_E:   m_source << "E";    break;
            case k_key_B:   m_source << "B";    break;
            case k_key_Fs:  m_source << "Fs";   break;
            case k_key_Cs:  m_source << "Cs";   break;
            case k_key_Cf:  m_source << "Cf";   break;
            case k_key_Gf:  m_source << "Gf";   break;
            case k_key_Df:  m_source << "Df";   break;
            case k_key_Af:  m_source << "Af";   break;
            case k_key_Ef:  m_source << "Ef";   break;
            case k_key_Bf:  m_source << "Bf";   break;
            case k_key_F:   m_source << "F";    break;
            case k_key_a:   m_source << "a";    break;
            case k_key_e:   m_source << "e";    break;
            case k_key_b:   m_source << "b";    break;
            case k_key_fs:  m_source << "fs";   break;
            case k_key_cs:  m_source << "cs";   break;
            case k_key_gs:  m_source << "gs";   break;
            case k_key_ds:  m_source << "ds";   break;
            case k_key_as:  m_source << "as";   break;
            case k_key_af:  m_source << "af";   break;
            case k_key_ef:  m_source << "ef";   break;
            case k_key_bf:  m_source << "bf";   break;
            case k_key_f:   m_source << "f";    break;
            case k_key_c:   m_source << "c";    break;
            case k_key_g:   m_source << "g";    break;
            case k_key_d:   m_source << "d";    break;
            default:                            break;
        }
    }

};


//---------------------------------------------------------------------------------------
class LenmusdocLmdGenerator : public LmdGenerator
{
protected:
    ImoDocument* m_pObj;

public:
    LenmusdocLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoDocument*>(pImo);
    }

    string generate_source()
    {
        m_source << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        start_element_with_attribs("lenmusdoc", m_pObj);
        add_attribute("vers", m_pObj->get_version());
        add_attribute("language", m_pObj->get_language());
        close_start_tag();
        add_comment();
        add_content();
        end_element();
        return m_source.str();
    }

protected:

    void add_comment()
    {
        start_comment();
        m_source << "LMD file generated by Lomse, version ";
//        m_source << wxGetApp().GetVersionNumber();
//        m_source << ". Date: ";
//        m_source << (wxDateTime::Now()).Format(_T("%Y-%m-%d"));
        end_comment();
    }

    void add_content()
    {
        ImoContent* pContent = m_pObj->get_content();
        start_element_no_attribs("content", pContent);
        int numItems = m_pObj->get_num_content_items();
        for (int i=0; i < numItems; i++)
        {
            m_source << " ";
            add_source_for( m_pObj->get_content_item(i) );
        }
        end_element();
    }

};


//---------------------------------------------------------------------------------------
class MusicDataLmdGenerator : public LmdGenerator
{
protected:
    ImoMusicData* m_pObj;

public:
    MusicDataLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoMusicData*>(pImo);
    }

    string generate_source()
    {
        start_element_no_attribs("musicData", m_pObj);
        add_staffobjs();
        empty_line();
        end_element();
        return m_source.str();
    }

protected:

    void add_staffobjs()
    {
        ImoObj::children_iterator it = m_pObj->begin();
        bool fNewMeasure = true;
        int nMeasure = 0;
        while (it != m_pObj->end())
        {
            if (fNewMeasure)
            {
                empty_line();
                start_comment();
                m_source << "measure " << nMeasure++;
                end_comment();
                fNewMeasure = false;
            }

            add_source_for(*it);
            fNewMeasure = (*it)->is_barline();
            ++it;
        }
      //  //iterate over the collection of StaffObjs, ordered by voice.
      //  //Measures must be processed one by one
      //  for (int nMeasure=1; nMeasure <= m_cStaffObjs.GetNumMeasures(); nMeasure++)
      //  {
      //      //add comment to separate measures
      //      sSource += _T("\n");
      //      sSource.append(nIndent * lmLMD_INDENT_STEP, _T(' '));
      //      sSource += wxString::Format(_T("//Measure %d\n"), nMeasure);

      //      int nNumVoices = m_cStaffObjs.GetNumVoicesInMeasure(nMeasure);
      //      int nVoice = 1;
      //      while (!m_cStaffObjs.IsVoiceUsedInMeasure(nVoice, nMeasure) &&
      //          nVoice <= lmMAX_VOICE)
      //      {
      //          nVoice++;
      //      }
      //      int nVoicesProcessed = 1;   //voice 0 is automatically processed
		    //lmBarline* pBL = (lmBarline*)NULL;
      //      bool fGoBack = false;
		    //float rTime = 0.0f;
      //      while (true)
      //      {
      //          lmSOIterator* pIT = m_cStaffObjs.CreateIterator();
      //          pIT->AdvanceToMeasure(nMeasure);
      //          while(!pIT->ChangeOfMeasure() && !pIT->EndOfCollection())
      //          {
      //              lmStaffObj* pSO = pIT->GetCurrent();
      //              //voice 0 staffobjs go with first voice if more than one voice
				  //  if (!pSO->IsBarline())
				  //  {
					 //   if (nVoicesProcessed == 1)
					 //   {
						//    if (!pSO->IsNoteRest() || ((lmNoteRest*)pSO)->GetVoice() == nVoice)
						//    {
						//	    LMD_AddShitTimeTagIfNeeded(sSource, nIndent, lmGO_FWD, rTime, pSO);
						//	    sSource += pSO->SourceLMD(nIndent, fUndoData);
						//	    rTime = LMD_AdvanceTimeCounter(pSO);
						//    }
					 //   }
					 //   else
						//    if (pSO->IsNoteRest() && ((lmNoteRest*)pSO)->GetVoice() == nVoice)
						//    {
						//	    LMD_AddShitTimeTagIfNeeded(sSource, nIndent, lmGO_FWD, rTime, pSO);
						//	    sSource += pSO->SourceLMD(nIndent, fUndoData);
						//	    rTime = LMD_AdvanceTimeCounter(pSO);
						//    }
				  //  }
				  //  else
					 //   pBL = (lmBarline*)pSO;

      //              pIT->MoveNext();
      //          }
      //          delete pIT;

      //          //check if more voices
      //          if (++nVoicesProcessed >= nNumVoices) break;
      //          nVoice++;
      //          while (!m_cStaffObjs.IsVoiceUsedInMeasure(nVoice, nMeasure) &&
      //              nVoice <= lmMAX_VOICE)
      //          {
      //              nVoice++;
      //          }
      //          wxASSERT(nVoice <= lmMAX_VOICE);

      //          //there are more voices. Add (goBak) tag
      //          fGoBack = true;
      //          sSource += _T("\n");
      //          sSource.append(nIndent * lmLMD_INDENT_STEP, _T(' '));
      //          sSource += _T("(goBack start)\n");
      //          rTime = 0.0f;
      //      }

      //      //if goBack added, add a goFwd to ensure that we are at end of measure
      //      if (fGoBack)
      //      {
      //          sSource.append(nIndent * lmLMD_INDENT_STEP, _T(' '));
      //          sSource += _T("(goFwd end)\n");
      //      }

		    ////add barline, if present
		    //if (pBL)
			   // sSource += pBL->SourceLMD(nIndent, fUndoData);

      //  }

    }

};


//---------------------------------------------------------------------------------------
class NoteLmdGenerator : public LmdGenerator
{
protected:
    ImoNote* m_pObj;

public:
    NoteLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoNote*>(pImo);
    }

    string generate_source()
    {
        start_element_no_attribs("note", m_pObj);
        add_pitch();
        add_duration(m_source, m_pObj->get_note_type(), m_pObj->get_dots());
        source_for_base_staffobj(m_pObj);
        end_element();
        return m_source.str();
    }

protected:

    void add_pitch()
    {
        start_element_no_attribs("pitch", NULL);
        static const string sNoteName[7] = { "c",  "d", "e", "f", "g", "a", "b" };
        static const string sOctave[13] = { "0",  "1", "2", "3", "4", "5", "6",
                                            "7", "8", "9", "10", "11", "12"  };

        if (m_pObj->get_step() == k_no_pitch)
        {
            m_source << "*";
            end_element(k_in_same_line);
            return;
        }

        EAccidentals acc = m_pObj->get_notated_accidentals();
        switch(acc)
        {
            case k_invalid_accidentals:     break;
            case k_no_accidentals:          break;
            case k_sharp:                   m_source << "+";  break;
            case k_sharp_sharp:             m_source << "++";  break;
            case k_double_sharp:            m_source << "x";  break;
            case k_natural_sharp:           m_source << "=+";  break;
            case k_flat:                    m_source << "-";  break;
            case k_flat_flat:               m_source << "--";  break;
            case k_natural_flat:            m_source << "=-";  break;
            case k_natural:                 m_source << "=";   break;
            default:                        break;
        }

        m_source << sNoteName[m_pObj->get_step()];
        m_source << sOctave[m_pObj->get_octave()];
        end_element(k_in_same_line);
    }

};


//---------------------------------------------------------------------------------------
class RestLmdGenerator : public LmdGenerator
{
protected:
    ImoRest* m_pObj;

public:
    RestLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoRest*>(pImo);
    }

    string generate_source()
    {
        start_element_no_attribs("rest", m_pObj);
        add_duration(m_source, m_pObj->get_note_type(), m_pObj->get_dots());
        source_for_base_staffobj(m_pObj);
        end_element();
        return m_source.str();
    }

};


//---------------------------------------------------------------------------------------
class ScoreLmdGenerator : public LmdGenerator
{
protected:
    ImoScore* m_pObj;

public:
    ScoreLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScore*>(pImo);
    }

    string generate_source()
    {
        int format = m_pExporter->get_score_format();
        switch(format)
        {
            case LmdExporter::k_format_lms:
                return generate_ldp();
            case LmdExporter::k_format_lmd:
                return generate_lmd();
            case LmdExporter::k_format_musicxml:
                return generate_musicxml();
            default:
            {
                stringstream s;
                s << "[ScoreLmdGenerator::generate_source] Invalid score format. Value="
                  << format;
                throw std::runtime_error(s.str());
            }
        }
    }

protected:

    string generate_ldp()
    {
        start_element_no_attribs("ldpmusic", m_pObj);

        LdpExporter exporter;
        exporter.set_indent( m_pExporter->get_indent() );
        m_source << exporter.get_source(m_pObj);

        end_element();
        return m_source.str();
    }

    string generate_musicxml()
    {
//        start_element_no_attribs("musicxml", m_pObj);
//
//        MusicXmlExporter exporter;
//        exporter.set_indent( m_pExporter->get_indent() );
//        m_source << exporter.get_source(m_pObj);
//
//        end_element();

        //TODO: MusicXmlExporter
        start_element_no_attribs("TODO: MusicXml exporter", m_pObj);
        end_element();
        return m_source.str();
    }

    string generate_lmd()
    {
        start_element_no_attribs("score", m_pObj);
        add_version();
        add_undo_data();
        add_creation_mode();
        add_styles();
        add_titles();
        add_page_layout();
        add_system_layout();
        add_cursor();
        add_options();
        add_instruments_and_groups();
        end_element();
        return m_source.str();
    }

    void add_version()
    {
        m_source << "(vers ";
        m_source << m_pObj->get_version();
        m_source << ")";
    }

    void add_undo_data()
    {
    ////ID counter value for undo/redo
    //if (fUndoData)
    //    sSource += wxString::Format(_T("   (undoData (idCounter  %d))\n"), m_nCounterID);
    }

    void add_creation_mode()
    {
//    //creation mode
//    if (!m_sCreationModeName.empty())
//    {
//        m_source << _T("   (creationMode ");
//        m_source << m_sCreationModeName;
//        m_source << _T(" ");
//        m_source << m_sCreationModeVers;
//        m_source << _T(")\n");
//    }
    }

    void add_styles()
    {
//    //styles
//    m_source << m_TextStyles.SourceLMD(1, fUndoData);
    }

    void add_titles()
    {
//    //titles and other attached auxobjs
//    if (m_pAuxObjs)
//    {
//	    for (int i=0; i < (int)m_pAuxObjs->size(); i++)
//	    {
//		    m_source << (*m_pAuxObjs)[i]->SourceLMD(1, fUndoData);
//	    }
//    }
    }

    void add_page_layout()
    {
//    //first section layout info
//    //TODO: sections
//    //int nSection = 0;
//    {
//        //page layout info
//#if 0
//		std::list<lmPageInfo*>::iterator it;
//		for (it = m_PagesInfo.begin(); it != m_PagesInfo.end(); ++it)
//            m_source << (*it)->SourceLMD(1, fUndoData);
//#else
//        lmPageInfo* pPageInfo = m_PagesInfo.front();
//        m_source << pPageInfo->SourceLMD(1, fUndoData);
//#endif
//        //first system and other systems layout info
//        m_source << m_SystemsInfo.front()->SourceLMD(1, true, fUndoData);
//        m_source << m_SystemsInfo.back()->SourceLMD(1, false, fUndoData);
//    }
    }

    void add_system_layout()
    {
    }

    void add_cursor()
    {
//    //score cursor information
//    if (fUndoData)
//    {
//        lmCursorState tState = m_SCursor.GetState();
//        m_source << wxString::Format(_T("   (cursor %d %d %s %d)\n"),
//                        tState.GetNumInstr(),
//                        tState.GetNumStaff(),
//		                DoubleToStr((double)tState.GetTimepos(), 2).c_str(),
//                        tState.GetObjID() );
//    }
    }

    void add_options()
    {
    //    //options with non-default values
    //    bool fBoolValue;
    //    long nLongValue;
    //    double rDoubleValue;
    //
    //    //bool
    //    for (int i=0; i < (int)(sizeof(m_BoolOptions)/sizeof(lmBoolOption)); i++)
    //    {
    //        fBoolValue = GetOptionBool(m_BoolOptions[i].sOptName);
    //        if (fBoolValue != m_BoolOptions[i].fBoolValue)
    //            m_source << wxString::Format(_T("   (opt %s %s)\n"), m_BoolOptions[i].sOptName.c_str(),
    //                                        (fBoolValue ? _T("true") : _T("false")) );
    //    }
    //
    //    //long
    //    for (int i=0; i < (int)(sizeof(m_LongOptions)/sizeof(lmLongOption)); i++)
    //    {
    //        nLongValue = GetOptionLong(m_LongOptions[i].sOptName);
    //        if (nLongValue != m_LongOptions[i].nLongValue)
    //            m_source << wxString::Format(_T("   (opt %s %d)\n"), m_LongOptions[i].sOptName.c_str(),
    //                                        nLongValue );
    //    }
    //
    //    //double
    //    for (int i=0; i < (int)(sizeof(m_DoubleOptions)/sizeof(lmDoubleOption)); i++)
    //    {
    //        rDoubleValue = GetOptionDouble(m_DoubleOptions[i].sOptName);
    //        if (rDoubleValue != m_DoubleOptions[i].rDoubleValue)
    //            m_source << wxString::Format(_T("   (opt %s %s)\n"), m_DoubleOptions[i].sOptName.c_str(),
    //                                        DoubleToStr(rDoubleValue, 4).c_str() );
    //    }
    }

    void add_instruments_and_groups()
    {
        int numInstr = m_pObj->get_num_instruments();
        for (int i=0; i < numInstr; ++i)
            add_source_for( m_pObj->get_instrument(i) );
    }


};


//---------------------------------------------------------------------------------------
class ScoreObjLmdGenerator : public LmdGenerator
{
protected:
    ImoScoreObj* m_pObj;

public:
    ScoreObjLmdGenerator(ImoObj* pImo, LmdExporter* pExporter)
        : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScoreObj*>(pImo);
    }

    string generate_source()
    {
        add_visible();
        add_color();
        source_for_base_contentobj(m_pObj);
        return m_source.str();
    }

protected:

    void add_visible()
    {
        if (!m_pObj->is_visible())
            m_source << " (visible no)";
    }

    void add_color()
    {
        //color (if not black)
        Color color = m_pObj->get_color();
        if (color.r != 0 || color.g != 0  || color.b != 0 || color.a != 255)
            m_source << " (color " << LmdExporter::color_to_ldp(color) << ")";
    }
};


//---------------------------------------------------------------------------------------
class StaffObjLmdGenerator : public LmdGenerator
{
protected:
    ImoStaffObj* m_pObj;

public:
    StaffObjLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoStaffObj*>(pImo);
    }

    string generate_source()
    {
        add_staff_num();
        source_for_base_scoreobj(m_pObj);
        return m_source.str();
    }

protected:

    void add_staff_num()
    {
        if (!m_pObj->is_key_signature()            //KS, TS & barlines are common to all staves.
            && !m_pObj->is_time_signature()
            && !m_pObj->is_barline() )
        {
//            m_source << " p" << (m_pObj->get_staff() + 1);
            //m_source << "<staff>" << (m_pObj->get_staff() + 1) << "</staff>";
            start_element_no_attribs("staff", NULL);
            m_source << (m_pObj->get_staff() + 1);
            end_element(k_in_same_line);
        }
    }

};


//---------------------------------------------------------------------------------------
class SpacerLmdGenerator : public LmdGenerator
{
protected:
    ImoSpacer* m_pObj;

public:
    SpacerLmdGenerator(ImoObj* pImo, LmdExporter* pExporter) : LmdGenerator(pExporter)
    {
        m_pObj = static_cast<ImoSpacer*>(pImo);
    }

    string generate_source()
    {
        start_element_no_attribs("spacer", m_pObj);
        //TODO: details
        end_element(k_in_same_line);
        return m_source.str();
    }
};



//=======================================================================================
// LmdGenerator implementation
//=======================================================================================
LmdGenerator::LmdGenerator(LmdExporter* pExporter)
    : m_pExporter(pExporter)
    , m_fTagOpen(false)
{
}

//---------------------------------------------------------------------------------------
void LmdGenerator::start_element_no_attribs(const string& name, ImoObj* pImo)
{
    start_element_with_attribs(name, pImo);
    close_start_tag();
}

//---------------------------------------------------------------------------------------
void LmdGenerator::start_element_with_attribs(const string& name, ImoObj* pImo)
{
    new_line_and_indent_spaces();
    m_source << "<" << name;
    m_elementName = name;
    if (m_pExporter->get_add_id())
        m_source << " id=\"" << std::dec << pImo->get_id() << "\"";
    increment_indent();
}

//---------------------------------------------------------------------------------------
void LmdGenerator::close_start_tag()
{
    m_source << ">";
}

//---------------------------------------------------------------------------------------
void LmdGenerator::start_attrib(const string& name)
{
    m_source << " " << name << "=\"";
}

//---------------------------------------------------------------------------------------
void LmdGenerator::end_attrib()
{
    m_source << "\"";
}

//---------------------------------------------------------------------------------------
void LmdGenerator::add_attribute(const string& name, const string& value)
{
    start_attrib(name);
    m_source << value;
    end_attrib();
}

//---------------------------------------------------------------------------------------
void LmdGenerator::end_element(bool fInNewLine)
{
    decrement_indent();
    if (fInNewLine)
        new_line_and_indent_spaces();
    m_source << "</" << m_elementName << ">";
}

//---------------------------------------------------------------------------------------
void LmdGenerator::start_comment()
{
    new_line_and_indent_spaces();
    m_source << "<!-- ";
}

//---------------------------------------------------------------------------------------
void LmdGenerator::end_comment()
{
    m_source << " -->";
}

//---------------------------------------------------------------------------------------
void LmdGenerator::empty_line()
{
    m_source.clear();
    m_source << endl;
}
//---------------------------------------------------------------------------------------
void LmdGenerator::new_line_and_indent_spaces(bool fStartLine)
{
    m_source.clear();
    if (fStartLine)
        m_source << endl;
    int indent = m_pExporter->get_indent() * lk_indent_step;
    while (indent > 0)
    {
        m_source << " ";
        indent--;
    }
}

//---------------------------------------------------------------------------------------
void LmdGenerator::add_source_for(ImoObj* pImo)
{
    m_source << m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void LmdGenerator::source_for_base_staffobj(ImoObj* pImo)
{
    StaffObjLmdGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LmdGenerator::source_for_base_scoreobj(ImoObj* pImo)
{
    ScoreObjLmdGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LmdGenerator::source_for_base_contentobj(ImoObj* pImo)
{
    ContentObjLmdGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LmdGenerator::source_for_base_imobj(ImoObj* pImo)
{
    increment_indent();
    ImObjLmdGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
    decrement_indent();
}

//---------------------------------------------------------------------------------------
void LmdGenerator::source_for_auxobj(ImoObj* pImo)
{
    m_source <<  m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void LmdGenerator::increment_indent()
{
    m_pExporter->increment_indent();
}

//---------------------------------------------------------------------------------------
void LmdGenerator::decrement_indent()
{
    m_pExporter->decrement_indent();
}

//---------------------------------------------------------------------------------------
void LmdGenerator::add_duration(stringstream& source, int noteType, int dots)
{
    start_element_no_attribs("type", NULL);
    switch(noteType)
    {
        case k_longa:   source << "l";  break;
        case k_breve:   source << "b";  break;
        case k_whole:   source << "w";  break;
        case k_half:    source << "h";  break;
        case k_quarter: source << "q";  break;
        case k_eighth:  source << "e";  break;
        case k_16th:    source << "s";  break;
        case k_32th:    source << "t";  break;
        case k_64th:    source << "i";  break;
        case k_128th:   source << "o";  break;
        case k_256th:   source << "f";  break;
        default:                        break;
    }

    while (dots > 0)
    {
        source << ".";
        --dots;
    }
    end_element(k_in_same_line);
}


//=======================================================================================
// LmdExporter implementation
//=======================================================================================
LmdExporter::LmdExporter()
    : m_nIndent(0)
    , m_fAddId(false)
    , m_scoreFormat(k_format_lmd)
{
}

//---------------------------------------------------------------------------------------
LmdExporter::~LmdExporter()
{
}

//---------------------------------------------------------------------------------------
string LmdExporter::get_source(ImoObj* pImo)
{
    LmdGenerator* pGen = new_generator(pImo);
    string source = pGen->generate_source();
    delete pGen;
    return source;
}

//---------------------------------------------------------------------------------------
LmdGenerator* LmdExporter::new_generator(ImoObj* pImo)
{
    //factory method

    switch(pImo->get_obj_type())
    {
        case k_imo_barline:         return LOMSE_NEW BarlineLmdGenerator(pImo, this);
//        case k_imo_beam_dto:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_bezier_info:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
        case k_imo_clef:            return LOMSE_NEW ClefLmdGenerator(pImo, this);
//        case k_imo_color_dto:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_instr_group:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_midi_info:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_option:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_system_info:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_tie_dto:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_tuplet_dto:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
        case k_imo_document:        return LOMSE_NEW LenmusdocLmdGenerator(pImo, this);
//        case k_imo_content:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
        case k_imo_instrument:      return LOMSE_NEW InstrumentLmdGenerator(pImo, this);
//        case k_imo_score:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
        case k_imo_key_signature:   return LOMSE_NEW KeySignatureLmdGenerator(pImo, this);
//        case k_imo_time_signature:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
        case k_imo_music_data:      return LOMSE_NEW MusicDataLmdGenerator(pImo, this);
        case k_imo_note:            return LOMSE_NEW NoteLmdGenerator(pImo, this);
        case k_imo_rest:            return LOMSE_NEW RestLmdGenerator(pImo, this);
//        case k_imo_go_back_fwd:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_metronome_mark:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_system_break:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_spacer:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_figured_bass:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
        case k_imo_score:           return LOMSE_NEW ScoreLmdGenerator(pImo, this);
        case k_imo_spacer:          return LOMSE_NEW SpacerLmdGenerator(pImo, this);
//        case k_imo_score_text:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_fermata:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_tie:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_beam:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
//        case k_imo_tuplet:         return LOMSE_NEW XxxxxxxLmdGenerator(pImo, this);
        default:
            return new ErrorLmdGenerator(pImo, this);
    }
}



// static methods

//---------------------------------------------------------------------------------------
string LmdExporter::clef_type_to_ldp(int clefType)
{
    //AWARE: indexes in correspondence with enum k_clef__type
    static const string name[] = {
        "G",
        "F4",
        "F3",
        "C1",
        "C2",
        "C3",
        "C4",
        "percussion",
        "C5",
        "F5",
        "G1",
        "8_G",     //8 above
        "G_8",     //8 below
        "8_F4",    //8 above
        "F4_8",    //8 below
        "15_G2",   //15 above
        "G2_15",   //15 below
        "15_F4",   //15 above
        "F4_15",   //15 below
    };
    static const string undefined = "undefined";


    if (clefType == k_clef_undefined)
        return undefined;
    else
        return name[clefType];
}

//---------------------------------------------------------------------------------------
string LmdExporter::color_to_ldp(Color color)
{
    stringstream source;
    source << "#";
    int r = color.r;
    int g = color.g;
    int b = color.b;
    int a = color.a;
    source << std::hex << setfill('0') << setw(2) << r;
    source << std::hex << setfill('0') << setw(2) << g;
    source << std::hex << setfill('0') << setw(2) << b;
    source << std::hex << setfill('0') << setw(2) << a;
    return source.str();
}

//---------------------------------------------------------------------------------------
string LmdExporter::float_to_string(float num)
{
    return "(TODO: float_to_string)";
}



}  //namespace lomse
