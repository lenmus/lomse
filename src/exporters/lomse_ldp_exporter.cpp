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

#include "lomse_ldp_exporter.h"

#include <iostream>
#include <iomanip>
#include "lomse_internal_model.h"
#include "lomse_im_note.h"


namespace lomse
{

#define lml_LDP_INDENT_STEP  3

//=======================================================================================
// LdpGenerator
//=======================================================================================
class LdpGenerator
{
protected:
    LdpExporter* m_pExporter;
    stringstream m_source;

public:
    LdpGenerator(LdpExporter* pExporter) : m_pExporter(pExporter) {}

    virtual std::string generate_source() = 0;

protected:
    void start_element();
    void end_element();
    void add_indent_spaces();
    void add_element_name(const std::string& name, ImoObj* pImo);
    void add_source_for(ImoObj* pImo);
    void source_for_base_staffobj(ImoObj* pImo);
    void source_for_base_scoreobj(ImoObj* pImo);
    void source_for_base_contentobj(ImoObj* pImo);
    void source_for_base_imobj(ImoObj* pImo);
    void source_for_auxobj(ImoObj* pImo);
    void increment_indent();
    void decrement_indent();

};



//=======================================================================================
// generators for specific elements
//=======================================================================================


//---------------------------------------------------------------------------------------
class xxxxxxLdpGenerator : public LdpGenerator
{
protected:
    //ImoXXXXX* m_pObj;

public:
    xxxxxxLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        //m_pObj = static_cast<ImoXXXXX*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        //add_element_name("xxxxx", m_pObj);
        end_element();
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class BarlineLdpGenerator : public LdpGenerator
{
protected:
    ImoBarline* m_pObj;

public:
    BarlineLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoBarline*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("barline", m_pObj);
//        add_barline_type();
        source_for_base_staffobj(m_pObj);
        end_element();
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class ClefLdpGenerator : public LdpGenerator
{
protected:
    ImoClef* m_pObj;

public:
    ClefLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoClef*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("clef", m_pObj);
        add_type();
        source_for_base_staffobj(m_pObj);
        end_element();
        return m_source.str();
    }

protected:

    void add_type()
    {
        m_source << LdpExporter::clef_type_to_ldp( m_pObj->get_clef_type() );
    }

};


//---------------------------------------------------------------------------------------
class ContentObjLdpGenerator : public LdpGenerator
{
protected:
    ImoContentObj* m_pObj;

public:
    ContentObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoContentObj*>(pImo);
    }

    std::string generate_source()
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
            m_source << " (dx " << LdpExporter::float_to_string(ux) << ")";

        Tenths uy = m_pObj->get_user_location_y();
        if (uy != 0.0f)
            m_source << " (dy " << LdpExporter::float_to_string(uy) << ")";
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
//                    //AWARE. This is necessary because LDP parser needs to have beam
//                    //info to crete the note, before it can process any other attachment.
//                    //Therefore, it was decided to generate beam tag before generating
//                    //attachment tags.
//                    if (!pRO->IsBeam())
//                    {
//                        if ( pRO->GetStartNoteRest() == (lmNoteRest*)this )
//                            m_source += pRO->SourceLDP_First(nIndent, fUndoData, (lmNoteRest*)this);
//                        else if ( pRO->GetEndNoteRest() == (lmNoteRest*)this )
//                            m_source += pRO->SourceLDP_Last(nIndent, fUndoData, (lmNoteRest*)this);
//                        else
//                            m_source += pRO->SourceLDP_Middle(nIndent, fUndoData, (lmNoteRest*)this);
//                    }
//                }
//                else if ( pAuxObj->IsRelObX() )
//                {
//                    lmRelObX* pRO = (lmRelObX*)pAuxObj;
//
//                    //exclude beams, as source code for them is generted in ImoNote.
//                    //AWARE. This is necessary because LDP parser needs to have beam
//                    //info to crete the note, before it can process any other attachment.
//                    //Therefore, it was decided to generate beam tag before generating
//                    //attachment tags.
//                    if (!pRO->IsBeam())
//                    {
//                        if (pRO->GetStartSO() == this)
//                            m_source += pRO->SourceLDP_First(nIndent, fUndoData, this);
//                        else if (pRO->GetEndSO() == this)
//                            m_source += pRO->SourceLDP_Last(nIndent, fUndoData, this);
//                        else
//                            m_source += pRO->SourceLDP_Middle(nIndent, fUndoData, this);
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
class ErrorLdpGenerator : public LdpGenerator
{
public:
    ErrorLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter) {}

