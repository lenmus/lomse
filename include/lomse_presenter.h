//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2013 Cecilio Salmeron. All rights reserved.
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

#include "lomse_reader.h"
#include "lomse_basic.h"
#include "lomse_document.h"

using namespace std;

namespace lomse
{

//forward declarations
class DocCommandExecuter;
class Notification;
class Presenter;
class View;
class LibraryScope;

class Document;
typedef SharedPtr<Document>     SpDocument;
typedef WeakPtr<Document>       WpDocument;

class Interactor;
typedef SharedPtr<Interactor>   SpInteractor;
typedef WeakPtr<Interactor>     WpInteractor;


//---------------------------------------------------------------------------------------
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


//---------------------------------------------------------------------------------------
//Presenter: A façade object responsible for maintaining the life cycle and
//relationships between MVC objects: Views, Interactors, Commands, Selections and the
//Document.
class Presenter
{
protected:
    SpDocument m_spDoc;
    std::list<SpInteractor> m_interactors;
    void* m_userData;
    DocCommandExecuter* m_pExec;
    void (*m_callback)(Notification* event);

public:
    Presenter(SpDocument spDoc, Interactor* pIntor, DocCommandExecuter* pExec);
    virtual ~Presenter();

    void close_document();
    void on_document_reloaded();

    //interactors management
    inline int get_num_interactors() { return static_cast<int>( m_interactors.size() ); }
    SpInteractor get_interactor_shared_ptr(int iIntor);
    Interactor* get_interactor_raw_ptr(int iIntor);
    WpInteractor get_interactor(int iIntor);

    //accessors
    inline Document* get_document_raw_ptr() { return m_spDoc.get(); }
    inline SpDocument get_document_shared_ptr() { return m_spDoc; }
    WpDocument get_document_weak_ptr();
    inline DocCommandExecuter* get_command_executer() { return m_pExec; }

    //to sent notifications to user application
    void set_callback( void (*pt2Func)(Notification* event) );
    void notify_user_application(Notification* event);

    //to save user data
    inline void set_user_data(void* pData) { m_userData = pData; }
    inline void* get_user_data() { return m_userData; }

};

//---------------------------------------------------------------------------------------
class Notification
{
protected:
    Presenter* m_pPresenter;
    Document*   m_pDoc;
    View*       m_pView;

public:
    Notification()
        : m_pPresenter(NULL), m_pDoc(NULL), m_pView(NULL)
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


}   //namespace lomse

#endif      //__LOMSE_PRESENTER_H__
