/**

@page print-api Printing documents overview

@tableofcontents

@section print-overview The print API

Lomse is platform independent code and knows nothing about how to print in the operating system used by your application. Therefore, it is your application responsibility to implement printing. Lomse just offers some supporting methods so that implementing printing does not require much work. In fact, implementing printing in your application is just printing bitmaps.

The Lomse print API consists in four methods, invoked in sequence:

-# First, determine how many pages to print. To know how many pages the document has, just call Interactor::get_num_pages().

-# In order to not interfere with screen display, a different rendering buffer and resolution are used by Lomse for printing. Therefore, you will have to:
    - Determine the current settings for printer resolution and inform Lomse by invoking Interactor::set_print_ppi().
    - Determine the bitmap size to used (see @ref print-buffer-size), allocate memory for this buffer and inform Lomse by invoking Interactor::set_printing_buffer().
    
-# Now, invoke Interactor::print_page() to request Lomse to render a page in the provided buffer. 

-# Finally, print the bitmap using the facilities of your operating system.

Normally, your application will have to do the last two steps as many times as pages to print.

Example:

@code
void DocumentWindow::print_page(PrinterDC* pDC, int page, int paperWidthPixels,
                                int paperHeightPixels)
{
    if (!m_pPresenter)
        return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        pDC->SetBackground(*WHITE_BRUSH);
        pDC->Clear();

        //inform Lomse about printer resolution
        int dpi = pDC->GetPPI();
        spInteractor->set_print_ppi( double(dpi) );

        //allocate print buffer
        RenderingBuffer rbuf_print;
        #define BYTES_PP 3      //3 bytes per pixel (RGB 24-bits). This must be the
                                //same format that was set when initializing Lomse
        int stride = paperWidthPixels * BYTES_PP;          //number of bytes per row

        Bitmap img(paperWidthPixels, paperHeightPixels);
        unsigned char* pdata = img.GetBuffer();    //ptr to the real bytes buffer
        rbuf_print.attach(pdata, paperWidthPixels, paperHeightPixels, stride);
        spInteractor->set_print_buffer(&rbuf_print);

        //render page on bitmap
        spInteractor->print_page(page-1);

        //and send it to the printer
        pDC->DrawBitmap(img, 0, 0);
    }
}
@endcode


@section print-buffer-size Determining print buffer size

The necesary bitmap size depends on the desired printing resolution and paper size.

For instance, a sheet of DIN A4 paper measures 210mm X 297mm. For printing on it the necessary bitmap size will depend on the required resolution, as given by this formula:

    pixels = size(mm) * resolution (dpi) / 25,4 

For instance, here you have the required bitmap sizes for some printing resolutions:

| Printing resolution | Bitmap size for A4 paper |
|------------|--------------------|
|    72 dpi  |  595 x  842 pixels |
|   150 dpi  | 1240 x 1754 pixels |
|   300 dpi  | 2480 x 3508 pixels |
|   600 dpi  | 4960 x 7016 pixels |

A list of paper sizes can be found <a href='https://en.wikipedia.org/wiki/Paper_size'>here</a>.

Probably, when a print operation is requested, your operating system has methods for determining the required buffer size and printer resolution, so your application will not have to do the computations.


@section print-tiles Save memory by tiling

Using large size papers or printing at very high resolutions can require huge bitmaps. A technique for avoiding this is to split the page in tiles and print tile by tile. To avoid artifacts in the borders of each tile (due to anti-aliasing) it is recommended to add a border to each tile and discard it once Lomse has rendered on the tile. Here you have an example:

@code
void DocumentWindow::print_page(PrinterDC* pDC, int page, int paperWidthPixels,
                                int paperHeightPixels)
{
    if (!m_pPresenter)
        return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        pDC->SetBackground(*WHITE_BRUSH);
        pDC->Clear();

        //inform Lomse about printer resolution
        int dpi = pDC->GetPPI();
        spInteractor->set_print_ppi( double(dpi) );

        //determine tile size (pixels)
        int width = min(1024, paperWidthPixels);
        int height = min(1024, paperHeightPixels);
        int border = 8;
        if (width < 1024 && height < 1024)
            border = 0;

        // B = border, w = width
        //From paper viewpoint, for copying a tile into paper, copy origin is
        //at (B, B) and copy size is (w-2B, h-2B). Initial paper org is at (0,0).
        //From render viewpoint, initial viewport origin is at (B, B) and tiles
        //size (for advancing viewport origin) is also (w-2B, h-2B).
        VPoint viewport(0,0);
        VPoint paperPos(0,0);
        int tileW = width - 2 * border;
        int tileH = height - 2 * border;

        //determine how many tiles to print
        int rows = int( ceil(float(paperHeightPixels) / float(tileH)) );
        int cols = int( ceil(float(paperWidthPixels) / float(tileW)) );

        //determine last row and last column tile sizes
        int lastW = paperWidthPixels - tileW * (cols - 1);
        int lastH = paperHeightPixels - tileH * (rows - 1);

        //allocate print buffer
        RenderingBuffer rbuf_print;
        #define BYTES_PP 3      //3 bytes per pixel (RGB 24-bits). This must be the
                                //same format that was set when initializing Lomse
        int stride = width * BYTES_PP;          //number of bytes per row

        Bitmap img(width, height);
        unsigned char* pdata = img.GetBuffer();    //ptr to the real bytes buffer
        rbuf_print.attach(pdata, width, height, stride);
        spInteractor->set_print_buffer(&rbuf_print);

        //loop to print tiles.
        MemoryDC memoryDC;
        for (int iRow=0; iRow < rows; ++iRow)
        {
            for (int iCol=0; iCol < cols; ++iCol)
            {
                spInteractor->print_page(page-1, viewport);

                //print this tile
                int tileWidth = (iCol == cols-1 ? lastW : tileW);
                int tileHeight = (iRow == rows-1 ? lastH : tileH);

                if (border > 0)
                {
                    memoryDC.SelectObjectAsSource(img);
                    pDC->Blit(paperPos.x, paperPos.y, tileWidth, tileHeight,
                              &memoryDC, border, border);
                    memoryDC.ReleaseSelectedObject();
                }
                else
                    pDC->DrawBitmap(bitmap, paperPos.x, paperPos.y);

                //advance origin by tile size
                viewport.x += tileW;
                paperPos.x += tileW;
            }
            //start new row
            viewport.x = 0;
            viewport.y += tileH;
            paperPos.x = 0;
            paperPos.y += tileH;
        }
    }
}
@endcode

*/

