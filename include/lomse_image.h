//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