    std::string generate_source()
    {
        m_source.clear();
        m_source << "(TODO: Add this element to LdpExporter::new_generator)";
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class ImObjLdpGenerator : public LdpGenerator
{
protected:
    ImoObj* m_pObj;

public:
    ImObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = pImo;
    }

    std::string generate_source()
    {
        return m_source.str();
    }
};


//---------------------------------------------------------------------------------------
class InstrumentLdpGenerator : public LdpGenerator
{
protected:
    ImoInstrument* m_pObj;

public:
    InstrumentLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoInstrument*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("instrument", m_pObj);

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
	    //sSource.append(nIndent * lmLDP_INDENT_STEP, _T(' '));
	    //sSource += wxString::Format(_T("(staves %d)\n"), m_pVStaff->GetNumStaves());
     //   int nStaves = m_pVStaff->GetNumStaves();
     //   for (int i=0; i < nStaves; i++)
     //       sSource += m_pVStaff->GetStaff(i+1)->SourceLDP(nIndent, fUndoData);
    }

    void add_midi_info()
    {
	    //sSource.append(nIndent * lmLDP_INDENT_STEP, _T(' '));
	    //sSource += wxString::Format(_T("(infoMIDI %d %d)\n"), m_nMidiInstr, m_nMidiChannel);
    }

    void add_name_abbreviation()
    {
     //   if (m_pName)
     //   {
	    //    sSource.append(nIndent * lmLDP_INDENT_STEP, _T(' '));
     //       sSource += m_pName->SourceLDP(_T("name"), fUndoData);
     //       sSource += _T("\n");
     //   }
     //   if (m_pAbbreviation)
     //   {
	    //    sSource.append(nIndent * lmLDP_INDENT_STEP, _T(' '));
     //       sSource += m_pAbbreviation->SourceLDP(_T("abbrev"), fUndoData);
     //       sSource += _T("\n");
     //   }
    }

    void add_music_data()
    {
        add_source_for( m_pObj->get_musicdata() );
    }
};


//---------------------------------------------------------------------------------------
class KeySignatureLdpGenerator : public LdpGenerator
{
protected:
    ImoKeySignature* m_pObj;

public:
    KeySignatureLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoKeySignature*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("key", m_pObj);

        add_key_type();

        end_element();
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
class LenmusdocLdpGenerator : public LdpGenerator
{
protected:
    ImoDocument* m_pObj;

public:
    LenmusdocLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoDocument*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("lenmusdoc", m_pObj);
        add_version();
        add_comment();
        add_content();
        end_element();
        return m_source.str();
    }

protected:

    void add_version()
    {
        m_source << "(vers ";
        m_source << m_pObj->get_version();
        m_source << ") ";
    }

    void add_comment()
    {
        //m_source << "   //LDP file generated by LenMus, version ";
        //m_source << wxGetApp().GetVersionNumber();
        //m_source << ". Date: ";
        //m_source << (wxDateTime::Now()).Format(_T("%Y-%m-%d"));
        //m_source << _T("\n");
    }

    void add_content()
    {
        m_source << "(content";
        int numItems = m_pObj->get_num_content_items();
        for (int i=0; i < numItems; i++)
        {
            m_source << " ";
            add_source_for( m_pObj->get_content_item(i) );
        }
        m_source << ")";
    }

};


//---------------------------------------------------------------------------------------
class MusicDataLdpGenerator : public LdpGenerator
{
protected:
    ImoMusicData* m_pObj;

public:
    MusicDataLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoMusicData*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("musicData", m_pObj);

        add_staffobjs();

        end_element();
        return m_source.str();
    }

protected:

