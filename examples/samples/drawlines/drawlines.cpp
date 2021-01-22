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

#include "drawlines.h"

#include "lomse_score_algorithms.h"


//=======================================================================================
// main: the entry point
//=======================================================================================

int main(int argc, char* args[])
{
    QApplication app(argc, args);
    app.setOrganizationName("LenMus");
    app.setApplicationName("Lomse. Drawlines sample");

    MainWindow window;
    window.show();

    return app.exec();
}

//=======================================================================================
// MainWindow implementation
//=======================================================================================
MainWindow::MainWindow()
    : QMainWindow()
    , m_canvas(nullptr)
    , m_startObjId(k_no_imoid)
    , m_startLineId(k_no_imoid)
    , m_endObjId(k_no_imoid)
    , m_endLineId(k_no_imoid)
{
    // create our one and only child: the canvas to display the score
    m_canvas = new MyCanvas(this, m_lomse);
    setCentralWidget(m_canvas);
    m_canvas->setMinimumSize(100, 100);

    setWindowTitle(tr("Lomse: 'drawlines' sample for Qt"));
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

    m_linesMenu = menuBar()->addMenu(tr("&Lines"));
    m_linesMenu->addAction(m_addLinesAction);
    m_linesMenu->addAction(m_removeLinesAction);

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

    m_addLinesAction = new QAction(tr("Add lines"), this);
    connect(m_addLinesAction, SIGNAL(triggered()), this, SLOT(on_add_lines()));

    m_removeLinesAction = new QAction(tr("Remove lines"), this);
    connect(m_removeLinesAction, SIGNAL(triggered()), this, SLOT(on_remove_lines()));
}

//---------------------------------------------------------------------------------------
void MainWindow::on_about()
{
    QMessageBox::about(this, tr("About this sample"),
                             tr("Lomse: 'drawlines' sample for Qt"));
}

//---------------------------------------------------------------------------------------
void MainWindow::on_add_lines()
{
    SpDocument doc = m_canvas->get_document();
    if (doc)
    {
        //get a raw pointer to the document
        Document* pDoc = doc.get();
        //Use the document to get the score to edit
        //The next line of code is just an example, in which it is assumed that
        //the score to edit is the first element in the document, as it is the case in
        //this sample. Also, this will be always the case when editing MusicXML
        //imported files.
        ImoScore* pScore = dynamic_cast<ImoScore*>( pDoc->get_content_item(0) );
        if (pScore)
        {

            //We are going to insert a vertical line at start of second measure

            //for inserting notes or other objects it is necessary to determine the
            //insertion point. For locating the insertion point there are
            //many possibilities, usually involving an iterator or a cursor object.

            //Lets use a cursor object:
            //after creation the cursor will be pointing to the first object in
            //the score, the clef in this example
            ScoreCursor cursor(pDoc, pScore);

            //Now move the cursor to the first barline
            int measure = 1;            //0 based. Measure 1 is the measure that starts after first barline
            int instr = 0;              //0 based. First and only instrument
            int staff = 0;              //0 based. First staff of the selected instrument
            cursor.to_measure(measure, instr, staff);     //cursor will point to first note after first barline
            cursor.move_prev();         //move back to point to the barline

            //get the pointed object
            ImoStaffObj* pAt = cursor.staffobj();

            //Let's create the start line in green color
            //All measurements are in tenths of staff line spacing. The
            //positions are relative to current object position.
            //see https://lenmus.github.io/ldp/auxobjs/graphical-objects.html
            string startLine("(line"
                            "(startPoint (dx 0) (dy -30.0))"      //relative to current object position, in tenths of staff line spacing
                            "(endPoint (dx 0) (dy 70.0))"
                            "(width 6.0)"       //in thenths
                            "(color #00ff0080)"   //in #rrggbb or #rrggbbaa format
                            "(lineStyle solid)"
                            "(lineCapStart none)"
                            "(lineCapEnd none) )"
                       );
            ImoAuxObj* pAO = dynamic_cast<ImoAuxObj*>(pDoc->create_object_from_ldp(startLine));

            //attach the line to the object and save its reference so that we can delete it later
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //VERY IMPORTANT: You MUST NOT store pointers to objects in the Document, as they can
            //be invalidated at any moment without your knowledge. Instead you must use its internal ID
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            if (pAO)
            {
                pAt->add_attachment(pDoc, pAO);
                m_startLineId = pAO->get_id();
                m_startObjId = pAt->get_id();
            }

            //Now the end line, in red color
            string endLine("(line"
                            "(startPoint (dx 0) (dy -30.0))"
                            "(endPoint (dx 0) (dy 70.0))"
                            "(width 6.0)"       //in thenths
                            "(color #ff000080)"   //in #rrggbb or #rrggbbaa format
                            "(lineStyle solid)"
                            "(lineCapStart none)"
                            "(lineCapEnd none) )"
                       );
            pAO = dynamic_cast<ImoAuxObj*>(pDoc->create_object_from_ldp(endLine));

            //position the cursor to next barline. As an example, I will move the cursor
            //by iterating
            cursor.move_next();
            pAt = cursor.staffobj();;
            while (pAt && !pAt->is_barline())
            {
                cursor.move_next();
                pAt = cursor.staffobj();
            }
            pAt = cursor.staffobj();

            //attach the line to the object and save its reference
            if (pAO)
            {
                pAt->add_attachment(pDoc, pAO);
                m_endLineId = pAO->get_id();
                m_endObjId = pAt->get_id();
            }

            //once the updates are finished, invoke close() method for
            //updating the internal data structures. This equivalent to
            //invoking pDoc->end_of_changes() but will only rebuild
            //the structures associated to the modified score
            pScore->end_of_changes();

            //as we are not going to do more modifications in the document,
            //notify all observers (i.e. to repaint the window)
            pDoc->notify_if_document_modified();
        }
    }
}

