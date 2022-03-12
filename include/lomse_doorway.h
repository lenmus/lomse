//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_DOORWAY_H__
#define __LOMSE_DOORWAY_H__

#include "lomse_build_options.h"
#include "lomse_basic.h"
#include "lomse_pixel_formats.h"

#include <string>
#include <iostream>
#include <memory>   //shared_ptr

///@cond INTERNAL
namespace lomse
{
///@endcond

//---------------------------------------------------------------------------------------
//forward declarations
class LibraryScope;
class View;
class VerticalBookView;
class HorizontalBookView;
class SimpleView;
class SingleSystemView;
class Document;
class Presenter;
class ScorePlayer;
class MidiServerBase;
class Metronome;
class MusicXmlOptions;
class BitmapDrawer;
class EventInfo;
typedef std::shared_ptr<EventInfo>  SpEventInfo;
class Request;
class LdpReader;
class Drawer;


typedef void (*pt2NotifyFunction)(void*, SpEventInfo);
typedef void (*pt2RequestFunction)(void*, Request*);

//---------------------------------------------------------------------------------------
/** %LomseDoorway is the main interface with the Lomse library. This class represent
    the gate between Lomse and your application:
    - For your application it is entry point for interacting with the library: the way
        for configuring Lomse, for accessing Lomse version information and for
        creating/opening documents and scores, which in turn provides accessors for
        managing the open documents.
    - For the Lomse library, it is the way to access platform dependent information and
        methods, by using callbacks initialized by the user during Lomse configuration.
*/
class LomseDoorway
{
protected:
    LibraryScope*       m_pLibraryScope;
    struct
    {
        EPixelFormat    pixel_format;   //see enum EPixelFormat
        bool            flip_y;         //true if you want to have the Y-axis flipped vertically
        double          screen_ppi;     //screen resolution in pixels per inch
    } m_platform;
    pt2NotifyFunction   m_pFunc_notify;
    pt2RequestFunction  m_pFunc_request;
    void*               m_pObj_notify;
    void*               m_pObj_request;

public:
	/** Constructor. Your application should create only one instance of LomseDoorway
        and its life scope should be as long as your application need to use Lomse.  */
    explicit LomseDoorway(std::ostream* logStream = nullptr,
                          std::ostream* forensicLogStream = nullptr);
    virtual ~LomseDoorway();

    /** @name Library initialization and configuration  */
    //@{

	/** Before using the library, method init_library() **must** be invoked for
        initializing the library and providing the necessary information. As Lomse
        renders scores and documents on a bitmap it is necessary to inform Lomse about
        the required bitmap properties. Also, you can set an ostream to be used by
        Lomse for reporting errors.

        @param pixel_format A value from the global enumeration type #EPixelFormat
        @param ppi The display resolution in pixels per inch (e.g. 96). Lomse uses
            vectorial graphics for all, typography included and, thus, screen
            resolution is no required and your application can always scale the image
            to as much resolution as you like. The real resolution is determined
            by the provided bitmap size (pixels) to be used as rendering buffer.
            Nevertheless, Lomse requires a screen resolution value to adjust
            internal scaling factors so that when the user scale is set to
            1.0 (100%) the document get displayed on the screen at real size.
            If this is not a
            requirement for your application, any value can be used (e.g. 72 or 96).
        @param reporter The ostream to be used by Lomse for reporting errors. By default,
            all errors will be send to cout.

        For instance, assume we are writing an application for Linux to display scores
        on screen:

        @code
        LomseDoorway m_lomse;                   //the only instance, representing the lomse library
        int pixel_format = k_pix_format_rgb24;  //pixel format: RGB 24bits
        int resolution = 96;                    //typical resolution

        //initialize the library with these values
        m_lomse.init_library(pixel_format, resolution);
        @endcode
	*/
    void init_library(int pixel_format, int ppi, std::ostream& reporter=std::cout);

///@cond INTERNAL
    //deprecated Jan/2021
    LOMSE_DEPRECATED_MSG("use instead version without 'reverse_y_axis' param.")
    void init_library(int pixel_format, int ppi, bool reverse_y_axis,
                      std::ostream& reporter=std::cout);
///@endcond

