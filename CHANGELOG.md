# Lomse Library. Log of changes


[Since last version] 0.26.0
=============================

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.26.0

- None.

##### COMPATIBLE CHANGES

- Added methods in `Interactor` to add/remove visual application markings. Created the
  first application markings class: `FragmentMark`. This class allows to display on any
  score brackets and vertical lines, spanning one or several staves, to define the start
  and end points of a score section or fragment.



Version [0.26.0] (29/Dec/2018)
=============================

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.25.0

- Pitch representation in Lomse internal model was not the best representation for
  supporting transposition or other pitch changes. So, it was necessary to review and
  change all this in the internal model. This change should not create backwards 
  incompatibilities in user applications, but it is an important change.

- External methods related to key signatures have been packaged as static methods in
  a new class KeyUtilities. Probably, current applications using Lomse never used
  these methods, so this change should not create backwards incompatibilities.

##### COMPATIBLE CHANGES

- Added new edition commands for transposition.
- Added class FIntval for managing intervals.
- MusicXML importer now accepts metronome directions.
- The algorithm for auto-scroll has been improved.
- During playback, when the time signature changes, metronome clicks
  interval is now adjusted for maintaining notes duration equivalence.
- Added new methods for controlling scroll and visual tracking during
  playback, when using an external player. A new sample 'extplayer' has
  been added to the 'examples' folder.
- Added Interactor method to customize visual tracking effects
- Added Interactor method to define what is a beat. This allows, for
  example, to subdivide metronome or to decide which note duration
  corresponds to a metronome click. It is also useful for methods that
  specify a location by using measure/beat parameters.
- The internal model now has information about measure attributes, such
  as the displayed measure number. All importers now parse and
  store this information.
- Measure numbers can now be rendered. Options for controlling measures
  numbering have been added. MusicXML importer modified for importing
  measure numbering options.
- Style "Measure numbers" has been added to the scores for controlling
  the appearance of measure numbers.
- Defined a new barline type: alternative rendering for double repetition,
  using two thick barlines instead of thin barlines (E.Gould, p.234)
- Fixes for removing Microsoft compiler warnings.
- Fixes for all high impact outstanding defects detected with coverity scan.
- Fixes for a few bugs causing a crash in specific scenarios:
    - Visual tracking effects now cleared after deleting the graphic model.
    - The sound thread is now not deleted when it has finished.
    - Crash caused by visual tracking when the score is embedded in a
	  text, at the end of a document page, but the score is laid out in
	  the next document page.
- Fixes for a few bugs related to scores layout:
    - Fixed bug preventing to modify spacing value parameter from score
	  option "Render.SpacingValue". Also it was incorrectly treated as
	  float in ScoreMeter and in LDP parser.
    - Fixed bugs related to Gourlay's lines justification and value for Dmin
	  parameter.
    - Spacing when there are invisible shapes (e.g. spacers) where
	  incorrectly computed
    - A clef change after prolog was not engraved in smaller size because
	  it was considered part of the prolog.
- Fixed some bugs related to visual tracking effects:
    - Color for highlight notes visual tracking was not initialized.
    - Bug in tempo line positioning preventing to move the line to the correct page.
    - Incorrect positioning visual tracking tempo line when anacrusis start.
- Other changes:
    - Removed unused method `Interactor::move_tempo_line_to(ImoStaffobj* pSO)`.
    - Incorrect computation of beat type (strong, weak, off-beat), that
	  affected to the computation of note volume, for stressing strong beats.
	  It also affected Lomse user applications that could be using this method, such
      as LenMus Phonascus.
    - Spacing algorithm has been refactored to facilitate maintenance. All
	  spacing methods have been reviewed and more tests added. All test
	  has been documented as well as the reasons for the different rules.
	  Fixes in a couple of spacing rules.



Version [0.25.0] (29/Jun/2018)
=============================

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.24.0