//---------------------------------------------------------------------------------------
void MainWindow::on_remove_lines()
{
    SpDocument doc = m_canvas->get_document();
    if (doc && m_startLineId != k_no_imoid && m_endLineId != k_no_imoid)
    {
        Document* pDoc = doc.get();

        //remove the first line
        ImoStaffObj* pSO = dynamic_cast<ImoStaffObj*>( pDoc->get_pointer_to_imo(m_startObjId) );
        ImoAuxObj* pAO = dynamic_cast<ImoAuxObj*>( pDoc->get_pointer_to_imo(m_startLineId) );
        pSO->remove_attachment(pAO);

        //remove the second line
        pSO = dynamic_cast<ImoStaffObj*>( pDoc->get_pointer_to_imo(m_endObjId) );
        pAO = dynamic_cast<ImoAuxObj*>( pDoc->get_pointer_to_imo(m_endLineId) );
        pSO->remove_attachment(pAO);

        //end of modifications
        pDoc->end_of_changes();
        pDoc->notify_if_document_modified();
    }
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

    //the desired resolution: 96 pixels per inch
    int resolution = 96;

    //Now, initialize the library with these values
    m_lomse.init_library(pixel_format, resolution);

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
    , m_pPresenter(nullptr)
    , m_pdata(nullptr)
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
    m_pPresenter = m_lomse.new_document(k_view_vertical_book,
        "(lenmusdoc (vers 0.0)"
            "(content "
                "(score (vers 2.0) "
                    "(instrument (musicData"
                        "(clef G)(key C)(time 6 8)(n d4 e v1)(barline simple)"
                        "(n f4 q v1)(n g4 e v1)(n a4 e. v1 (beam 14 +))(n b4 s v1 (beam 14 =b))"
                        "(n a4 e v1 (beam 14 -))(barline simple)"
                        "(n g4 q v1)(n e4 e v1)(n c4 e. v1 (beam 21 +))(n d4 s v1 (beam 21 =b))"
                        "(n e4 e v1 (beam 21 -))(barline simple)"
                        "(n f4 q v1)(n d4 e v1)(n d4 e v1 (beam 28 +))(n c4 e v1 (beam 28 =))"
                        "(n d4 e v1 (beam 28 -))(barline simple)"
                        "(n e4 q v1)(n c4 e v1)(n a3 q v1)(n d4 e v1)(barline simple)"
                        "(n f4 q v1)(n g4 e v1)(n a4 e. v1 (beam 40 +))(n b4 s v1 (beam 40 =b))"
                        "(n a4 e v1 (beam 40 -))(barline simple)"
                        "(n g4 q v1)(n e4 e v1)(n c4 e. v1 (beam 47 +))(n d4 s v1 (beam 47 =b))"
                        "(n e4 e v1 (beam 47 -))(barline simple)"
                        "(n f4 e. v1 (beam 52 +))(n e4 s v1 (beam 52 =b))(n d4 e v1 (beam 52 -))"
                        "(n +c4 e v1 (beam 56 +))(n b3 e v1 (beam 56 =))(n c4 e v1 (beam 56 -))"
                        "(barline simple)"
                        "(n d4 q v1)(n d4 e v1)(n d4 q. v1)(barline simple)"
                        ")"     //musicdata
                    ")"     //instrument
                ")"     //score
            ")"     //content
        ")",
        Document::k_format_ldp
    );

    //get the pointer to the interactor and register to receive desired events
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        spInteractor->add_event_handler(k_update_window_event, this, wrapper_update_window);
    }
}

//---------------------------------------------------------------------------------------
void MyCanvas::wrapper_update_window(void* pThis, SpEventInfo pEvent)
{
    if (pEvent->get_event_type() == k_update_window_event)
    {
        MyCanvas* pWnd = static_cast<MyCanvas*>(pThis);
        SpEventPaint pEv( static_pointer_cast<EventPaint>(pEvent) );
        pWnd->update_window(pEv->get_damaged_rectangle());
    }
}

//---------------------------------------------------------------------------------------
void MyCanvas::update_window(VRect damagedRect)
{
    Q_UNUSED(damagedRect);

    // Invoking update_window() results in just putting immediately the content
    // of the currently rendered buffer to the window without neither calling
    // any lomse methods nor generating any events (i.e. window on_paint)
    if (m_pPresenter)
    {
        update_rendering_buffer_if_needed();
        if (!m_pdata)
            return;

        repaint();
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
    m_pdata = nullptr;
}


//---------------------------------------------------------------------------------------
void MyCanvas::create_rendering_buffer(int width, int height)
{
    //allocate memory for the Lomse rendering buffer.

    #define BYTES_PER_PIXEL 4   //the chosen format is RGBA, 32 bits

    //delete current buffer
    delete_rendering_buffer();

    // allocate a new rendering buffer
    m_nBufWidth = width;
    m_nBufHeight = height;
    m_pdata = (unsigned char*)malloc(m_nBufWidth * m_nBufHeight * BYTES_PER_PIXEL);

    //use this memory as Lomse rendering buffer
    if (m_pPresenter)
    {
        if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
            spInteractor->set_rendering_buffer(m_pdata, m_nBufWidth, m_nBufHeight);
    }

    m_view_needs_redraw = true;
}

//---------------------------------------------------------------------------------------
SpDocument MyCanvas::get_document() const
{
    if (!m_pPresenter)
        return nullptr;
    return m_pPresenter->get_document_shared_ptr();
}

//---------------------------------------------------------------------------------------
SpInteractor MyCanvas::get_interactor() const
{
    if (!m_pPresenter)
        return nullptr;
    return m_pPresenter->get_interactor(0).lock();
}


