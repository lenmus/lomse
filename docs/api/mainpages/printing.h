/**

@page page-printing Printing documents overview

@tableofcontents

@section page-printing-overview The print API

Lomse is platform independent code and knows nothing about how to print in the operating system used by your application. Therefore, it is your application responsibility to implement printing. Lomse just offers some supporting methods so that implementing printing does not require much work. The default solution offered by Lomse for implementing printing in your application is based on printing bitmaps. Nevertheless, any user application can implement its own drawer classes and do all drawing natively without having to use bitmaps and the BitmapDrawer class implemented by Lomse. See @subpage page-user-drawers.


The Lomse print API is four methods, invoked in sequence:

-# First, determine how many pages to print. To know how many pages the document has, just call Interactor::get_num_pages().

-# In order to not interfere with screen display, a different rendering buffer and resolution are used by Lomse for printing. Therefore, you will have to:
    - Determine the current settings for printer resolution.
    - Determine the paper size, in pixels, based on printer resolution and the paper size that is going to be used, and inform Lomse by invoking Interactor::set_print_paper_size(). Lomse will automatically scale the score pages so that the fit on the specified size.
    - Determine the bitmap size to used (see @ref page-printing-buffer-size), allocate memory for this buffer and inform Lomse by invoking Interactor::set_printing_buffer().

-# Now, invoke Interactor::print_page() to request Lomse to render a page in the provided buffer. 

-# Finally, print the bitmap using the facilities of your operating system.

Normally, your application will have to do the last two steps as many times as pages to print.

Example (using wxWidgets, adapt for your framework/OS):

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
        spInteractor->set_print_page_size(paperWidthPixels, paperHeightPixels);

        //allocate print buffer
        Bitmap img(paperWidthPixels, paperHeightPixels);
        unsigned char* pdata = img.GetBuffer();    //ptr to the real bytes buffer
        spInteractor->set_print_buffer(pdata, paperWidthPixels, paperHeightPixels);

        //render page on bitmap
        spInteractor->print_page(page-1);

        //and send it to the printer
        pDC->DrawBitmap(img, 0, 0);
    }
}
@endcode


@section page-printing-buffer-size Determining print buffer size

The necesary bitmap size depends on the desired printing resolution and paper size. Take, for example, a sheet of DIN A4 paper; it measures 210mm X 297mm. For printing on it, the necessary bitmap size will depend on the required resolution, as given by this formula:

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


@section page-printing-tiles Save memory by tiling

Using large size papers or printing at very high resolutions can require huge bitmaps. A technique for avoiding this is to split the page in tiles and print tile by tile. To avoid artifacts in the borders of each tile (due to anti-aliasing) it is recommended to add a border to each tile and discard it once Lomse has rendered on the tile. Here you have an example using the wxWidgets framework:

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
        spInteractor->set_print_page_size(paperWidthPixels, paperHeightPixels);

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
        Bitmap img(width, height);
        unsigned char* pdata = img.GetBuffer();    //ptr to the real bytes buffer
        spInteractor->set_print_buffer(pdata, width, height);

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


@section page-printing-other Other ways of printing

If you do not like the idea of using bitmaps for printing it is feasible to write a Driver class to perform printing using methods specific to your application and operating system. See @ref page-user-drawers


*/

