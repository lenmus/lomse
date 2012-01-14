//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_IMAGE_H__
#define __LOMSE_IMAGE_H__

#include "lomse_basic.h"
#include "lomse_pixel_formats.h"
#include "lomse_smart_pointer.h"

#include <string>
using namespace std;


namespace lomse
{

//---------------------------------------------------------------------------------------
//basic object to represent an image
//As images can take a lot of memory, to facilitate sharing instances the Image class
//is reference counted and a specific smart pointer class (SpImage) is defined
class Image : public RefCounted
{
protected:
    unsigned char* m_bmap;
    VSize m_bmpSize;
    USize m_imgSize;
    EPixelFormat m_format;

public:
    Image() : m_bmap(NULL) {}
    Image(unsigned char* imgbuf, VSize bmpSize, EPixelFormat format, USize imgSize);
    virtual ~Image();

    //creation
    void load(unsigned char* imgbuf, VSize bmpSize, EPixelFormat format, USize imgSize);
    void load_from_file(const string& locator);

    //accessors
    inline unsigned char* get_buffer() { return m_bmap; }
    inline LUnits get_image_width() { return m_imgSize.width; }
    inline LUnits get_image_height() { return m_imgSize.height; }
    inline USize& get_image_size() { return m_imgSize; }
    inline Pixels get_bitmap_width() { return m_bmpSize.width; }
    inline Pixels get_bitmap_height() { return m_bmpSize.height; }
    inline VSize& get_bitmap_size() { return m_bmpSize; }
    int get_stride() { return m_bmpSize.width * (get_bits_per_pixel() / 8); }
    inline int get_format() { return m_format; }

    int get_bits_per_pixel();
    bool has_alpha();

protected:

};

typedef SmartPtr<Image>     SpImage;


}   //namespace lomse

#endif	// __LOMSE_IMAGE_H__

