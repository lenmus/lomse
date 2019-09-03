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
#ifndef DRAWLINES_SAMPLE_H
#define DRAWLINES_SAMPLE_H

#include <QtWidgets>

#include <iostream>

//lomse headers
#include <lomse_doorway.h>
#include <lomse_document.h>
#include <lomse_graphic_view.h>
#include <lomse_interactor.h>
#include <lomse_presenter.h>
#include <lomse_events.h>

using namespace lomse;


//forward declarations
class MyCanvas;

//=======================================================================================
// MainWindow is the main frame for the application
// In this sample it contains the main menu and the canvas on which the
// score will be displayed
// It also contains the methods for providing the content for the Lomse document to
// be displayed
//=======================================================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void on_about();
    void on_add_lines();
    void on_remove_lines();

public:
    ~MainWindow();

    //commands
    void open_test_document();

protected:
    void create_actions();
    void create_menu();

    //lomse related
    void initialize_lomse();

protected:

    //Qt stuff, for the GUI
    MyCanvas* m_canvas;
    QScrollArea* scrollArea;

    QMenu* m_fileMenu;
    QAction* m_exitAction;

    QMenu* m_helpMenu;
    QAction* m_aboutAction;

    QMenu* m_linesMenu;
    QAction* m_addLinesAction;
    QAction* m_removeLinesAction;

    //references to the objects containing the lines and to the lines
    ImoId m_startObjId;
    ImoId m_startLineId;
    ImoId m_endObjId;
    ImoId m_endLineId;

    //Lomse stuff
    LomseDoorway    m_lomse;        //the Lomse library doorway
};

//=======================================================================================
// MyCanvas is a window to display Lomse documents.
// You can use it as any other QWidget object.
//=======================================================================================
class MyCanvas : public QWidget
{
public:
    MyCanvas(QWidget* parent, LomseDoorway& lomse);
    ~MyCanvas();

    void update_view_content();

    //commands
    void open_test_document();
    void display_document(const string& filename, int viewType);

    //accessors
    inline LibraryScope& get_library_scope() { return *m_lomse.get_library_scope(); }
    SpDocument get_document() const;
    SpInteractor get_interactor() const;

    //callback wrappers
    static void wrapper_update_window(void* pThis, SpEventInfo pEvent);


protected:
    //event handlers
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent* event);

    void delete_rendering_buffer();
    void create_rendering_buffer(int width, int height);
    void update_rendering_buffer_if_needed();
    void update_window(VRect damagedRect = VRect(0,0,0,0));


        //Lomse stuff

    // In this first tutorial we are just going to display an score on a window.
    // Let's define the necessary variables:
    LomseDoorway&   m_lomse;        //the Lomse library doorway
    Presenter*      m_pPresenter;

    //the Lomse View renders its content on a bitmap. To manage it, Lomse
    //associates the bitmap to a RenderingBuffer object.
    //It is your responsibility to render the bitmap on a window.
    //Here you define the rendering buffer and its associated memory
    //for this View
    RenderingBuffer     m_rbuf_window;
    unsigned char*      m_pdata;                    //ptr to the bitmap
    int                 m_nBufWidth, m_nBufHeight;	//size of the bitmap

    //some additinal variables
    bool    m_view_needs_redraw;      //to control when the View must be re-drawn
};


#endif // DRAWLINES_SAMPLE_H