    void add_staffobjs()
    {
        ImoObj::children_iterator it = m_pObj->begin();
        while (it != m_pObj->end())
        {
            add_source_for(*it);
            ++it;
        }
      //  //iterate over the collection of StaffObjs, ordered by voice.
      //  //Measures must be processed one by one
      //  for (int nMeasure=1; nMeasure <= m_cStaffObjs.GetNumMeasures(); nMeasure++)
      //  {
      //      //add comment to separate measures
      //      sSource += _T("\n");
      //      sSource.append(nIndent * lmLDP_INDENT_STEP, _T(' '));
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
						//	    LDP_AddShitTimeTagIfNeeded(sSource, nIndent, lmGO_FWD, rTime, pSO);
						//	    sSource += pSO->SourceLDP(nIndent, fUndoData);
						//	    rTime = LDP_AdvanceTimeCounter(pSO);
						//    }
					 //   }
					 //   else
						//    if (pSO->IsNoteRest() && ((lmNoteRest*)pSO)->GetVoice() == nVoice)
						//    {
						//	    LDP_AddShitTimeTagIfNeeded(sSource, nIndent, lmGO_FWD, rTime, pSO);
						//	    sSource += pSO->SourceLDP(nIndent, fUndoData);
						//	    rTime = LDP_AdvanceTimeCounter(pSO);
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
      //          sSource.append(nIndent * lmLDP_INDENT_STEP, _T(' '));
      //          sSource += _T("(goBack start)\n");
      //          rTime = 0.0f;
      //      }

      //      //if goBack added, add a goFwd to ensure that we are at end of measure
      //      if (fGoBack)
      //      {
      //          sSource.append(nIndent * lmLDP_INDENT_STEP, _T(' '));
      //          sSource += _T("(goFwd end)\n");
      //      }

		    ////add barline, if present
		    //if (pBL)
			   // sSource += pBL->SourceLDP(nIndent, fUndoData);

      //  }

    }

};


//---------------------------------------------------------------------------------------
class NoteLdpGenerator : public LdpGenerator
{
protected:
    ImoNote* m_pObj;

public:
    NoteLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoNote*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("n", m_pObj);
        add_pitch();
        add_duration();
        source_for_base_staffobj(m_pObj);
        end_element();
        return m_source.str();
    }

protected:

    void add_pitch()
    {
        static const string sNoteName[7] = { "c",  "d", "e", "f", "g", "a", "b" };
        static const string sOctave[13] = { "0",  "1", "2", "3", "4", "5", "6",
                                            "7", "8", "9", "10", "11", "12"  };

        if (m_pObj->get_step() == k_no_pitch)
        {
            m_source << "* ";
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
        m_source << " ";
    }

    void add_duration()
    {
        switch(m_pObj->get_note_type())
        {
            case k_longa:   m_source << "l";  break;
            case k_breve:   m_source << "b";  break;
            case k_whole:   m_source << "w";  break;
            case k_half:    m_source << "h";  break;
            case k_quarter: m_source << "q";  break;
            case k_eighth:  m_source << "e";  break;
            case k_16th:    m_source << "s";  break;
            case k_32th:    m_source << "t";  break;
            case k_64th:    m_source << "i";  break;
            case k_128th:   m_source << "o";  break;
            case k_256th:   m_source << "f";  break;
            default:                          break;
        }

        int dots = m_pObj->get_dots();
        while (dots > 0)
        {
            m_source << ".";
            --dots;
        }

    }

};


//---------------------------------------------------------------------------------------
class RestLdpGenerator : public LdpGenerator
{
protected:
    ImoRest* m_pObj;

public:
    RestLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoRest*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("r", m_pObj);
        add_duration();
        source_for_base_staffobj(m_pObj);
        end_element();
        return m_source.str();
    }

protected:

    void add_duration()
    {
        m_source << " (TODO: duration)";
    }

};


//---------------------------------------------------------------------------------------
class ScoreLdpGenerator : public LdpGenerator
{
protected:
    ImoScore* m_pObj;

public:
    ScoreLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScore*>(pImo);
    }

    std::string generate_source()
    {
        start_element();
        add_element_name("score", m_pObj);

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

protected:

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
//    m_source << m_TextStyles.SourceLDP(1, fUndoData);
    }

    void add_titles()
    {
//    //titles and other attached auxobjs
//    if (m_pAuxObjs)
//    {
//	    for (int i=0; i < (int)m_pAuxObjs->size(); i++)
//	    {
//		    m_source << (*m_pAuxObjs)[i]->SourceLDP(1, fUndoData);
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
//            m_source << (*it)->SourceLDP(1, fUndoData);
//#else
//        lmPageInfo* pPageInfo = m_PagesInfo.front();
//        m_source << pPageInfo->SourceLDP(1, fUndoData);
//#endif
//        //first system and other systems layout info
//        m_source << m_SystemsInfo.front()->SourceLDP(1, true, fUndoData);
//        m_source << m_SystemsInfo.back()->SourceLDP(1, false, fUndoData);
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
class ScoreObjLdpGenerator : public LdpGenerator
{
protected:
    ImoScoreObj* m_pObj;

public:
    ScoreObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScoreObj*>(pImo);
    }

    std::string generate_source()
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
            m_source << " (color " << LdpExporter::color_to_ldp(color) << ")";
    }
};