* Until now all visual tracking effects were named score highlight effects
  due to the fact that highlighting notes/rests was the first programmed
  visual effect. But, currently, Lomse supports more visual tracking
  effects and, for coherence and to use more descriptive names, all
  references to score highlight effects have been changed, when appropriate,
  to 'visual tracking' effects. In particular, the more important changes
  that will affect your code are:
	- Class `EventScoreHighlight` changed to `EventVisualTracking`.
	- Type `SpEventcoreHighlight` changed to `SpEventVisualTracking`.
	- Enum item `k_highlight_event` changed to `k_tracking_event`.
	- Method `Interactor::on_visual_highlight` changed to `Interactor::on_visual_tracking`.
	- Enum item `k_end_of_higlight` changed to `k_end_of_visual_tracking`.
	- Enum item `k_advance_tempo_line` changed to `k_move_tempo_line`.
	- Enum item `k_highlight_event` changed to `k_tracking_event`.


##### COMPATIBLE CHANGES

* Added method `Interactor::set_visual_tracking_mode(int mode)` for
  selecting the visual trackin effect to use. A new enum `EVisualTrackingMode`
  defines valid modes.
* Changes to allow extension '.musicxml' for uncompressed MusicXML files.
* Added some initial code to deal with `global` elements in experimental
  MNX importer.
* Changes to allow compilers other than GCC and MSVC.
* Several fixes to make Lomse buildable in macOS with clang compiler.
* Several changes for trying to reduce or eliminate dependencies from
  other libraries. In particular:
	- PR #144 Replaced boost::format with standard functions
	- PR #146 Replaced boost::variant with union class



Version [0.24.0] (4/Mar/2018)
=============================

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.23.0

Removed enum in ViewFactory. All values 'ViewFactory::k_view_xxxxx' must
be replaced by just 'k_view_xxxxx'.


##### COMPATIBLE CHANGES

	* Added a new View (SingeSystemView) to display scores as a single
	  system.
	* Added method Interactor::set_view_background() for customizing
	  the background color in all views.



Version [0.23.0] (24/Feb/2018)
==============================

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.22.0

- While documenting class Document and starting to document the internal
  model, some methods were renamed to use more meaningfull names:
	- `Document::removed_from_model()` renamed to `Document::on_removed_from_model()`.
	- `Document::get_imodoc()` renamed to `Document::get_im_root()`.
	- `ImoScore::close()` renamed to `ImoScore::end_of_changes()`.

- InternalModel class was removed. It was useless as it was just a wrapper
  class without additional functionality.


##### COMPATIBLE CHANGES

- Make Document a facade for accessing some ImoDocument frequently used
  methods.

- Added more code to the MNX importer, for testing current
  specification draft.

- Created an MNX exporter, also for testing current specification draft
and to prepare Lomse for fully supporting the MNX specification.



Version [0.22.0] (9/Jan/2018)
=============================

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.21.0

