//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#ifndef __LOMSE_IMAGE_H__
#define __LOMSE_IMAGE_H__

#include "lomse_basic.h"
#include "lomse_pixel_formats.h"

#include <string>
using namespace std;


namespace lomse
{

//---------------------------------------------------------------------------------------
//basic object to represent an image
//As images can take a lot of memory, to facilitate sharing instances the Image class
//is reference counted and a specific smart pointer class (SpImage) is defined
class Image
{
protected:
    unsigned char* m_bmap;
    VSize m_bmpSize;
    USize m_imgSize;
    EPixelFormat m_format;
    string m_error;

public:
    Image();
    Image(unsigned char* imgbuf, VSize bmpSize, EPixelFormat format, USize imgSize);
    virtual ~Image();

    //copy constructor and assignment operator
    Image(const Image& img);
    Image& operator=(const Image &img);

    //creation
    void load(unsigned char* imgbuf, VSize bmpSize, EPixelFormat format, USize imgSize);
    void set_error_msg(const string& msg);

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
    inline string& get_error_msg() { return m_error; }
    inline bool is_ok() { return m_error.empty(); }

    int get_bits_per_pixel();
    bool has_alpha();

protected:

};

typedef std::shared_ptr<Image>     SpImage;


}   //namespace lomse

#endif	// __LOMSE_IMAGE_H__

