//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2018. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_PRESENTER_H__
#define __LOMSE_PRESENTER_H__

#include <list>
#include <iostream>

#include "lomse_document.h"

using namespace std;

///@cond INTERNALS
namespace lomse
{
///@endcond

//forward declarations
class DocCommandExecuter;
class Notification;
class Presenter;
class View;
class LibraryScope;

class Document;
typedef std::shared_ptr<Document>     SpDocument;
typedef std::weak_ptr<Document>       WpDocument;

class Interactor;
/** A shared pointer for an Interactor.
    @ingroup typedefs
    @#include <lomse_presenter.h>
*/
typedef std::shared_ptr<Interactor>   SpInteractor;

/** A weak pointer for an Interactor.
    @ingroup typedefs
    @#include <lomse_presenter.h>
*/
typedef std::weak_ptr<Interactor>     WpInteractor;


//---------------------------------------------------------------------------------------
///@cond INTERNALS
//excluded from public API. Only for internal use.
// PresenterBuilder: responsible for creating Presenter objects
class PresenterBuilder
{
protected:
    LibraryScope&   m_libScope;

public:
    PresenterBuilder(LibraryScope& libraryScope);
    virtual ~PresenterBuilder();

    //presenter creation
    Presenter* new_document(int viewType, const std::string& content="",
                            ostream& reporter = cout,
                            int format=Document::k_format_lmd);
    Presenter* open_document(int viewType, const std::string& filename,
                             ostream& reporter = cout);
    Presenter* open_document(int viewType, LdpReader& reader,
                             ostream& reporter = cout);

};
///@endcond


//---------------------------------------------------------------------------------------
/** The %Presenter is a facade object responsible for maintaining the life cycle and
    relationships between the objects in the Lomse MVC model: Views, Interactors and the
    Document.

    All these objects are created when invoking one of the LomseDoorway::new_document()
    or LomseDoorway::open_document() methods.

    The %Presenter, returned by previous methods, gives acess to the Document and to
    the Interactors associated to the existing View objects for the Document.

    See @subpage page-render-overview.
*/
class Presenter
{
protected:
    SpDocument m_spDoc;
    std::list<SpInteractor> m_interactors;
    void* m_userData;
    DocCommandExecuter* m_pExec;
    void (*m_callback)(Notification* event);

public:
	/** Destructor
		@attention As %Presenter ownership is transferred to the user application,
		you have to take care of deleting the %Presenter when no longer needed. Deleting
		the %Presenter will automatically cause deletion of all MVC involved objects:
		the Document, all existing Views and their Interactors, selection sets,
		undo/redo stack, etc.
	*/
     virtual ~Presenter();

    //interactors management
	/** Returns the number of Interactors associated to the %Presenter, at least 1.
        There exist as many Interactors as document Views.    */
    inline int get_num_interactors() { return static_cast<int>( m_interactors.size() ); }

    /** Returns a shared pointer to the specified Interactor index.
        @param iIntor is the number of the desired Interactor. It must be 0 for the
            first interactor, and it must be lower than the value returned by method
            get_num_interactors(); otherwise an exception will be thrown.
    */
    SpInteractor get_interactor_shared_ptr(int iIntor);

    /** Returns a weak pointer to the specified Interactor index.
        @param iIntor is the number of the desired Interactor. It must be 0 for the
            first interactor, and it must be lower than the value returned by method
            get_num_interactors(); otherwise an exception will be thrown.
    */
    WpInteractor get_interactor(int iIntor);


    //accessors
    /** Returns a shared pointer to the Document associated to this %Presenter.    */
    inline SpDocument get_document_shared_ptr() { return m_spDoc; }

    /** Returns a shared pointer to the Document associated to this %Presenter.    */
    WpDocument get_document_weak_ptr();

    /** Returns the raw pointer to the Document associated to this %Presenter.    */
    inline Document* get_document_raw_ptr() { return m_spDoc.get(); }


    //to save user data
    /** Associates the given untyped application data pointer with this %Presenter.
        @param pData	The application data to associate with the %Presenter.

        This method is a commodity for your application, in case you would like to save
        some data associated to %Presenter objects, i.e. for Document identification
        or other.

        @attention Your application has the ownership of the associated data.
            Therefore, this data will not be deleted when the %Presenter is deleted.
    */
    inline void set_user_data(void* pData) { m_userData = pData; }

    /** Returns a pointer to the user data associated with this %Presenter (if any).
        @return
            A pointer to the user data, or NULL if no data saved.
    */
    inline void* get_user_data() { return m_userData; }

    ///@cond INTERNALS
    //excluded from public API. Only for internal use.
    Presenter(SpDocument spDoc, Interactor* pIntor, DocCommandExecuter* pExec);

    Interactor* get_interactor_raw_ptr(int iIntor);
    inline DocCommandExecuter* get_command_executer() { return m_pExec; }

    void on_document_updated();

    //to sent notifications to user application
    void set_callback( void (*pt2Func)(Notification* event) );
    void notify_user_application(Notification* event);

    ///@endcond

};

//---------------------------------------------------------------------------------------
///@cond INTERNALS
//excluded from public API. Only for internal use.
class Notification
{
protected:
    Presenter* m_pPresenter;
    Document*   m_pDoc;
    View*       m_pView;

public:
    Notification()
        : m_pPresenter(nullptr), m_pDoc(nullptr), m_pView(nullptr)
    {
    }

    Notification(Presenter* pPresenter, Document* pDoc, View* pView)
        : m_pPresenter(pPresenter), m_pDoc(pDoc), m_pView(pView)
    {
    }

    //getters and setters
    inline View* get_view() { return m_pView; }
    inline Document* get_document() { return m_pDoc; }
    inline Presenter* get_presenter() { return m_pPresenter; }
    inline void set_view(View* pView) { m_pView = pView; }
    inline void set_document(Document* pDoc) { m_pDoc = pDoc; }
    inline void set_presenter(Presenter* pPresenter) { m_pPresenter = pPresenter; }

};
///@endcond


}   //namespace lomse

#endif      //__LOMSE_PRESENTER_H__
