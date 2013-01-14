//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

    virtual string generate_source(ImoObj* pParent=NULL) = 0;

protected:
    void start_element(const string& name, int id, bool fInNewLine=true);
    void end_element(bool fStartLine = true);
    void start_comment();
    void end_comment();
    void empty_line();
    void new_line_and_indent_spaces(bool fStartLine = true);
    void new_line();
    void add_source_for(ImoObj* pImo);
    void source_for_base_staffobj(ImoObj* pImo);
    void source_for_base_scoreobj(ImoObj* pImo);
    void source_for_base_contentobj(ImoObj* pImo);
    void source_for_base_imobj(ImoObj* pImo);
    void source_for_auxobj(ImoObj* pImo);
    void source_for_relobj(ImoObj* pImo, ImoObj* pParent);

    void increment_indent();
    void decrement_indent();

    void add_duration(stringstream& source, int noteType, int dots);
    void add_visible(bool fVisible);
    void add_color_if_not_black(Color color);
    void add_location_if_not_zero(Tenths x, Tenths y);
    void add_location(TPoint pt);
    void add_width_if_not_default(Tenths width, Tenths def);


};

#define k_in_same_line      false
#define k_in_new_line       true
#define k_no_id             -1
#define k_indent_step       3


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

    string generate_source(ImoObj* pParent=NULL)
    {
        //start_element("xxxxx", m_pObj->get_id());
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

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("barline", m_pObj->get_id());
        add_barline_type();
        source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_barline_type()
    {
        int type = m_pObj->get_type();
        switch(type)
        {
            case ImoBarline::k_simple:
                m_source << "simple";
                break;
            case ImoBarline::k_double:
                m_source << "double";
                break;
            case ImoBarline::k_start:
                m_source << "start";
                break;
            case ImoBarline::k_end:
                m_source << "end";
                break;
            case ImoBarline::k_end_repetition:
                m_source << "endRepetition";
                break;
            case ImoBarline::k_start_repetition:
                m_source << "startRepetition";
                break;
            case ImoBarline::k_double_repetition:
                m_source << "doubleRepetition";
                break;
            default:
            {
                stringstream s;
                s << "[BarlineLdpGenerator::add_barline_type] Invalid barline type. Value="
                  << type;
                throw std::runtime_error(s.str());
            }
        }
    }

};

//---------------------------------------------------------------------------------------
class BeamLdpGenerator : public LdpGenerator
{
protected:
    ImoRelObj* m_pRO;
    ImoNoteRest* m_pNR;

public:
    BeamLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pRO = static_cast<ImoRelObj*>(pImo);
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        m_pNR = static_cast<ImoNoteRest*>( pParent );
        if ( m_pNR == m_pRO->get_start_object() )
            return source_for_first();
        else if ( m_pNR == m_pRO->get_end_object() )
            return source_for_last();
        else
            return source_for_middle();
    }

protected:

    string source_for_first()
    {
        start_element("beam", k_no_id, k_in_same_line);
        add_beam_number();
        add_segments_info();
        end_element(k_in_same_line);
        return m_source.str();
    }

    string source_for_middle()
    {
        return source_for_first();
    }

    string source_for_last()
    {
        return source_for_first();
    }

    void add_beam_number()
    {
        m_source << m_pRO->get_id();
    }

    void add_segments_info()
    {
        m_source << " ";
        for (int i=0; i < 6; ++i)
        {
            int type = m_pNR->get_beam_type(i);
            if (type == ImoBeam::k_none)
                break;
            else if (type == ImoBeam::k_begin)
                m_source << "+";
            else if (type == ImoBeam::k_continue)
                m_source << "=";
            else if (type == ImoBeam::k_end)
                m_source << "-";
            else if (type == ImoBeam::k_forward)
                m_source << "f";
            else if (type == ImoBeam::k_backward)
                m_source << "b";
        }
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

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("clef", m_pObj->get_id());
        add_type();
        source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line);
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

    string generate_source(ImoObj* pParent=NULL)
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
            ImoAttachments* pAuxObjs = m_pObj->get_attachments();
            int size = pAuxObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoAuxObj* pAO = static_cast<ImoAuxObj*>( pAuxObjs->get_item(i) );
                source_for_auxobj(pAO);
            }
        }
    }
};

