//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
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

#include "lomse_right_aligner.h"

#include <algorithm>

using namespace std;

namespace lomse
{

#define LOMSE_MAX_X     -1.0f     //infinite
#define LOMSE_MAX_Y     -1.0f     //infinite

//---------------------------------------------------------------------------------------
// Implementation of RightAligner
//---------------------------------------------------------------------------------------
RightAligner::RightAligner()
    : m_width(0.0f)
{
    add_border_segment(0.0f, LOMSE_MAX_Y, LOMSE_MAX_X);
}

//---------------------------------------------------------------------------------------
RightAligner::~RightAligner()
{
}

//---------------------------------------------------------------------------------------
int RightAligner::add_box(URect rect)
{
    //Add a new box by 'pushing' it from the right until it touches exiting content,
    //as if the right was the bottom and the boxes were drop from top.

    //determine touch x pos
    LUnits xPos = get_touch_x_pos(rect.y, rect.y+rect.height);
    rect.x = xPos - rect.width;
    if (rect.x < 0.0f)
    {
        shift_boxes_right( -rect.x );
        shift_border( -rect.x );
        rect.x = 0.0f;
    }

    //add new box
    m_boxes.push_back(rect);
    add_border_segment(rect.y, rect.y+rect.height, rect.x);

    return int( m_boxes.size() ) - 1;
}

//---------------------------------------------------------------------------------------
URect RightAligner::get_box(int iBox)
{
    if (int(m_boxes.size()) > iBox)
    {
        std::list<URect>::iterator it = m_boxes.begin();
        std::advance(it, iBox);
        return *it;
    }
    else
        return URect();
}

//---------------------------------------------------------------------------------------
void RightAligner::shift_boxes_right(LUnits shift)
{
    std::list<URect>::iterator it;
    for (it = m_boxes.begin(); it != m_boxes.end(); ++it)
    {
        (*it).x += shift;
    }
	m_width += shift;
}

//---------------------------------------------------------------------------------------
void RightAligner::shift_border(LUnits shift)
{
    std::list<UPoint>::iterator it;
    for (it = m_border.begin(); it != m_border.end(); ++it)
    {
        if ((*it).x != LOMSE_MAX_X)
            (*it).x += shift;
    }
}

//---------------------------------------------------------------------------------------
void RightAligner::add_border_segment(LUnits y0, LUnits y1, LUnits x)
{
    if (m_border.size() == 0)
    {
        m_border.push_back( UPoint(x, y0) );
        m_border.push_back( UPoint(LOMSE_MAX_X, y1) );
    }
    else
    {
        //find first point with y >= y0
        LUnits xNext = LOMSE_MAX_X;
        std::list<UPoint>::iterator it;
        for (it = m_border.begin(); it != m_border.end(); ++it)
        {
            if ((*it).y == y0)
            {
                //replace
                xNext = (*it).x;
                (*it).x = x;
                ++it;
                break;
            }
            else if ((*it).y > y0 || (*it).y == LOMSE_MAX_Y)
            {
                //insert before
                m_border.insert(it, UPoint(x, y0));  //'it' still points to greater point
                break;
            }
            xNext = (*it).x;
        }

        //insert second point
        if (it != m_border.end())   //sanity check: first point found
        {
            //remove intermediate points
            while((*it).y < y1 && (*it).y != LOMSE_MAX_Y)
            {
                xNext = (*it).x;
                m_border.erase(it++);
            }

            if ((*it).y == y1)
            {
                //no need to do anything. point is still valid!
            }
            else if ((*it).y > y1 || (*it).y == LOMSE_MAX_Y)
            {
                //insert before
                UPoint p2(xNext, y1);
                m_border.insert(it, p2);
            }
        }
    }
}

//---------------------------------------------------------------------------------------
LUnits RightAligner::get_touch_x_pos(LUnits y0, LUnits y1)
{
    LUnits xPos = m_width;
    std::list<UPoint>::iterator it;
    for (it = m_border.begin(); it != m_border.end(); ++it)
    {
        if (y0 <= (*it).y || (*it).y == LOMSE_MAX_Y)
        {
            if (xPos == LOMSE_MAX_X)
                xPos = m_width;
            break;
        }
        xPos = (*it).x;
    }

    //start x of segment found. locate end of segment

    for (; it != m_border.end(); ++it)
    {
        //skip intermediate points
        while((*it).y < y1 && (*it).y != LOMSE_MAX_Y)
        {
            if ((*it).x != LOMSE_MAX_X)
                xPos = min(xPos, (*it).x);
            ++it;
        }

        if ((*it).y > y1 || (*it).y == LOMSE_MAX_Y)
        {
            //this point is out. Use x of previous point
            break;
        }
    }

    return xPos;
}

//---------------------------------------------------------------------------------------
LUnits RightAligner::get_total_height()
{
    list<UPoint>::iterator it = m_border.end();
    --it;   //last element (-1, -1)
    --it;   //element with higher y coord.
    return (*it).y;
}


}  //namespace lomse