//---------------------------------------------------------------------------------------
class StaffObjLdpGenerator : public LdpGenerator
{
protected:
    ImoStaffObj* m_pObj;

public:
    StaffObjLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoStaffObj*>(pImo);
    }

    std::string generate_source()
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
            m_source << " p" << (m_pObj->get_staff() + 1);
        }
    }

};



//=======================================================================================
// LdpGenerator implementation
//=======================================================================================
void LdpGenerator::start_element()
{
    add_indent_spaces();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::end_element()
{
    m_source << ")";
    if (m_pExporter->get_indent() > 0)
        m_source << endl;
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_indent_spaces()
{
    m_source.clear();
    int indent = m_pExporter->get_indent() * lml_LDP_INDENT_STEP;
    while (indent > 0)
    {
        m_source << " ";
        indent--;
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_element_name(const std::string& name, ImoObj* pImo)
{
    m_source << "(" << name;
    if (m_pExporter->get_add_id())
        m_source << "#" << std::dec << pImo->get_id();
    m_source << " ";
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_source_for(ImoObj* pImo)
{
    m_source << m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_staffobj(ImoObj* pImo)
{
    StaffObjLdpGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_scoreobj(ImoObj* pImo)
{
    ScoreObjLdpGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_contentobj(ImoObj* pImo)
{
    ContentObjLdpGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_base_imobj(ImoObj* pImo)
{
    ImObjLdpGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_auxobj(ImoObj* pImo)
{
    m_source <<  m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void LdpGenerator::increment_indent()
{
    //TODO
}

//---------------------------------------------------------------------------------------
void LdpGenerator::decrement_indent()
{
    //TODO
}



//=======================================================================================
// LdpExporter implementation
//=======================================================================================
LdpExporter::LdpExporter()
    : m_nIndent(0)
    , m_fAddId(false)
{
}

//---------------------------------------------------------------------------------------
LdpExporter::~LdpExporter()
{
}

//---------------------------------------------------------------------------------------
std::string LdpExporter::get_source(ImoObj* pImo)
{
    LdpGenerator* pGen = new_generator(pImo);
    std::string source = pGen->generate_source();
    delete pGen;
    return source;
}

//---------------------------------------------------------------------------------------
LdpGenerator* LdpExporter::new_generator(ImoObj* pImo)
{
    //factory method

    switch(pImo->get_obj_type())
    {
        case k_imo_barline:         return LOMSE_NEW BarlineLdpGenerator(pImo, this);
//        case k_imo_beam_dto:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_bezier_info:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
        case k_imo_clef:            return LOMSE_NEW ClefLdpGenerator(pImo, this);
//        case k_imo_color_dto:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_instr_group:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_midi_info:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_option:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_system_info:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_tie_dto:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_tuplet_dto:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
        case k_imo_document:        return LOMSE_NEW LenmusdocLdpGenerator(pImo, this);
//        case k_imo_content:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
        case k_imo_instrument:      return LOMSE_NEW InstrumentLdpGenerator(pImo, this);
//        case k_imo_score:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
        case k_imo_key_signature:   return LOMSE_NEW KeySignatureLdpGenerator(pImo, this);
//        case k_imo_time_signature:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
        case k_imo_music_data:      return LOMSE_NEW MusicDataLdpGenerator(pImo, this);
        case k_imo_note:            return LOMSE_NEW NoteLdpGenerator(pImo, this);
        case k_imo_rest:            return LOMSE_NEW RestLdpGenerator(pImo, this);
//        case k_imo_go_back_fwd:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_metronome_mark:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_system_break:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_spacer:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_figured_bass:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
        case k_imo_score:           return LOMSE_NEW ScoreLdpGenerator(pImo, this);
//        case k_imo_score_text:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_fermata:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_tie:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_beam:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_tuplet:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
        default:
            return new ErrorLdpGenerator(pImo, this);
    }
}



// static methods

//---------------------------------------------------------------------------------------
std::string LdpExporter::clef_type_to_ldp(int clefType)
{
    //AWARE: indexes in correspondence with enum k_clef__type
    static const std::string name[] = {
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
    static const std::string undefined = "undefined";


    if (clefType == k_clef_undefined)
        return undefined;
    else
        return name[clefType];
}

//---------------------------------------------------------------------------------------
std::string LdpExporter::color_to_ldp(Color color)
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
std::string LdpExporter::float_to_string(float num)
{
    return "(TODO: float_to_string)";
}



}  //namespace lomse
