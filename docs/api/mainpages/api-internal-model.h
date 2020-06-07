/**
@page page-api-internal-model The Document API

@tableofcontents


@section api-internal-model-intro  Accesing and modifying the document
<!----------------------------------------------------------------------------------------------------->

Lomse operates in the domain of music books and music scores. That is, Lomse deals with documents, written in LDP language, containing texts, images, scores, exercises, graphics, headers, footers, etc. Also deals with music scores written in other languages, such as MusicXML.

A file written in MusicXML or LDP languages is an 'external representation' of a music score or a music document, that is, something oriented to be stored in a file. This type of representation is not the most appropriate for processing. Therefore, when a MusicXML or LDP file is read by Lomse, the file content is transformed into a suitable format in memory, more flexible and powerful for processing. This representation is named the @IM . 

The internal model is accessible for your application through the 'lomse internal model API'. It is a programming interface for manipulating the internal model representing a lomse document, so that your application can interact with the document, get information from it and change things. You can even create the document content from scratch instead of loading its content from an external file.

Until now, this API didn't exist and to manipulate the internal model you had to access lomse internal objects and understand the @IM. The need for an stable API hiding all internal details is clear, to get two main benefits:
    1. The internal model will be really ‘internal’ and could be improved for new features and fixes without affecting user applications.
    2. The library API will be stable, and user oriented, removing all internals.
    
Therefore, lomse is starting to define and offer a user oriented API for accessing the internal model. This API is in development and, currently, only a few classes and methods are available. Meanwhile you application will continue having access to the lomse internal model classes but this access will be, in future, removed.

Defining and implementing the public API will take its time and will be done little by little. It is strongly recommended to use the new API classes when available instead of accessing internal objects. But, for backwards compatibility, as your application could be using methods not yet moved to the public API, all API classes include a bridge method `get_internal_object()` that provides access to the most relevant internal model object, so that you can migrate your code and still use internal classes for methods not yet migrated. Notice that this helper method <b>will be removed in future</b> so, please, if you need to use it, open an issue explaining the need, so that the public API could be fixed as soon as possible and your app. would not be affected in future when these backwards compatibility methods are removed.



@section api-internal-model-idocument The IDocument class
<!----------------------------------------------------------------------------------------------------->

The document is accessible through the IDocument class, that represents the whole document. This object can be accessed by method Presenter::get_document(). You could think of the IDocument as the root node of a DOM like structure representing the document. And the children of this root element represent the basic blocks for building a document: headers, paragraphs, music scores, lists, tables, images, etc.

For music scores the DOM model analogy is not truly feasible as many music notation markings (like a slur or beam) are represented by pairs or sets of elements. By using just a tree structure there is no automatic way to ensure consistency between these elements. Therefore, lomse or any other program have to maintain self-consistency when there are 'paired elements' representing the starts and ends of things like beams, crescendo markings, slurs, and so on. Therefore, the @IM contains additional structures for correctly representing a music score.

It is important to note that the @IM and its associated structures form an abstract representation for a document, containing all necessary information to derive any other representation when needed. It is not oriented to any particular use (rendering, playback, printing, music analysis, document edition, ...) but try to serve all them equally. In this way the Document can serve to the purpose of any View object: a view can either be a print out of a conventional music score; or it can be a textual representation in LDP or any other language; or it can be a playback oriented representation, such a table of MIDI events; or any other representation for any purpose. When a particular use is intended, the corresponding optimized representation is derived from the @IM by Lomse.


@subsection api-internal-model-idocument-content  IDocument content
<!----------------------------------------------------------------------------------------------------->

You can think on a document as a container for the visible content (music scores, paragraphs, etc.) and some additional properties and metadata:

@code
Document := metadata + content
content := collection of <i>block-level</i> objects (paragraphs, music scores, tables, etc).
@endcode

For structuring the content of a document, lomse follows an approach similar to that of HTML (but not exactly equal), and classifies document content objects in two classes: <i>block-level</i> objects and <i>inline-level</i> objects.

In a lomse, document first level content items are always <i>block-level</i> objects. They define independent areas in a document, such as a paragraph or a music score. <i>block-level</i> objects can be considered as boxes to wrap the content, and they may content other <i>block-level</i> objects and <i>inline-level</i> objects. Generally, <i>block-level</i> objects begin on new lines, whereas <i>inline-level</i> objects do not.

<i>Inline-level</i> objects must always be included as part of a <i>block-level</i> object, and may contain only data and other <i>inline-level</i> objects.

In lomse, there are three types of <i>block-level</i> objects:
1. blocks-container (class IBlocksContainer): they only contain more <i>block-level</i> objects, e.g.: ITable, IList, or ITableRow.
2. inlines-container (class IInlinesContainer): they only contain <i>inline-level</i> objects, e.g.: IParagraph or IHeading.
3. music score (class IScore): a blocks-container specialized to contain a music score.

As you can see, document content is organized in a tree, and the first level nodes are always <i>block-level</i> objects. For example:
@code
document content:
    heading             (blocks-container)
        txt                 (inline-level object)
    para                (inlines-container)
        txt                 (inline-level object)
        image               (inline-level object)
        txt                 (inline-level object)
    contentBox          (blocks-container)
        para                (inlines-container)
            txt                 (inline-level object)
        para                (inlines-container)
            txt                 (inline-level object)
    heading             (inlines-container)
        txt                 (inline-level object)
    score               (block-level object)
    contentBox          (blocks-container)
        image               (inline-level object)
        ...                 ...
@endcode

Let's analyse the structure of a real document, for example, the following LMD document:

@code{.xml}
<?xml version="1.0" encoding="UTF-8"?>
<lenmusdoc vers="0.0" language="en">
   <content> 
      <section level="1">1. A simple music score</section> 
      <para>
        The next example is just a simple music score.
        It only has one staff, a G clef, a key signature (A major)
        and two notes: a quarter note and an eighth note.
      </para> 
      <ldpmusic>
        (score (vers 2.0)
            (instrument
                (musicData
                    (clef G)
                    (key A)
                    (n a4 q)
                    (n c5 e)
                )
            )
        )
      </ldpmusic>
      <para>Do you like it?</para> 
   </content>
</lenmusdoc>
@endcode

will be is rendered by Lomse as:

@image html document-example.png "Image: Rendering of previous LMD document."


And from the API point of view you can think of this document as a tree represented by the following objects:

@verbatim

                              IDocument
                                  |
         ┌────────────────┬───────┴────────┬──────────────────┐
         |                |                |                  |
      IHeading       IParagraph         IScore           IParagraph
         |                |                |                  |
       IText            IText             ...               IText


.        
@endverbatim

As you can see, the API tree follows, basically, the same structure than that of a LenMus document (LMD) and an score
follows the same structure than its LDP source.

For content other than music scores the structure is similar to that of an HTML document. But for music scores there is some additional information added to the model. See next section @ref api-internal-model-scores for details.




@section api-internal-model-scores  The structure of a music score
<!----------------------------------------------------------------------------------------------------->
         
IScore represents a full music score, that is, an object comprising all of the music for all of the players and their instruments, typically laid out in a specific order. In a full score, instruments are usually grouped by sharing barlines and having some visual clues, such as a brace or bracket, and a group name. The visual groupings we see in a piece of music can be classified as:

- Staff - a set of horizontal lines, where musical notation can be arranged for display.

- Grand staff - two staves vertically linked by a curly brace, and sharing measure barlines, used for instruments having enough range to be confusing when put on a single staff. (e.g. piano or organ). For some instruments, such as organ, it could have more than two staves.

- Instruments group - a set of staves or grand staves connected by a curly or square brace, used to indicate that the instruments involved are closely linked (e.g. SATB voices or 8 Horns). Sometimes, all instruments in the group share measure barlines. 

- System - a set of staves, grand staves, and/or grouped staves connected, at start, by a vertical system barline, representing every part in the current layout.

The two first, staves and grand staves, are represented in lomse by an IInstrument object: the staff or staves associated to a physical musical instrument.

The third one, instruments groups, are represented in lomse by IInstrGroup objects: the layout information for grouping some instruments.

Finally, systems, are represented by internal objects managed by lomse.

Thus, in lomse, an score is, basically, some global information such as score titles, and two collections: the collection instruments (IInstrument objects) and the collection of defined instrument groups (IInstrGroup objects).


///@cond INTERNALS

    In brackets [] the equivalent HTLM element.

    INode (ImoObj) - abstract base class                    HTML Node
      ├── ImoSimpleObj (A) () No renderizable                   HTML Attribute
      |
      └── IElement (ImoContentObj) - abstract base class        HTML Element
            |
            ├── IBlockLevelObj - abstract base class
            |     |
            |     ├── IScore - a music score
            |     |
            |     ├── IBlocksContainer - abstract base class
            |     |     |       A block-level container for block-level objects. 
            |     |     |
            |     |     ├── IDocument [body] - the container for all document content
            |     |     ├── IContent [div] - a generic block-level container
            |     |     |     └── IDynamic [object] 
            |     |     |
            |     |     ├── IMultiColumn - a container subdivided in columns
            |     |     ├── ITable [table] - a container for table related objects
            |     |     ├── IList [ol, ul]
            |     |     ├── ITableRow [tr] - a container for a row of cells
            |     |     ├── IListItem [li]
            |     |     └── ITableCell [td, th]
            |     |
            |     └── IInlinesContainer - abstract base class
            |           |        A block-level container for inline-level objs.
            |           |
            |           ├── IAnonymousBlock []
            |           ├── IParagraph [p]
            |           └── IHeading [h1, h2, h3, h4, h5]
            |
            |
            ├── IInlineLevelObj - abstract base class for inline-level objects
            |     |
            |     ├── IBoxInline - abstract base class
            |     |     |        An inline-level box container.
            |     |     |
            |     |     ├── IInlineWrapper [span] - A generic inline-box container
            |     |     └── ILink [a] - Anchor 
            |     |
            |     ├── IButton [button] - a button
            |     ├── IControl - a user defined GUI control obj.
            |     |     └── IScorePlayer - a control for managing score playback
            |     |
            |     ├── ITextItem [] - a piece of text with the same style
            |     └── IImage [img] - an image                        
            |               
            |
            └── IScoreObj - abstract base class. Content for scores


///@endcond

*/

