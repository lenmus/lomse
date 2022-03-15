/**
@page page-render-svg Render scores in SVG format

@tableofcontents


@section svg-rendering How to render SVG

The SVG rendering facilities are very simple. It is basically opening a document and requesting its rendition. The View will render the document on a std::ostream that your application must provide, and once Lomse has generated the SVG code, it is your application responsibility to do whatever is needed with that code: injecting it in an HTML page and displaying it on a browser window embedded into your application, exporting it as an image file, etc. For instance:

@code
#include <iostream>
using namespace std;

#include <lomse_doorway.h>
#include <lomse_graphic_view.h>     //for view types
#include <lomse_interactor.h>       //Interactor
using namespace lomse;

int main()
{
    //initialize lomse library. As we are only going to generate SVG, any 
    //values for the pixel format and resolution parameters will be acceptable
    //as they are not going to be used
    lomse::LomseDoorway lomse;
    lomse.init_library(k_pix_format_rgba32, 96);

    //open an score
    lomse::Presenter* pPresenter = 
        lomse.open_document(k_view_vertical_book,
                            "<path>/<to>/<the>/MusicXMLscore.xml");

    if (SpInteractor spInteractor = pPresenter->get_interactor(0).lock())
    {
        //generate the SVG rendition for the first page
        std::stringstream svg;
        int page = 0;
        pIntor->svg_add_newlines(true);     //for human legibility
        spInteractor->render_as_svg(svg, page);

        //do whatever you like with the svg code
        cout << svg.str() << endl;
    }
    else
        cout << "Could not open document!" << endl;

    
    delete pPresenter;
    return 0;
}
@endcode


Lomse behaviour for generating SVG is as follows:

- The lomse infinite logical space is also the infinite SVG document canvas.

- Lomse generates SVG code to render the View type specified when opening/creating de document. The generated SVG code is only the @a \<svg\> element and all its content but it does not contain “width” and “height” attributes. Therefore, the behaviour will depend on how your application uses the generated SVG code. For instance, if the SVG code is inserted in an HTML page, the @a \<svg\> element will inherit the “width” and “height” attributes from the context. 

- For View types oriented to generate pages (<i>k_view_vertical_book</i> and <i>k_view_horizontal_book</i>) the SVG viewBox is adjusted to include only a whole page and the page to generate is selected when invoking the render_as_svg() method.

- For the other View types the SVG viewBox is adjusted to cover the full document.

- For View types oriented to pages, the width of the document determines the width of the viewBox. But for the <i>k_view_free_flow</i> View type, there is no reference for Lomse about the target width and thus, your application must first inform Lomse of the desired width before invoking the SVG rendering method:

@code
interactor->set_svg_canvas_width(Pixels x);
@endcode

  This method only works for <i>k_view_free_flow</i>. For all other view types any value set using this method will be overriden by the document page width.



@section rendering-svg-formatting  SVG formatting options

Default behaviour is to generate the most compact SVG code and, therefore, it does not include new lines, indentation, or other formatting elements. Nevertheless, to facilitate reading by humans or for other purposes, you can use methods Interactor::svg_indent() and Interactor::svg_add_newlines(). For instance:

@code
    std::stringstream svg;
    int page = 0;
    pIntor->svg_indent(4);              //4 spaces per indentation level
    pIntor->svg_add_newlines(true);     //add a line break after each element
    spInteractor->render_as_svg(svg, page);
@endcode


@section rendering-svg-attributes  SVG 'id' and 'class' attributes

The generation of 'id' and 'class' attributes in SVG elements is optional. Default behaviour is to generate the most compact SVG code and, therefore, 'id' and 'class' attributes are not generated. But if your applications would like to identify or manipulate the generated SVG elements, it is optional to generate either 'id', 'class' or both. For this you can use methods Interactor::svg_add_id() and Interactor::svg_add_class(). For instance:

@code
    std::stringstream svg;
    int page = 0;
    pIntor->svg_add_id(true);       //add attribute 'id'
    pIntor->svg_add_class(true);    //add attribute 'class'
    spInteractor->render_as_svg(svg, page);
@endcode


'id' will go only in 'main' shapes, that is, in shapes representing a notation element in source code. All other shapes will only have 'class', e.g.:

@code
<g id='m83' class='note'>
    <path class='ledger-line' ... />
    <text class='notehead' ... >...</text>
    <path class='stem' ... />
</g>
@endcode

Notations that always generate a single SVG command will never be enclosed by a @a \<g\> element, e.g.:

@code
<text id='m78' class='clef' ... >...</text>
@endcode

But notations that can generate two or more SVG commands will always be enclosed in a @a \<g\> element, e.g. a D major key signature:

@code
<g id='m36' class='key-signature'>
    <text class='accidental-sign' ... >...</text>
    <text class='accidental-sign' ... >...</text>
</g>
@endcode

Note that for these notations, when they do not have visible content, e.g. a C major key signature, the generated code would be just an empty @a \<g\> element:

@code
<g id='m36' class='key-signature'>
</g>
@endcode

Complex notations could have two or more levels of @a \<g\> elements, e.g.:

@code
<g id='m39' class='note'>
    <path class='ledger-line' ... />
    <text class='notehead' ... >...</text>
    <g id='m40' class='arpeggio'>
        <text ... >...</text>
        <text ... >...</text>
    </g>
    <path class='stem' ... />
</g>
@endcode


As a music notation (e.g. a clef) can generate several images (the clef repeated in every staff), the id of these repeated images is the id of the main image followed by the image index, e.g.:  'm83-2', 'm83-3', etc.

Also, in the generated SVG there are elements associated to the same notational object. For instance a score part (Lomse instrument) is responsible for drawing the staff lines, part name, part abbreviation, brackets, etc. All these elements will have the same id than the parent notation but followed by a unique identifier, e.g.:

@code
<path id='m59-brace-1' class='instrument-brace' ... />
<path id='m59-staff-1' class='staff-lines' ... />
<path id='m59-name-1' class='instrument-name' ... />
@endcode
    





*/

