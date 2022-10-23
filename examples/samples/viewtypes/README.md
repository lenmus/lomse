# Sample: viewtypes

This sample is a test for displaying and playing back scores using the different Lomse View types.


## Building in Linux for wxWidgets

```
g++ -std=c++11 viewtypes-wx.cpp ../../tutorials/wxMidi/wxMidi.cpp ../../tutorials/wxMidi/wxMidiDatabase.cpp -o viewtypes `pkg-config --cflags liblomse` `wx-config --cflags` -I ../../tutorials/wxMidi/ `pkg-config --libs liblomse` `wx-config --libs` -lstdc++ -lportmidi -lporttime
```

## Running

```
.\viewtypes
```

Please note that when selecting a new view type the window is not updated automatically. It is necessary to re-open the document, as the `Presenter` is tied to the old view type and a new view must be created.

