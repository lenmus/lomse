//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2019. All rights reserved.
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

#ifndef __LOMSE_FRAGMENT_MARK_H__
#define __LOMSE_FRAGMENT_MARK_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_visual_effect.h"
#include "lomse_internal_model.h"       //for ELineType

//other
#include <list>
using namespace std;


///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class GmoBoxSystem;

//---------------------------------------------------------------------------------------
/** %ApplicationMark is the base class for all application markings objects.

    Application markings are just temporary visual markers created by an application
    to provide feedback to the user about relevant issues, such as start-end points
    for playback, the set of objects selected, current notes being played, etc.

    Lomse does not use these marks and knows nothing about its purpose or meaning. Lomse
    just displays them.

    @attention  Application markings are just visual effects and are not stored in the
        Document object. They must be deleted when the document is modified and
        re-created if they are still needed. This is a very simple operation and is
        required because when the document is modified nothing ensures that existing
        markers are still valid. Pointed objects could no longer exist in the document
        or the positions at which the markers were initially placed are no longer the
        right places in the updated document rendering.

    <br/>
*/
class ApplicationMark : public VisualEffect
{
protected:

public:

///@cond INTERNALS
//excluded from public API. Only for internal use.

    ApplicationMark(GraphicView* view, LibraryScope& libraryScope)
        : VisualEffect(view, libraryScope) {}

    //mandatory overrides from VisualEffect
    void on_draw(ScreenDrawer* pDrawer)=0;
    URect get_bounds()=0;

protected:
    friend class OverlaysGenerator;
    virtual ~ApplicationMark() {}

///@endcond

};

//-----------------------------------------------------------------------------
/** @ingroup enumerations

    This enum describes values for start-end mark type.

    @#include <lomse_fragment_mark.h>
*/
enum EFragmentMark
{
    k_mark_line = 0,          ///< Vertical line
    k_mark_open_squared,      ///< Squared bracket with horizontal lines at right
    k_mark_close_squared,     ///< Squared bracket with horizontal lines at left
    k_mark_open_curly,        ///< Curly bracket with curved lines pointing to right
    k_mark_close_curly,       ///< Curly bracket with curved lines pointing to left
    k_mark_open_rounded,      ///< Rounded bracket with curved lines pointing to right
    k_mark_close_rounded,     ///< Rounded bracket with curved lines pointing to left
};


