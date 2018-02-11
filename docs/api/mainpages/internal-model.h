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
  +-- ImoSimpleObj                  Non-renderizable. Just contains other objects or properties
  |                                 (e.g. option, style) 
  |           
  +---ImoContentObj                 Renderizable objects representing document content
        |
        +-- ImoBlockLevelObj        Block-level objects (e.g. document, table, list, score, ...):
        |     |
        |     +-- ImoBlocksContainer    * Block-level containers for block-level objects.
        |     |                           (e.g. document, table, list)
        |     |
        |     +-- ImoInlinesContainer   * Block-level containers for inline-level objs.
        |     |                           (e.g. paragraph, heading, anonymous-block)
        |     |
        |     +-- ImoScore              * A block-level object representing a music score
        |
        +-- ImoInlineLevelObj       Inline-level objects (e.g. wrapper[span], link, button, text, image)
        |
        +-- ImoScoreObj             Content specific for music scores:
              |
              +-- ImoStaffObj       * Main music content (e.g. note, clef, key signature, barline)
              |
              +-- ImoAuxObj         * Modifiers. Attached to other score objects (e.g. fermata, notation)
              |                     
              +-- ImoRelObj         * Relations. Relates two or more StaffObjs (e.g. slur, tie, beam)
                  
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
         +----------------+----------------+------------------+
         |                |                |                  |
     ImoHeading     ImoParagraph       ImoScore         ImoParagraph
         |                |                |                  |
      ImoText          ImoText       ImoInstrument         ImoText
                                           |
                                      ImoMusicData
                                           |
                      +-------------+-------------+---------+
                      |             |             |         |
                   ImoClef   ImoKeySignature   ImoNote   ImoNote


.        
@endverbatim

You can see that the @IM tree follows, basically, the same structure than that of a LenMus document (LMD). 



@section internal-model-scores  The structure of a music score
<!----------------------------------------------------------------------------------------------------->
         
A music score is represented by a ImoScore object, that derives from ImoBlockLevelObj. Therefore, ImoScore is a block level object. The approach followed for modelling music scores has been to capture the score structure by splitting the representation in *containers* and *content* objects. For example, we could imagine the staff as a container for notes and rests. 

There are only two container classes:

1. the score itself (ImoScore object), that represents a whole music score, and
2. the instrument (ImoInstrument class), that represents an score part.

The score is a container for instruments, and an instrument is a container for the objects representing the music for that instrument. But although bot, score and instrument, are container classes, there is an important difference between them: the score is a block level object, that is, it is a constituent block for document content. But not the instrument, which is just an auxiliary collection of objects. Because of this reason, score and instrument are in different places in the hierarchy. ImoScore derives from ImoBlockLevelObj and ImoInstrument from ImoContainerObj.


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
        +------------+------------+
        |            |            |
     ImoClef      ImoNote      ImoNote


.        
@endverbatim





@subsection internal-model-scores-auxobjs  Auxiliary objects: classes ImoAuxObj and ImoRelObj 
<!----------------------------------------------------------------------------------------------------->

All other objects in the score that are not ImoStaffObj objects are considered auxiliary objects, They are like <i>modifiers</i> describing additional properties or adding some editorial highlight on something. For instance, a fermata modifies the duration of a note. A tie modifies the two tied notes by joining their durations, And a metronome mark adds playback information to an score.


There are two types of auxiliary objects. The first group is formed by those auxiliary objects representing additional properties or modifiers for the owner object (one-to-one relations). For instance, a fermata modifies the duration of the note owning the fermata. These <i>simple</i> auxiliary objects are modeled by objects derived from class ImoAuxObj. They are included in the internal model just as children nodes of the objects they modify. For instance, a fermata in a note is modeled by a ImoFermata child node, whose parent is the ImoNote object. For example, the following score:



.. figure:: fermata-simple-auxobj.png
    :scale: 75 %

    :class: center
    :alt: Modeling a fermata (simple auxiliary object)


    **Figure:** Modeling a fermata (simple auxiliary object).


In LDP this score is written as:


