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

#include "lomse_image_reader.h"

#include <png.h>
#include <pngconf.h>

#include <iostream>
using namespace std;

#include <cstdlib>
using ::malloc;
using ::free;

namespace lomse
{

//=======================================================================================
// ImageReader implementation
//=======================================================================================
SpImage ImageReader::load_image(const string& locator)
{
    InputStream* pFile = FileSystem::open_input_stream(locator);
    cout << "[ImageReader::load_image] " << locator << endl;

    //find a reader that can decode the file
    {
        //PNG Format
        PngImageDecoder decoder;
        if (decoder.can_decode(pFile))
            return decoder.decode_file(pFile);
    }
    {
        //JPG Format
        JpgImageDecoder decoder;
        if (decoder.can_decode(pFile))
            return decoder.decode_file(pFile);
    }
    throw("Image format not supported");
}

//=======================================================================================
// PngImageDecoder implementation
// See: http://www.libpng.org/pub/png/book/chapter13.html
//=======================================================================================


//=======================================================================================
//some helper internal functions, not declared as protected members to avoid
//having to contaminate the lomse header files with PNG types
//=======================================================================================
void read_callback(png_structp png, png_bytep data, png_size_t length)
{
    static_cast<InputStream*>( png_get_io_ptr(png) )->read(data, int(length));
}

//---------------------------------------------------------------------------------------
void error_callback (png_structp, png_const_charp)
{
    throw "error reading png image";
}

//=======================================================================================
// PngImageDecoder members implementation
//=======================================================================================
bool PngImageDecoder::can_decode(InputStream* file)
{
    //A PNG file starts with an 8-byte signature. The hexadecimal byte values are:
    // 89 50 4E 47 0D 0A 1A 0A; the decimal values are 137 80 78 71 13 10 26 10.
    //From http://en.wikipedia.org/wiki/Portable_Network_Graphics

    unsigned char header[8];

    if (file->read(header, 8) != 8)
        return false;

    cout << "header=" << hex << (int)header[0] << (int)header[1] << (int)header[2] << (int)header[3]
          << (int)header[4] << (int)header[5] << (int)header[6] << (int)header[7] << (int)header[8] << endl;
    return header[0] == 0x89
        && header[1] == 'P'
        && header[2] == 'N'
        && header[3] == 'G'
        && header[4] == 0x0D
        && header[5] == 0x0A
        && header[6] == 0x1A
        && header[7] == 0x0A;
}

//---------------------------------------------------------------------------------------
SpImage PngImageDecoder::decode_file(InputStream* file)
{
    //TODO: error checking and handling


    //create an empty image to be returned when errors
    Image* pImage = LOMSE_NEW Image();

    //create read struct
    png_structp pReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                     NULL, NULL, NULL);
    if (!pReadStruct)
        return SpImage(pImage);

    //create info struct
    png_infop pInfoStruct = png_create_info_struct(pReadStruct);
    if (!pInfoStruct)
    {
        png_destroy_read_struct(&pReadStruct, 0, 0);
        return SpImage(pImage );
    }


    png_set_error_fn(pReadStruct, 0, error_callback, error_callback );


    png_uint_32 width, height;
    int bitDepth, colorType, interlaceType;

    //we will take care of reading the file in our callback method
    png_set_read_fn(pReadStruct, file, read_callback);

    //inform about file position (we already read the 8 signature bytes)
    png_set_sig_bytes(pReadStruct, 8);

    //read all PNG info up to image data
    png_read_info(pReadStruct, pInfoStruct);

    //read image info. Don't care about compression_type and filter_type => NULLs
    png_get_IHDR(pReadStruct, pInfoStruct, &width, &height, &bitDepth, &colorType,
                 &interlaceType, NULL, NULL);

    //set some transformations
        //expand palette images to 24-bit RGB
    if (colorType == PNG_COLOR_TYPE_PALETTE)
        png_set_expand(pReadStruct);
        //expand low-bit-depth images to 8 bits
    if (bitDepth < 8)
        png_set_expand(pReadStruct);
        //expand transparency chunks to full alpha channel
    if (png_get_valid(pReadStruct, pInfoStruct, PNG_INFO_tRNS))
        png_set_expand(pReadStruct);
        //if more than 8 bits per colors, reduce to 8 bits
    if (bitDepth == 16)
        png_set_strip_16(pReadStruct);
        //convert grayscale to RGB[A]
    if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(pReadStruct);

    //TODO: gamma correction
//        //if the image doesn't have gamma info, don't do any gamma correction
//        if (png_get_gAMA(pReadStruct, pInfoStruct, &gamma))
//            png_set_gamma(pReadStruct, display_exponent, gamma);

    //set default alpha channel value to 255 (opaque)
    png_set_add_alpha(pReadStruct, 0xff, PNG_FILLER_AFTER);

    //all transformations have been defined.
    //Now allocate a buffer for the full bitmap
    unsigned char* imgbuf = NULL;
    int stride = int(width) * 4;
    if ((imgbuf = (unsigned char*)malloc(height * stride)) == NULL)
    {
        png_destroy_read_struct(&pReadStruct, &pInfoStruct, NULL);
        return SpImage(pImage);
    }

    //allocate pointers to rows
    png_bytepp pRows = NULL;
    if ((pRows = (png_bytepp)malloc(height*sizeof(png_bytep))) == NULL)
    {
        png_destroy_read_struct(&pReadStruct, &pInfoStruct, NULL);
        free(imgbuf);
        imgbuf = NULL;
        return SpImage(pImage);
    }

    //point row pointers to image buffer
    png_bytep pDest = (png_bytep)imgbuf;
    for (int y=0; y < int(height); ++y)
    {
        pRows[y] = pDest;
        pDest += stride;
    }

    //go ahead and read the whole image
    png_read_image(pReadStruct, pRows);
    png_read_end(pReadStruct, pInfoStruct);

    //image bitmap in RGBA format is now ready. Free memory used for row pointers
    free(pRows);
    pRows = NULL;

    //create the Image object
    VSize bmpSize(height, width);
    EPixelFormat format = k_pix_format_rgba32;
    //TODO: get display reolution from lomse initialization. Here it is assumed 96 ppi
    USize imgSize(float(height) * 2540.0f / 96.0f, float(width) * 2540.0f / 96.0f);
    delete pImage;
    pImage = LOMSE_NEW Image(imgbuf, bmpSize, format, imgSize);

    //delete helper structs
    png_destroy_read_struct(&pReadStruct, &pInfoStruct, 0);

    //done!
    return SpImage(pImage);
}



//=======================================================================================
// JpgImageDecoder implementation
//=======================================================================================
bool JpgImageDecoder::can_decode(InputStream* file)
{
//    unsigned char hdr[2];
//
//    if ( !stream.Read(hdr, WXSIZEOF(hdr)) )
//        return false;
//
//    return hdr[0] == 0xFF && hdr[1] == 0xD8;
////----------------------------
//    const int bytesNeeded = 10;
//    uint8 header [bytesNeeded];
//
//    if (in.read (header, bytesNeeded) == bytesNeeded)
//    {
//        return header[0] == 0xff
//            && header[1] == 0xd8
//            && header[2] == 0xff
//            && (header[3] == 0xe0 || header[3] == 0xe1);
//    }
//
//    return false;

    return false;
}

//---------------------------------------------------------------------------------------
SpImage JpgImageDecoder::decode_file(InputStream* file)
{
    //TODO
    return SpImage( LOMSE_NEW Image() );
}


}  //namespace lomse
