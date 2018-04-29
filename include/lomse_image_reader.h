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

#if (LOMSE_ENABLE_PNG == 1)
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
#endif // LOMSE_ENABLE_PNG

}   //namespace lomse

#endif      //__LOMSE_IMAGE_READER_H__
