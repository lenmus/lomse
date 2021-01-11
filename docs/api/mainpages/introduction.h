/**

@page page-introduction Introduction

@ingroup pages

@tableofcontents

@section lomse-philosophy Lomse philosophy

In summary:
- It is platform independent code, with no knowledge about your platform way of doing things.
- Its aim is to provide the services and to delegate in your application for building the presentation / interaction layer.

Lomse has no knowledge about your platform native components, windows, or ways of doing things, such as how to produce sounds. Therefore, Lomse does not have methods for doing things such as creating windows or printing documents.

Lomse aims at not constraining your application. Lomse provides all the necessary services (i.e. for reading documents, rendering and editing them, and for playing back scores). But the interface with the user (i.e. the window on which the document is going to be rendered, or the sound engine for producing the sounds) must be provided by your application. Your application is responsible for the presentation and interaction layer. You can use Lomse in any context: interactive programs with a GUI, batch scripts for processing music scores, etc.

For understanding Lomse API you should know about a very few important classes and components described in the following sections. 



@section overview-doorway Initializing the Lomse library

LomseDoorway is the access point to the Lomse library and the main interface with the library. It must be used by your application at two points:

  -# Before using the Lomse library it is necessary to initialize it. This means setting up certain global options about rendering and events handling.

  -# Later, your application has to use it for:
    - Opening and creating Document objects.
    - Accessing global objects and variables in Lomse library.

Details about Lomse initialization can be found in  @ref page-render-overview-init-lomse.



@section overview-mvc The Document and related classes

Class Document represents a document with any kind of content: music scores, texts, paragraphs, images, tables, lists, etc. Documents are created by invoking LomseDoorway::new_document() and LomseDoorway::open_document() methods.

When a Document is created, additional objects for marshaling the actions between your application and the Document are created. These additional objects are part of the Lomse Model-View-Controller (MVC) architecture. The main objects associated to a Document are:

- The Interactor: It is the interface for interacting with the Document. You can consider it as the controller in the MVC model.

- The View: It is an observer of the Document, and its main responsibility is to render it. The rendering type depends on the specific view class used. For instance, class <i>GraphicView</i> renders the document on a bitmap. Other classes would be possible (i.e. a view class to render the document as an SVG stream, a view for rendering as Braille code, a view for rendering as source code, etc.) but currently Lomse only has implemented several variations of GraphicView (VerticalBookView, HorizontalBookView, and SingleSystemView). The Lomse MVC model supports having many simultaneous views for a document.  The Interactor is the owner of the View, and there is an Interactor per View.

- The Presenter - It is the owner of the Document and of all its Interactor objects. The Presenter's main responsibility is to build (and delete) the MVC model for a Document.

All these issues are described in @ref mvc-overview.



@section overview-rendering Displaying documents


Lomse is platform independent code and knows nothing about how to create a window or how to display a document on the screen. Lomse works by rendering the documents on a bitmap buffer, that is, on an array of consecutive memory bytes. This buffer can be any type of memory, such as a real bitmap, a window's buffer, etc. The simplest and usual way of rendering documents on a window is:

-# Create a new empty bitmap when necessary (i.e when the window is created or resized),
-# Ask Lomse to render the desired portion of the document on this bitmap, and
-# Copy the bitmap onto the window.

These operations are usually triggered by your application when handling some operating system events, such as <b>window paint</b> events.

The details and methods you have to use for displaying documents are described in @ref page-render-overview.



@section overview-print Printing documents

As Lomse is platform independent code, it knows nothing about how to print in the operating system used by your application. Therefore, it is your application responsibility to implement printing. Lomse just offers some supporting methods so that implementing printing does not require much work. In fact, implementing printing in your application is just printing bitmaps.

The details and methods you have to use for printing are described in @ref page-printing.



@section overview-edition Editing documents

Editing documents in Lomse is very simple: just invoke Interactor::exec_command() method and pass the command to execute, e.g. <i>insert note</i> or <i>delete paragraph</i>.

Lomse supports undo/redo operations and takes care of all the houskeeping for this. In your application just invoke:
- Interactor::exec_undo() method for undoing the last command, and
- Interactor::exec_redo() method for redoing the last undone command.

Most edition commands require a reference point in the document. For this, Lomse maintains two objects: a Cursor and a set of selected objects (the SelectionSet object).

For moving the cursor to another position and for selecting or deselecting objects your application just issue specific edition commands, such as <i>advance cursor</i>, <i>move cursor to start of score</i> or <i>select object</i>.

And this is, basically, the document edition API. It is very simple and gives full freedom to your application for implementing the GUI as you'd like, or for not a having a GUI! For more details see  @ref page-edit-overview.



@section overview-sound-engine The sound engine

Lomse is platform independent code and cannot generate sounds for your specific platform. Therefore, lomse implements playback by generating real-time events and sending them to your application. It is responsibility of your application to handle these events and do whatever is needed, i.e. transform sound events into real sounds or doing whatever you would like with the sound events.

The details and classes you have to use for scores playback are described in @ref page-sound-generation.

*/

