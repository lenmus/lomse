//---------------------------------------------------------------------------------------
// http://www.math.msu.su/~vvb/2course/Borisenko/CppProjects/GWindow/xintro.html
// http://www.linuxjournal.com/article/4879?page=0,0

// Remember to compile try:
//      1) g++ example_1_main.cpp -o example_1 -lX11    <<========= this one ok
//      2) g++ example_1_main.cpp -I/usr/include/X11 -L/usr/lib/X11 -L/usr/lib -lX11

//
//  Brian Hammond 2/9/96.    Feel free to do with this as you will!
//
//---------------------------------------------------------------------------------------


// header files required for X11. The order is important:
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

//some additional needed stuff
#include <stdio.h>
#include <stdlib.h>


//---------------------------------------------------------------------------------------
// X has a number of important variables for handling windows:

Display *dis;   //The display points to the X Server.

int screen;     //The screen refers to which screen of the display to use.
                //Setting up the connection from the X Client to the X Server
                //typically involves a line like:
                //      setenv DISPLAY my.machine.where.ever:0.
                //The my.machine.where.ever is tied in with the Display* and
                //the screen is connected with the :0 part of the variable.

Window win;     //the actual window itself

GC gc;          //And the GC is the graphics context.

//Typically calls to get input and output to the window use one or more
//of the above variables


//---------------------------------------------------------------------------------------
// forward declaration of our routines
void init_x();
void close_x();
void redraw();


//---------------------------------------------------------------------------------------
//entry point and main loop
main ()
{
    XEvent event;       // the XEvent declaration !!!
    KeySym key;         // a dealie-bob to handle KeyPress Events
    char text[255];     // a char buffer for KeyPress Events

    init_x();

    // look for events forever...
    while(1)
    {
        // get the next event and stuff it into our event variable.
        // Note:  only events we set the mask for are detected!

        XNextEvent(dis, &event);

        if (event.type == Expose && event.xexpose.count == 0)
        {
            // the window was exposed redraw it!
            redraw();
        }

        else if (event.type == KeyPress
				  && XLookupString(&event.xkey, text, 255, &key,0) == 1)
        {
            // use the XLookupString routine to convert the invent
            //KeyPress data into regular text.  Weird but necessary...

            if (text[0] == 'q')
            {
                close_x();
            }
            printf("You pressed the %c key!\n",text[0]);
        }

        else if (event.type == ButtonPress)
        {
            // tell where the mouse Button was Pressed
            int x=event.xbutton.x,
                y=event.xbutton.y;

            strcpy(text,"X is FUN!");
    		unsigned long black = BlackPixel(dis, screen);
            XSetForeground(dis, gc, black);
            XDrawString(dis, win, gc, x, y, text, strlen(text));
        }
    }
}

//---------------------------------------------------------------------------------------
void init_x()
{
    // use the information from the environment variable DISPLAY
    // to create the X connection:
    dis = XOpenDisplay((char *)0);
    screen = DefaultScreen(dis);

    unsigned long black = BlackPixel(dis, screen);  // get color black
    unsigned long white = WhitePixel(dis, screen);  // get color white

    // once the display is initialized, create the window.
    // This window will be have be 600 pixels across and 400 down.
    // It will have the foreground black and background white
    win=XCreateSimpleWindow(dis, DefaultRootWindow(dis), 0, 0, 600, 400, 0,
                            black, white);

    // here is where some properties of the window can be set.
    // The third and fourth items indicate the name which appears
    // at the top of the window and the name of the minimized window
    // respectively.
    XSetStandardProperties(dis, win, "Lomse examples. Example_1", "Lomse_1", None, NULL, 0, NULL);

    // this routine determines which types of input are allowed in
    // the input. See the appropriate section for details...
    XSelectInput(dis, win, ExposureMask | ButtonPressMask | KeyPressMask);

    // create the Graphics Context
    gc = XCreateGC(dis, win, 0, 0);

    // here is another routine to set the foreground and background
    // colors _currently_ in use in the window.
    XSetBackground(dis, gc, white);
    XSetForeground(dis, gc, black);

    // clear the window and bring it on top of the other windows
    XClearWindow(dis, win);
    XMapRaised(dis, win);
};

//---------------------------------------------------------------------------------------
//Another important task is closing the window correctly: it is good programming
//practice to return system resources to the system...
void close_x()
{
    XFreeGC(dis, gc);
    XDestroyWindow(dis,win);
    XCloseDisplay(dis);
    exit(1);
};


void redraw()
{
    XClearWindow(dis, win);
};

