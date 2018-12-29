# Lomse tutorials and sample code

This folder contains tutorials and sample code for using Lomse.

**Tutorials** should be studied first. They are functional apps. Appart from the source code each tutorial include a document with a detailed step-by-step explanation of how the application is designed and coded and about how Lomse works.

**Samples**, on the contrary, are just minimal functional apps for highlighting how to do or to use some functionality, and they do not include any explanatory document, just a README.md file.

The name of the tutorials and sample code has a suffix for marking the environment/platform used by that code. The meaning of the suffix is as follows:
* wx = wxWidgets (platform independent)
* qt = Qt (platform independent)
* x11 = X11 (Linux)
* win = MS Windows

Folder `wxMidi` contain the sources for wxMidi package that is used in some tutorials and samples for MIDI playback.

This is a summary of available tutorials and sample code:
```
examples
  |
  + tutorials
  |    |
  |    +-- tutorial-1. Displaying a music score. 
  |    +-- tutorial-2. Interacting with the score. 
  |    +-- tutorial-3. Score playback.
  |    +-- tutorial-4. Visual tracking during playback. 	
  |
  + samples
       |
       +-- extplayer - controlling scroll and the tempo line when using an external player.
       +-- transpose - an example for testing transposition commands.
       +-- viewtypes - a test for displaying and playing back scores using the different Lomse View types.
```


