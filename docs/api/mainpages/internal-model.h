/**
@page page-internal-model Document internal representation

@tableofcontents


@section internal-model-intro  The internal model overview
<!----------------------------------------------------------------------------------------------------->

Lomse operates in the domain of music books and music scores. That is, Lomse deals with documents, written in LDP language, containing texts, images, scores, exercises, graphics, headers, footers, etc. Also deals with music scores written in other languages, such as MusicXML.

A file writen in MusicXML or LDP languages is an 'external representation' of a music score or a music document, that is, something oriented to be stored in a file. This type of representation is not the most appropriate for processing. Therefore, when a MusicXML or LDP file is read by Lomse, the file content is transformed into a suitable format in memory, more flexible and powerful for processing. This representation is named the @IM and is wrapped in a container class: the Document class. Therefore, tThe Document class is mainly a facade object that contains the internal model for the MusicXML or LDP document. 

How is the internal model structured? For Lomse the decision was not to represent internally the document as an XML DOM. Among other reasons, for music scores there are two important issues to consider:

-# Texts, HTML pages and XML documents work with a single hierarchy of elements and can be modelled by a tree structure. But music scores are not just a hierarchy of musical symbols, because some notations create relationships between other musical symbols. For instance, slurs, ties, dynamics wedges and many other notations create relationships between many notes a rests, sometimes in different staves and systems. As a consequece, a music score is better modelled by a lattice than by a tree.

-# Music has two dimensions, the horizontal one, the melody, and the vertical one, the harmony. When considering how to represent music, it is necessary to take into account that depending on the task to perform, music events (notes and rests) will have to be traversed following the music vertical dimension (time) or following the horizontal dimension (one voice after the other). Therefore the internal representation should be addecuate to facilitate both traversing needs.


The approach followed in Lomse has been to model the document by a tree structure of objects in memory, named the @IM, with additional structures that are synchronized when the @IM is modified.

The root element of the @IM tree, stored in the Document object, represents the whole document. It is a node of class ImoDocument and can be accessed by invoking Document::get_im_root() method. And the children of the root element represent the basic blocks for building a document: headers, paragraphs, music scores, lists, tables, images, etc. The @IM is, basically, a model similar to the XML DOM with two important differences for elements related to music scores:

- To facilitate traversing music scores by time and by voice, an additional model, the ColStaffObjs object, is maintained and is associated to each node representing a music score in the document.

- Music notations that create relationships between many musical symbols can not be nodes in the tree model as this would transform the tree into a lattice. Therefore, these notations are stored not as child nodes but as internal data.

It is important to note that the @IM and its associated structures form an abstract representation for a document, containing all necessary information to derive any other representation when needed. It is not oriented to any particular use (rendering, playback, printing, music analysis, document edition, ...) but try to serve all them equally. In this way the Document can serve to the purpose of any View object: a view can either be a print out of a conventional music score; or it can be a textual representation in LDP or any other language; or it can be a playback oriented representation, such a table of MIDI events; or any other representation for any purpose.

When a particular use is intended, the corresponding optimised representation is derived from the @IM by Lomse. For instance:

- For interchange with other applications: LDP, LMD and MusicXML source code.
- For rendering: the GraphicModel class.
- For score playback: the SoundEventsTable class.



@section internal-model-details The structure of a document
<!----------------------------------------------------------------------------------------------------->

The @IM is a tree of objects, derived from abstract class ImoObj. All these objects have @a Imo as class prefix and are sometimes named in Lomse documentation as @a IMOs: Internal Model Objects.

The root of the @IM tree is an object of class ImoDocument and represents the content of a whole document. 

@note
	Please be aware that class Document represents the document and contains the ImoDocument node, that is, the root of the @IM. This root ImoDocument object can be obtained by using Document::get_im_root() method.

The main blocks for building the @IM are the objects shown in the following diagram:

@verbatim
        
ImoObj                              Base class for all tree nodes
  |
  ├── ImoSimpleObj                  Non-renderizable. Just contains other objects or properties
  |                                 (e.g. option, style) 
  |           
  └──-ImoContentObj                 Renderizable objects representing document content
        |
        ├── ImoBlockLevelObj        Block-level objects (e.g. document, table, list, score, ...):
        |     |
        |     ├── ImoBlocksContainer    * Block-level containers for block-level objects.
        |     |                           (e.g. document, table, list)
        |     |
        |     ├── ImoInlinesContainer   * Block-level containers for inline-level objs.
        |     |                           (e.g. paragraph, heading, anonymous-block)
        |     |
        |     └── ImoScore              * A block-level object representing a music score
        |
        ├── ImoInlineLevelObj       Inline-level objects (e.g. wrapper[span], link, button, text, image)
        |
        └── ImoScoreObj             Content specific for music scores:
              |
              ├── ImoStaffObj       * Main music content (e.g. note, clef, key signature, barline)
              |
              ├── ImoAuxObj         * Modifiers. Attached to other score objects (e.g. fermata, notation)
              |                     
              └── ImoRelObj         * Relations. Relates two or more StaffObjs (e.g. slur, tie, beam)
                  
.
@endverbatim


As you can see, the main building blocks (ImoContentObj objects) are of three types: 
1. Block-level objects (e.g. document, table, list, score, ...),
2. Inline-level objects (e.g. wrapper[span], link, button, text, image), and
3. Content specific for music scores.


For example, the following LMD document:

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


And, internally, this document will be represented by the following tree:

@verbatim

                             ImoDocument
                                  |
                             ImoContent
                                  |
         ┌────────────────┬───────┴────────┬──────────────────┐
         |                |                |                  |
     ImoHeading     ImoParagraph       ImoScore         ImoParagraph
         |                |                |                  |
      ImoText          ImoText       ImoInstrument         ImoText
                                           |
                                      ImoMusicData
                                           |
                      ┌─────────────┬──────┴──────┬─────────┐
                      |             |             |         |
                   ImoClef   ImoKeySignature   ImoNote   ImoNote


.        
@endverbatim

You can see that the @IM tree follows, basically, the same structure than that of a LenMus document (LMD) and an score
follows the same structure than its LDP source.

For content other than music scores the @IM is similar to that of an HTML document. But for music scores there is some additional information added to the @IM tree. See section @ref internal-model-scores for details.



@section internal-model-hierarchy  Internal model objects hierarchy
<!----------------------------------------------------------------------------------------------------->


The base class for @IM objects is ImoObj. From it, a hierachy of derived classes is created. 

At the root of this hierarchy, ImoObj specializes in three main types: 

1. Temporay Data Transfer Objects used during model building (ImoDto), but never part of the final tree. They have 'Dto' suffix.
2. No renderizable objects containing just properties or relationships (ImoSimpleObj).
3. Objects describing the content of the document (ImoContentObj). They are renderizable and have properties.

Content objects (ImoContentObj) can be subdivided into two main groups (although no base class for them has been created):
- components, representing atomic objects, and
- containers, that is, objects that have components.

Specific objects for music score are divided into two main categories: staff objects (ImoStaffObj) and auxiliary objects
 (ImoAuxObj and ImoRelObj). Staff objects represent the 'main' music content (basically notes, rests, clefs, key signatures and time signatures), that is, those objects that must me rendered on a staff and that are are positioned by time (music 'events').

All other notations are considered auxiliary objects. In turn, there are two main types of auxiliary objects:
- ImoAuxObj objects: those modelling a simple musical notation (i.e. a fermata, an accent, an attacehd text), and
- ImoRelObj objects: those modelling a relationship between objects (i.e. a tie, and slur, a dynamics wedge).


Simple auxiliary objects require only a parent; for instance, in a fermata, the owner is a note, a rest or a barline. Therefore, simple auxiliary objects are modelled by a single ImoAuxObj object and can be stored as tree nodes.

On the contrary, relationship objects need to be linked to all the objects that are part of the relationship. For instance in a tie, the two tied notes. Relations can not be modelled explicitly as a node linked by two or more nodes, as this would transform the tree in a network. For this reason, RelObjs are objects that never will be tree nodes; instead, these objects will be stored in a special container node (ImoAttachments) and treated as data inside this node, not as child nodes of it.


The following diagram shows current hierarchy of objects. Those marked with '(A)' are abstract classes. In some cases the equivalence to HTML tags is marked:

@verbatim

    ImoObj (A) (type, id)
      |
      ├── ImoDto (A) () Data Transfer Objects (Dto suffix).  Temporay use 
      |     |           during model building, but never part of the final
      |     |           tree (the internal model)
      |     |
      |     ├── ImoFontStyleDto  
      |     ├── ImoColorDto
      |     ├── ImoBeamDto 
      |     ├── ImoSlurDto
      |     ├── ImoTieDto
      |     ├── ImoTupletDto
      |     ├── ImoGlissandoDto
      |     ├── ImoVoltaBracketDto
      |     ├── ImoBorderDto - not used!!!!
      |     ├── ImoPointDto - not used!!!!
      |     └── ImoSizeDto - not used!!!!
      |
      |
      ├── ImoSimpleObj (A) () No renderizable
      |     |                 They just contains other objects or properties
      |     |
      |     |  ___________________________________________________________
      |     |  === Value Objects to encapsulate data ==
      |     |     Some of them are stored as nodes in the tree; others,
      |     |     as member variables in other objects. Nevertheless,.
      |     |     all defined as tree nodes for uniform treatment.
      |     |
      |     ├── ImoBezierInfo -- data (*) in SlurData, TieData, TieDto, SlurDto
      |     ├── (ImoFiguredBassInfo)
      |     ├── (ImoFigBassIntervalInfo)
      |     ├── ImoInstrGroup -- data in ImoInstrument(*)
      |     ├── ImoLineStyle -- data in ImoTextBox, ImoLine(*)
      |     ├── ImoPageInfo -- data in ImoDocument, ImoScore
      |     ├── ImoStaffInfo -- data in ImoInstrument(list)
      |     ├── ImoStyle - data in ImoStyles (std::map)
      |     ├── ImoSystemInfo -- data in ImoScore
      |     ├── ImoTextInfo (string, style) -- data in ImoScoreText, ImoLyricsTextInfo
      |     |
      |     |       // Nodes in tree
      |     |
      |     ├── ImoMidiInfo -- child of ImoSounds, ImoSoundChange
      |     ├── ImoOptionInfo - child of ImoOptions
      |     ├── ImoLyricsTextInfo (text, syllableType, elision) -- child of ImoLyricsData
      |     ├── ImoParamInfo - child of ImoDynamics
      |     ├── ImoSoundInfo -- child of ImoSounds
      |     |
      |     |  ___________________________________________________________
      |     |  === Value objects to contain relation data for a node =====
      |     |
      |     ├── ImoRelDataObj (A)
      |     |     ├── ImoBeamData
      |     |     ├── ImoSlurData
      |     |     ├── ImoTieData
      |     |     ├── ImoGlissandoData
      |     |     └── ImoLyricsData
      |     |  
      |     |  ___________________________________________________________
      |     |  === Container objects; no additional properties ===========
      |     |
      |     ├── ImoCollection (A) () A node. Branches are the stored objs
      |     |     |
      |     |     ├── ImoAttachments  <auxobj>
      |     |     ├── ImoInstruments  <instrument>
      |     |     ├── ImoInstrGroups  <instrgroup>
      |     |     ├── ImoMusicData    <staffobj>
      |     |     ├── ImoOptions      <option>
      |     |     └── ImoTableSection(A)
      |     |           ├── ImoTableHead [thead]    <tableRow>
      |     |           └── ImoTableBody [tbody]    <tableRow>
      |     |  
      |     ├── ImoStyles     <style>  [a map to store style objects]
      |     ├── ImoRelations  <relobj> [a list to store relobjs]
      |     |
      |     └── ImoContainerObj (A) ()
      |           |
      |           └── ImoInstrument (... , [musicData])
      |     
      |
      └── ImoContentObj (A) (style, location, visible, [attachments])
            |
            ├── ImoBlockLevelObj (A)
            |     |       Abstract class for all block-level objects
            |     |
            |     ├── ImoScore
            |     |
            |     ├── ImoBlocksContainer (A)
            |     |     |       A block-level container for block-level objects. All 
            |     |     |       ImoBlocksContainer objects (except ImoContent and
            |     |     |       ImoDynamic) have a ImoContent object as container.
            |     |     |
            |     |     ├── ImoDocument [html/body]
            |     |     ├── ImoContent [div] - a generic block-level container
            |     |     |     └── ImoDynamic [object] 
            |     |     |
            |     |     ├── ImoMultiColumn - a container subdivided in columns
            |     |     ├── ImoTable [table] - a container for table related objects
            |     |     ├── ImoList [ol, ul]
            |     |     ├── ImoTableRow [tr] - a container for a row of cells
            |     |     ├── ImoListItem [li]
            |     |     └── ImoTableCell [td, th]
            |     |
            |     └── ImoInlinesContainer (A)
            |           |        A block-level container for inline-level objs.
            |           |
            |           ├── ImoAnonymousBlock []
            |           ├── ImoParagraph [p]
            |           └── ImoHeading [h1, h2, h3, h4, h5]
            |
            |
            ├── ImoInlineLevelObj (A)
            |     |       Inline-level objects
            |     |
            |     |
            |     ├── ImoBoxInline (A) (size, content) - an abstract 
            |     |     |        inline-level box container.
            |     |     |
            |     |     ├── ImoInlineWrapper [span] - A generic inline-box container
            |     |     └── ImoLink [a] - Anchor 
            |     |
            |     ├── ImoButton [button] - a button
            |     ├── ImoControl (control) - A user defined GUI obj.
            |     |     └── ImoScorePlayer - A control for managing score playback
            |     |
            |     ├── ImoTextItem [] - a piece of text with the same style
            |     └── ImoImage [img] - Inline image                        
            |               
            |
            └── ImoScoreObj (A) (color) content for scores
                  |
                  ├── ImoStaffObj (A) (timePos, numStaff)
                  |     |
                  |     ├── ImoBarline
                  |     ├── ImoClef
                  |     ├── ImoDirection
                  |     ├── ImoKeySignature
                  |     ├── ImoMetronomeMark
                  |     ├── ImoNoteRest (A) (noteType, dots, voice)
                  |     |     ├── ImoNote
                  |     |     └── ImoRest
                  |     ├── ImoTimeSignature
                  |     ├── ImoGoBackFwd
                  |     ├── ImoSoundChange
                  |     ├── ImoSpacer
                  |     └── ImoSystemBreak
                  |
                  ├── ImoAuxObj (A) () Can go attached to any score object
                  |     |
                  |     ├── ImoArticulation (A)
                  |     |     ├── ImoArticulationSymbol
                  |     |     └── ImoArticulationLine
                  |     |
                  |     ├── ImoFermata
                  |     ├── ImoDynamicsMark
                  |     ├── ImoOrnament
                  |     ├── ImoTechnical
                  |     ├── ImoSymbolRepetitionMark
                  |     |
                  |     ├── ImoBlock (A)
                  |     |     └── ImoTextBox (anchor line, paragraphs)
                  |     |
                  |     ├── ImoScoreLine
                  |     └── ImoScoreText (wrapper for ImoTextInfo)
                  |           ├── ImoScoreTitle (h-align) 
                  |           ├── ImoTextRepetitionMark (repeat-type)
                  |         ( └── ImoInstrNameAbbrev )
                  | 
                  └── ImoRelObj (A)  Relates two or more StaffObjs
                        ├── ImoTie
                        ├── ImoBeam
                        ├── ImoChord
                        ├── ImoSlur
                        ├── ImoTuplet
                        ├── ImoGlissando
                        ├── ImoLyrics
						└── ImoVoltaBracket


    [*] Double inheritance: also from ImoAuxObj class.

.
@endverbatim


Marked with '*': Double inheritance: also from ImoAuxObj class.








@section internal-model-scores  The structure of a music score
<!----------------------------------------------------------------------------------------------------->
         
A music score is represented by a ImoScore object, that derives from ImoBlockLevelObj. Therefore, ImoScore is a block level object. The approach followed for modelling music scores has been to capture the score structure by splitting the representation in *containers* and *content* objects. For example, we could imagine the staff as a container for notes and rests. 

There are only two container classes:

1. the score itself (ImoScore object), that represents a whole music score, and
2. the instrument (ImoInstrument class), that represents an score part.

The score is a container for instruments, and an instrument is a container for the objects representing the music for that instrument. But although both, score and instrument, are container classes, there is an important difference between them: the score is a block level object, that is, it is a constituent block for document content. But not the instrument, which is just an auxiliary collection of objects. Because of this reason, score and instrument are in different places in the hierarchy. ImoScore derives from ImoBlockLevelObj and ImoInstrument from ImoContainerObj.


All other score related objects are considered *content* objects, and are modeled by class ImoScoreObj. All these objects that can be included in a score are classified in two main groups:

1. staff objects (ImoStaffObj class). You can think about staff objects as those symbols that are placed on the staff and are the basis for describing the music: I am referring to symbols such as notes, rests, clefs, time and key signatures, barlines and other symbols with similar characteristics, objects that must be placed on the staff, represent events and the main notation, are ordered by time and are essential to describe a melodic line. And
2. auxiliary objects (classes ImoAuxObj and ImoRelObj), that represent all other auxiliary notations, usually attached to notes and rests.


An ImoInstrument is, basically, a collection of ImoStaffObj. But, instead of adding the ImoStaffObj nodes as direct children of ImoInstrument, an auxiliary node, ImoMusicData, is created:

@verbatim

                 ImoScore
                     |
                ImoInstrument
                     |
                ImoMusicData
                     |
        ┌────────────┼────────────┐
        |            |            |
     ImoClef      ImoNote      ImoNote


.        
@endverbatim





@subsection internal-model-scores-auxobjs  Auxiliary objects: classes ImoAuxObj and ImoRelObj 
<!----------------------------------------------------------------------------------------------------->

All other objects in the score that are not ImoStaffObj objects are considered auxiliary objects, They are like <i>modifiers</i> describing additional properties or adding some editorial highlight on something. For instance, a fermata modifies the duration of a note. A tie modifies the two tied notes by joining their durations, And a metronome mark adds playback information to an score.


@subsubsection internal-model-scores-attachments  Attachments: the ImoAuxObj class 

There are two types of auxiliary objects. The first group is formed by those auxiliary objects representing additional properties or modifiers for the owner object (one-to-one relations). For instance, a fermata modifies the duration of the note owning the fermata. These <i>simple</i> auxiliary objects are modeled by objects derived from class ImoAuxObj. They are included in the internal model just as children nodes of the objects they modify. For instance, a fermata in a note is modeled by a ImoFermata child node, whose parent is the ImoNote object. For example, the following score:

@image html internal-model-fermata-auxobj.png "Image: Modeling a fermata (simple auxiliary object)."

This score will create the following internal tree:


@verbatim
                        ImoScore
                            |
                       ImoInstrument
                            |
                       ImoMusicData
                            |
        ┌────────────┬──────┴─────┬────────────┐
        |            |            |            |
     ImoClef      ImoNote      ImoNote    ImoBarline
                                  |
                            ImoAttachments
                                  |
                              ImoFermata

.
@endverbatim

Notice that, in practice, ImoFermata is not a direct child of ImoNote. Instead, an intermediate container node ImoAttachments is created. This is just to keep together all ImoAuxObj objects attached to a parent node.

ImoAuxObjs can be attached to *any* score object. For instance, a title to an score, And also to other ImoAuxObj objects. For instance a text notice attached to a fermata that, in turn , is attached to a note.



@subsubsection internal-model-scores-relationships  Relationships: the ImoRelObj class 

The other group of auxiliary objects is formed by those objects modeling a relationship **between two or more staff objects**. For instance, a tie, and slur, or a dynamics wedge. These auxiliary objects are modeled by abstract class ImoRelObj.

ImoRelObj objects can not be included in the internal tree as children of the staff objects that are part of the relationship, as this would transform the tree into a network. For instance, consider the following score with two tied notes:

@image html internal-model-tie-relobj.png "Image: A tie relationship can not be modeled as a child node."

The @IM can not include a <i>Tie</i> node as child of each tied note, because this will create an invalid tree::

@verbatim
                  Score
                    |
                Instrument
                    |
    ┌─────────┬─────┴───────┬─────────┐
    |         |             |         |
  Clef      Note          Note     Barline
               \           /         
                \         /
                 \       /
                  \     /
					Tie
.
@endverbatim


For this reason, ImoRelObj objects can not be nodes in the tree model. To solve this problem, they are stored not as child nodes but as internal data in a list of attached ImoRelObj objects. This list is contained in node ImoRelations::

@verbatim
                       ImoScore
                           |
                      ImoInstrument
                           |
                      ImoMusicData
                           |
       ┌─────────────┬─────┴───────┬─────────┐
       |             |             |         |
    ImoClef       ImoNote       ImoNote  ImoBarline
                     |             |
                ImoRelations  ImoRelations 
                     |             |
                  ImoTie        ImoTie	<─── These objects derive from 
                     \            /          ImoRelObj and represent the
                    ┌─────────────┐          existence of a relationship.
                    |  \       /  |  
                    | ImoTieData  |<─── This object is not part of the tree.
                    |             |     The two ImoTie nodes store a pointer
		            └─────────────┘     to this shared object. It contains
									    the common data for the relationship.
.
@endverbatim


So, simple auxiliary objects (ImoAuxObj) are inserted as children in a ImoAttachments child node, and relation auxiliary objects (ImoRelObj) objects are stored as internal data in a ImoRelations child node::

@verbatim
                                 |
                     ┌───────────┴───────────┐
                     |                       |
                  ImoNote                 ImoNote
                     |                       |
           ┌─────────┴─────────┐       ImoRelations
           |                   |             |
    ImoAttachments       ImoRelations        |
           |                   |             |
      ImoFermata             ImoTie        ImoTie
                               \            /
                          ------------------------- 
                                 \       /    Not part of the tree 
                                  TieData         
.
@endverbatim

Let's see how the relationship data is stored. In any relationship you will find two types of data:

1. Data about the relationship itself: its existence, its properties and information that is common to all participants. For instance, class ImoBeam represents the grouping of several notes/rests by using a beam, and it contains the information that is common to all notes included in the beam, such as the beam color.

2. Specific data for each participant in the relationship, such as its role. For instance, for a beam, it is necessary to specify, for each note in the beam, the details about how to draw the beam for that note: as continuous line, as a forward hook, as a backward hook, etc.

Objects derived from class ImoRelObj represents a relationship, but they only contains the first type of data: the relationship properties and information that is common to all participants.

For modeling the specific data about each participant, another base class is used: class ImoRelDataObj. For instance, to model a group of tree beamed notes, the following objects are created:

* One ImoBeam object, derived from ImoRelObj. It represents the beam.
* Three ImoBeamData objects, derived from ImoRelDataObj. They represent the specific beaming information for each note: i.e. the beam type.

Each data object is associated to each participant object (i.e. the notes in the beam) by creating a std::pair object. And all these pairs are stored in the ImoRelObj in a list of participants. Thus, the internal structure for any ImoRelObj is the following:

@verbatim


    ptrs. to participants:  ImoStaffObj          ImoStaffObj
                               ^                    ^
    ptrs. to participants'     |                    |
    data (i.e. its role):      | ImoRelDataObj      | ImoRelDataObj
                               |     ^              |     ^
                               |     |              |     | 
                       ┌-------|-----|--------------|-----|-------- ...
    List of            |  first|     |second   first|     |second
    participants ───────────> std::pair ─────────> std::pair ─────> ...
                       |       
                       |       
                       └------------------------------------------- ...   
                       ImoRelObj
.
@endverbatim

In some relations, there is no specific data to store in the ImoRelDataObj objects. In these cases no ImoRelDataObj is created, and the internal pointers in ImoRelObj contains nullptr.

For instance, here are the objects involved in modeling some relationships:

@verbatim
    ImoRelObj                  ImoRelDataObj
      |                           |
      ├── ImoBeam                 ├── ImoBeamData
      ├── ImoChord                |      no chord data needed
      ├── ImoSlur                 ├── ImoSlurData
      ├── ImoTie                  ├── ImoTieData
      └── ImoTuplet               └── ImoTupletData
.
@endverbatim


@todo Finish this by describing the ImoAuxRelObjs (e.g. Lyrics)



@subsection internal-model-scores-measures-intro  How measures are modelled
<!----------------------------------------------------------------------------------------------------->

Initially, the idea of using a *measure container* to hold the staff objects was considered. But measures only exist when time signatures and barlines exist, and Lomse should be able to represent multi-metric and non-measured music.

The approach followed in Lomse has been to represent the music as a linearized sequence of staff objects (notes, rests, etc.) with explicit barline objects. As a measure extends from one barline to the next one, both representations (with or without measures containers) are equivalent: given the barlines it is easy to deduce measure limits in data which has no such measure containers; and given the measure containers, it is easy to produce a linearized sequence with only barlines. So both approaches are equivalent. But by using a measure container, Lomse will have to force a special case for music that has no measures. Therefore, the decision was not to include measure containers in the internal model. But as not all barlines defines the boundaries for measures (e.g. intermediate barlines) it is necessary to identify these barlines.

Nevertheless, as the concept of measure is very important in most common Western music, there are attributes (e.g. the displayed measure number) that have to be captured when they exist. In Lomse, the barlines contain the measure attributes for the measure that is ending in that barline, such as the displayed measure number. For accessing to this information use method `ImoBarline::get_measure_info`.

And for non-measured scores or when the score is incomplete and the last measure is not finished in a barline, the attributes for this last incomplete measure, if exist, are stored in the ImoInstrument object, and are accessible by method `ImoInstrument::get_last_measure_info()`. Notice that this method will return @nullptr when the score finishes in barline.


@subsubsection internal-model-scores-measures-needs  Tables related to measures
<!----------------------------------------------------------------------------------------------------->

For any method based on measure location information (e.g., for scrolling to measure/beat or for visual tracking effects during payback), it is necessary to solve two needs:

a) The first one is to convert the measure location data into timepos data. This information is inmutable for an internal model the conversion method can be provided by the internal model.

b) And the second one is for converting timepos data into spacial position in the graphic model and vice-versa (e.g., to determine the timepos for a mouse position). As this information is specific for each graphic model, the conversion method must be provided by the graphic model.

For solving these two needs there exist two objects: ImMeasuresTable and TimeGridTable objects.


<b>The ImMeasuresTable object</b>
<!----------------------------------------------------------------------------------------------------->

For solving the need to convert measure location data (e.g. measure/beat) into timepos data, the internal model contains a measures table. In fact, it is necessary to maintain a table per instrument, as for multi-metric music the number of measures is different in each score part (instrument).

The table is contained and managed by class ImMeasuresTable. Each ImoInstrument object contains an instance, with the specific information for that instrument. For single-metric scores, the tables in all instruments will contain the same data, as the number of measures will be the same for all instruments. Each table is accesible by method ImoInstrument::get_measures_table();

ImMeasuresTable objects are created by the model builder (class ModelBuilder) once the ColSatffObjs object is created. 



<b>The TimeGridTable object</b>
<!----------------------------------------------------------------------------------------------------->

For solving the need to convert timepos data into spacial position in the graphic model and vice-versa, the TimeGridTable object is responsible for supplying all occupied timepos and their positions.

There exist a TimeGridTable object for each system. It is owned by the GmoBoxSystem object, which provide access to it. Each %TimeGridTable object contains a table with the relation timepos <-> x-position for all positions currently occupied by staff objects in the system. The last position is normally the position of the barline at the end of the system. The system provides the y-position and the table the x-postion. 



*/


*/

