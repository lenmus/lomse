/**
@page page-examples Tutorials and samples

In the [Lomse repository](https://github.com/lenmus/lomse), folder [examples](https://github.com/lenmus/lomse/tree/master/examples) contains tutorials and sample code for using Lomse.


@section page-examples-tutorials Tutorials

Tutorials are functional apps. that should be studied first. Apart from the source code each tutorial include a *README.md* file with a detailed step-by-step explanation of how the application is designed and coded, and about how Lomse works. It also includes instructions for building.

The name of the tutorials has a suffix for marking the environment/platform used by that code. The meaning of the suffix is as follows:
- wx = [wxWidgets framework](https://www.wxwidgets.org/) (platform independent), e.g.: *tutorial-1-wx*.
- qt = [Qt framework](https://www.qt.io/) (platform independent), e.g.: *tutorial-1-qt*.
- x11 = For Linux with X11, e.g.: *tutorial-1-x11*.
- win = For Microsoft Windows, e.g.: *tutorial-1-win*.

This is a list of available tutorials:

<dl>
	<dt>tutorial-1-xxx</dt>
	<dd>Displaying a music score.</dd>
	<dt>tutorial-2-xxx</dt>
	<dd>Interacting with the score.</dd>
	<dt>tutorial-3-xxx</dt>
	<dd>Score playback.</dd>
	<dt>tutorial-4-xxx</dt>
	<dd>Visual tracking during playback.</dd>
	<dt>wxMidi</dt>
	<dd>This folder contains the sources for the wxMidi package. It is used in some tutorials and samples, for MIDI playback.</dd>
</dl>


@section page-examples-samples Samples

Samples are just minimal functional apps for highlighting how to do or to use some functionality, and they do not include normally any explanatory document, just a *README.md* file explaining the purpose of the sample. I created these samples for testing some functionality and for illustration purposes.

This is a list of available samples:

<dl>
    <dt>drawlines</dt>
    <dd>Drawing marker lines on the score and removing them.</dd>
    <dt>extplayer</dt>
    <dd>Controlling scroll and the tempo line when using an external player.</dd>
    <dt>transpose</dt>
    <dd>An example for testing transposition commands.</dd>
    <dt>viewtypes</dt>
    <dd>A test for displaying and playing back scores using the different Lomse View types.</dd>
</dl>

*/
