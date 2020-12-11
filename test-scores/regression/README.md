# Visual regression tests

Lomse has an extensive set of unit tests to make sure each core component of Lomse works properly. But they do not check that Lomse code, as a unit, works properly. So, Lomse also includes a visual regression test system for ensuring that Lomse works properly, that visual appearance of rendered scores doesn't change in unexpected ways from one Lomse build to the next one and, in general, to improve the quality assurance (QA) of the Lomse library. 

The goal of this system is to detect regressions in the rendered output without having to rely on human visual review. The tests go as follows:

* First, Lomse loads a test score and renders it. The result is saved as a JPG image in folder `lomse/test-scores/regression/generated`.
* Then the generated image is compared with the expected blessed image that is in folder `lomse/test-scores/regression/target`.
* If there are differences, the test script generates a GIF image, which flips between the two images, so that any differences are easily spotted. The GIF image is placed in folder `lomse/test-scores/regression/failures`. 
* Finally, an html page is automatically created with the results, in `lomse/test-scores/regression/results.html`.


## Running the visual tests

### Prerequisites

For running the tests it is necessary to have installed the [ImageMagick](http://www.imagemagick.org/) program. It is used to compare the generated image with the expected target image and, if differences found, to generate a GIF image to facilitate visual comparison of differences. To install ImageMagick program:
* Linux, use the package manager (e.g. Ubuntu): `$ sudo apt-get install imagemagick`
* macOS, with HomeBrew: `$ brew install imagemagick`.
* Windows, I don't know how to do it. I don't use Windows. Feel free to fix this instructions and send a PR.

The test system also uses a small provided program (`lclt`) to open a test file, render it with Lomse and save the resulting images as JPG files. The `lclt` program is re-built as part of the visual regression test process, to ensure that the latest Lomse version is used. You only need to download the sources (they are rarely updated). It is **required** that lclt root folder be placed in the same folder that Lomse root folder, e.g.:

```
   my-projects
       │
       ├── lomse
       │     ├── src
       │     ├── scripts
       │     ┆
       │
       ├── lclt
       │     ├── src
       │     ┆
       ┆
```

To download the lclt sources just clone the lclt repo:
```
$ cd path-to-my-projects-folder
$ git clone https://github.com/lenmus/lclt.git
```


## How to run the visual regression tests

> :warning: **Warning** I have only used and tested this in Linux, but it should work without problems in macOS. As to Windows, I don't use Windows and never tried this. The provided scripts are Linux bash scripts, so you will need to port the scripts or you could try the Linux facilities available in Windows 10. Feel free to fix this instructions and send a PR.


After you install ImageMagick and the lclt sources, you can run the test at any moment by doing:

```
$ cd path-to-my-projects-folder
$ cd lomse/scripts
$ ./build-lomse.sh      #builds Lomse and runs the unit tests
$ ./install-lomse.sh    #install the new Lomse build. Asks for the root password to install lomse
$ ./build-lclt.sh       #builds the test program and links it with the new version of Lomse. Install it.
$ ./regression.sh       #runs the visual regression tests
```

The `regression.sh` script will generate a lot of error messages. **It is normal!**

Now, open the results page, in `lomse/test-scores/regression/regression.html`, with your favorite browser. And visually inspect the failures (there are links at page top). If you're happy with the new output you should copy the new blessed image from `lomse/test-scores/regression/generated` to `lomse/test-scores/regression/target`.


