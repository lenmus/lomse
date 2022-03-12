//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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
    bool can_decode(InputStream* file) override;
    SpImage decode_file(InputStream* file) override;
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
    bool can_decode(InputStream* file) override;
    SpImage decode_file(InputStream* file) override;

};
#endif // LOMSE_ENABLE_PNG

}   //namespace lomse

#endif      //__LOMSE_IMAGE_READER_H__
