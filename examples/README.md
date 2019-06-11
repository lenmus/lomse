# Lomse tutorials and sample code

This folder contains tutorials and sample code for using Lomse.

**Tutorials** should be studied first. They are functional apps. Apart from the source code each tutorial include a README.md file with a detailed step-by-step explanation of how the application is designed and coded, and about how Lomse works. It also includes instructions for building.

**Samples**, on the contrary, are just minimal functional apps for highlighting how to do or to use some functionality, and they do not include normally any explanatory document, just a README.md file explaining the purpose of the sample. I created these samples for testing some functionality.

The name of the tutorials and sample code has a suffix for marking the environment/platform used by that code. The meaning of the suffix is as follows:
- wx = [wxWidgets framework](https://www.wxwidgets.org/) (platform independent), e.g.: *tutorial-1-wx*.
- qt = [Qt framework](https://www.qt.io/) (platform independent), e.g.: *tutorial-1-qt*.
- x11 = For Linux with X11, e.g.: *tutorial-1-x11*.
- win = For Microsoft Windows, e.g.: *tutorial-1-win*.

Folder *wxMidi* contains the sources for the wxMidi package. It is used in some tutorials and samples, for MIDI playback.

This is a summary of available tutorials and sample code:
```
examples
  |
  + images			  Just some images used in the tutorials explanations.
  |
  + tutorials
  |    |
  |    +-- tutorial-1-xxx   Displaying a music score. 
  |    +-- tutorial-2-xxx   Interacting with the score. 
  |    +-- tutorial-3-xxx   Score playback.
  |    +-- tutorial-4-xxx   Visual tracking during playback.
  |
  + samples
       |
       +-- extplayer   Controlling scroll and the tempo line when using an external player.
       +-- transpose   An example for testing transposition commands.
       +-- viewtypes   A test for displaying and playing back scores using the different Lomse View types.
```