	/** This method is used by for informing Lomse about the callback method that Lomse
        should invoke when having to communicate an event to your application.

        @param pThis    A pointer to the instance of object that will receive the
            notifications.
        @param pt2Func  A pointer to the method in previous object that Lomse will
            invoke when an event is generated in Lomse.

        See @subpage page-callbacks.

        For example, to prepare our application for handling visual tracking events
        during score playback we could define a callback method:

        @code
        class MyApp
        {
        public:
            //callback wrapper
            static void wrapper_for_lomse_event_handler(void* pThis, SpEventInfo pEvent)
            {
                static_cast<MyApp*>(pThis)->on_lomse_event(pEvent);
            }
            ...

        protected:
            void on_lomse_event(SpEventInfo pEvent);
            ...
        };
        @endcode

        And inform lomse about it. We do it at lomse initialization:

        @code
        void MyApp::initialize_lomse()
        {
            //initialize the library
            ...
            m_lomse.init_library(pixel_format, resolution, reverse_y_axis);

            //set callbacks
            m_lomse.set_notify_callback(this, wrapper_for_lomse_event_handler);

            ...
        }
        @endcode

	*/
    void set_notify_callback(void* pThis, void (*pt2Func)(void*, SpEventInfo));

	/** This method is used by for informing Lomse about the callback method that Lomse
        should invoke when having to request information to your application.
        See Request, RequestDynamic, RequestFont

        @param pThis    A pointer to the instance of object that will receive the
            notifications.
        @param pt2Func  A pointer to the method in previous object that Lomse will
            invoke when a requests is generated in Lomse.

        See @subpage page-callbacks
        for an explanation of Lomse callbacks and its parameters.

        For example:

        @code
        class MyApp
        {
        public:
            //requests wrapper
            static void wrapper_for_lomse_requests(void* pThis, Request* pRequests)
            {
                static_cast<MyApp*>(pThis)->on_lomse_request(pRequest);
            }
            ...

        protected:
            void on_lomse_request(Request* pRequest);
            ...
        };
        @endcode

        And inform lomse about it. We do it at lomse initialization:

        @code
        void MyApp::initialize_lomse()
        {
            //initialize the library
            ...
            m_lomse.init_library(pixel_format, resolution, reverse_y_axis);

            //set callbacks
            m_lomse.set_request_callback(this, wrapper_for_lomse_requests);

            ...
        }
        @endcode
	*/
    void set_request_callback(void* pThis, void (*pt2Func)(void*, Request*));

	/** Method set_default_fonts_path() is used for informing Lomse about the path in
        which additional fonts are installed. Default fonts used by Lomse are located
        in folders known by Lomse, but Lomse has no knowledge about where the additional
        fonts the user would like to use are located. They are probably in the folder
        used by the operating system for fonts. But as Lomse is platform independent,
        currently it has no knowledge about operating system folders for fonts.
        Therefore, applications using additional fonts other than Lomse default fonts
        should use this method for informing Lomse about the path for the fonts.

        @param fontsPath    Absolute path to the fonts folder in which Lomse will look
            for fonts other than Lomse default fonts. Please note that Lomse will not
            look into sub-folders in the passed path.
	*/
    void set_default_fonts_path(const std::string& fontsPath);

	/** Returns settings for rendering bitmap resolution. This is the value set
        when method init_library() was called.    */
    inline double get_screen_ppi() { return m_platform.screen_ppi; }

	/** Returns settings for rendering bitmap format. This is the value set
        when method init_library() was called. The meaning of the returned value is
        given by enum #EPixelFormat.    */
    inline int get_pixel_format() { return m_platform.pixel_format; }

    //@}    //Library initialization and configuration



     /** @name Document creation methods */
    //@{

    //common operations on documents
    /** Create a new empty document, as well as a View for rendering it and all
        additional components (Interactor, Presenter, etc.) necessary for managing this
        document and interacting with it. Optionally, default BitmapDrawer objects
        used by Lomse can be replaced by any Drawer objects created by your
        application.

        @param viewType     The view type that will be used for rendering this document.
        @param screenDrawer  The Drawer to use as main drawer. Ownership of this Drawer
            is transferred to the View. You must not delete it. If not specified or
            @nullptr a BitmapDrawer will be used.
        @param printDrawer   The Drawer to use for printing. Ownership of this Drawer
            is transferred to the View. You must not delete it. If not specified or
            @nullptr a BitmapDrawer will be used.

        @return A pointer to the Presenter to be used for interacting with this document.

        @attention As Presenter ownership is transferred to user application, you have
            to take care of deleting the Presenter when no longer needed. Deleting the
            Presenter will automatically cause deletion of all MVC involved objects:
            the Document, all existing Views and Interactors, etc., as well as any
            passed Drawer objects.

        @see @subpage page-render-overview, @subpage page-user-drawers
	*/
    Presenter* new_document(int viewType, Drawer* screenDrawer=nullptr,
                            Drawer* printDrawer=nullptr);


