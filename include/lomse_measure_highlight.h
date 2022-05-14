//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_MEASURE_HIGHLIGHT_H__
#define __LOMSE_MEASURE_HIGHLIGHT_H__

#include "lomse_basic.h"
#include "lomse_injectors.h"
#include "lomse_visual_effect.h"
#include "lomse_internal_model.h"       //for ELineType
#include "lomse_fragment_mark.h"       //for class ApplicationMark

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
/** %MeasureHighlight is an ApplicationMark to display a colored background on any score
    measure.

    %MeasureHighlight objects are created by the Interactor associated to the View
     on which the highlight is going to be displayed.

	See: @ref Interactor::add_measure_highlight().

    @image html MeasureHighlight-1.png "Image: Example of MeasureHighlight marker."


    <h3>Marker: vertical position and height</h3>

    By default, the highlight will cover the whole staff from top line of first staff to
    bottom line of bottom staff, plus an additional height above and below (the added
    space is the spacing between systems, half is added on top and half on bottom. This
    additional height can be chaged by method extra_height().

    Methods top() and bottom() allows to define the vertical position and height of
    the mark by defining the reference instruments and staves that will be covered
    by the highlight. When using any of these methods, the additional height will be changed
    to be the staves margin. You can later change this value by using method
    extra_height().

    The height and vertical position of the highlight can be changed at any moment.
    See top(), bottom() and extra_height() methods.


    <h3>Example of use:</h3>

    Previous image was created with the following code, when clicking on third measure:

    @code
    ImoId scoreId = pScore->get_id();

    MeasureHighlight* mark = spInteractor->add_measure_highlight(scoreId, data.ml);
    mark->color(Color(0,255,0,64));
    @endcode

    Observe that the marker is overlayed on top of the score image. Thus, you should use
    transparent colours.

    The marker can be trimmed to occupy just some instruments or staves. For instance,
    the next code trims the marker to the clicked instrument:

    @code
    ImoId scoreId = pScore->get_id();

    MeasureHighlight* mark = spInteractor->add_measure_highlight(scoreId, data.ml);
    mark->top(data.ml.iInstr)->bottom(data.ml.iInstr);
    @endcode

    And this is the result:

    @image html MeasureHighlight-2.png "Image: Trimming the marker to the clicked instrument."

    <br/>
*/
class MeasureHighlight : public ApplicationMark
{
protected:
    Color           m_color;
    GmoBoxSystem*   m_pBoxSystem;
    int             m_iPage;
    URect           m_bounds;
    LUnits          m_xLeft;        //mark position
    LUnits          m_xRight;       //mark position
    LUnits          m_yTop;         //mark position
    LUnits          m_yBottom;      //bottom point, without any added extension
    LUnits          m_extension;    //added on top and bottom

public:

    /// @name Customizable properties
    ///@{

    /** Place the start of the mark at top line of the specified staff of an instrument.
        @param instr The instrument number (0..n-1) of the instrument.
        @param staff The staff number (0..m-1), relative to the number of staves in
            the instrument. Special value -1 means "the last staff of the instrument".

        When invoking this method, the extra height for the mark will be changed to be
        the staves margin. You can later change this value by using method extra_height().
    */
    MeasureHighlight* top(int instr, int staff=0);

    /** Place the bottom of the mark at first line of the specified staff of an
        instrument.
        @param instr The instrument number (0..n-1) of the instrument.
        @param staff The staff number (0..m-1), relative to the number of staves in
            the instrument. Special value -1 means "the last staff of the instrument".

        When invoking this method, the extra height for the mark will be changed to be
        the staves margin. You can later change this value by using method extra_height().
    */
    MeasureHighlight* bottom(int instr, int staff=-1);

    /** Define the additional height to add, at top and bottom of the mark line. Please
        note that the line heigth will be incremented in twice the passed value, as the
        extra height value will be added at top and also at bottom.

        This extra lenght is specified in tenths, referred to the first staff in the
        system on which the mark is placed.
    */
    MeasureHighlight* extra_height(Tenths value=0);

    /** Set the color of the mark.  */
    inline MeasureHighlight* color(Color value) { m_color = value; return this; }


    ///@}    //Customizable properties


    /// @name Access to current values
    ///@{

    /** Return the current mark color. */
    inline Color get_color() const { return m_color; }

    ///@}    //Access to current values


///@cond INTERNALS
//excluded from public API. Only for internal use.

    MeasureHighlight(GraphicView* view, LibraryScope& libraryScope);

    //mandatory overrides from VisualEffect
    void on_draw(BitmapDrawer* pDrawer) override;
    URect get_bounds() override;

    //initial data for position and context
    void initialize(LUnits xPos, LUnits xRight, GmoBoxSystem* pBoxSystem);

protected:
    friend class OverlaysGenerator;
    virtual ~MeasureHighlight() {}

///@endcond

};


}   //namespace lomse

#endif      //__LOMSE_MEASURE_HIGHLIGHT_H__
