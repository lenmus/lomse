//---------------------------------------------------------------------------------------

/**
    @ingroup enumerations

    This enum describes the supported formats for the rendering buffer.

    \#include <lomse_pixel_formats.h>
*/
enum EPixelFormat
{
    k_pix_format_undefined = 0,  ///< By default. No conversions are applied 
    k_pix_format_gray8 = 2,      ///< Simple 256 level grayscale 
    k_pix_format_gray16 = 3,     ///< Simple 65535 level grayscale 
    k_pix_format_rgb555 = 4,     ///< 15 bit rgb. Architecture dependent due to byte ordering! 
    k_pix_format_rgb565 = 5,     ///< 16 bit rgb. Architecture dependent due to byte ordering!  
    k_pix_format_rgbAAA = 6,     ///< 30 bit rgb. Architecture dependent due to byte ordering!  
    k_pix_format_rgbBBA = 7,     ///< 32 bit rgb. Architecture dependent due to byte ordering!  
    k_pix_format_bgrAAA = 8,     ///< 30 bit bgr. Architecture dependent due to byte ordering!  
    k_pix_format_bgrABB = 9,     ///< 32 bit bgr. Architecture dependent due to byte ordering!  
    k_pix_format_rgb24 = 10,     ///< R-G-B, one byte per color component 
    k_pix_format_bgr24 = 11,     ///< B-G-R, native win32 BMP format 
    k_pix_format_rgba32 = 12,    ///< R-G-B-A, one byte per color component 
    k_pix_format_argb32 = 13,    ///< A-R-G-B, native MAC format 
    k_pix_format_abgr32 = 14,    ///< A-B-G-R, one byte per color component 
    k_pix_format_bgra32 = 15,    ///< B-G-R-A, native win32 BMP format 
    k_pix_format_rgb48 = 16,     ///< R-G-B, 16 bits per color component 
    k_pix_format_bgr48 = 17,     ///< B-G-R, native win32 BMP format 
    k_pix_format_rgba64 = 18,    ///< R-G-B-A, 16 bits byte per color component 
    k_pix_format_argb64 = 19,    ///< A-R-G-B, native MAC format 
    k_pix_format_abgr64 = 20,    ///< A-B-G-R, one byte per color component 
    k_pix_format_bgra64 = 21,    ///< B-G-R-A, native win32 BMP format 
};

