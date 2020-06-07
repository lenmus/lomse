/**

@page page-edit-overview Editing documents overview

@tableofcontents

@section edit-overview How to modify a document

When a source document (in LDP or MusicXML format) is loaded, it is not stored as text or as an XML Document Object Model (<i>DOM</i>). Instead, the document is parsed and an @IM is built. The @IM is, mainly, a tree structure in memory. The root element of the @IM tree represents the whole document. And the children of the root element represent the basic blocks for building a document: headers, paragraphs, music scores, lists, etc. For an overview of the @IM see @ref page-api-internal-model.

Once the @IM is created, for modifying the document your application has two options, either
-# use the edition commands API; or
-# directly modify the internal model.

<b>Edition commands</b> are a high level API for supporting edition. It is based on executing commands and provides undo/redo (that is, reverting the last change done to the document and to reverting the undo by re-executing the undone command). It is appropriate for applications whising to provide an interface to the user for editing documents (e.g. a music score notation editor application).

<b>Direct internal model manipulation</b> is a low level API. Your application directly accesses the @IM and modifies the internal @IM structures. Obviously, there are no high level related services such as undo/redo and good knowledge of the internal model and related structures is needed for maintainin consistency. This API, faster and more flexible, is appropriate for applications that programatically create and modify documents (e.g. an application for creating random music scores).

Of course, both APIs are compatible and can be used by your application for offering different services.

The rest of this page describes the high level API using edition commands. For a description of the low level API see @ref page-api-internal-model.


@section edit-high-level The high-level API: edition commands

All edition commands are objects derived from base abstract class DocCommand, and they work by modifying the @IM tree, either by inserting or removing elements or by modifying the attributes of an element. For instance, CmdInsertBlockLevelObj() is a command for inserting a block level object. This command, as well as most commands for inserting new elements, require a parameter with the source code of the element to insert. Currently the source code for all edition commands must be in LMD format.

Executing a command is just invoking Interactor::exec_command() method and passing the command to execute, for instance:

@code
    //insert a paragraph and an empty music score
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        string src = "<para>This is a short paragraph</para>";
        SpInteractor->exec_command( new CmdInsertBlockLevelObj(src, "Add paragraph") );

        src = "<ldpmusic>(score (vers 2.0)(instrument (musicData)))</ldpmusic>";
        SpInteractor->exec_command( new CmdInsertBlockLevelObj(src, "Add empty music score") );
    }
}
@endcode


@subsection edit-overview-undo Undo/Redo


Undo/redo operations refer, respectively, to reverting the last change done to the document and to reverting the undo by re-executing the undone command.

Lomse provides full support for implementing undo/redo in your application. It maintains two queues: the history of executed commands and the list of commands that can be re-done. And Lomse provides two methods for undo/redo operations:

- For undoing the last command in the queue just invoke Interactor::exec_undo() method.
- For redoing the operation in the queue just invoke Interactor::exec_redo() method.

In addition, to facilitate that your application can enable and disable undo/redo buttons, commands and other GUI widgets related to undo/redo, Lomse provides information about the undo and redo history:
    - method Interactor::should_enable_edit_undo() returns @true if there are commands in the undo queue, and
    - method Interactor::should_enable_edit_redo() returns @true if there are commands in the redo queue.


@subsection edit-overview-cursor Cursor and selections

Most edition commands require a reference point in the document. For instance, a command such as <i>delete paragraph</i> would require a pointer to the paragraph to delete. Or a command <i>insert note</i> would require a pointer to the point in which the note must be inserted. For providing these references, Lomse maintains two objects: a DocCursor object and a SelectionSet object. 

The DocCursor object is just a pointer to an element in the document. You can think of it as the graphic cursor in text edition applications, but in Lomse, there are two objects: the DocCursor and the Caret.

The cursor is just an internal invisible pointer; and the caret is the graphical representation of the cursor position. Lomse maintains the synchronization between the cursor and the caret, so advancing the cursor automatically forces Lomse to render the caret on the new cursor position. The cursor is always necessary, as it provides a reference point for edition commands. But the caret is just a graphical widget that your application can use or not, depending of the needs. For instance, in a batch application it would be non-sense to use a Caret! For moving the cursor to another position your application just issue specific edition commands, such as <i>advance cursor</i> or <i>move cursor to start of score</i>.

Your application can also issue edition commands for selecting and deselecting document elements, such as paragraphs, words, notes, rests, clefs, staves, etc. Object SelectionSet is just the collection of current @e selected elements.

The DocCursor and the SelectionSet are not parameters of the commands. These objects are maintained by Lomse and when a command is going to be executed the necessary information about target objects and context is extracted from current cursor position and/or from current set of selected objects.

And this is, basically, the document edition API. It is very simple and gives full freedom to your application for implementing the edition GUI as you'd like, or for not a having it at all!


@section edit-modes Supported edition modes

What should be the behavior when inserting or deleting a note? For people with no previous experience with music edition software, the most intuitive behavior is one similar to what is expected from someone who comes from a pencil-and-paper background or from using a text processor: inserting/deleting a note or a rest shifts all the music that comes after it. But what about barlines? Should they be also shifted or should they remain at the same place? Each alternative has advantages and drawbacks:

-# If barlines are also shifted, current measure could result in an irregular measure. Let's name this behavior <i>limited ripple mode</i> (notes are shifted but limited to current measure because barline is also shifted).

-# If barlines remains at the same place and only notes and rests are shifted and can cross barlines boundaries and get placed in another measure, this behavior can be useful in some scenarios. For instance when you need to make one (ore more) beats completely disappear (and thus shift the remainder of the composition to the left by one beat). Or need two insert some *blank space* (one or more beats o whole measures), shifting to the right the rest of the music, and then you can fill in that space with what you wanted to insert. But, in general, this behavior causes a lot of trouble in music score edition. Notice that removing a note/rest in one voice and shifting all other notes in that voice implies completely @e altering the music in all the score, because music in shifted voice and music in the other voices and instruments are no longer synchronized. Let's name this behavior <i>full ripple mode</i> (notes are shifted across all the score). 

To avoid problems, another possibility is to change the behavior when a note or rest is inserted or deleted. In most cases you don't need to shift music. What you need is to add notes and rests in empty measures, but in measures having content what you need is to change a few notes or rests, but always keeping measures duration and not shifting the music. Let's call this behavior <i>replace mode</i> (the notes are replaced and measure duration is preserved).

None of these behaviors is perfect. What should be Lomse behavior? Although the most usual behavior in music edition programs is the <i>replace mode</i>, the other modes can be useful in some scenarios or for some applications. As Lomse aims at not creating unnecessary constrains to your application, it was decided to support the three behaviors. For this, all edition commands related to adding or removing notes and rests require an <i>edition mode</i> parameter for selecting the desired behavior:

- <b>Limited ripple mode</b>: when inserting or deleting notes/rests the barlines are also shifted, and this could result in irregular measures, but other measures are not affected.

- <b>Replace mode</b>: Notes and rests are not inserted. Instead they @a overwrite the music so that measure duration is maintained. Also, notes are not deleted but replaced by rests.

For supporting full ripple mode, instead of adding more complexity to commands related to adding or removing notes and rests, it is simpler, more flexible and causes less trouble to end users to use two specific insert/delete commands that always work in <i>full ripple mode</i>:

- insert blank space (beats or whole measures); and 
- delete beats/measures.

These commands accepts a parameter defining the scope: to affect only one or selected voices, one or selected staves, one or selected instruments or the whole score.

@attention Currently Lomse does not ensure the correctness of the music notation in operations resulting in music shifts. For instance, if a quarter note crosses a barline by the duration of a removed eighth note, the quarter note should be splitted into two tied eighth notes. Or when two tied notes in different measures are shifted an are now placed in the same measure, it is no longer clear if they should continue tied or should be replaced by a longer single note. As these decisions are not trivial, Lomse does not attempt (for now) to fix these situations and therefore the commands limit the operation to inserting or deleting the requested notes or rests without more considerations.



@section edit-commands Edition commands

This section presents a list of available commands (but more commands need to be programmed and a couple could be removed). For more information on each command read the class documentation for the command.

<b>Document edition commands:</b>

- CmdAddChordNote() - Add a new note to an existing note or chord.
- CmdAddNoteRest() - Add a new note or rest at current cursor position.
- CmdAddTie() - Add a tie to the two selected notes.
- CmdAddTuplet() - Add a tuplet to the selected notes/rests.
- CmdBreakBeam() - Break a beam at the note/rest pointed by the cursor.
- CmdChangeAccidentals() - Change the accidentals of the selected notes.
- CmdChangeAttribute() - Change the value of an attribute.
- CmdChangeDots() - Change the number of dots in the selected notes and rests.
- CmdDeleteBlockLevelObj() - Delete a block level object (e.g. paragraph, music score, header, etc.).
- CmdDeleteRelation() - Delete relation objects (e.g. beam, chord, slur, tie, tuplet, etc.).
- CmdDeleteSelection() - Delete all selected objects.
- CmdDeleteStaffObj() - Delete the staff object (e.g. note, rest, clef, barline, etc.) pointed
                        by the cursor.
- CmdInsertBlockLevelObj() - Insert a new block level object (e.g. paragraph, music score, header, etc.).
- CmdInsertManyStaffObjs() - Insert a group of consecutive staff objects (e.g. note, rest, clef, barline, etc.).
- CmdInsertStaffObj() - Insert one staff object (e.g. a note, rest, clef, barline, etc.)  at current cursor position.
- CmdJoinBeam() - Create and join beams, and add notes to a beam, depending on current selection.
- CmdTransposeChromatically() - Shift chromatically every pitch, up or down, by the interval you specify.
- CmdTransposeDiatonically() - Shift diatonically every pitch, up or down, by the number of steps you specify.
- CmdTransposeKey() - Changes all the keys in the selection, and transposes chromatically
        			  all the notes in the selection.

<b>Selection commands:</b>

- CmdSelection() - Changes the content of the set of selected objects.
- CmdClearSelection() - An alias for the specific CmdSelection() command for clearing the set of selected objects.


<b>Cursor commands:</b>

- CmdCursor() - Move the cursor to a new position.


*/

