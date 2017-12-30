/**

@page sound-generation Scores playback overview

@tableofcontents

@section sound-generation-overview How Lomse playback works

Lomse is platform independent code and cannot generate sounds for your specific platform. Therefore, lomse implements playback by generating real-time events and sending them to your application. It is responsibility of your application to handle these events and do whatever is needed, i.e. transform sound events into real sounds or doing whatever you would like with the sound events.

From Lomse internal point of view, playing back an score involves, basically, two steps:

- The score internal model is parsed to build the sound model.
- The sound model is then traversed and three kind of events are generated:

  -# Sound events, for generating sounds.
  -# Highlight events, for adding visual tracking effects to the displayed score. For instance, highlighting notes as they are being played or displaying a vertical line at current beat position.
  -# End of playback events, oriented to facilitate GUI controls synchronization and housekeeping.

It is responsibility of your application to handle these events and do whatever is necessary. But for visual effects, lomse provides an implementation for generating them so, if desired, your application can delegate in lomse for this task.


@section playback-summary Your application set-up: summary

For playing back an score your application has to:

- Define a class, derived from ``MidiServerBase``. This class will receive the sound events and will have the responsibility for generating the sounds. See @ref handling-sound-events.

- Create an instance of class ``ScorePlayer``. This class takes care of most of the work to do. By using it, playing an score is just two tasks:
  -# Load the score to play in the ``ScorePlayer`` instance.
  -# Ask ``ScorePlayer`` to play it, specifying the desired options (i.e. visual tracking, metronome settings, count-off, etc).

  See @ref how-to-play-score.

- Deal with @a highlight events. All @a highlight events will be sent to the standard callback for events. For processing them, your application could delegate in Lomse by invoking method Interactor::on_highlight_event(). See @ref handling-highlight-events.

- Optionally, you should create a class derived from "PlayerGui". This will allow you to link your application playback controls to lomse, so that lomse can collect current settings when needed. See @ref implementing-player-gui.

As you can see, implementing score playback in an application is not complex, and the only burden for your application is coding a @c MidiServer class for generating the sounds.


@section handling-sound-events How to handle sound events

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



@section how-to-play-score How to play an score

Class ScorePlayer provides the necessary methods for controlling all playback (start, stop, pause, etc.). Your application will have to request lomse the instance of ScorePlayer by invoking LomseDoorway::create_score_player() method and passing the @c MidiServer to use:

@code
    MyAppMidiServer* pMidi = new MyAppMidiServer();
    ScorePlayer* pPlayer = m_lomse->create_score_player(pMidi);
@endcode

This can be done only once if your application saves the ScorePlayer instance in a global variable and ensures the appropriate life scope for your @c MyAppMidiServer object. 

Once you have the ScorePlayer instance, playback is just loading the score to play (by invoking ScorePlayer::load_score() method) and invoking the appropriate methods, such as ScorePlayer::play() or ScorePlayer::stop() or ScorePlayer::pause();

The only tricky issue, when starting to learn how to use lomse, is that method ScorePlayer::load_score() requires a pointer to the score to play. How to do get this pointer depends on your application, but a simple way of doing it is by using the Document methods for traversing the document and accessing its components. One of these methods is Document::get_content_item() that takes as argument the index to the desired content item. For instance:


@code
    //get the score player you have set up for your application
    ScorePlayer* pPlayer  = myAppGlobals->get_score_player();

    //get the first score
    ImoScore* pScore = NULL;
    int i = 0;
    while (true)
    {
        ImoObj* pObj = pDocument->get_content_item(i);
        if (pObj == NULL)   //end of document!
            break;
        if (pObj->is_score())
        {
            pScore = static_cast<ImoScore*>(pObj);
            break;
        }
        ++i;
    }
    
    //load the score and start playback
    if (pScore)
    {
        pPlayer->load_score(pScore, NULL);

        //settings for playback. Probably you would get settings from GUI controls
        bool fVisualTracking = true;    //generate visual tracking effects
        bool fCountOff = false          //no count off before start play
        long nMM = 60;                  //beats per minute
        Interactor* pInteractor = ...   //get the interactor for this document

        //start playback
        pPlayer->play(fVisualTracking, fCountOff, k_play_normal_instrument, nMM, pInteractor);
    }

@endcode

@todo Check example code: pPlayer->play(fVisualTracking, fCountOff, k_play_normal_instrument, nMM, pInteractor);



@section handling-highlight-events Handling highlight events

Apart from generating sound events, lomse also generates @a highlight events, that is, events to add visual tracking effects, synchronized with sound, on the displayed score.

For generating visual effects you have two options, either do it yourself by modifying the lomse graphical model as desired or, simpler, delegate in lomse for generating standard visual effects. Lomse offers two type of visual effects:

- Coloring notes as they are being played. This is currently fully operational.
- Displaying a vertical colored tempo line across the system, positioned at current beat. This is not yet finished and, therefore, is not yet available.

@a Highlight events are sent to your application via the event handling callback, that you set up at Lomse initialization. When handling a @a highlight event, if your application would like to delegate in Lomse for visual effects generation, the only thing to do is to pass the event to the interactor:

@code
    pInteractor->handle_event(pEvent);
@endcode

Lomse will handle the event and will send an <i>update window</i> event to your application, for updating the display.

@todo Advanced topic: direct modification of the graphic model.


@section implementing-player-gui The PlayerGui object

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

If you look at example code in section @ref how-to-play-score (relevant lines duplicated here):

@code
    //load the score and start playback
    if (pScore)
    {
        pPlayer->load_score(pScore, NULL);
@endcode

the second parameter for load_score is NULL, meaning that no PlayerGui is used. As you can deduce, the way of informing lomse of the GUI proxy to use is by passing a pointer to the PlayerGui in this method:

@code
    //get your PlayerGui object
    MyPlayerGui* pGui = ... 

    //load the score and link playback options to your MyPlayerGui object
    pPlayer->load_score(pScore, pGui);

    //setting tempo to 0 forces lomse to use the tempo returned by your MyPlayerGui object.
    //a value different from 0 forces lomse to use that tempo
    pPlayer->play(fVisualTracking, 0, pInteractor);
@endcode



*/

