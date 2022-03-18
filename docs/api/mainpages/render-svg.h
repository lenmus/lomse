/**
@page page-render-svg Render in SVG format

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
    //create the instance of the library doorway
    lomse::LomseDoorway lomse;

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


The 'id' is always letter 'm' followed by the id of the musical notation, e.g. @a 'm412'. Optionally, when a musical notation can generate more than one image (e.g. the clef repeated in every staff or the instrument name repeated in every system) the 'id' will be followed by a suffix to make it unique. See below for details.

Notations that always generate a single SVG element will never be enclosed by a @a \<g\> element, e.g.:

@code
<text id='m78' class='clef' ... >...</text>
@endcode

But notations that can generate two or more SVG elements will always be enclosed in a @a \<g\> element, e.g. a D major key signature:

@code
<g id='m83' class='note'>
    <path class='ledger-line' ... />
    <text class='notehead' ... >...</text>
    <path class='stem' ... />
</g>

<g id='m36' class='key-signature'>
    <text class='accidental-sign' ... >...</text>
    <text class='accidental-sign' ... >...</text>
</g>
@endcode

Note that for notations that require a @a \<g\> element, when the notation does not have visible content (e.g. a C major key signature), the generated code would be just an empty @a \<g\> element:

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



@section rendering-svg-many-shapes  Id for notations that generate many images

For musical notations that generate more than one image (e.g. the clef repeated in every staff or the instrument name repeated in every system) the 'id' will be followed by a suffix to make it unique. The structure of this suffix depend on each notation:

<b>a) Clefs and key signatures</b>

Clefs and key signatures will normally generate many images, as they are repeated at start of every system. The id of these repeated images is the id of the music notation followed by a unique number (not necessarily in sequence), e.g.:

@code
<text id='m40' class='clef' ...>...</text>      The first image
<text id='m40-3' class='clef' ...>...</text>    Image in next system
<text id='m40-12' class='clef' ...>...</text>   etc.
@endcode

<b>b) Images generated by an instrument (score part)</b>

A score part (Lomse instrument) is responsible for drawing several elements, such as the staff lines, part name, part abbreviation, braces, etc. The id for these elements will be always the id of the instrument (e.g. @a 'm59') followed by a suffix composed as follows:

- @b brace: string @a "brace-n", where @a n is the system number (1...n).
- @b instrument @b name: string @a "name-n", where @a n is the system number (1...n).
- @b instrument @b abbreviation: string @a "abbrev-n", where @a n is the system number (1...n).
- @b staff @b lines: string @a "staff-m-n", where @a m is the staff number (1...n) and n is the system number (1...n).

For example:
@code
<path id='m59-brace-1' class='instr-brace' ... />       Brace in 1st system
<path id='m59-staff-1-1' class='staff-lines' ... />     Lines, staff 1, system 1
<path id='m59-staff-2-1' class='staff-lines' ... />     Lines, staff 2, system 1
<text id='m59-name-1' class='instr-name' ... >          Instr. name in 1st system

<path id='m59-brace-2' class='instr-brace' ... />       Brace in 2nd system
<path id='m59-staff-1-2' class='staff-lines' ... />     Lines, staff 1, system 2
<path id='m59-staff-2-2' class='staff-lines' ... />     Lines, staff 2, system 2
<text id='m59-abbrev-2' class='instr-abbrev' ... >      Instr. abbreviation, sys.2
@endcode


<b>c) Images generated by a group of instruments</b>

Instruments can be grouped using braces and brackets and the groups can have names and abbreviations. The id for these elements will be always the id of the group (e.g. @a 'm134') followed by a suffix composed as follows:

- @b brace: string @a "brace-n", where @a n is the system number (1...n).
- @b bracket: string @a "bracket-n", where @a n is the system number (1...n).
- @b squared @b bracket: string @a "squared-bracket-n", where @a n is the system number (1...n).
- @b group @b name: string @a "name-n", where @a n is the system number (1...n).
- @b group @b abbreviation: string @a "abbrev-n", where @a n is the system number (1...n).

For example:
@code
<path id='m134-bracket-1' class='group-bracket' ... />   Bracket in 1st system
<text id='m134-name-1' class='group-name' ... >          Group name in 1st system

<path id='m134-bracket-2' class='group-bracket' ... />   Bracket in 2nd system
<text id='m134-abbrev-2' class='group-abbrev' ... >      Group abbreviation, sys.2
@endcode


<b>d) Systemic barlines</b>

Systemic barlines are the barlines at start of each system, joining all staves in the system. The id for systemic barlines is always the id of the score (e.g. @a 'm2') followed by string @a "barline" and the system count (1 for the first system):  e.g.:

@code
<path id='m2-barline-1' class='systemic-barline' ... />    Barline for 1st system
<path id='m2-barline-2' class='systemic-barline' ... />    Barline for 2nd system
@endcode


<b>e) Measure numbers</b>

Measure numbers are normally associated to the first instrument (score part) in the score, but for polymetric music the numers are different in each instrument and will go associated to the respective instrument. Therefore, measure numbers are always associated to an instrument and its id is the id of the instrument followed by string @a "measure-num" and the measure index:  e.g.:

@code
<text id='m35-measure-num-1' class='measure-number' ... >...</text>
<text id='m35-measure-num-5' class='measure-number' ... >...</text>
@endcode


*/

