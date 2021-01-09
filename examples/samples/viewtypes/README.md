# Sample: viewtypes

This sample is a test for displaying and playing back scores using the different Lomse View types.


## Building in Linux for wxWidgets

```
g++ -std=c++11 viewtypes-wx.cpp ../../tutorials/wxMidi/wxMidi.cpp ../../tutorials/wxMidi/wxMidiDatabase.cpp -o viewtypes `pkg-config --cflags liblomse` `wx-config --cflags` -I ../../tutorials/wxMidi/ `pkg-config --libs liblomse` `wx-config --libs` -lstdc++ -lportmidi -lporttime
```

