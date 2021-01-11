/**

@page page-sound-generation Scores playback overview

@tableofcontents

@section page-sound-generation-overview How Lomse playback works

Lomse is platform independent code and cannot generate sounds for your specific platform. Therefore, Lomse implements playback by generating real-time events and sending them to your application. It is responsibility of your application to handle these events and do whatever is needed, i.e. transform sound events into real sounds or doing whatever you would like with the sound events.

From Lomse internal point of view, playing back an score involves, basically, two steps:

- The score internal model is parsed to build the sound model.
- The sound model is then traversed and three kind of events are generated:

  -# Sound events, for generating sounds.
  -# Visual tracking events, for adding visual tracking effects to the displayed score during playback. For instance, highlighting notes as they are being played or displaying a vertical line at current beat position.
  -# End of playback events, oriented to facilitate GUI controls synchronization and housekeeping.

It is responsibility of your application to handle these events and do whatever is necessary. But for visual effects, lose provides an implementation for generating them so, if desired, your application can delegate in lose for this task.

Alternatively, your application could do playback by other mechanisms (e.g., using an external player) and to provide visual feedback by synchronizing the performance with the displayed score by using Lomse methods for this. See @ref page-sound-generation-external-player.




@section page-sound-generation-summary Your application set-up: summary

For playing back an score your application has to:

- Define a class, derived from ``MidiServerBase``. This class will receive the sound events and will have the responsibility for generating the sounds. See @ref page-sound-generation-events.

- Create an instance of class ``ScorePlayer``. This class takes care of most of the work to do. By using it, playing an score is just two tasks:
  -# Load the score to play in the ``ScorePlayer`` instance.
  -# Ask ``ScorePlayer`` to play it, specifying the desired options (i.e. visual tracking, metronome settings, count-off, etc).

  See @ref page-sound-generation-play-score.

- Deal with @a visual @a tracking events. All @a visual @a tracking events will be sent to the standard callback for events. For processing them, your application could delegate in Lomse by invoking method Interactor::on_visual_tracking_event(). See @ref page-sound-generation-tracking.

- Optionally, you should create a class derived from "PlayerGui". This will allow you to link your application playback controls to lomse, so that lomse can collect current settings when needed. See @ref page-sound-generation-player-gui.

As you can see, implementing score playback in an application is not complex, and the only burden for your application is coding a @c MidiServer class for generating the sounds.


@section page-sound-generation-events How to handle sound events

For playing back an score your application has just to define a class, derived from MidiServerBase. This class defines the interface for processing sound events. Your application has to define a derived class (i.e. @c MyMidiServer) and implement the following virtual methods: 

@code
    virtual void program_change(int channel, int instr) {}
    virtual void voice_change(int channel, int instr) {}
    virtual void note_on(int channel, int pitch, int volume) {}
    virtual void note_off(int channel, int pitch, int volume) {}
    virtual void all_sounds_off() {}
@endcode

Sound events will be sent, directly, to your @c MyMidiServer class just by invoking any of the virtual methods. Invocation of these methods is done in real time, that is, lomse will determine the exact time at which a note on / note off has to take place, and will invoke the respective method, ``note_on()`` or ``note_off()``, at the appropriate time. This implies that your midi server implementation is only responsible for generating or stopping sounds when requested, and no time computations are needed.

Lomse does not impose any restriction about how to generate sounds other than low latency. Perhaps, the simpler method to generate sounds is to rely on the MIDI synthesizer of the PC sound card.
    
@attention As playback is a real-time task, your code must return quickly. If it needs to do some significant amount of work then you must schedule this work asynchronously, for example by posting a windows message, or you should use a separate thread. Your application should not retain control for much time as this would result in freezing lomse playback thread.



@section page-sound-generation-play-score How to play an score

Class ScorePlayer provides the necessary methods for controlling all playback (start, stop, pause, etc.). Your application will have to request lomse the instance of ScorePlayer by invoking LomseDoorway::create_score_player() method and passing the @c MidiServer to use:

@code
    MyAppMidiServer* pMidi = new MyAppMidiServer();
    ScorePlayer* pPlayer = m_lomse->create_score_player(pMidi);
@endcode

This can be done only once if your application saves the ScorePlayer instance in a global variable and ensures the appropriate life scope for your @c MyAppMidiServer object. 

Once you have the ScorePlayer instance, playback is just loading the score to play (by invoking ScorePlayer::load_score() method) and invoking the appropriate methods, such as ScorePlayer::play() or ScorePlayer::stop() or ScorePlayer::pause();

Method ScorePlayer::load_score() requires the score to play. How to get it depends on your application, but a simple way of doing it is by using the Document methods for traversing the document and accessing its components. For instance:


@code
    //get the score player you have set up for your application
    ScorePlayer* pPlayer  = myAppGlobals->get_score_player();

    //get the first score, load it in the player and start playback
    if (SpInteractor spInteractor = m_pPresenter->get_interactor(0).lock())
    {
        ADocument doc = m_pPresenter->get_document();
        AScore score = doc.first_score();
        if (score.is_valid())
        {
            //load the score in the player
            pPlayer->load_score(score, nullptr);

		    //optional: select desired visual tracking effect
            spInteractor->set_visual_tracking_mode(k_tracking_tempo_line);

            //get settings for playback, probably from GUI controls
            bool fVisualTracking = true;    //generate visual tracking effects
            long nMM = 60;                  //beats per minute

            //start playback
            pPlayer->play(fVisualTracking, nMM, spInteractor.get());
        }
    }
@endcode



@section page-sound-generation-tracking Handling visual tracking events

Apart from generating sound events, lomse also generates @a visual @a tracking events, that is, events to add visual tracking effects, synchronized with sound, on the displayed score.

For generating visual effects you have two options, either do it yourself by modifying the lomse graphical model as desired or, simpler, delegate in lomse for generating standard visual effects. Lomse offers two type of visual effects:

- Highlighting notes as they are being played.
- Displaying a vertical colored tempo line across the system, positioned at current beat.

The type of visual tracking event to generate is controlled by method Interactor::set_visual_tracking_mode(). Valid values for visual tracking effects are defined in enum EVisualTrackingMode. By default, if method Interactor::set_visual_tracking_mode() is not invoked, Lomse will highlight_notes and rests as they are played back. Several visual effects can be used simultaneously by combining values with the OR ('|') operator. For example:

@code
spInteractor->set_visual_tracking_mode(k_tracking_tempo_line | k_tracking_highlight_notes);
@endcode

@a Visual @a tracking events are sent to your application via the event handling callback, that you set up at Lomse initialization. When handling a @a visual @a tracking event, if your application would like to delegate in Lomse for visual effects generation, the only thing to do is to pass the event to the interactor:

@code
    spInteractor->handle_event(pEvent);
@endcode

Lomse will handle the event and will send an <i>update window</i> event to your application, for updating the display.

Some properties of the tracking effects, such as its colour, can be customized. See method Interactor::get_tracking_effect(). For the customizable properties see the documentation of each specific visual effect. Example:
@code
VisualEffect* pVE = spInteractor->get_tracking_effect(k_tracking_tempo_line);
if (pVE)
{
	TempoLine* pTL = static_cast<TempoLine*>(pVE);
	pTL->set_color(Color(255,0,0,128));     //transparent red
	pTL->set_width(200);		            //logical units: 2 mm
}
@endcode

@todo Advanced topic: direct modification of the graphic model.


@section page-sound-generation-player-gui The PlayerGui object

For controlling playback some GUI controls (buttons, menu items, etc. to trigger start, stop, pause actions, sliders or other for setting tempo speed, etc.) are normally required.

The simplest way for passing the value of a playback option (i.e. the tempo speed) is to pass the value directly to lomse when asking to play the score in method ScorePlayer::play(). But this approach has a drawback: if the user changes, for instance, the tempo slider, lomse will not be informed of the change.

For solving these kind of problems, the solution is to link your application playback controls to lomse, so that lomse can be informed when changes take place or can collect current settings when needed. For doing this linking your application will have to define a class derived from PlayerGui (it can be your main window) and implement a few virtual methods to allow lomse to access the current values of your application playback controls:

@code
    virtual int get_play_mode() = 0;
    virtual int get_metronome_mm() = 0;
    virtual Metronome* get_metronome() = 0;
    virtual bool countoff_status() = 0;
    virtual bool metronome_status() = 0;
    virtual void on_end_of_playback() = 0;
@endcode

The implementation of these methods is usually as simple as linking each method to the appropriate control in your application. For instance:

@code
bool MainWindow::countoff_status()
{
    return GetMenuBar()->IsChecked(k_menu_play_countoff);
}
@endcode

PlayerGui is also the object that will receive <i>end of playback events</i>. These are events generated by lomse for signaling the end of playback, so that your application can restore GUI controls related to playback or do other things. This event is sent <b>simultaneously</b> to your application by to mechanisms:
- Via the event handling callback, set up at Lomse initialization, and
- by invoking method PlayerGui::on_end_of_play_back() if a PlayerGui was set when loading the score.

If you look at example code in section @ref page-sound-generation-play-score (relevant lines duplicated here):

@code
    //load the score and start playback
    if (score.is_valid())
    {
        pPlayer->load_score(score, nullptr);
@endcode

the second parameter for load_score is nullptr, meaning that no PlayerGui is used. As you can deduce, the way of informing lomse of the GUI proxy to use is by passing a pointer to the PlayerGui in this method:

@code
    //get your PlayerGui object
    MyPlayerGui* pGui = ... 

    //load the score and link playback options to your MyPlayerGui object
    pPlayer->load_score(score, pGui);

    //setting tempo to 0 forces lomse to use the tempo returned by your MyPlayerGui object.
    //a value different from 0 forces lomse to use that tempo
    pPlayer->play(fVisualTracking, 0, spInteractor.get());
@endcode


@section page-sound-generation-external-player Using an external player

If your application would like to use an external player and to provide visual feedback by synchronizing the performance with the displayed score, Lomse can not do this automatically as it doesn't control the playback, but Lomse provides some methods that can help your application to achieve the sound/display synchronization.

For synchronizing the performance with the displayed score it would be necessary:
1. to synchronize the visual tracking effects with the performance, and
2. to scroll the view as playback advances.

Visual tracking effects can be managed by your application by invoking Interactor methods for displaying, hiding and positioning the desired visual effect. Thus the only requirement is that your application can provide the necessary information for positioning the visual effect as playback advances.

Probably, the most easy approach for visual tracking effects will be to use the tempo line or the tempo block, as for using them it is only required that your application can identify the location (current measure and beat, or current time position) being played back. If your application can get that information, the procedure for synchronizing the tempo line with the playback would be as follows:

1. Enable the desired visual effect. For this just invoke:
@code
spInteractor->set_visual_tracking_mode(k_tracking_tempo_line);
@endcode
This can be done at any moment before playback. So perhaps the best time to do it is when the View is created or when the @c Play button is clicked.

2. Once the playback has started your application has to take care of advancing the tempo line each time a new beat is going to be played back and of doing scroll if necessary:
@code
int measure =  ... 	//0..n
int beat = ... //relative to measure. First beat is beat #0
spInteractor->move_tempo_line_and_scroll_if_necessary(scoreId, measure, beat);
@endcode

The above method will do scroll only when Lomse determines it is necessary. If you would like to use your own algorithms for scrolling, then, instead of using `move_tempo_line_and_scroll_if_necessary()`, you should use `move_tempo_line()` and to invoke `scroll_to_measure()` when your application considers this convenient: 
	@code
	int measure =  ... 	//0..n
	int beat = ... //relative to measure. First beat is beat #0
	spInteractor->move_tempo_line(scoreId, measure, beat);
	if (scroll_is_necessary())
		spInteractor->scroll_to_measure(scoreId, measure, beat);
	@endcode


3. Finally, when playback is stopped or has finished it is necessary to ensure that any displayed visual effect is removed:
@code
spInteractor->remove_all_visual_tracking();
@endcode


And that's all. You can see a working example in examples/samples/extplayer.


Trying to highlight/unhighlight the notes and rests as playback advances would be too complex as this would require that your application could provide a pointer to the note/rest being played back and to detect when it has to be unhighlighted. But the approach will be similar.


*/

