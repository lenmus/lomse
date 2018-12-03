# Sample: transpose

This sample shows how to use edition commands for transposing an score.


## Building for wxWidgets

```
g++ -std=c++11 transpose-wx.cpp -o transpose-wx `pkg-config --cflags liblomse` `wx-config --cflags` `pkg-config --libs liblomse` `wx-config --libs` -lstdc++
```

