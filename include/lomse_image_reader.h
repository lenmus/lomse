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

#ifndef __LOMSE_IMAGE_READER_H__
#define __LOMSE_IMAGE_READER_H__

#include "lomse_file_system.h"

#include "lomse_image.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// ImageReader: knows how to read images. Delegates in a ImageDecoder
class ImageReader
{
public:
    ImageReader() {}
    ~ImageReader() {}

    static SpImage load_image(const string& locator);
};

//---------------------------------------------------------------------------------------
// ImageDecoder: abstract base for any image decoder
class ImageDecoder
{
public:
    ImageDecoder() {}
    virtual ~ImageDecoder() {}

    virtual bool can_decode(InputStream* file) = 0;
    virtual SpImage decode_file(InputStream* file) = 0;
};

//---------------------------------------------------------------------------------------
// JpgImageDecoder: knows how to read JPG images
class JpgImageDecoder : public ImageDecoder
{
public:
    JpgImageDecoder() : ImageDecoder() {}
    virtual ~JpgImageDecoder() {}

    //mandatory overrides
    bool can_decode(InputStream* file);
    SpImage decode_file(InputStream* file);
};

//---------------------------------------------------------------------------------------
// PngImageDecoder: knows how to read a PNG image
class PngImageDecoder : public ImageDecoder
{
public:
    PngImageDecoder() : ImageDecoder() {}
    virtual ~PngImageDecoder() {}

    //mandatory overrides
    bool can_decode(InputStream* file);
    SpImage decode_file(InputStream* file);

};


}   //namespace lomse

#endif      //__LOMSE_IMAGE_READER_H__
