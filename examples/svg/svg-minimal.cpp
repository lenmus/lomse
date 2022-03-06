// svg-minimal.cpp
//
// Sample code to generate SVG with Lomse
// Feel free to use this example code in any way you see fit (Public Domain)
//
// Usage:
// - modify the paths for the score to display and the file to generate
//   (lines 36 & 51) and save.
// - build:
//      g++ -std=c++11 svg-minimal.cpp -o svg-minimal \
//        `pkg-config --cflags liblomse` `pkg-config --libs liblomse` -lstdc++
// - run:
//      ./svg-minimal
// - and open the generated HTML page in a browser.

#include <iostream>
using namespace std;

#include <lomse_doorway.h>
#include <lomse_graphic_view.h>     //for k_view_vertical_book
#include <lomse_presenter.h>        //Presenter
#include <lomse_interactor.h>       //Interactor
using namespace lomse;

int main()
{
    //initialize lomse library. As we are only going to generate SVG, any 
    //values for the pixel format and resolution parameters will be acceptable
    //as they are not going to be used
    lomse::LomseDoorway lomse;
    lomse.init_library(k_pix_format_rgba32, 96);

    //open a score
    lomse::Presenter* pPresenter = 
        lomse.open_document(k_view_vertical_book,
                            "<path>/<to>/<the>/MusicXMLscore.mxl");

    if (SpInteractor spInteractor = pPresenter->get_interactor(0).lock())
    {
        //generate the SVG rendition for the first page
        std::stringstream svg;
        int page = 0;
        spInteractor->svg_add_newlines(true);     //for human legibility
        spInteractor->render_as_svg(svg, page);

        //do whatever you like with the svg code. For instance, display it in the
        //console or export it to an html document

        //cout << svg.str() << endl;

        std::ofstream file1("svg-test.html", ios::out);
        if (file1.good())
        {
            char htmlStart[] = "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                "<title>Lomse SVG</title></head><body><h1>Test of Lomse SVG</h1>"
                "<div style='width: 800px;border: 1px solid #000;'>";
            file1.write(htmlStart, strlen(htmlStart));

            string str = svg.str();
            file1.write(str.c_str(), str.size());

            char htmlEnd[] = "</div></body></html>";
            file1.write(htmlEnd, strlen(htmlEnd));

            file1.close();
        }
        else
        {
            cout << "Error creating file!" << endl;
        }
    }
    else
    {
        cout << "Could not open document!" << endl;
    }
    
    delete pPresenter;
    return 0;
}
