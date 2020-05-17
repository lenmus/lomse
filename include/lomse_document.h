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

#ifndef __LOMSE_DOCUMENT_H__
#define __LOMSE_DOCUMENT_H__

#include "lomse_internal_model.h"
#include "private/lomse_document_p.h"


///@cond INTERNALS
namespace lomse
{
///@endcond


// Start of public API IDocument ========================================================


class Document;

//---------------------------------------------------------------------------------------
/** The %IDocument class is is the API root object that contains, basically, the @IM,
    a model similar to the DOM in HTML.
    It represents the whole document and allows to access and modify the content
    of the document. By accessing and modifying this internal model you
    have full control over the document content and its properties.

    The %IDocument class also encapsulates all the internals, providing the basic API
    for creating, modifying and using a document.
*/
class LOMSE_EXPORT IDocument
{
private:
    Document* m_pImpl;

    friend class Document;
    IDocument(Document* impl);

    inline const Document* pimpl() const { return m_pImpl; }
    inline Document* pimpl() { return m_pImpl; }

public:
    IDocument(IDocument &&) noexcept = default;
    IDocument& operator=(IDocument &&) noexcept = default;
    IDocument(const IDocument& other) = default;
    IDocument& operator=(const IDocument& other) = default;

    // start of public API --------------------------------------------

    //facade methods to hide ImoDocument

//        //global styles
//    LmStyles* get_styles();
//    ImoStyle* find_style(const string& name);
//    ImoStyle* get_default_style();
//    ImoStyle* get_style_or_default(const string& name);
//    void add_style(ImoStyle* pStyle);
//    IStyle* create_style(const string& name, const string& parent="Default style");
//    IStyle* create_private_style(const string& parent="Default style");

        //content
    //convenience method for documents containing only a music score
    IScore get_first_score();

//        //content
//    void insert_block_level_obj(ImoBlockLevelObj* pAt, ImoBlockLevelObj* pImoNew);
//    void delete_block_level_obj(ImoBlockLevelObj* pAt);


//        //document attributes
//    void set_version(const string& version) { m_version = version; }
//    void set_language(const string& language) { m_language = language; }
    string& get_version();
//    string& get_language();



    /// @name Page content scale
    //@{

    /** Return the scaling factor to apply to the content when rendered divided into
        pages of the size defined by the paper size. Normally this factor is 1.0.
    */
    float get_page_content_scale();

    /** Set the scaling factor to apply to the content when rendered divided into
        pages of the size defined by the paper size. By default this factor is 1.0.
    */
    void set_page_content_scale(float scale);

    //@}    //Page content scale

        //document page size
    LUnits get_page_left_margin();
    LUnits get_page_right_margin();
    LUnits get_page_top_margin();
    LUnits get_page_bottom_margin();
    LUnits get_page_binding_margin();
    USize get_page_size();
    /** Return the paper width intended for rendering this %Document. The returned value
        is in logical units (cents of a millimeter). */
    LUnits get_page_width();
    /** Return the paper height intended for rendering this %Document. The returned value
        is in logical units (cents of a millimeter). */
    LUnits get_page_height();
    void set_page_left_margin(LUnits value);
    void set_page_right_margin(LUnits value);
    void set_page_top_margin(LUnits value);
    void set_page_bottom_margin(LUnits value);
    void set_page_binding_margin(LUnits value);
    void set_page_size(USize uPageSize);
    void set_page_width(LUnits value);
    void set_page_height(LUnits value);

    /** Return the ImoPageInfo node for this %Document. ImoPageInfo contains the
        document intended paper size. Example:
        @code
        //set page size to DIN A3 (297 x 420 mm)
        Document* pDoc = m_pPresenter->get_document_raw_ptr();
        ImoPageInfo* pPageInfo = pDoc->get_page_info();
        pPageInfo->set_page_height(42000.0f);   //logical units, cents of one millimeter
        pPageInfo->set_page_width(29700.0f);
        @endcode
    */
    ImoPageInfo* get_page_info();


    //methods from Document


    /** When you modify the content of a %Document it is necessary to update some
        structures associated to music scores.
        For this it is mandatory to invoke this method. Alternatively, you can
        invoke IScore::end_of_changes(), on the modified scores. */
    void end_of_changes();

    /** Define the duration for one beat, for metronome and for methods that use
        measure/beat parameters to define a location. This value is shared by all
        scores contained in the document and can be changed at any time.
        Changes while the score is being played back are ignored until playback finishes.
        @param beatType A value from enum #EBeatDuration.
        @param duration The duration (in Lomse Time Units) for one beat. You can use
            a value from enum ENoteDuration casted to TimeUnits. This parameter is
            required only when value for parameter `beatType` is `k_beat_specified`.
            For all other values, if a non-zero value is specified, the value
            will be used for the beat duration in scores without time signature.
    */
    void define_beat(int beatType, TimeUnits duration=0.0);

    /** Return the beat type to use for scores in this document.

        See define_beat()
    */
    int get_beat_type();

    /** Return the duration for beats to use in scores without time signature and when
        beat type is `k_beat_specified`.

        See define_beat()
    */
    TimeUnits get_beat_duration();

    /** Transitional, to facilitate migration to the new public API.
        Notice that this method will be removed in future so, please, if you need to
        use this method open an issue at https://github.com/lenmus/lomse/issues
        explaining the need, so that the public API
        could be fixed and your app. would not be affected in future when this method
        is removed.
    */
    Document* get_internal_object();

};

//---------------------------------------------------------------------------------------
/** @ingroup enumerations

    This enum defines the duration of one beat, for metronome and for methods that use
    measure/beat to define a location.

	@#include <lomse_document_p.h>
*/
enum EBeatDuration
{
    k_beat_implied = 0,     ///< Implied by the time signature; e.g. 4/4 = four
                            ///< beats, 6/8 = two beats, 3/8 = one beat.
                            ///< The number of implied beats for a time signature is
                            ///< provided by method ImoTimeSignature::get_num_pulses().
                            ///< Basically, for simple time signatures, such as 4/4,
                            ///< 3/4, 2/4, 3/8, and 2/2, the number of beats is given by
                            ///< the time signature top number, with the exception of
                            ///< 3/8 which is normally conducted in one beat. In compound
                            ///< time signatures (6/x, 12/x, and 9/x) the number of beats
                            ///< is given by dividing the top number by three.

    k_beat_bottom_ts,       ///< Use the note duration implied by the time signature
                            ///< bottom number; e.g. 3/8 = use eighth notes. Notice
                            ///< that the number of beats will coincide with the
                            ///< time signature top number, e.g. 3 beats for 3/8.

    k_beat_specified,       ///< Use specified note value for beat duration.
};



}   //namespace lomse

#endif      //__LOMSE_DOCUMENT_H__
