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
/** %FragmentMark is a visual effect to display brackets and vertical lines,
    spanning one or several staves, to define on the score the start and end points of
    a section or fragment.

    FragmentMark objects are created by the Interactor associated to the View on which
    the mark is going to be displayed.

    See @ref Interactor::add_fragment_mark_at_note_rest(),
        Interactor::add_fragment_mark_at_barline(),
        Interactor::add_fragment_mark_at_staffobj().

    <b>Example of use:</b>

    @code
    ImoId scoreId = ...
    TimeUnits timepos = ...
    FragmentMark* mark = pInteractor->add_fragment_mark(scoreId, timepos);

    //customize the mark: rounded open bracket, magenta solid color, covering
    //the second and third instruments
    mark->color(Color(255,0,255))->thickness(5)->top(1)->bottom(2);
    mark->type(k_mark_open_rounded);
        ...

    //when no longer needed remove it
    pInteractor->remove_mark(mark);
    @endcode

    Marks cannot be repositioned. If this is needed, just delete current mark (by
    invoking Interactor::remove_mark() ) and create a new one at the new
    desired position.
*/
class FragmentMark : public VisualEffect
{
protected:
    EFragmentMark   m_type;
    Color           m_color;
    ELineStyle      m_lineStyle;
    GmoBoxSystem*   m_pBoxSystem;
    int             m_iPage;
    URect           m_bounds;
    LUnits          m_yTop;         //top point, without any added extension
    LUnits          m_yBottom;      //bottom point, without any added extension
    LUnits          m_extension;    //added on top and bottom

public:

    /// @name Customizable properties
    ///@{

    /** Define the horizontal shift to add to the default place for the mark.
        @param dx The shift to add. The shift is specified in tenths, referred to the
        first staff in the system on which the mark is placed.
    */
    FragmentMark* const x_shift(Tenths dx);

    /** Place the start of the mark at top line of the specified staff of an instrument.
        @param instr The instrument number (0..n-1) of the instrument.
        @param staff The staff number (0..m-1), relative to the number of staves in
            the instrument. Special value -1 means "the last staff of the instrument".

        When invoking this method, the extra height for the mark will be changed to be
        the staves margin. You can later change this value by using method extra_height().
    */
    FragmentMark* const top(int instr, int staff=0);

    /** Place the bottom of the mark at first line of the specified staff of an
        instrument.
        @param instr The instrument number (0..n-1) of the instrument.
        @param staff The staff number (0..m-1), relative to the number of staves in
            the instrument. Special value -1 means "the last staff of the instrument".

        When invoking this method, the extra height for the mark will be changed to be
        the staves margin. You can later change this value by using method extra_height().
    */
    FragmentMark* const bottom(int instr, int staff=-1);

    /** Define the additional height to add, at top and bottom of the mark line. Please
        note that the line heigth will be incremented in twice the passed value, as the
        extra height value will be added at top and also at bottom.

        This extra lenght is specified in tenths, referred to the first staff in the
        system on which the mark is placed.
    */
    FragmentMark* const extra_height(Tenths value=0);

    /** Set the color of the mark.  */
    inline FragmentMark* const color(Color value) { m_color = value; return this; }

    /** Set the type of the mark.  */
    inline FragmentMark* const type(EFragmentMark value) { m_type = value; return this; }

    /** Change the thickness of the mark vertical line. When the mark is created, it has
        a default thickness of six tenths.

        The thickness is specified in tenths, referred to the first staff in the
        system on which the mark is placed.
    */
    FragmentMark* const thickness(Tenths value);

    /** Set the line style.  */
    inline FragmentMark* const line_style(ELineStyle value) { m_lineStyle = value; return this; }

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
    void initialize(LUnits xPos, GmoBoxSystem* pBoxSystem);

protected:
    friend class OverlaysGenerator;
    virtual ~FragmentMark() {}

///@endcond

};


}   //namespace lomse

#endif      //__LOMSE_FRAGMENT_MARK_H__
