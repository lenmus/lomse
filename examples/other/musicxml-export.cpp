// musicxml-export.cpp
//
// Sample code to export MusicXML from Lomse
// Feel free to use this example code in any way you see fit (Public Domain)
//
// Usage:
// - build:
//      g++ -std=c++11 musicxml-export.cpp -o musicxml-export \
//        `pkg-config --cflags liblomse` `pkg-config --libs liblomse` -lstdc++
// - run:
//      ./musicxml-export
//
#include <iostream>
using namespace std;

#include <lomse_doorway.h>
#include <lomse_graphic_view.h>     //for view types
#include <lomse_interactor.h>       //Interactor
#include <lomse_presenter.h>        //Presenter
#include <lomse_mxl_exporter.h>     //MusicXML exporter
using namespace lomse;

int main()
{
    //create the instance of the library doorway
    lomse::LomseDoorway lomse;

    //create a simple LDP score
    std::string src = "(score (vers 2.0) (instrument (musicData "
                      "(clef G)(time 2 4)(n d4 q)(r q)(barline) )))";
    lomse::Presenter* pPresenter =
        lomse.new_document(k_view_vertical_book, src, Document::k_format_ldp);

    //export it to MusicXML
    if (SpInteractor spInteractor = pPresenter->get_interactor(0).lock())
    {
        ADocument doc = pPresenter->get_document();
        MxlExporter exporter(*lomse.get_library_scope());
        std::string source = exporter.get_source( doc.first_score() );

        //do whatever you like with the MusicXML source code. For instance,
        //display it in the console
        cout << source << endl;
    }
    else
        cout << "Could not create document!" << endl;
    
    delete pPresenter;
    return 0;
}
