/**
@page viewport-overview Coordinate systems, units, scaling and viewport

@tableofcontents


@section lomse-units Device and logical units

In Lomse there is a distinction between @e logical units and @e device units.

All document dimensions are expressed in real world units. In particular, Lomse uses as unit one cent of a millimeter. Real world units are usually named @b logical units. Lomse and uses type @a LUnits for variables expressed cents of a millimeter.

But as the document is going to be displayed on a screen (or other device) it is necesary to transform real world dimensions into device dimensions. @b Device units are the units native to a particular device; e.g. for a screen,
a device unit is a @e pixel. For a printer, the device unit is defined by the
resolution of the printer (usually given in @c DPI: dots-per-inch).


@section scale-factor Scaling

Logical units are mapped to device units (pixels) using the current rendering resolution. For screen oriented method, this resolution is set at Lomse initialization, in method LomseDoorway::init_library(). For printing, the resolution is set in method Interactor::set_print_ppi(). Nevertheless, this mapping can be modified by using an scale factor. 

See: Interactor::get_scale(), Interactor::set_scale(), Interactor::zoom_in(), Interactor::zoom_out(), Interactor::zoom_fit_full(),
Interactor::zoom_fit_width().


@section viewport-concept Scrolling: the Viewport

Lomse maintains a virtual renderization of the full document at real size (e.g. millimeters). When requesting Lomse to render the document onto your application window it is not expected that Lomse will squeeze all the document pages into that window, but just the specific part of the document that the user wants to visualize, as it is expected that the users can pan and zoom to see different areas of the document.

In computer graphics theory, this part of the document that the user wants to visualize is named the <i>world window</i> (not be confused with your application window or <i>GUI window</i>) and its dimensions and position are expresed in real world units (Lomse uses cents of one millimeter).

But as the document is going to be displayed on a screen (or other device) it is necesary to transform real world dimensions into device dimensions. Once the document dimensions are transformed into device units (pixels) the equivalent to the <i>world window</i> is named the <i>viewport</i>. That is, the <i>viewport</i> is the visible portion of the document, but in device units (pixels). If the document is larger than the viewport, the user can shift the viewport around by changing the viewport origin (this is the way for implementing scrolling).

See: Interactor::new_viewport(), Interactor::set_viewport_at_page_center(), Interactor::get_viewport(), Interactor::get_view_size().

*/