@code
(score (vers 2.0)
    (instrument 
        (musicData
            (clef G)
            (n g4 q)
            (n c5 q (fermata above))
            (barline)
        )
    )
)
@endcode


And it will create the following internal tree:


@verbatim
                        ImoScore
                            |
                       ImoInstrument
                            |
                       ImoMusicData
                            |
        +------------+------+-----+------------+
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







The other group of auxiliary objects is formed by those objects modeling a relationship **between two or more staff objects**. For instance, a tie, and slur, or a dynamics wedge. These auxiliary objects are modeled by abstract class ImoRelObj.



ImoRelObj objects can not be included in the internal tree as children of the staff objects that are part of the relationship, as this would transform the tree into a network. For instance, consider the following score with two tied notes:



.. figure:: tie-rel-auxobj.png

    :scale: 75 %

    :class: center

    :alt: A tie (relationship auxiliary object) can not be modeled as a child node.



    **Figure:** A tie (relationship auxiliary object) can not be modeled as a child node.

    




@todo Finish the @IM description

*/

/*
The Document class encapsulates all the internals, providing the basic API for creating, modifying and using a document.

It is an observable class and, therefore, inherits methods for notifying observers (other registered objects) when changes occur to the document. For example, when the document is modified.


 For instance, consider two tied notes. The @IM will include a <i>Tie</i> node as child of each tied note. But the real relationship is modelled by an external node pointed by the two Tie nodes:

@verbatim
                  Score
                    |
                Instrument
                    |
    +---------+-----+-------+---------+
    |         |             |         |
  Clef      Note          Note     Barline

              |             |
             Tie           Tie
               \            /         
		      +-------------+
              |  \       /  |  This object is not part of the tree.
              |   TieInfo   |  The two Tie nodes store a pointer
              |             |  to this shared object.
		      +-------------+
      
@endverbatim


@subsection scores-issues Music scores issues
<!----------------------------------------------------------------------------------------------------->

For content other than music scores the @IM is similar to that of an HTML document. But for music scores there is some additional information added to the @IM tree:

-# The tree structure representing the music score content is complemented with an auxiliary representation, the ColStaffObjs object.

-# IMOs representing relationships between other IMOs (i.e. ImoTie) have to be linked to the objects that form part of the relationship. But these relation objects can not be modelled as tree nodes as that would transform the tree in a network. For instance, the following score with two tied notes:

@verbatim

    (score (vers 1.6)
        (instrument 
            (musicData
                (clef G)
                (key C)
                (n a4 q (tie 1 start))
                (n a4 q (tie 1 stop))
            )
        )
    )

   would create the following invalid tree:

                         ImoScore
                            |
                       ImoInstrument
                            |
                       ImoMusicData
                            |
           +-------------+--+----------+--------+
           |             |             |        |
        ImoClef   ImoKeySignature   ImoNote  ImoNote
                                       \        / 
                                        \      /
                                         ImoTie

@endverbatim

For this reason, the notation object itself, ImoTie in previous example, is not included as part of the tree, but stored in a special container class. This is described in detail in section :ref:`relation-auxobjs`.


In LDP this score is written as::

    (score (vers 1.6)
        (instrument 
            (musicData
                (clef G)
                (n a4 q (tie 1 start))
                (n a4 q (tie 1 stop))
                (barline)
            )
        )
    )

If the tie is modeled as a child node common to the related notes, it would create the following invalid tree::

                 ImoScore
                     |
                ImoInstrument
                     |
                ImoMusicData
                     |
       +--------+----+---+---------+
       |        |        |         |
    ImoClef  ImoNote  ImoNote  ImoBarline
                \        / 
                 \      /
                  ImoTie



For this reason, ImoRelObj objects can not be nodes in the tree model. To solve this problem, they are stored not as child nodes but as internal data in a list of attached ImoRelObj objects. This list is contained in node ImoRelations::


                       ImoScore
                           |
                      ImoInstrument
                           |
                      ImoMusicData
                           |
       +-------------+-----+-------+---------+
       |             |             |         |
    ImoClef       ImoNote       ImoNote  ImoBarline
                     |             |
                ImoRelations  ImoRelations 
                     \            /         
  --- --- --- --- --- --- --- --- --- ---- ---
                       \       /    Not part of the tree. Stored
                         ImoTie     as object data




So, simple auxiliary objects (ImoAuxObj) are inserted as children in a ImoAttachments child node, and relation auxiliary objects (ImoRelObj) objects are stored as internal data in a ImoRelations child node::


                                 |
                     +-----------+-----------+
                     |                       |
                  ImoNote                 ImoNote
                     |                       |
           +---------+---------+       ImoRelations
           |                   |             |
    ImoAttachments       ImoRelations        |
           |                   \            /         
      ImoFermata          - --- --- --- --- --- ---- ---
                                 \       /    Not part of the tree 
                                   ImoTie         




..
    Implications for tree traversing
<!----------------------------------------------------------------------------------------------------->

    As all ImoObj are organized in a tree they can be accessed by tree traversing methods or by using visitors. For instance, Tree class provides methods such as::

        get_parent()
        append_child()
        remove_child()


    But nodes of class ImoAttachments are an exception. These nodes can have children (simple ImoAuxObjs), and these children will be traversed by normal tree traversing methods. But ImoAuxObjs has also an internal list containing all attached ImoRelObj objects. As these objects are not part of the tree, special traversing methods are needed.

    To simplify precesses, the existence of this exception and its treatment I've tried to isolate and enclosed the treatment of this exception. For this, * **use of Tree traversing methods is forbidden** * and, to replace them, new methods are created. These new method takes care of proper treatment for ImoAttachment nodes. To hide the original Tree methods and replace them by new ones, that could deal properly with ImoAttachment nodes, I derived from Tree a new ImoTree class. And overrided some methods::

        get_parent_imo()
        append_child_imo()
        remove_child_imo()

    But for Visitors I didn't find a clean solution, so when programming a new Visitor class you have to take care of dealing with ImoAttachments nodes peculiarities.



Relation data objects: class ImoRelDataObj
<!----------------------------------------------------------------------------------------------------->

In any relationship you will find two types of data:

#. Data about the relationship itself: its existence, its properties and information that is common to all participants. For instance, class ImoBeam represents the grouping of several notes/rests by using a beam, and it contains the information that is common to all notes included in the beam, such as the beam color.

#. Specific data for each participant in the relationship, such as its role. For instance, for a beam, it is necessary to specify, for each note in the beam, the details about how to draw the beam for that note: as continuous line, as a forward hook, as a backward hook, etc.


Objects derived from class ImoRelObj represents a relationship, but they only contains the first type of data: the relationship properties and information that is common to all participants.

For modeling the specific data about each participant, another base class is used: class ImoRelDataObj. For instance, to model a group of tree beamed notes, the following objects are created:

* One ImoBeam object, derived from ImoRelObj. It represents the beam.
* Three ImoBeamData'' objects, derived from ImoRelDataObj. They represent the specific beaming information for each note: i.e. the beam type.

Each data object is associated to each participant object (i.e. the notes in the beam) by creating a std::pair object. And all these pairs are stored in the ImoRelObj in a list of participants. Thus, the internal structure for any ImoRelObj is the following::


    ptrs. to participants:  ImoStaffObj          ImoStaffObj
                               ^                    ^
    ptrs. to participants'     |                    |
    data (i.e. its role):      | ImoRelDataObj      | ImoRelDataObj
                               |     ^              |     ^
                               |     |              |     | 
                       +- -- --|-- --|-- -- -- -- --|-- --|-- -- -- ...
    List of            |  first|     |second   first|     |second
    participants -----------> std::pair ---------> std::pair -------> ...
                       |       
                       |       
                       +- -- -- -- -- -- -- -- -- -- -- -- -- -- -- ...   
                       ImoRelObj



In some relations, there is no specific data to store in the ImoRelDataObj objects. In these cases no `ImoRelDataObj is created, and the internal pointers in ImoRelObj contains NULL.

Here are the objects involved in modeling currently implemented relationships::

    ImoRelObj                  ImoRelDataObj
      |                           |
      +-- ImoBeam                 +-- ImoBeamData
      +-- ImoChord                |      no chord data needed
      +-- ImoTie                  +-- ImoTieData
      +-- ImoTuplet               +-- ImoTupletData
      +-- ImoSlur                 +-- ImoSlurData




Notes about some objects
<!----------------------------------------------------------------------------------------------------->

ImoSpacer and anchor objects
---------------------------------

ImoSpacer is an ImoStaffObj whose purpose is to add more space between the staff objects than precede and follow the ImoSpacer. That is, it is like the `space` character in text processing.

But ImoSpacer plays also an important role: it is the *anchor* object for attaching auxiliary objects to an staff. As ImoAuxObj objects are not ImoStaffObj they can not be included directly as content for an staff. Remember that ImoAuxObj objects can only be attached to staff objects. To avoid creating artificial dependencies (attaching auxiliary objects to notes or rest that have nothing to do with the attached object) it is better to attach the auxiliary object to a 'neutral' staff object with no meaning, an *anchor* object: an ImoSpacer of zero width.


..
    Owning ImoAuxObj objects
<!----------------------------------------------------------------------------------------------------->

    simple ImoAuxObj --> any ImoContentObj
    ImoRelObj --> only to ImoStaffObj

    It should be possible to have AuxObjs not attached only to StaffObjs but to many places. For example: assume a text box (AuxObj) owned by a fermata (AuxObj), owned by a note (StaffObj).

    Another example: Score titles, Instrument names, etc are examples of AuxObjs not owned by StaffObjs.

    This implies that AuxObjs can own other AuxObjs. But AuxObjs layout requires that:
      a) the owner must provide positioning information. AuxObjs are relative positioned
      b) the owner must be able to do TenthsToLogical conversions.

    Therefore, the only restriction would be that the root of owners chain must not be an AuxObj. In summary:

     * In order to own AuxObjs it is necessary to be able to do TenthsToLogical conversions.
     * An AuxObj can own other AuxObjs but the root of owners chain must not be an AuxObj (otherwise it will never be rendered!).
     * At creation, an AuxObj has no owner. The owner must be set at attachment time, and must be updated at each attachment/detachment
     * If its owner is a StaffObj, measurements can be in tenths, as can be converted to LUnits. Otherwise, if owner chain reaches a VStaff/Staff, measurements can also be in tenths and this grand-father VStaff will be used for units conversion. Otherwise, units must be absolute.



@subsection im-hierarchy Internal model objects hierarchy
<!----------------------------------------------------------------------------------------------------->

The base class for @IM objects is ImoObj. From it, a hierachy of derived classes is created. 

At the root of this hierarchy, ImoObj specializes in three main types: 

-# Temporay Data Transfer Objects used during model building (ImoDto), but never part of the final tree. They have 'Dto' suffix.
-# No renderizable objects containing just properties or relationships (ImoSimpleObj).
-# Objects describing the content of the document (ImoContentObj). They are renderizable and have properties.

Content objects (ImoContentObj) can be subdivided into two main groups (although no base class for them has been created): components, representing atomic objects and containers, that is objects that have components.

Specific objects for music score are divided into two main categories: staff objects and auxiliary objects. Staff objects represent the 'main' content of the music (basically notes, rests, clefs, key signatures and time signatures), that is, those objects that must me rendered on a staff and that are are positiones by time (music 'events).

All other notations are considered auxiliary objects. In turn, there are two main types of auxiliary objects: those modelling a simple musical notation (i.e. a fermata, an accent, an attacehd text) and those modelling a relationship between objects (i.e. a tie, and slur, a dynamics wedge).

* Simple auxiliary objects require only a parent; for instance, in a fermata, the owner is a note, a rest or a barline. Therefore, simple auxiliary objects are modelled by a single ImoAuxObj object and can be stored as tree nodes.

* On the contrary, relationship objects need to be linked to all the objects that are part of the relationship. For instance in a tie, the two tied notes. Relations can not be modelled explicitly as a node linked by two or more nodes, as this would transform the tree in a network. For this reason, RelObjs are objects that never will be tree nodes; instead, these objects will be stored in a special container node (ImoAttachments) and treated as data inside this node, not as child nodes of it.

The following picture shows current hierarchy of objects. Those marked with '(A)' are abstract classes. In some cases the equivalence to HTML tags is marked:

@verbatim

    ImoObj (A) (type, id)
      |
      +-- ImoDto (A) () Data Transfer Objects (Dto suffix).  Temporay use 
      |     |           during model building, but never part of the final
      |     |           tree (the internal model)
      |     |
      |     +-- ImoBeamDto 
      |     +-- ImoColorDto
      |     +-- ImoFontStyleDto  
      |     +-- ImoSlurDto
      |     +-- ImoTieDto
      |     +-- ImoTimeModificationDto
      |     +-- ImoTupletDto
      |     +-- ImoBorderDto - not used!!!!
      |     +-- ImoPointDto - not used!!!!
      |     +-- ImoSizeDto - not used!!!!
      |
      |
      +-- ImoSimpleObj (A) () No renderizable
      |     |                 They just contains other objects or properties
      |     |
      |     |  ___________________________________________________________
      |     |  === Value Objects to encapsulate data ==
      |     |     Some of them are stored as nodes in the tree; others,
      |     |     as member variables in other objects. Nevertheless,.
      |     |     all defined as tree nodes for uniform treatment.
      |     |
      |     +-- ImoBezierInfo -- data (*) in SlurData, TieData, TieDto, SlurDto
      |     +-- (ImoFiguredBassInfo)
      |     +-- (ImoFigBassIntervalInfo)
      |     +-- ImoInstrGroup -- data in ImoInstrument(*)
      |     +-- ImoLineStyle -- data in ImoTextBox, ImoLine(*)
      |     +-- ImoLyricsText_info,
      |     +-- ImoLyricsExtend_info,
      |     +-- ImoMidiInfo -- data in ImoInstrument
      |     +-- ImoPageInfo -- data in ImoDocument, ImoScore
      |     +-- ImoStaffInfo -- data in ImoInstrument(list)
      |     +-- ImoSystemInfo -- data in ImoScore
      |     +-- ImoTextInfo (string, style) -- data in ImoScoreText
      |     +-- ImoTextBlockInfo,
      |     +-- ImoTextStyle,
      |     |
      |     |       // Nodes in tree
      |     |
      |     +-- ImoOptionInfo - node in ImoOptions
      |     +-- ImoParamInfo - node in ImoDynamics
      |     +-- ImoStyle - not node, but element in ImoStyles (std::map)
      |     |
      |     |  ___________________________________________________________
      |     |  === Value objects to contain relation data for a node =====
      |     |
      |     +-- ImoRelDataObj (A)
      |     |     +-- ImoBeamData
      |     |     +-- ImoSlurData
      |     |     +-- ImoTieData
      |     |     +-- ImoTupletData
      |     |     +-- ImoGlissandoData
      |     |  
      |     |  ___________________________________________________________
      |     |  === Container objects; no additional properties ===========
      |     |
      |     +-- ImoCollection (A) () A node. Branches are the stored objs
      |     |     |
      |     |     +-- ImoAttachments  <auxobj>
      |     |     +-- ImoInstruments  <instrument>
      |     |     +-- ImoInstrGroups  <group>
      |     |     +-- ImoMusicData    <staffobj>
      |     |     +-- ImoOptions      <option>
      |     |     +-- ImoTableSection(A)
      |     |           +-- ImoTableHead [thead]    <tableRow>
      |     |           +-- ImoTableBody [tbody]    <tableRow>
      |     |  
      |     +-- ImoStyles     <style>  [a map to store style objects]
      |     +-- ImoRelations  <relobj> [a list to store relobjs]
      |     |
      |     +-- ImoContainerObj (A) ()
      |           |
      |           +-- ImoInstrument (... , [musicData])
      |     
      |
      +---ImoContentObj (A) (style, location, visible, [attachments])
            |
            +-- ImoBlockLevelObj (A)
            |     |       Abstract class for all block-level objects
            |     |
            |     +-- ImoScore
            |     |
            |     +-- ImoBlocksContainer (A)
            |     |     |       A block-level container for block-level objects. All 

            |     |     |       ImoBlocksContainer objects (except ImoContent and
            |     |     |       ImoDynamic) have a ImoContent object as container.
            |     |     |
            |     |     +-- ImoDocument [html/body]
            |     |     +-- ImoContent [div] - a generic block-level container
            |     |     |     +-- ImoDynamic [object] 
            |     |     |
            |     |     +-- ImoMultiColumn - a container subdivided in columns
            |     |     +-- ImoTable [table] - a container for table related objects
            |     |     +-- ImoList [ol, ul]
            |     |     +-- ImoTableRow [tr] - a container for a row of cells
            |     |     +-- ImoListItem [li]
            |     |     +-- ImoTableCell [td, th]
            |     |
            |     +-- ImoInlinesContainer (A)
            |           |        A block-level container for inline-level objs.
            |           |
            |           +-- ImoAnonymousBlock []
            |           +-- ImoParagraph [p]
            |           +-- ImoHeading [h1, h2, h3, h4, h5]
            |
            |
            +-- ImoInlineLevelObj (A)
            |     |       Inline-level objects
            |     |
            |     |
            |     +-- ImoBoxInline (A) (size, content) - an abstract 
            |     |     |        inline-level box container.
            |     |     |
            |     |     +-- ImoInlineWrapper [span] - A generic inline-box container
            |     |     +-- ImoLink [a] - Anchor 
            |     |
            |     +-- ImoButton [button] - a button
            |     +-- ImoControl (control) - A user defined GUI obj.
            |     |     +-- ImoScorePlayer - A control for managing score playback
            |     |
            |     +-- ImoTextItem [] - a piece of text with the same style
            |     +-- ImoImage [img] - Inline image                        
            |               
            |
            +-- ImoScoreObj (A) (color) content for scores
                  |
                  +-- ImoStaffObj (A) (timePos, numStaff)
                  |     |
                  |     +-- ImoBarline
                  |     +-- ImoClef
                  |     +-- ImoKeySignature
                  |     +-- ImoMetronomeMark
                  |     +-- ImoTimeSignature
                  |     +-- ImoGoBackFwd
                  |     +-- ImoSystemBreak
                  |     +-- ImoSpacer
                  |     +-- ImoNoteRest (A) (noteType, dots, voice)
                  |           +-- ImoNote
                  |           +-- ImoRest
                  |
                  +-- ImoAuxObj (A) () Can go attached to any score object
                  |     |
                  |     +-- ImoArticulation (A)
                  |     |     +-- ImoArticulationSymbol
                  |     |     +-- ImoArticulationLine
                  |     |
                  |     +-- ImoFermata
                  |     +-- ImoDynamicsMark
                  |     +-- ImoOrnament
                  |     +-- ImoTechnical
                  |     |
                  |     +-- ImoBlock (A)
                  |     |     +-- ImoTextBox (anchor line, paragraphs)
                  |     |
                  |     +-- ImoScoreLine
                  |     +-- ImoScoreText (text-info)
                  |           +-- ImoScoreTitle (h-align) 
                  |         ( +-- ImoInstrNameAbbrev )
                  | 
                  +-- ImoRelObj (A)  Relates two or more StaffObjs
                        +-- ImoTie
                        +-- ImoBeam
                        +-- ImoChord
                        +-- ImoSlur
                        +-- ImoTuplet
                        +-- ImoGlissando

@endverbatim

Marked with '*': Double inheritance: also from ImoAuxObj class.



@subsection imoobj-issues Notes about some ImoObj objects
<!----------------------------------------------------------------------------------------------------->

<h3>ImoContentObj and ImoTable</h3>

Both contain ImoStyle objects. But they can not be stored as child nodes (either directly or by using an ImoCollection object) because all ImoStyle objects are also stored at ImoDocumnet level. Storeing them also in other ImoObj object would produce crashes when deleting the ImoObj tree, because these duplicated ImoStyle objects will be deleted twice.


*/

