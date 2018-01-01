/**
@example example_1_qt.cpp
Code for Qt example-1: How to display a music score

@code
//---------------------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http:unlicense.org>
//---------------------------------------------------------------------------------------

#include "example_1_qt.h"

//=======================================================================================
// main: the entry point
//=======================================================================================

int main(int argc, char* args[])
{
    QApplication app(argc, args);
    app.setOrganizationName("LenMus");
    app.setApplicationName("Lomse. Tutorial 1");

    MainWindow window;
    window.show();

    return app.exec();
}

//=======================================================================================
// MainWindow implementation
//=======================================================================================
MainWindow::MainWindow()
    : QMainWindow()
    , m_canvas(NULL)
{
    // create our one and only child: the canvas to display the score
    m_canvas = new MyCanvas(this, m_lomse);
    setCentralWidget(m_canvas);
    m_canvas->setMinimumSize(100, 100);

    setWindowTitle(tr("Lomse sample 1 for Qt"));
    create_actions();
    create_menu();
    initialize_lomse();

    resize(790, 400);

    // load the score to display
    open_test_document();
}

//---------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
}

//---------------------------------------------------------------------------------------
void MainWindow::create_menu()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_exitAction);

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAction);
}

//---------------------------------------------------------------------------------------
void MainWindow::create_actions()
{
    m_aboutAction = new QAction(tr("&About"), this);
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(on_about()));

    m_exitAction = new QAction(tr("E&xit"), this);
    connect(m_exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

//---------------------------------------------------------------------------------------
void MainWindow::on_about()
{
    QMessageBox::about(this, tr("About Lomse sample for Qt"),
                             tr("This is Lomse sample 1 for Qt"));
}

//---------------------------------------------------------------------------------------
void MainWindow::initialize_lomse()
{
    // Lomse knows nothing about windows. It renders everything on bitmaps and the
    // user application uses them. As Lomse supports a lot of pixel formats, before
    // using the Lomse library we MUST specify which bitmap format should Lomse use.
    // Lomse bitmaps are platform dependent because byte order is different in
    // big endian and little endian architectures.
    //
    // For Qt we will use a QImage for rendering the bitmap. A suitable format for
    // the QImage is QImage::Format_RGBA8888. According to Qt documentation this format
    // is a byte-ordered format, which means the 32bit encoding differs between
    // big endian and little endian architectures, being respectively (0xRRGGBBAA) 
    // and (0xAABBGGRR). So this format matches Lomse format k_pix_format_rgba32.

    // Therefore, I will use pixel format the pixel format k_pix_format_rgba32 for
    // Lomse and QImage::Format_RGBA8888 for Qt. Both are internally the same format:
    // an array of pixels in the top-to-bottom, left-to-right order, Each pixel is
    // encoded in four bytes.
    //
    // Let's the initialize Lomse with the requiered information:

    //the pixel format
    int pixel_format = k_pix_format_rgba32;

    //the desired resolution. For Linux and Windows 96 pixels per inch works ok.
    int resolution = 96;    //96 ppi

    //Normal y axis direction is 0 coordinate at top and increase downwards. You
    //must specify if you would like just the opposite behaviour. For Windows and
    //Linux the default behaviour is the right behaviour.
    bool reverse_y_axis = false;

    //Now, initialize the library with these values
    m_lomse.init_library(pixel_format,resolution, reverse_y_axis);

}

//---------------------------------------------------------------------------------------
void MainWindow::open_test_document()
{
    m_canvas->open_test_document();
}



//=======================================================================================
// MyCanvas implementation
//=======================================================================================
MyCanvas::MyCanvas(QWidget* parent, LomseDoorway& lomse)
    : QWidget(parent)
    , m_lomse(lomse)
    , m_pPresenter(NULL)
    , m_pdata(NULL)
    , m_view_needs_redraw(true)
{
}

//---------------------------------------------------------------------------------------
MyCanvas::~MyCanvas()
{
    delete_rendering_buffer();

    //delete the Presenter. This will also delete the Document, the Interactor,
    //the View and other related objects
    delete m_pPresenter;
}

//---------------------------------------------------------------------------------------
void MyCanvas::resizeEvent(QResizeEvent* event)
{
    QSize size = event->size();
    create_rendering_buffer(size.width(), size.height());
}

//---------------------------------------------------------------------------------------
void MyCanvas::paintEvent(QPaintEvent* event)
{
    if (m_pPresenter)
    {
        update_rendering_buffer_if_needed();
        if (!m_pdata)
            return;

        QPainter painter(this);
        QImage image(m_pdata, m_nBufWidth, m_nBufHeight, QImage::Format_RGBA8888);
        QRect dirtyRect = event->rect();
        painter.drawImage(dirtyRect, image, dirtyRect);
    }
}


//-------------------------------------------------------------------------
void MyCanvas::open_test_document()
{
    //Normally you will load the content of a file. But in this
    //simple example we will create an empty document and define its content
    //from a text string

    //First, we will create a 'Presenter' object. It takes care of cretaing 
    //and maintaining all objects and relationships between the document, 
    //its views and the interactors to interact with the view
    delete m_pPresenter;
    m_pPresenter = m_lomse.new_document(ViewFactory::k_view_vertical_book,
        "(lenmusdoc (vers 0.0)"
            "(content "
                "(para (txt \"Hello world!\"))"
                "(score (vers 2.0) "
                    "(instrument (musicData (clef G)(key C)(time 2 4)(n c4 q) )))"
            ")"
        ")",
        Document::k_format_ldp
    );

    //get the pointer to the interactor, set the rendering buffer and 
    //register for receiving desired events
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        //connect the View with the window buffer
        spInteractor->set_rendering_buffer(&m_rbuf_window);
    }
}

//-------------------------------------------------------------------------
void MyCanvas::update_view_content()
{
    if (!m_pPresenter) return;

    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
        spInteractor->redraw_bitmap();
}

//---------------------------------------------------------------------------------------
void MyCanvas::update_rendering_buffer_if_needed()
{
    if (m_view_needs_redraw)
        update_view_content();

    m_view_needs_redraw = false;
}

//---------------------------------------------------------------------------------------
void MyCanvas::delete_rendering_buffer()
{
    free(m_pdata);
    m_pdata = NULL;
}


//---------------------------------------------------------------------------------------
void MyCanvas::create_rendering_buffer(int width, int height)
{
    //allocate memory for the Lomse rendering buffer.
    //Any existing buffer is automatically deleted by Lomse.

    #define BYTES_PER_PIXEL 4   //the chosen format is RGBA, 32 bits

    // allocate a new rendering buffer
    delete_rendering_buffer();
    m_nBufWidth = width;
    m_nBufHeight = height;
    m_pdata = (unsigned char*)malloc(m_nBufWidth * m_nBufHeight * BYTES_PER_PIXEL);

    //Attach this memory to be used as Lomse rendering buffer
    int stride = m_nBufWidth * BYTES_PER_PIXEL;     //number of bytes per row
    m_rbuf_window.attach(m_pdata, m_nBufWidth, m_nBufHeight, stride);

    m_view_needs_redraw = true;
}

@endcode
*/