	/** Create a new document initialized with the passed content.
        Also creates a View for rendering it as well as all additional
        components (Interactor, Presenter, etc.) necessary for managing this document
        and interacting with it. Optionally, default BitmapDrawer objects
        used by Lomse can be replaced by any Drawer objects created by your
        application.

        @param viewType The view type that will be used for rendering this document.
        @param source A string with the content for the document to create.
        @param format A value specifying the format for the source content. Valid values
            are defined in Document class. Valid values are:
            - Document::k_format_ldp = 0, for LenMus documents in LDP syntax.
            - Document::k_format_lmd = 1, for LenMus documents in XML syntax (LMD format).
            - Document::k_format_mxl = 2, for MusicXML documents
        @param reporter The ostream to be used for reporting any errors. By default,
            all errors will be send to cout.
        @param screenDrawer  The Drawer to use as main drawer. Ownership of this Drawer
            is transferred to the View. You must not delete it. If not specified or
            @nullptr a BitmapDrawer will be used.
        @param printDrawer   The Drawer to use for printing. Ownership of this Drawer
            is transferred to the View. You must not delete it. If not specified or
            @nullptr a BitmapDrawer will be used.

        @return A pointer to the Presenter to be used for interacting with this document.

        @attention As Presenter ownership is transferred to user application, you have to
            take care of deleting the Presenter when no longer needed. Deleting the
            Presenter will automatically cause deletion of all MVC involved objects:
            the Document, all existing Views and Interactors, etc., as well as any
            passed Drawer objects.

        @see @subpage page-render-overview
	*/
    Presenter* new_document(int viewType, const std::string& source, int format,
                            std::ostream& reporter = std::cout,
                            Drawer* screenDrawer=nullptr, Drawer* printDrawer=nullptr);

	/** Open a document. Its content is read from a file.
        Also creates a View for rendering it as well as all additional
        components (Interactor, Presenter, etc.) necessary for managing this document and
        interacting with it. Optionally, default BitmapDrawer objects
        used by Lomse can be replaced by any Drawer objects created by your
        application. Optionally, default BitmapDrawer objects
        used by Lomse can be replaced by any Drawer objects created by your
        application.

        @param viewType The view type that will be used for rendering this document.
        @param filename A string with the absolute path to the file containing the document.
        @param reporter The ostream to be used for reporting any errors. By default, all
            errors will be send to cout.
        @param screenDrawer  The Drawer to use as main drawer. Ownership of this Drawer
            is transferred to the View. You must not delete it. If not specified or
            @nullptr a BitmapDrawer will be used.
        @param printDrawer   The Drawer to use for printing. Ownership of this Drawer
            is transferred to the View. You must not delete it. If not specified or
            @nullptr a BitmapDrawer will be used.

        @return A pointer to the Presenter to be used for interacting with this document.

        @attention As Presenter ownership is transferred to user application, you have to
            take care of deleting the Presenter when no longer needed. Deleting the
            Presenter will automatically cause deletion of all MVC involved objects:
            the Document, all existing Views and Interactors, etc., as well as any
            passed Drawer objects.

        @see @subpage page-render-overview
	*/
    Presenter* open_document(int viewType, const std::string& filename,
                             std::ostream& reporter = std::cout,
                             Drawer* screenDrawer=nullptr, Drawer* printDrawer=nullptr);

