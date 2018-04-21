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

#include "lomse_image_reader.h"

#include "lomse_logger.h"

#include <png.h>
#include <pngconf.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace std;

#include <cstdlib>
using ::malloc;
using ::free;

namespace lomse
{

//declaration of some internal functions, to avoid compiler warnings
void read_callback(png_structp png, png_bytep data, png_size_t length);
void error_callback (png_structp, png_const_charp);
void warning_callback (png_structp, png_const_charp);


//=======================================================================================
// ImageReader implementation
//=======================================================================================
SpImage ImageReader::load_image(const string& locator)
{
    InputStream* pFile = nullptr;
    try
    {
        pFile = FileSystem::open_input_stream(locator);
        //find a reader that can decode the file
        {
            //PNG Format
            PngImageDecoder decoder;
            if (decoder.can_decode(pFile))
            {
                SpImage img = decoder.decode_file(pFile);
                delete pFile;
                return img;
            }
        }
        {
            //JPG Format
            JpgImageDecoder decoder;
            if (decoder.can_decode(pFile))
            {
                SpImage img = decoder.decode_file(pFile);
                delete pFile;
                return img;
            }
        }

        //other formats not supported. throw error
        delete pFile;
        stringstream s;
        s << "[ImageReader::load_image] Image format not supported. Locator: "
          << locator;
        LOMSE_LOG_ERROR(s.str());
        throw runtime_error(s.str());
    }
    catch(exception& e)
    {
        SpImage img( LOMSE_NEW Image() );
        img->set_error_msg(e.what());
        cerr << e.what() << " (catch in ImageReader::load_image)" << endl;
        return img;
    }
    catch(...)
    {
        SpImage img( LOMSE_NEW Image() );
        img->set_error_msg("Non-standard unknown exception");
        cerr << "Non-standard unknown exception (catch in ImageReader::load_image)" << endl;
        return img;
    }
    return SpImage( LOMSE_NEW Image() );   //compiler happy
}

//=======================================================================================
// PngImageDecoder implementation
//
// See: http://www.libpng.org/pub/png/book/chapter13.html
//      http://www.piko3d.com/tutorials/libpng-tutorial-loading-png-files-from-streams
//=======================================================================================


//=======================================================================================
//some helper internal functions, not declared as protected members to avoid
//having to contaminate the lomse header files with PNG types
//=======================================================================================
void read_callback(png_structp png, png_bytep data, png_size_t length)
{
    static_cast<InputStream*>( png_get_io_ptr(png) )->read(data, long(length));
}

//---------------------------------------------------------------------------------------
void error_callback (png_structp, png_const_charp)
{
    LOMSE_LOG_ERROR("error reading png image");
    throw "error reading png image";
}

//---------------------------------------------------------------------------------------
void warning_callback (png_structp, png_const_charp msg)
{
    LOMSE_LOG_WARN("warning reading png image: %s", msg);
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

    if (file->read(header, 8) == 8 && png_check_sig(header, 8))
        return true;
    else
    {
        //TODO: rewind file
        return false;
    }
}

//---------------------------------------------------------------------------------------
SpImage PngImageDecoder::decode_file(InputStream* file)
{
    //TODO: error checking and handling

    //create an empty image to be returned when errors
    Image* pImage = LOMSE_NEW Image();

    //create read struct
    png_structp pReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                     nullptr, nullptr, nullptr);
    if (!pReadStruct)
    {
        pImage->set_error_msg("[PngImageDecoder::decode_file] out of memory creating read struct");
        return SpImage(pImage);
    }

    //create info struct
    png_infop pInfoStruct = png_create_info_struct(pReadStruct);
    if (!pInfoStruct)
    {
        pImage->set_error_msg("[PngImageDecoder::decode_file] out of memory creating info struct");
        png_destroy_read_struct(&pReadStruct, 0, 0);
        return SpImage(pImage );
    }


    png_set_error_fn(pReadStruct, 0, error_callback, warning_callback);


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
                 &interlaceType, nullptr, nullptr);

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
    unsigned char* imgbuf = nullptr;
    int stride = int(width) * 4;
    if ((imgbuf = (unsigned char*)malloc(height * stride)) == nullptr)
    {
        pImage->set_error_msg("[PngImageDecoder::decode_file] error allocating memory for image");
        png_destroy_read_struct(&pReadStruct, &pInfoStruct, nullptr);
        return SpImage(pImage);
    }

    //allocate pointers to rows
    png_bytepp pRows = nullptr;
    if ((pRows = (png_bytepp)malloc(height*sizeof(png_bytep))) == nullptr)
    {
        pImage->set_error_msg("[PngImageDecoder::decode_file] error allocating memory for line ptrs.");
        png_destroy_read_struct(&pReadStruct, &pInfoStruct, nullptr);
        free(imgbuf);
        imgbuf = nullptr;
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
    pRows = nullptr;

    //create the Image object
    VSize bmpSize(width, height);
    EPixelFormat format = k_pix_format_rgba32;
    //TODO: get display reolution from lomse initialization. Here it is assumed 96 ppi
    USize imgSize(float(width) * 2540.0f / 96.0f, float(height) * 2540.0f / 96.0f);
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
bool JpgImageDecoder::can_decode(InputStream* UNUSED(file))
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
SpImage JpgImageDecoder::decode_file(InputStream* UNUSED(file))
{
    //TODO: JpgImageDecoder::decode_file
    return SpImage( LOMSE_NEW Image() );
}


}  //namespace lomse