While documenting Lomse events some issues where found (see issue #111)
and it was decided to refactor some events. Unfortunatelly most of these
changes create incompatibilities due to renames. In particular:

- Some useless event classification methods in EventInfo have been removed.

- EventPlayScore has been renamed:
	- EventPlayScore of types k_do_play_score_event, k_pause_score_event and
      k_stop_playback_event renamed as EventPlayCtrl.
    - EventPlayScore of type k_end_of_playback_event renamed as EventEndOfPlayback

- Removed event types k_highlight_on_event, k_highlight_off_event,
  k_end_of_higlight_event and k_advance_tempo_line_event, and replaced by
  an enum in EventScoreHighlight. 

- Event EventMouse (type k_show_contextual_menu_event) renamed as
  EventContextualMenu (type k_show_contextual_menu_event)

- Created EventMouse, type k_link_clicked_event. It replaces on_click
  events on ImoLink because the way in which they are sent to the user
  application is different and, therefore, it is confusing how to handle
  them. The new event type is also more specific.

- Removed EventView and created bases classes EventAction and EventPlayback

- Method Interactor::send_end_of_play_event()" renamed as 
  "on_end_of_play_event()" to avoid raising questions on users about 
  the contradiction of having to ask the Interactor to
  "send_end_of_play_event" when an end of play event has been received
  and is being processed.


##### COMPATIBLE CHANGES

- Playback improvements: playback now takes into account repetitions 
  implied by barlines with repetition dots and volta brackets, as well
  as repetions implied by textual repetition marks (i.e. Da Capo, 
  To Coda, etc, ...) (issue #79).

- Auto-scroll during playback (issue #88): When playing back an score
  it is necessary to ensure that current played notes are visible. 
  Therefore, as the playback advances, the score should auto-scroll to
  maintaint the viewport on the notes being played back. The implemented
  solution defines a new Lomse event, *EventUpdateViewport*, that
  informs about the need to change the viewport and provides the
  coordinates for the new viewport origin. It is up to the user
  application to do it or not.

- Technical changes:
	- As c++11 and std::regex support is required, CMakeLists.txt has
	  been updated:
		- cmake required version is now 2.8.10 or greater, and
		- gcc required version is now 4.9 or greater.
	- Added initial code for start experimentation with MNX format.

- Bugs fixed:
	- #98. Tempo speed setting problem using PlayerGui object.
	- #96. Score deletion crash due to double inclusion in RelObjs.


Version [0.21.0] (9/Nov/2017)
=============================

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.20.0

- Building Lomse now requires to use c++11 or greater.

##### COMPATIBLE CHANGES

- Many changes for improving support for MusicXML:
	- Import and render volta brackets.
	- Import and render 'words'. Detect strings implying repetition
  	  marks (i.e. "To Coda", "D.C.", ...)
	- Fix beams in malformed MusicXML files (issue #67).
	- Assume some default value for clef in MusicXML importer (issue #68)
	- Added options for controlling behaviour of MusicXML importer when
  	  uncomplete or malformed MusicXML files.
	- MusicXML importer now deals with all midi information in score-part
      (MusicXML 3.0) and is added to the internal model (issue #82).
	- Fixed error in the MusicXML importer: the pitch for notes with
  	  accidentals was computed wrong (issue #77).
- Improvements in tuplets renderization and support:
    - Nested tuplets now fully supported.
    - MusicXML importer now imports tuplets.
    - Nested tuplets now fully supported in LDP.
    - Layout of tuplets improved and also nested tuplets are now drawn.
- Improvements in layout:
	- Systems justification changed for using approximate sff.
	- Line breaker algorithm improved. Now it considers different break
	  modes.
	- Fixed spacing issues, related to clefs and prolog objects, detected 
  	  in regression tests.
	- Fixed an issue with justification, detected in regression tests. It
  	  caused that in some cases the system was not justified.
- LDP related changes:
	- Blank space in LDP exporter has been normalized.
	- LDP exporter reviewed for following the same syntax rules than 
  	  LDP Analyser, thus ensuring a round trip in LDP export-import.
	- Some small fixes in LDP exporter.
- Fixed issue #81. Instead of assigning MIDI channel 0 to any instrument
  without MIDI settings, now a different channel is assigned to each
  instrument.
- Fixed issue #69: Playback very slow for 3/8 tempo
- Fixed issue #73: Highlight is not synced. when moving to next page.
- Fixed issue #70: Score highlight does not work in some scores.
- Fixed issue #10: Note flags too long for short stems.
- Fixed issue #99: small issue that can cause a crash.
- Implemented method SimpleView::get_view_size() that was empty!


Version [0.20.0] (1/Sep/2016)
=============================

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.19.0

- LibraryScope methods 'set_draw_anchors()' and 'draw_anchors()'
  have been renamed as 'set_draw_anchor_objects()' and 
  'draw_anchor_objects()', respectively.

##### COMPATIBLE CHANGES

- The spacing algorithm has been replaced. Previous algorithm was scattered
  along several objects and was difficult to understand and, therefore, to 
  modify and to improve. It was first isolated in a single object with a 
  clear interface, and an abstract class was defined to facilitate 
  replacement and experimentation with different algorithms in future. Then 
  a new algorithm, based on Gourlay's approach, was implemented. The new 
  algorithm is easier to understand and gives more flexibility.

- The lines break algorithm was still valid, but the penalty function has 
  been changed. This improves layout and gives more control and flexibility. 

- Lyrics are now taken into account for spacing notes, removing the lyrics
  layout problems that were present in previous version. The internal model
  for lyrics has been replaced by a new model, to allow for better control
  of the layout process.
  Also additional space between staves is added when lyrics are present.
  The solution for vertical staves spacing is provisional and requires more
  changes (in study). Nevertheless, in preparation for these changes, the
  score layout algorithm have been modified for dealing with systems 
  with variable height.

- Trace and debug options has been added to the library, to facilitate
  experimentation and debugging of spacing algorithm and lines breaker 
  algorithm.

- Although in finished scores is normal practice to justify the last system,
  there are cases were this is not required or even need to be prevented. 
  Also, truncation of staff lines after last object is a need in some 
  cases. Therefore, scenarios for justification and truncation of last 
  system have been analyzed, and specific options for controlling the 
  behaviour have been added to the library. Now users can fully control 
  these aspects. LDP options for this have been reviewed and adapted. 
  As a consequence, two existing options ('Score.JustifyFinalBarline' 
  and 'StaffLines.StopAtFinalBarline') have been deprecated and replaced
  by two new options ('Score.JustifyLastSystem' and 'StaffLines.Truncate').

- For backwards compatibility to prevent undesired changes in existing 
  scores due to the new algorithms (that is, for ensuring that the visual 
  appearance of existing LDP scores, mainly for LenMus eBooks, is preserved), 
  the spacing algorithm parameters have been adjusted. New LDP options 
  ('Render.SpacingOptions' and "Render.SpacingFopt") has been added for 
  controlling the behaviour of the new spacing algorithm and the lines 
  breaker algorithm, so that users can have now full control of spacing 
  and lines breaking algorithm options.

- Renderization is improved in several aspects:
    - Score titles are now rendered.
    - Additional syllables and elision symbols in lyrics are now rendered.
    - Added more symbols for fermata.
    - Names and brackets/braces for groups of instruments now rendered.
    - Barlines for groups now can be joined. Also mensurstrich layout is
      supported.
    - Left barline at start of systems now also drawn in empty scores.

- Added support, in LDP, for many music notations: lyrics, articulations
  (accents & stress marks), dynamics marks, groups of instruments and
  score titles.

- Also several fixes and small changes:
    - Fixes for avoiding compilation errors with c++11 compilers.
    - Whitespace in text now properly managed in LMD files (issue #42).
    - Default styles added to headings, paragraphs and tables (issue #43).
    - New system tag now works again (issue #1).
    - MusicXML importer now supports part groups.
    - Added squared bracket for part groups (group symbol == line)


Version [0.19.0] (2/May/2016)
==============================

Continue defining and documenting the public API.

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.18.0

Some typos fixed in public members and enum values:

- Constant *k_glyph_small_32th_note* in enum EGlyphIndex renamed as *k_glyph_small_32nd_note*
- Constant *k_32th* in enum ENoteTypes renamed as *k_32nd*
- Constant *k_duration_32th_dotted* in enum ENotesDuration renamed as *k_duration_32nd_dotted*
- Constant *k_duration_32th* in enum ENotesDuration renamed as *k_duration_32nd*
- Method *Interactor::get_ellapsed_times()* renamed as *Interactor::get_elapsed_times()*

Some methods changed:
- Removed unnecesary parameter in some CmdCursor constructors.

And some methods removed from public API.


Version [0.18.0] (26/Mar/2016)
==============================

The public API of Lomse is being defined and documented.
API documentation is available at http://lenmus.github.io/lomse/
  

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.17.20

Version 0.18.0 includes changes for defining the public API of the
following classes:

- LomseDoorway
- Interactor
- Presenter

This implies that some methods are no longer available in public API.

Also, the API for printing has been reviewed:

- Removed method Interactor::get_page_size_in_pixels().
- Added method Interactor::set_print-ppi().
- Method Interactor::set_printing_buffer() renamed as 
     Interactor::set_print_buffer().
- Method Interactor::on_print_page() renamed as Interactor::print_page().
  Parameter 'scale' has been removed.



Version [0.17.20] (5/Mar/2016)
==============================

Mainly, changes for improving the MusicXML importer, for supporting
more music notation elements and for implementing visual regression tests.
Development moved to GitHub.
    

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.16.1

- none

##### COMPATIBLE CHANGES

- Version numbering scheme modified to add build inormation. Patch level
  automatically increased with every merge.
- Added method Interactor::get_num_pages() for improving the print API
- The XML parser (rapidxml) has been replaced by pugixml for better
  Unicode support.
- Line number added to error messages related to XML files, for better
  error reporting.
- Score layouting improved:
    - Fixed bug 80222: Some measures are splitted in two systems
    - Fixed bug 80223: lack of space after intermediate barlines
    - ColumnsBreaker algorithm reviewed and fixed. More tests added.
- Added support for more music notation elements:
    - articulations
    - ornaments
    - technical notations (just a few)
    - dynamics notations
    - slurs
    - lyrics
- Continue development of MusicXML importer:
	- barlines, repetition barlines and barlines at start of measures
	- augmentation dots for notes and rests
	- beams
	- backup/forward
	- full-measure rests
	- fermatas
	- ties
    - articulations
    - ornaments
    - technical notations (just a few)
    - dynamics notations
    - slurs
    - lyrics
    - derive note type from duration when type not present
- Modifications in internal model:
	- duration now explicit value for note/rest in internal model
	- explicit mark in rests for full measure rests
	- orientation flag added to ties, for avoiding having to use a bezier 
        element for changing default orientation.
    - added several classes for supporting the added elements.
- Renderization:
    - Added engravers and all need objects (glyphs, shapes, etc) for
        the new added music notation elements.
- Small fixes:
	- Removed background borders in document images generated for printing.
- Other chages:
    - CMakeLists.txt build script modified so that library name will be
        always 'liblomse', without version numbers.
    - CmakeLists.txt  build script modified so that by default, the unit
        tests runner program (testlib) is always built.
    - All tests files reviewed and reorganized for visual regression
        tests. Scripts for this created, so that regression tests will now
        systematically be run after every library build.
    - Added tutorial 1 for using Lomse in Qt applications.
    - Fixed all warnings due to 'unused parameter'
    

Version [0.16.1] (7/Sep/2015)
=============================

Mainly, changes for using SMuFL compliant music fonts and for implementing support
for new features required by eBook L3_MusicReading in Phonascus. Some bug fixes.
    

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.15.0

- Constants ImoStyle::k_bold and k_italics renamed as k_font_style_italic and
k_font_weight_bold. Constant k_font_normal splitted into k_font_style_normal
and k_font_weight_normal. Required changes: replace the renamed constants:
    - ImoStyle::k_bold    &rarr; replace by  ImoStyle::k_font_weight_bold
    - ImoStyle::k_italic  &rarr; replace by  ImoStyle::k_font_style_italic
    - ImoStyle::k_normal  &rarr; replace by  ImoStyle::k_font_weight_normal or
    - ImoStyle::k_font_style_normal, depending on case.
    	
##### COMPATIBLE CHANGES

- ImFactory modified to ensure that Id is assigned *before* the imo is created.
This fixes a design problem causing bug 80211.
- Code modified for using SMuFL compliant music fonts. Default font changed to
Bravura font. LenMus font (lmbasic2.ttf) removed, and from now unsupported.
- Defined default style for metronome marks. Added code for controlling that
default styles are not exported to source code unless they are modified.
- Added support for common time and cut time signatures (LDP language, LdpAnalyser,
LdpExporter, TimeSignatureEngraver, ImoTimeSignature and other related changes).
- Fixed bugs 80213 and 80214 in examples for tutorials
- Score chopin_prelude_v20.lms updated for fixing bug 80212
- Included all note symbols and dots in metronome marks
- Trace levels added to spacing algorithms
- Fixed bug 80215: First goFwd doesn't work properly and notes get overlapped.
- Fixed bug 80217: Stem length in chords incorrectly computed: too long.
- Fixed bug 80216: Fermata in note on second staff is positioned on first staff.
	

Version [0.15.0] (27/Mar/2015)
============================

Development continues. Mainly changes for supporting document edition. A few backwards
incompatible changes, that will require a couple of fixes in existing code using this
library. 
	

##### BACKWARDS INCOMPATIBLE CHANGES WITH 0.14.0

- enum EBarlines created.
&rarr; Backwards compatibility: You have to replace all barline constants, 
  such as "ImoBarline::k_simple" by "k_barline_simple", etc
- The update screen policy has been reviewed and clarified. As a consequence some
changes has been made affecting, mainly, to Interactor class. 
&rarr; Backwards compatibility: Interactor::new_viewport() and all Interactor
  methods for changing scale and zooming factor, now force a redraw. A third
  parameter (bool fForceRedraw=true) has been added to these methods. You can 
  modify your existing code by specifying fForceRedraw=false for preventing
  redraws and keep the old behaviour
- File format parameter added to LomseDorway::new_document() and 
PresenterBuilder::new_document() for choosing source code format.
&rarr; Backwards compatibility: You will have to review your code and add 
  this parameter with value 'Document::k_format_ldp' as that was the
  expected format in previous versions.
	
##### COMPATIBLE CHANGES

- Version changed to 0.15.0
- Shape id now generated and assigned to shapes. Methods in GModel to find
  a shape reviewed
- ScoreCursor now skips implicit key and time signatures
- TimeInfo class implemented, for computing timecode and related data
- ScoreCursor now provides TimeInfo data
- DocCommandExecuter now saves cursor state and uses it in undo/redo
- DocCommand::perform_action() now return int indicating success/failure
- DocCommand added to undo/redo stack only if execution success
- Problems detected with delete staffobj command => Solution: goBack/goFwd
  reconsidered. goBack removed. goFwd as special non-visible rest. Voice
  now mandatory. LDP version changed to 2.0
- ColStaffObjs changed to deal with LDP 2.0
- Added 'add staff obj.' command
- Added 'add tie' command
- Added 'add tuplet' command
- Added 'change dots' command
- Added 'delete block level obj.' command
- Added 'delete relation' command
- Added 'delete staff obj.' command
- Added 'break beam' command
- Added 'join beam' command
- Added 'insert block level obj.' command
- Added 'insert staff obj.' command
- Added 'insert many staff objs.' command
- Lomse now provides events when the set of selected objects change and when
  a commnad is executed
- SelectionSet now contains and provides an ordered ColStaffObjs object 
  containing all selected staffobjs
- SelectionSet now provides validations on selections, for enabling/disabling
  tools. You can override these default validations to have a different 
  behaviour in your applications
- Commands now receive optional name, to support i18n
- Added im_attributes
- Reviewed engravers for taking into account object color
- Commands redesigned for accepting target from selection set
- TimeGridTable now created for each GmoBoxSystem
- Class TimeGrid created. Grid displayed on system in which caret is placed
- Created TaskDataEntry for supporting data entry using mouse
- Added OverlaysGenerator and VisualEffect classes, for improving performance,
  optimizing repaints and providing a common architecture for all visual
  effects
- Paint event (k_update_window_event) now includes damaged rectangle 
  information so that user application can optimize repaints
- Selection rectangle, caret, playback highlight and time grid are now implemented 
  as VisualEffect objects, for improving visual speed and for uniform treatment
  of all dynamic visual effects
- Added DraggedImage class for supporting dragged images
- TaskSelection behavior changed to deal with more complex behaviour required by
  interactive edition. More tasks added: TaskSelectionRectangle, TaskMoveObject
- Added Handler visual effect class. Support implemented in GmoShapeTie
- Class NoterestEngraver removed
- Some engravers modified for creating shapes to be dragged without requiring
  to tie them to an ImoObj.
- Select objects with click, add to selection with Ctrl+click.
- Shapes related to notes and rests modified to take voice into account so 
  that voices can now be coloured, hidden, etc
- RenderingOptions now includes options for highlighting current selected 
  voice and for colouring voices
- Document modified for detecting if it has been changed since las time it was
  saved. DocCommandExecuter now controls when document is modified and keeps
  it synchronized with undo/redo.
- All ImoObj objects now have flags to control edit options on each object
- Composite commands implemented
- CmdAddNoteRest for replace edition mode implemented
- Caret behaviour redefined and all cursors re-implemented
- Commands refactored for using new methods for updating cursor
- ModelBuilder was only dealing with scores at first level. This is now 
  fixed and can deal with scores at inner levels
- Edition flags added to all ImoObj, to allow for selective edition
- DocCursor improved to traverse documents with nested content
- DocCursor now supports 'jailed' mode: cursor cannot exit the specified object
- Added 'restricted' edition mode: Only specified object can be edited, and it cannot be deleted.
- Added method 'play_measures()' to allow playing a certain number of
  measures starting at any meaure.
- Fixed dots position in repetition barlines
- Starting MusicXML support:
    - Added classes MxlCompiler, MxlAnalyser
- Class LmdParser renamed as XmlParser. Files renamed.
    - Changes for MusicXML import: Minimal functionality for MxlAnalyser implementated and tested
	

Version [0.14.0] (3/May/2013)
============================

Fixes to support Chinese language, bug fixes related to multi-threading,
other fixes and some refactoring.


- Version number increased to 0.14.0 due to backwards incompatible API changes.
- Document and Interactor raw pointers are now converted to shared pointers
  when ownership is transferred to Presenter.
- All Presenter API raw pointers to Document and to Interactor changed 
  to weak_ptr.
- All events receive now a weak_ptr to Interactor instead of a raw_ptr so
  that code processing event can check if the event is still valid.
- Limitation removed: Time units now represented as double.
- Fixed bug in Ctrol objects: bad font selected for Chinese language.
- Changed GmoShapeText and ImoScoreText to include language .
- PresentersCollection removed: not used and not needed.
- Fixed bug in ColStaffObjs iterator causing problems in destructor.
- ScorePlayerCtrol: changes to use a graphical UI.
- Refactoring: classes RefToGmo and MultiRefToGmo removed (not used).
- Refactoring: GmoShape Id type changed from int to ShapeId.
- Refactoring: ImoObj Id type changed from long to ImoId.
- Refactoring: GmoRef defined. Maps to get Gmo for Imo revised.
- Bug fixed in DefaultTextSplitter causing an infinite loop (app. hangs).
- Bug fixed in ScorePlayer causing occational crashes.
- Bug fixed in Interactor mouse events processing, causing crashes.
- Logger class developped. Logging messages added in many places.
    


Version [0.13.1] (18/Mar/2013)
=============================

Implementing edition (2). Caret and CaretPositioner.

- Version number increased to 0.13.1
- Caret object implemented: a displayable cursor on the document.
- CaretPositioner implemented: responsible for Caret layout and position.
- Interactor/Presented modified to create/own DocCursor & DocCommandExecuter
- GraphicView modified to display Caret
- Interactor: added methods to control Caret state and position
- ScoreCursor: state now includes timepos for ref_obj.
- ScoreCursor: added method to find previous staffobj
- ScoreCursor: added helper methods to test for position type.
- GModel modified for accesing a GmoBox given the id of its creator Imo
- GModel: added helper methods for positioning caret.
    

Version [0.13.0] (17/Feb/2013)
=============================

Implementing edition commands (1). Refactoring and required functionality.

- More funcionality:
    - DocumentCursor and children cursors created.
    - ScoreCursor: changes for moving on a time grid
    - Added ScoreModifier class: Preliminary code to delete a staffobj
- Changes for ensuring integrity after an edit operation:
    - IdAssiner now responsible for translation from ImoObj Id to pointer to ImoObj.
    - ImoObj pointer replaced by ImoObj id. in many API methods
- Changes for supporting undo/redo:
    - LDP: comments now also posible in C style (/*..*/)
    - Document: generates checkpoints and restore from checkpoint.
    - LmdExporter: main tags operational with scores in ldp format.
- Refactoring:
    - version number increased to 0.13, to reflect backward incompatible API changes
    - Added ButtonCtrl. All Controls reviewed to ensure uniform approach.
    - ColStaffObjs: adding support inserting/deleting objects
        - time now kept in staffobj instead of in ColStaffObjEntry
        - change ColStaffObjs to a double linked list. Sort during insertion
        - change ColStaffObjsBuilder to remove goBack/goFwd nodes


Version 0.12.5 (24/Nov/2012)
============================

- Added public method GmoShapeImage::set_image().
- Added progress bar control (class ProgressBarCtrl).
- Bug fixed: background color doesn't change if a different color specified.


Version 0.12 (7/Sep/2012)
=========================

- TextSplitter class added, to support different languages.
- Language and font-file tags added to LDP and LMD analysers.
- Added default font for Chinese.
- Added Metronome class.
- Added CheckboxCtrl class.
- Changes in PlayerGui class. Defined PlayerNoGui class.
- Changes for allowing customization of ScorePlayerCtrl at Lomse initialization.
- Changes in ScorePlayer for better usage of PlayerGui.
- Fixed bug in ScorePlayerCtrol, preventing labels translation.
- ScorePlayer::quit() method added, to avoid problems during user application quit.


Version 0.11
============

- Added method LomseDoorway::set_default_fonts_path() so that applications linking Lomse statically can inform Lomse about the path where default fonts are located.


Version 0.10.b1
===============

- Initial public release, used in Phonascus 5.0 beta for Linux.


[Since last version]: https://github.com/lenmus/lomse/compare/0.26.0...HEAD
[0.26.0]: https://github.com/lenmus/lomse/compare/0.25.0...0.26.0
[0.25.0]: https://github.com/lenmus/lomse/compare/0.24.0...0.25.0
[0.24.0]: https://github.com/lenmus/lomse/compare/0.23.0...0.24.0
[0.23.0]: https://github.com/lenmus/lomse/compare/0.22.0...0.23.0
[0.22.0]: https://github.com/lenmus/lomse/compare/0.21.0...0.22.0
[0.21.0]: https://github.com/lenmus/lomse/compare/0.20.0...0.21.0
[0.20.0]: https://github.com/lenmus/lomse/compare/0.19.0...0.20.0
[0.19.0]: https://github.com/lenmus/lomse/compare/0.18.0...0.19.0
[0.18.0]: https://github.com/lenmus/lomse/compare/0.17.20...0.18.0
[0.17.20]: https://github.com/lenmus/lomse/compare/0.16.1...0.17.20
[0.16.1]: https://github.com/lenmus/lomse/compare/0.15.0...0.16.1
[0.15.0]: https://github.com/lenmus/lomse/compare/0.14.0...0.15.0
[0.14.0]: https://github.com/lenmus/lomse/compare/0.13.1...0.14.0
[0.13.1]: https://github.com/lenmus/lomse/compare/0.13.0...0.13.1
[0.13.0]: https://github.com/lenmus/lomse/compare/0.12.5...0.13.0