//---------------------------------------------------------------------------------------
/** %FragmentMark is an ApplicationMark to display on any score brackets and vertical
    lines, spanning one or several staves, to define the start and end points of
    a score section or fragment.

    FragmentMark objects are created by the Interactor associated to the View on which
    the mark is going to be displayed.

	See:
	- @ref Interactor::add_fragment_mark_at_note_rest(),
	- @ref Interactor::add_fragment_mark_at_barline(),
	- @ref Interactor::add_fragment_mark_at_staffobj().

	The following picture displays the different types of %FragmentMark markers that can
	be created. The markers were created with the code shown at the end of this
	description.

    @image html FragmentMark-types.png "Image: Different types of FragmentMark markers."



    <h3>Marks' horizontal position</h3>

    Marks of type `k_mark_line` are positioned centered on the reference timepos
    vertical line.

    For all other types:
    - When created using method Interactor::add_fragment_mark_at_barline() the marks are
        also positioned centered on the reference timepos vertical line, that is the
        barline left border.
    - When created by any other Interactor methods:
        - Type `open` are displayed with the inner bracket vertical border aligned with
            the timepos vertical line.
        - Type `close` are displayed with the outside bracket vertical border aligned
            with the timepos vertical line.

    It is possible to add an horizontal shift to the mark default position.
    See x_shift(). The horizontal shift can be modified when necessary.

    But marks cannot be repositioned, that is, its reference timepos can not be changed.
    If this is needed, just delete current mark (by invoking Interactor::remove_mark() )
    and create a new one at the new desired position.


    <h3>Marks' vertical position and height</h3>

    By default, the mark will cover the whole staff from top line of first staff to
    bottom line of bottom staff, plus an additional height of the spacing between
    systems (half is added on top and half on bottom). This additional height to add
    can be chaged by method extra_height().

    Methods top() and bottom() allows to define the vertical position and height of
    the mark by defining the reference instruments and staves that will be covered
    by the mark. When using any of these methods, the additional height will be changed
    to be the staves margin. You can later change this value by using method
    extra_height().

    The height and vertical position of the mark can be changed at any moment.
    See top(), bottom() and extra_height() methods.


    <h3>Example of use:</h3>

    The marks in previous image were created with this code:

    @code
    ImoId scoreId = pScore->get_id();

    //green mark: open rounded at staffobj (key signature).
    //From second instrument to last one
    ScoreCursor cursor(pDoc, pScore);     //cursor points to clef
    cursor.move_next();         //now points to key signature
    ImoStaffObj* pSO = dynamic_cast<ImoStaffObj*>(*(cursor));
    FragmentMark* mark = spInteractor->add_fragment_mark_at_staffobj(pSO);
    mark->color(Color(0,255,0))->top(1);
    mark->type(k_mark_open_rounded);

    cursor.move_next();         //now points to time signature
    pSO = dynamic_cast<ImoStaffObj*>(*(cursor));
    mark = spInteractor->add_fragment_mark_at_staffobj(pSO);
    mark->color(Color(0,255,0))->top(1);
    mark->type(k_mark_close_rounded);

    //blue curly marks: at first barline (measure 1, beat 0) and second
    //barline  (measure 2, beat 0). The whole system
    TimeUnits timepos = ScoreAlgorithms::get_timepos_for(pScore, 1, 0);
    mark = spInteractor->add_fragment_mark_at_barline(scoreId, timepos);
    mark->color(Color(0,0,255,128))->type(k_mark_open_curly);

    timepos = ScoreAlgorithms::get_timepos_for(pScore, 2, 0);
    mark = spInteractor->add_fragment_mark_at_barline(scoreId, timepos);
    mark->color(Color(0,0,255,128))->type(k_mark_close_curly);

    //red line mark: first note after first barline (measure 1, beat 0)
    timepos = ScoreAlgorithms::get_timepos_for(pScore, 1, 0);
    spInteractor->add_fragment_mark_at_note_rest(scoreId, timepos);

    //magenta line mark: at interpolated timepos. Only third instrument
    timepos += k_duration_eighth;    //at first quarter dotted note position
    mark = spInteractor->add_fragment_mark_at_note_rest(scoreId, timepos);
    mark->color(Color(255,0,255,128))->type(k_mark_line)->top(2);

    //solid blue curly mark: instruments 2,3 & 4
    timepos = ScoreAlgorithms::get_timepos_for(pScore, 1, 1);
    mark = spInteractor->add_fragment_mark_at_note_rest(scoreId, timepos);
    mark->color(Color(0,0,255))->top(1,0)->bottom(3);
    mark->type(k_mark_open_curly)->x_shift(-5.0);   //some space before

    timepos = ScoreAlgorithms::get_timepos_for(pScore, 1, 2);
    mark = spInteractor->add_fragment_mark_at_note_rest(scoreId, timepos);
    mark->color(Color(0,0,255))->top(1,0)->bottom(3);
    mark->type(k_mark_close_curly)->x_shift(25.0);  //some space to skip noteheads

    //magenta squared mark: instruments 2,3 & 4
    timepos = ScoreAlgorithms::get_timepos_for(pScore, 2,1);
    mark = spInteractor->add_fragment_mark_at_note_rest(scoreId, timepos);
    mark->color(Color(255,0,255,128))->top(1,0)->bottom(3);
    mark->type(k_mark_open_squared);

    timepos = ScoreAlgorithms::get_timepos_for(pScore, 2,2);
    mark = spInteractor->add_fragment_mark_at_note_rest(scoreId, timepos);
    mark->color(Color(255,0,255,128))->top(1,0)->bottom(3);
    mark->type(k_mark_close_squared);

    //green line mark: instruments 2,3 & 4
    timepos = ScoreAlgorithms::get_timepos_for(pScore, 3,0);
    mark = spInteractor->add_fragment_mark_at_note_rest(scoreId, timepos);
    mark->color(Color(0, 255,0))->top(1,0)->bottom(3);
    mark->type(k_mark_line);

    //red rounded mark: instruments 2,3 & 4
    timepos = ScoreAlgorithms::get_timepos_for(pScore, 4,0);
    mark = spInteractor->add_fragment_mark_at_note_rest(scoreId, timepos);
    mark->color(Color(255,0,0))->top(1,0)->bottom(3);
    mark->type(k_mark_open_rounded);

    timepos = ScoreAlgorithms::get_timepos_for(pScore, 4,1);
    mark = spInteractor->add_fragment_mark_at_note_rest(scoreId, timepos);
    mark->color(Color(255,0,0))->top(1,0)->bottom(3);
    mark->type(k_mark_close_rounded);
    @endcode

    <br/>
*/
class FragmentMark : public ApplicationMark
{
protected:
    EFragmentMark   m_type;
    Color           m_color;
    ELineStyle      m_lineStyle;
    GmoBoxSystem*   m_pBoxSystem;
    int             m_iPage;
    URect           m_bounds;
    LUnits          m_xLeft;        //mark position
    LUnits          m_yTop;         //mark position
    LUnits          m_yBottom;      //bottom point, without any added extension
    LUnits          m_extension;    //added on top and bottom
    LUnits          m_thickness;    //line thickness
    bool            m_fCentered;    //the mark must be centered

public:

