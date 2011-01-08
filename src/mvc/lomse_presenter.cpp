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

#include "lomse_presenter.h"

#include "lomse_injectors.h"
#include "lomse_document.h"
//#include "lomse_user_command.h"
#include "lomse_view.h"
#include "lomse_interactor.h"
#include <sstream>
using namespace std;

namespace lomse
{

//=======================================================================================
//PresentersCollection implementation
//=======================================================================================
PresentersCollection::PresentersCollection()
{
}

//---------------------------------------------------------------------------------------
PresentersCollection::~PresentersCollection()
{
    std::list<Presenter*>::iterator it;
    for (it=m_presenters.begin(); it != m_presenters.end(); ++it)
    {
        delete *it;
    }
    m_presenters.clear();
}

//---------------------------------------------------------------------------------------
Document* PresentersCollection::get_document(int iDoc)
{
    Presenter* pPresenter = get_presenter(iDoc);
    return pPresenter->get_document();
}

////---------------------------------------------------------------------------------------
//UserCommandExecuter* PresentersCollection::get_command_executer(int iDoc)
//{
//    Presenter* pPresenter = get_presenter(iDoc);
//    return pPresenter->get_command_executer();
//}

//---------------------------------------------------------------------------------------
Presenter* PresentersCollection::get_presenter(int iDoc)
{
    std::list<Presenter*>::iterator it;
    int i = 0;
    for (it=m_presenters.begin(); it != m_presenters.end() && i != iDoc; ++it, ++i);
    if (i == iDoc)
        return *it;
    else
        throw "invalid index";
}

//---------------------------------------------------------------------------------------
Presenter* PresentersCollection::get_presenter(Document* pDoc)
{
    std::list<Presenter*>::iterator it;
    for (it=m_presenters.begin(); it != m_presenters.end(); ++it)
    {
        if (pDoc == (*it)->get_document())
            return *it;
    }
    throw "invalid pointer";
}

//---------------------------------------------------------------------------------------
void PresentersCollection::add(Presenter* pPresenter)
{
    m_presenters.push_back(pPresenter);
}

//---------------------------------------------------------------------------------------
void PresentersCollection::close_document(int iDoc)
{
    std::list<Presenter*>::iterator it;
    int i = 0;
    for (it=m_presenters.begin(); it != m_presenters.end() && i != iDoc; ++it, ++i);
    if (iDoc == i)
    {
        delete *it;
        m_presenters.erase(it);
    }
    else
        throw "invalid index";
}

//---------------------------------------------------------------------------------------
void PresentersCollection::close_document(Document* pDoc)
{
    std::list<Presenter*>::iterator it;
    for (it=m_presenters.begin(); it != m_presenters.end(); ++it)
    {
        if (pDoc == (*it)->get_document())
        {
            delete *it;
            m_presenters.erase(it);
            break;
        }
    }
}

////---------------------------------------------------------------------------------------
//UserCommandExecuter* PresentersCollection::get_command_executer(Document* pDoc)
//{
//    Presenter* pPresenter = get_presenter(pDoc);
//    //View* pView
//    return pPresenter->get_command_executer();
//}

//---------------------------------------------------------------------------------------
void PresentersCollection::add_interactor(Document* pDoc, Interactor* pIntor)
{
    //Presenter* pPresenter = get_presenter(pDoc);
    //pPresenter->add_view(pView);
}

//---------------------------------------------------------------------------------------
int PresentersCollection::get_num_views(Document* pDoc)
{
    Presenter* pPresenter = get_presenter(pDoc);
    return pPresenter->get_num_interactors();
}

//---------------------------------------------------------------------------------------
void PresentersCollection::on_document_reloaded(Document* pDoc)
{
    Presenter* pPresenter = get_presenter(pDoc);
    pPresenter->on_document_reloaded();
}


//=======================================================================================
//PresenterBuilder implementation
//=======================================================================================
PresenterBuilder::PresenterBuilder(LibraryScope& libraryScope)
    : m_libScope(libraryScope)
{
}

//---------------------------------------------------------------------------------------
PresenterBuilder::~PresenterBuilder()
{
}

//---------------------------------------------------------------------------------------
Presenter* PresenterBuilder::new_document(int viewType, const std::string& content)
{
    Document* pDoc = Injector::inject_Document(m_libScope);
    if (content != "")
        pDoc->from_string(content);
    else
        pDoc->create_empty();

    return Injector::inject_Presenter(m_libScope, viewType, pDoc);
}

//---------------------------------------------------------------------------------------
Presenter* PresenterBuilder::open_document(int viewType, const std::string& filename)
{
    Document* pDoc = Injector::inject_Document(m_libScope);
    pDoc->from_file(filename);

    return Injector::inject_Presenter(m_libScope, viewType, pDoc);
}


//=======================================================================================
//Presenter implementation
//=======================================================================================
Presenter::Presenter(Document* pDoc, Interactor* pIntor)   //, UserCommandExecuter* pExec)
    : m_pDoc(pDoc)
    , m_userData(NULL)
    //, m_pExec(pExec)
    , m_callback(NULL)
{
    m_interactors.push_back(pIntor);
    m_pDoc->add_observer(pIntor);
    //pIntor->set_owner(this);
}

//---------------------------------------------------------------------------------------
Presenter::~Presenter()
{
    std::list<Interactor*>::iterator it;
    for (it=m_interactors.begin(); it != m_interactors.end(); ++it)
        delete *it;
    m_interactors.clear();

    delete m_pDoc;
    //delete m_pExec;
}

//---------------------------------------------------------------------------------------
void Presenter::close_document()
{
}

//---------------------------------------------------------------------------------------
Interactor* Presenter::get_interactor(int iIntor)
{
    std::list<Interactor*>::iterator it;
    int i = 0;
    for (it=m_interactors.begin(); it != m_interactors.end()&& i != iIntor; ++it, ++i);
    if (i == iIntor)
        return *it;
    else
        throw "invalid index";
}

//---------------------------------------------------------------------------------------
void Presenter::on_document_reloaded()
{
    std::list<Interactor*>::iterator it;
    for (it=m_interactors.begin(); it != m_interactors.end(); ++it)
        (*it)->on_document_reloaded();
}

//---------------------------------------------------------------------------------------
void Presenter::notify_user_application(Notification* event)
{
    if (m_callback)
        m_callback(event);
}

//---------------------------------------------------------------------------------------
void Presenter::set_callback( void (*pt2Func)(Notification* event) )
{
    m_callback = pt2Func;
}


}  //namespace lomse