	/** Open a document. Its content is provided by an LdpReader object.
        Also creates a View for rendering it as well as all additional
        components (Interactor, Presenter, etc.) necessary for managing this document and
        interacting with it. Optionally, default BitmapDrawer objects
        used by Lomse can be replaced by any Drawer objects created by your
        application.

        @param viewType The view type that will be used for rendering this document.
        @param reader An object of class LdpReader that will be used for reading the
            content of the document.
        @param reporter The ostream to be used for reporting any errors. By default,
            all errors will be send to cout.
        @param screenDrawer  The Drawer to use as main drawer. Ownership of this Drawer
            is transferred to the View. You must not delete it. If not specified or
            @nullptr a BitmapDrawer will be used.
        @param printDrawer   The Drawer to use for printing. Ownership of this Drawer
            is transferred to the View. You must not delete it. If not specified or
            @nullptr a BitmapDrawer will be used.

        @return A pointer to the Presenter to be used for interacting with this document.

        @attention As Presenter ownership is transferred to user application, you have
            to take care of deleting the Presenter when no longer needed. Deleting the
            Presenter will automatically cause deletion of all MVC involved objects:
            the Document, all existing Views and Interactors, etc., as well as any
            passed Drawer objects.

        @see @subpage page-render-overview
	*/
    Presenter* open_document(int viewType, LdpReader& reader,
                             std::ostream& reporter = std::cout,
                             Drawer* screenDrawer=nullptr, Drawer* printDrawer=nullptr);

    //@}    //Document creation methods



     /** @name Playback related methods  */
    //@{

	/** This method is used for informing Lomse about the metronome control to use to
        determine tempo in playback.
        By default, Lomse takes metronome settings from ?. But your application
        can specify another metronome control, normally a global metronome for the all
        your application.

        @todo Clarify the purpose and usage of this method

        @param pMtr Pointer to the Metronome object to be used.
	*/
    void set_global_metronome_and_replace_local(Metronome* pMtr);

	/** Method create_score_player() informs Lomse about the MIDI server that will be
        used for scores play back.

        @param pSoundServer Pointer to an instance of a class derived from
            MidiServerBase. Once this method is invoked, Lomse will route all midi
            events originated in score play back to this object.

        @see @subpage page-sound-generation
	*/
    ScorePlayer* create_score_player(MidiServerBase* pSoundServer);

    //@}    //Playback related methods



     /** @name Library information  */
    //@{

	/** Returns Lomse version as string "major.minor.patch" e.g., "0.17.20"    */
    std::string get_version_string();

	/** Returns Lomse version and build information as string. e.g., "0.17.20+aaf5e23"  */
    std::string get_version_long_string();

	/** Returns the build date and time of the Lomse library. The string is twenty
        characters long and looks like "12-Feb-2016 17:54:03". Date and time are
        separated by one space. Date is in format dd-mm-yyyy and time in format hh:mm:ss.
        Day, hour, minutes and seconds are always padded with zero if only one digit
        e.g., "02-Mar-2016 07:54:03"    */
    std::string get_build_date();

	/** Returns Lomse major version as integer number. For instance, if version string
        is "0.17.20" this method will return 0.    */
    int get_version_major();

	/** Returns Lomse minor version as integer number. For instance, if version string
        is "0.17.20" this method will return 17.    */
    int get_version_minor();

	/** Returns Lomse patch version as integer number. For instance, if version string
        is "0.17.20" this method will return 20.    */
    int get_version_patch();

    //@}    //Library information


     /** @name Other methods  */
    //@{

	/** Returns the current options object with the settings to use for dealing and
        fixing errors and malformed
        MusicXML files. The import options can be changed multiple times, whenever
        it is needed. For example:

        @code
            LomseDoorway* pLomse = ...
            MusicXmlOptions* opt = pLomse->get_musicxml_options();
            opt->fix_beams(true);
            Presenter* pPresenter = pLomse->open_document(k_view_vertical_book,
                                                          "my_score.xml");
            ...
            opt->fix_beams(false);
            Presenter* pPresenter = pLomse->open_document(k_view_vertical_book,
                                                          "other_score.xml");

        @endcode

        @see class @ref MusicXmlOptions
	*/
    MusicXmlOptions* get_musicxml_options();

	/** Get the pointer to an object of class LibraryScope. This object gives access to
        all Lomse global functions and information.    */
    inline LibraryScope* get_library_scope() { return m_pLibraryScope; }

    //@}    //Other methods


///@cond INTERNAL       //excluded from public API. Only for internal use.

    //default callbacks
    static void null_notify_function(void* pObj, SpEventInfo event);
    static void null_request_function(void* pObj, Request* event);

    //communication with user application
    void post_event(SpEventInfo pEvent);
    void post_request(Request* pRequest);

///@endcond
};

}   //namespace lomse

#endif      //__LOMSE_DOORWAY_H__
