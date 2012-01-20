//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_PRESENTER_H__
#define __LOMSE_PRESENTER_H__

#include <list>
#include <iostream>

#include "lomse_reader.h"

using namespace std;

namespace lomse
{

//forward declarations
class Document;
//class DocCommandExecuter;
//class UserCommandExecuter;
class Notification;
class Presenter;
class View;
class LibraryScope;
class Interactor;


//---------------------------------------------------------------------------------------
//PresentersCollection: Responsible for managing the collection of Presenter objects.
class PresentersCollection
{
protected:
    std::list<Presenter*> m_presenters;

public:

    PresentersCollection();
    ~PresentersCollection();

    //add elements
    void add(Presenter* pPresenter);

    //remove elements
    void close_document(int iDoc);
    void close_document(Document* pDoc);

    //get elements
    Presenter* get_presenter(int iDoc);
    Presenter* get_presenter(Document* pDoc);

    //other
    void add_interactor(Document* pDoc, Interactor* pIntor);
    void on_document_reloaded(Document* pDoc);

    //access to info
    //UserCommandExecuter* get_command_executer(Document* pDoc);
    int get_num_views(Document* pDoc);

    //for unit tests
    inline int get_num_documents() { return static_cast<int>(m_presenters.size()); }
    Document* get_document(int iDoc);
    //UserCommandExecuter* get_command_executer(int iDoc);

};


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
                            ostream& reporter = cout);
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
    Document* m_pDoc;
    std::list<Interactor*> m_interactors;
    void* m_userData;
    //UserCommandExecuter* m_pExec;
    void (*m_callback)(Notification* event);

public:
    Presenter(Document* pDoc, Interactor* pIntor);     //, UserCommandExecuter* pExec);
    virtual ~Presenter();

    void close_document();
    void on_document_reloaded();

    //interactors management
    inline int get_num_interactors() { return static_cast<int>( m_interactors.size() ); }
    Interactor* get_interactor(int iIntor);

    //accessors
    inline Document* get_document() { return m_pDoc; }
    //inline UserCommandExecuter* get_command_executer() { return m_pExec; }

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
