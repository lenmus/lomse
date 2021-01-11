Sources for Lomse API documentation
========================================

This folder contains source and auxiliary files for building the Lomse
API documentation.

Current published version of the API documentation is available
at https://lenmus.github.io/lomse/


Directory content
---------------------

Lomse uses Doxygen to process header input files with embedded
documentation in the form of C++ comments. Doxygen will create
documentation in HTML (Doxygen itself can also output Latex,
manpages, RTF, PDF etc).
See http://www.doxygen.org for more info about Doxygen.

All relevant documentation for the Lomse API is in
the headers of Lomse library. But not all header files are included in the
documentation, only those explicitly referenced in Doxygen configuration
file, at `Lomse/docs/api/doxyfile`, in the **`INPUT`** configuration option.
An example follows:
```
INPUT     = mainpages \
			../../include/lomse_command.h \
			../../include/lomse_doorway.h \
			../../include/lomse_events.h \
			../../include/lomse_im_attributes.h \
			../../include/lomse_import_options.h \
			../../include/lomse_interactor.h \
			../../include/lomse_internal_model.h \
			../../include/lomse_pitch.h \
			../../include/lomse_pixel_formats.h \
			../../include/lomse_presenter.h \
			examples \
	        groups
```

Apart from documentation in header files, there are specific topic
documents maintained in folder `Lomse/docs/api/mainpages/`.

All images for the API documentation are in folder
`Lomse/docs/api/images/`.


Building the documentation
----------------------------

First, make sure you have a the required version of Doxygen installed in
your system (currently Doxygen 1.8.13 is used). Different versions of 
Doxygen are practically never compatible and using a later version could require
adapting the configuration file.

For building the API documentation you can run the script:
	 `Lomse/scripts/build-api-docs.sh`

The output of Doxygen will be generated in a
folder, named `api-docs`, at the same level than the Lomse root folder.
This folder will be created by Doxygen if does not exist. The html files
will be in folder `api-docs/html` and the root of the documentation is
in file `api-docs/html/index.html`.


