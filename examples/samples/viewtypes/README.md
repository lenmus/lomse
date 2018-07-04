# Sample: viewtypes

This sample is a test for displaying and playing back scores using the different Lomse View types.


## Building for wxWidgets

```
g++ -std=c++11 viewtype-wx.cpp ./wxMidi/wxMidi.cpp ./wxMidi/wxMidiDatabase.cpp -o example-4-wx `pkg-config --cflags liblomse` `wx-config --cflags` -I ./wxMidi/ `pkg-config --libs liblomse` `wx-config --libs` -lstdc++ -lportmidi -lporttime
```