//---------------------------------------------------------------------------------------
class ErrorLdpGenerator : public LdpGenerator
{
protected:
    ImoObj* m_pImo;

public:
    ErrorLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
        , m_pImo(pImo)
    {
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("TODO: ", m_pImo->get_id());
        m_source << m_pImo->get_name() << "   type=" << m_pImo->get_obj_type()
                 << ", id=" << m_pImo->get_id();
        end_element(k_in_same_line);
        return m_source.str();
    }
};

//---------------------------------------------------------------------------------------
class FermataLdpGenerator : public LdpGenerator
{
protected:
    ImoFermata* m_pObj;

public:
    FermataLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoFermata*>(pImo);
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("fermata", m_pObj->get_id());
        add_placement();
        source_for_base_scoreobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:
    void add_placement()
    {
        int placement = m_pObj->get_placement();
        if (placement == k_placement_default)
            m_source << ")";
        else if (placement == k_placement_above)
            m_source << " above";
        else
            m_source << " below";
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

    string generate_source(ImoObj* pParent=NULL)
    {
        return m_source.str();
    }
};

//---------------------------------------------------------------------------------------
class GoBackFwdLdpGenerator : public LdpGenerator
{
protected:
    ImoGoBackFwd* m_pObj;

public:
    GoBackFwdLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoGoBackFwd*>(pImo);
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        empty_line();
        bool fFwd = m_pObj->is_forward();
        start_element((fFwd ? "goFwd" : "goBack"), m_pObj->get_id());
        add_time(fFwd);
        source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_time(bool fFwd)
    {
        if (m_pObj->is_to_start())
            m_source << "start";
        else if (m_pObj->is_to_end())
            m_source << "end";
        else
        {
            if (fFwd)
                m_source << m_pObj->get_time_shift();
            else
                m_source << - m_pObj->get_time_shift();

        }
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

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("instrument", m_pObj->get_id());

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

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("key", m_pObj->get_id());

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
class LenmusdocLdpGenerator : public LdpGenerator
{
protected:
    ImoDocument* m_pObj;

public:
    LenmusdocLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoDocument*>(pImo);
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("lenmusdoc", m_pObj->get_id());
        add_version();
        add_comment();
        add_content();
        end_element();
        return m_source.str();
    }

protected:

    void add_version()
    {
        start_element("vers", k_no_id);
        m_source << m_pObj->get_version();
        end_element(k_in_same_line);
    }

    void add_comment()
    {
        if (!m_pExporter->get_remove_newlines())
        {
            start_comment();
            m_source << "LDP file generated by Lomse";
            string& version = m_pExporter->get_library_version();
            if (!version.empty())
                m_source << ", version " << version;
            m_source << ". Date: ";
    //        m_source << (wxDateTime::Now()).Format(_T("%Y-%m-%d"));
            end_comment();
        }
    }

    void add_content()
    {
        ImoContent* pContent = m_pObj->get_content();
        start_element("content", pContent->get_id());
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
class MusicDataLdpGenerator : public LdpGenerator
{
protected:
    ImoMusicData* m_pObj;

public:
    MusicDataLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoMusicData*>(pImo);
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("musicData", m_pObj->get_id());
        add_staffobjs();
        end_element();
        return m_source.str();
    }

protected:

    void add_staffobjs()
    {
        ImoObj::children_iterator it = m_pObj->begin();
        bool fNewMeasure = true;
        int nMeasure = 1;
        while (it != m_pObj->end())
        {
            if (fNewMeasure)
            {
                if (!m_pExporter->get_remove_newlines())
                {
                    empty_line();
                    start_comment();
                    m_source << "measure " << nMeasure++;
                    end_comment();
                }
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
class MetronomeLdpGenerator : public LdpGenerator
{
protected:
    ImoMetronomeMark* m_pImo;

public:
    MetronomeLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
    {
        m_pImo = static_cast<ImoMetronomeMark*>( pImo );
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("metronome", m_pImo->get_id());
        add_marks();
        source_for_base_staffobj(m_pImo);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_marks()
    {
        int type = m_pImo->get_mark_type();
        switch (type)
        {
            case ImoMetronomeMark::k_note_value:
                m_source << m_pImo->get_ticks_per_minute();
                break;
            case ImoMetronomeMark::k_note_note:
                add_duration(m_source, m_pImo->get_left_note_type(),
                             m_pImo->get_left_dots());
                m_source << " ";
                add_duration(m_source, m_pImo->get_right_note_type(),
                             m_pImo->get_right_dots());
                break;
            case ImoMetronomeMark::k_value:
                add_duration(m_source, m_pImo->get_left_note_type(),
                             m_pImo->get_left_dots());
                m_source << " ";
                m_source <<  m_pImo->get_ticks_per_minute();
                break;
            default:
            {
                stringstream s;
                s << "[MetronomeLdpGenerator::add_marks] Invalid type. Value=" << type;
                throw std::runtime_error(s.str());
            }
        }

    }

    void add_parenthesis()
    {
        if (m_pImo->has_parenthesis())
            m_source << " parentheses";
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

    string generate_source(ImoObj* pParent=NULL)
    {
        if (m_pObj->is_start_of_chord())
            start_element("chord", k_no_id);

        start_element("n", m_pObj->get_id());
        add_pitch();
        add_duration(m_source, m_pObj->get_note_type(), m_pObj->get_dots());
        add_stem();
        source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line);

        if (m_pObj->is_end_of_chord())
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

    void add_stem()
    {
        int stem = m_pObj->get_stem_direction();
        switch(stem)
        {
            case k_stem_default:
                break;
            case k_stem_up:
                m_source << " (stem up)";
                break;
            case k_stem_down:
                m_source << " (stem down)";
                break;
            case k_stem_none:
                m_source << " (stem none)";
                break;
            case k_stem_double:
                m_source << " (stem double)";
                break;
            default:
            {
                stringstream s;
                s << "[NoteLdpGenerator::add_stem] Invalid stem. Value=" << stem;
                throw std::runtime_error(s.str());
            }
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

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("r", m_pObj->get_id());
        add_duration(m_source, m_pObj->get_note_type(), m_pObj->get_dots());
        source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
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

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("score", m_pObj->get_id());
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
class ScoreLineLdpGenerator : public LdpGenerator
{
protected:
    ImoScoreLine* m_pObj;

public:
    ScoreLineLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScoreLine*>(pImo);
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("line", m_pObj->get_id());
        add_start_point();
        add_end_point();
        add_width_if_not_default( m_pObj->get_line_width(), 1.0f);
        add_color_if_not_black( m_pObj->get_color() );
        add_line_style();
        add_cap("lineCapStart", m_pObj->get_start_cap());
        add_cap("lineCapEnd", m_pObj->get_end_cap());
        end_element();
        return m_source.str();
    }

protected:

    void add_start_point()
    {
        start_element("startPoint", k_no_id, k_in_same_line);
        add_location(m_pObj->get_start_point());
        end_element(k_in_same_line);
    }

    void add_end_point()
    {
        start_element("endPoint", k_no_id, k_in_same_line);
        add_location(m_pObj->get_end_point());
        end_element(k_in_same_line);
    }

    void add_line_style()
    {
        int type = m_pObj->get_line_style();
        if (type == k_line_none)
            return;

        start_element("lineStyle", k_no_id, k_in_same_line);
        switch(type)
        {
            case k_line_solid:
                m_source << "solid";
                break;
            case k_line_long_dash:
                m_source << "longDash";
                break;
            case k_line_short_dash:
                m_source << "shortDash";
                break;
            case k_line_dot:
                m_source << "dot";
                break;
            case k_line_dot_dash:
                m_source << "dotDash";
                break;
            default:
            {
                stringstream s;
                s << "[ScoreLineLdpGenerator::add_line_style] Invalid line style. Value="
                  << type;
                throw std::runtime_error(s.str());
            }
        }
        end_element(k_in_same_line);
    }

    void add_cap(const string& tag, int type)
    {
        if (type == k_cap_none)
            return;

        start_element(tag, k_no_id, k_in_same_line);
        switch(type)
        {
            case k_cap_arrowhead:
                m_source << "arrowhead";
                break;
            case k_cap_arrowtail:
                m_source << "arrowtail";
                break;
            case k_cap_circle:
                m_source << "circle";
                break;
            case k_cap_square:
                m_source << "square";
                break;
            case k_cap_diamond:
                m_source << "diamond";
                break;
            default:
            {
                stringstream s;
                s << "[ScoreLineLdpGenerator::add_cap] Invalid line cap. Value="
                  << type;
                throw std::runtime_error(s.str());
            }
        }
        end_element(k_in_same_line);
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

    string generate_source(ImoObj* pParent=NULL)
    {
        add_visible( m_pObj->is_visible() );
        add_color_if_not_black( m_pObj->get_color() );
        source_for_base_contentobj(m_pObj);
        return m_source.str();
    }

};

//---------------------------------------------------------------------------------------
class ScoreTextLdpGenerator : public LdpGenerator
{
protected:
    ImoScoreText* m_pObj;

public:
    ScoreTextLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoScoreText*>(pImo);
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("text", m_pObj->get_id());
        add_text();
        add_style();
        add_location_if_not_zero(m_pObj->get_user_location_x(),
                                 m_pObj->get_user_location_y());
        end_element();
        return m_source.str();
    }

protected:

    void add_text()
    {
        m_source << "\"" << m_pObj->get_text() << "\"";
    }

    void add_style()
    {
        start_element("style", k_no_id);
        m_source << "\"" << m_pObj->get_style()->get_name() << "\"";
        end_element(k_in_same_line);
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

    string generate_source(ImoObj* pParent=NULL)
    {
        add_staff_num();
        add_relobjs();
        source_for_base_scoreobj(m_pObj);
        return m_source.str();
    }

protected:

    void add_staff_num()
    {
        if (!m_pObj->is_key_signature()            //KS, TS & barlines are common to all staves.
            && !m_pObj->is_time_signature()
            && !m_pObj->is_barline()
            && !m_pObj->is_go_back_fwd() )
        {
            m_source << " p" << (m_pObj->get_staff() + 1) << " ";
//            m_source << " ";
//            start_element("staffNum", k_no_id, k_in_same_line);
//            m_source << (m_pObj->get_staff() + 1);
//            end_element(k_in_same_line);
        }
    }

    void add_relobjs()
    {
        if (m_pObj->get_num_relations() > 0)
        {
            ImoRelations* pRelObjs = m_pObj->get_relations();
            int size = pRelObjs->get_num_items();
            for (int i=0; i < size; ++i)
            {
                ImoRelObj* pRO = pRelObjs->get_item(i);
                if (!pRO->is_chord())
                {
                    //AWARE: chords are excluded because they are generated
                    //in NoteLdpGenerator
                    source_for_relobj(pRO, m_pObj);
                }
            }
        }
    }

};

//---------------------------------------------------------------------------------------
class SystemBreakLdpGenerator : public LdpGenerator
{
protected:
    ImoSystemBreak* m_pImo;

public:
    SystemBreakLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
    {
        m_pImo = static_cast<ImoSystemBreak*>( pImo );
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("newSystem", m_pImo->get_id());
        end_element(k_in_same_line);
        return m_source.str();
    }
};

//---------------------------------------------------------------------------------------
class SpacerLdpGenerator : public LdpGenerator
{
protected:
    ImoSpacer* m_pObj;

public:
    SpacerLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoSpacer*>(pImo);
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("spacer", m_pObj->get_id());
        add_space_width();
        source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_space_width()
    {
        m_source << m_pObj->get_width();
    }

};

//---------------------------------------------------------------------------------------
class TieLdpGenerator : public LdpGenerator
{
protected:
    ImoTie* m_pTie;
    ImoNote* m_pNote;

public:
    TieLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pTie = static_cast<ImoTie*>(pImo);
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        m_pNote = static_cast<ImoNote*>( pParent );

        start_element("tie", m_pTie->get_id());
        add_tie_number();
        bool fStart = (m_pNote == m_pTie->get_start_note());
        add_tie_type(fStart);
        add_bezier_info(fStart);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_tie_number()
    {
        m_source << m_pTie->get_tie_number();
    }

    void add_tie_type(bool fStart)
    {
        m_source << (fStart ? " start" : " stop");
    }

    void add_bezier_info(bool fStart)
    {
        ImoBezierInfo* pInfo = (fStart ? m_pTie->get_start_bezier()
                                       : m_pTie->get_stop_bezier() );
        if (pInfo)
        {
            start_element("bezier", k_no_id);

            static string sPointNames[4] = { "start", "end", "ctrol1", "ctrol2" };

            for (int i=0; i < 4; i++)
            {
                TPoint& pt = pInfo->get_point(i);

                if (pt.x != 0.0f || pt.y != 0.0f)
                {
                    if (pt.x != 0.0f)
                    {
                        start_element( sPointNames[i] + "-x", k_no_id, k_in_same_line);
                        m_source << pt.x;
                        end_element(k_in_same_line);
                    }
                    if (pt.y != 0.0f)
                    {
                        start_element( sPointNames[i] + "-y", k_no_id, k_in_same_line);
                        m_source << pt.y;
                        end_element(k_in_same_line);
                    }
                }
            }
            end_element();
        }
    }

};

//---------------------------------------------------------------------------------------
class TimeSignatureLdpGenerator : public LdpGenerator
{
protected:
    ImoTimeSignature* m_pObj;

public:
    TimeSignatureLdpGenerator(ImoObj* pImo, LdpExporter* pExporter)
        : LdpGenerator(pExporter)
    {
        m_pObj = static_cast<ImoTimeSignature*>(pImo);
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        start_element("time", m_pObj->get_id());
        add_type();
        source_for_base_staffobj(m_pObj);
        end_element(k_in_same_line);
        return m_source.str();
    }

protected:

    void add_type()
    {
        m_source << m_pObj->get_top_number() << " " << m_pObj->get_bottom_number();
    }

};

//---------------------------------------------------------------------------------------
class TupletLdpGenerator : public LdpGenerator
{
protected:
    ImoTuplet* m_pTuplet;
    ImoNoteRest* m_pNR;

public:
    TupletLdpGenerator(ImoObj* pImo, LdpExporter* pExporter) : LdpGenerator(pExporter)
    {
        m_pTuplet = static_cast<ImoTuplet*>(pImo);
    }

    string generate_source(ImoObj* pParent=NULL)
    {
        m_pNR = static_cast<ImoNoteRest*>( pParent );

        if (m_pNR == m_pTuplet->get_start_object())
        {
            start_element("t", m_pTuplet->get_id());
            //add_tuplet_number();      //TODO: For now not generated
            add_tuplet_type(true);
            add_actual_notes();
            add_normal_notes();
            add_tuplet_options();
            end_element(k_in_same_line);
        }
        else if (m_pNR == m_pTuplet->get_end_object())
        {
            start_element("t", m_pTuplet->get_id());
            //add_tuplet_number();      //TODO: For now not generated
            add_tuplet_type(false);
            end_element(k_in_same_line);
        }
        else
        {
            m_source.clear();
        }
        return m_source.str();
    }

protected:

    inline void add_tuplet_number()
    {
        m_source << m_pTuplet->get_id();
    }

    inline void add_tuplet_type(bool fStart)
    {
        m_source << (fStart ? "+" : "-");
    }

    inline void add_actual_notes()
    {
        m_source << " " << m_pTuplet->get_actual_number();
    }

    inline void add_normal_notes()
    {
        m_source << " " << m_pTuplet->get_normal_number();
    }

    inline void add_tuplet_options()
    {
        add_bracket_type();
        add_display_bracket();
        add_display_number();
    }

    void add_bracket_type()
    {
    }

    void add_display_bracket()
    {
        int opt = m_pTuplet->get_show_bracket();
        if (opt == k_yesno_no)
            m_source << " noBracket";
    }

    void add_display_number()
    {
        int number = m_pTuplet->get_show_number();
        if (number == ImoTuplet::k_number_actual)
            return;     //default option

        m_source << " ";
        start_element("displayNumber", k_no_id, k_in_same_line);

        if (number == ImoTuplet::k_number_both)
            m_source << "both";
        else if (number == ImoTuplet::k_number_none)
            m_source << "none";
        else
        {
            stringstream s;
            s << "[TupletLdpGenerator::add_display_number] Invalid option. Value="
              << number;
            throw std::runtime_error(s.str());
        }

        end_element(k_in_same_line);
    }

};


//=======================================================================================
// LdpGenerator implementation
//=======================================================================================
void LdpGenerator::start_element(const string& name, int id, bool fInNewLine)
{
    if (fInNewLine)
        new_line_and_indent_spaces();

    m_source << "(" << name;
    if (m_pExporter->get_add_id() && id != k_no_id)
        m_source << "#" << std::dec << id;
    m_source << " ";
    increment_indent();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::end_element(bool fStartLine)
{
    decrement_indent();
    if (fStartLine)
        new_line_and_indent_spaces(fStartLine);
    m_source << ")";
}

//---------------------------------------------------------------------------------------
void LdpGenerator::start_comment()
{
    new_line_and_indent_spaces();
    m_source << "/* ";
}

//---------------------------------------------------------------------------------------
void LdpGenerator::end_comment()
{
    m_source << " */";
}

//---------------------------------------------------------------------------------------
void LdpGenerator::empty_line()
{
    m_source.clear();
    new_line();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::new_line_and_indent_spaces(bool fStartLine)
{
    m_source.clear();
    if (!m_pExporter->get_remove_newlines())
    {
        if (fStartLine)
            new_line();
        int indent = m_pExporter->get_indent() * k_indent_step;
        while (indent > 0)
        {
            m_source << " ";
            indent--;
        }
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::new_line()
{
    if (!m_pExporter->get_remove_newlines())
        m_source << endl;
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
    increment_indent();
    ImObjLdpGenerator gen(pImo, m_pExporter);
    m_source << gen.generate_source();
    decrement_indent();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_auxobj(ImoObj* pImo)
{
    m_source <<  m_pExporter->get_source(pImo);
}

//---------------------------------------------------------------------------------------
void LdpGenerator::source_for_relobj(ImoObj* pRO, ImoObj* pParent)
{
    m_source <<  m_pExporter->get_source(pRO, pParent);
}

//---------------------------------------------------------------------------------------
void LdpGenerator::increment_indent()
{
    m_pExporter->increment_indent();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::decrement_indent()
{
    m_pExporter->decrement_indent();
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_duration(stringstream& source, int noteType, int dots)
{
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

}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_visible(bool fVisible)
{
    if (!fVisible)
        m_source << " (visible no)";
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_color_if_not_black(Color color)
{
    if (color.r != 0 || color.g != 0  || color.b != 0 || color.a != 255)
        m_source << "(color " << LdpExporter::color_to_ldp(color) << ")";
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_width_if_not_default(Tenths width, Tenths def)
{
    if (width != def)
    {
        start_element("width", k_no_id, k_in_same_line);
        m_source << width;
        end_element(k_in_same_line);
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_location_if_not_zero(Tenths x, Tenths y)
{
    if (x != 0.0f || y != 0.0f)
    {
        if (x != 0.0f)
        {
            start_element("dx", k_no_id, k_in_same_line);
            m_source << x;
            end_element(k_in_same_line);
        }
        if (y != 0.0f)
        {
            start_element("dy", k_no_id, k_in_same_line);
            m_source << y;
            end_element(k_in_same_line);
        }
    }
}

//---------------------------------------------------------------------------------------
void LdpGenerator::add_location(TPoint pt)
{
    start_element("dx", k_no_id, k_in_same_line);
    m_source << pt.x;
    end_element(k_in_same_line);

    start_element("dy", k_no_id, k_in_same_line);
    m_source << pt.y;
    end_element(k_in_same_line);
}

//=======================================================================================
// LdpExporter implementation
//=======================================================================================
LdpExporter::LdpExporter(LibraryScope* pLibraryScope)
    : m_pLibraryScope(pLibraryScope)
    , m_version( pLibraryScope->get_version_string() )
    , m_nIndent(0)
    , m_fAddId(false)
    , m_fRemoveNewlines(false)
{
}

LdpExporter::LdpExporter()
    : m_pLibraryScope(NULL)
    , m_version()
    , m_nIndent(0)
    , m_fAddId(false)
    , m_fRemoveNewlines(false)
{
}

//---------------------------------------------------------------------------------------
LdpExporter::~LdpExporter()
{
}

//---------------------------------------------------------------------------------------
string LdpExporter::get_source(ImoObj* pImo, ImoObj* pParent)
{
    LdpGenerator* pGen = new_generator(pImo);
    string source = pGen->generate_source(pParent);
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
        case k_imo_beam:            return LOMSE_NEW BeamLdpGenerator(pImo, this);
        case k_imo_clef:            return LOMSE_NEW ClefLdpGenerator(pImo, this);
//        case k_imo_instr_group:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_option:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
//        case k_imo_system_info:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
        case k_imo_document:        return LOMSE_NEW LenmusdocLdpGenerator(pImo, this);
        case k_imo_fermata:         return LOMSE_NEW FermataLdpGenerator(pImo, this);
//        case k_imo_figured_bass:         return LOMSE_NEW XxxxxxxLdpGenerator(pImo, this);
        case k_imo_go_back_fwd:     return LOMSE_NEW GoBackFwdLdpGenerator(pImo, this);
        case k_imo_instrument:      return LOMSE_NEW InstrumentLdpGenerator(pImo, this);
        case k_imo_key_signature:   return LOMSE_NEW KeySignatureLdpGenerator(pImo, this);
        case k_imo_metronome_mark:  return LOMSE_NEW MetronomeLdpGenerator(pImo, this);
        case k_imo_music_data:      return LOMSE_NEW MusicDataLdpGenerator(pImo, this);
        case k_imo_note:            return LOMSE_NEW NoteLdpGenerator(pImo, this);
        case k_imo_rest:            return LOMSE_NEW RestLdpGenerator(pImo, this);
        case k_imo_system_break:    return LOMSE_NEW SystemBreakLdpGenerator(pImo, this);
        case k_imo_score:           return LOMSE_NEW ScoreLdpGenerator(pImo, this);
        case k_imo_score_text:      return LOMSE_NEW ScoreTextLdpGenerator(pImo, this);
        case k_imo_score_line:      return LOMSE_NEW ScoreLineLdpGenerator(pImo, this);
        case k_imo_spacer:          return LOMSE_NEW SpacerLdpGenerator(pImo, this);
        case k_imo_time_signature:  return LOMSE_NEW TimeSignatureLdpGenerator(pImo, this);
        case k_imo_tie:             return LOMSE_NEW TieLdpGenerator(pImo, this);
        case k_imo_tuplet:          return LOMSE_NEW TupletLdpGenerator(pImo, this);
        default:
            return new ErrorLdpGenerator(pImo, this);
    }
}



// static methods

//---------------------------------------------------------------------------------------
string LdpExporter::clef_type_to_ldp(int clefType)
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
string LdpExporter::color_to_ldp(Color color)
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
string LdpExporter::float_to_string(float num)
{
    return "(TODO: float_to_string)";
}



}  //namespace lomse