    /// @name Customizable properties
    ///@{

    /** Define the horizontal shift to add to the default place for the mark.
        @param dx The shift to add. The shift is specified in tenths, referred to the
        first staff in the system on which the mark is placed.
    */
    FragmentMark* x_shift(Tenths dx);

    /** Place the start of the mark at top line of the specified staff of an instrument.
        @param instr The instrument number (0..n-1) of the instrument.
        @param staff The staff number (0..m-1), relative to the number of staves in
            the instrument. Special value -1 means "the last staff of the instrument".

        When invoking this method, the extra height for the mark will be changed to be
        the staves margin. You can later change this value by using method extra_height().
    */
    FragmentMark* top(int instr, int staff=0);

    /** Place the bottom of the mark at first line of the specified staff of an
        instrument.
        @param instr The instrument number (0..n-1) of the instrument.
        @param staff The staff number (0..m-1), relative to the number of staves in
            the instrument. Special value -1 means "the last staff of the instrument".

        When invoking this method, the extra height for the mark will be changed to be
        the staves margin. You can later change this value by using method extra_height().
    */
    FragmentMark* bottom(int instr, int staff=-1);

    /** Define the additional height to add, at top and bottom of the mark line. Please
        note that the line heigth will be incremented in twice the passed value, as the
        extra height value will be added at top and also at bottom.

        This extra lenght is specified in tenths, referred to the first staff in the
        system on which the mark is placed.
    */
    FragmentMark* extra_height(Tenths value=0);

    /** Set the color of the mark.  */
    inline FragmentMark* color(Color value) { m_color = value; return this; }

    /** Set the type of the mark.  */
    inline FragmentMark* type(EFragmentMark value) { m_type = value; return this; }

    /** Change the thickness of the mark vertical line. When the mark is created, it has
        a default thickness of six tenths.

        The thickness is specified in tenths, referred to the first staff in the
        system on which the mark is placed.
    */
    FragmentMark* thickness(Tenths value);

    /** Set the line style.  */
    inline FragmentMark* line_style(ELineStyle value) { m_lineStyle = value; return this; }

    ///@}    //Customizable properties


    /// @name Access to current values
    ///@{

    /** Return the current mark color. */
    inline Color get_color() const { return m_color; }

    /** Return the current mark type. */
    inline EFragmentMark get_type() const { return m_type; }

    /** Return the current line style in use for drawing the mark. */
    inline ELineStyle get_line_style() const { return m_lineStyle; }

    ///@}    //Access to current values


///@cond INTERNALS
//excluded from public API. Only for internal use.

    FragmentMark(GraphicView* view, LibraryScope& libraryScope);

    //mandatory overrides from VisualEffect
    void on_draw(ScreenDrawer* pDrawer);
    URect get_bounds();

    //initial data for position and context
    void initialize(LUnits xPos, GmoBoxSystem* pBoxSystem, bool fBarline=false);

protected:
    friend class OverlaysGenerator;
    virtual ~FragmentMark() {}

    //debug
    void draw_bounding_box(ScreenDrawer* pDrawer);

///@endcond

};


}   //namespace lomse

#endif      //__LOMSE_FRAGMENT_MARK_H__
