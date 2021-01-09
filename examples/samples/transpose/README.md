# Sample: transpose

This sample shows how to use edition commands for transposing an score.

When running the sample, use the mouse for selecting the desired notes or the whole score (drag a rectangle around the desired area), and then select the transposition options from main menu.


## Building in Linux for wxWidgets

```
g++ -std=c++11 transpose-wx.cpp -o transpose-wx `pkg-config --cflags liblomse` `wx-config --cflags` `pkg-config --libs liblomse` `wx-config --libs` -lstdc++
```

