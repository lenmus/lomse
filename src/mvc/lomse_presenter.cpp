//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-2012 Cecilio Salmeron. All rights reserved.
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
// Helper class to determine file format
//=======================================================================================
class FileFormatFinder
{
public:
    FileFormatFinder() {}
    ~FileFormatFinder() {}

    static int determine_format(const string& fullpath)
    {
        size_t length = fullpath.size();
        if (length < 5)
            return Document::k_format_unknown;

        string ext = fullpath.substr(length - 4);
        if (ext == ".lms")
            return Document::k_format_ldp;
        else if (ext == ".lmd")
            return Document::k_format_lmd;
        else
            return Document::k_format_unknown;
    }

};


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
        throw std::runtime_error("[PresentersCollection::get_presenter] invalid index");
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
    throw std::runtime_error("[PresentersCollection::get_presenter] invalid pointer");
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
        throw std::runtime_error("[PresentersCollection::close_document] invalid index");
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
Presenter* PresenterBuilder::new_document(int viewType, const std::string& content,
                                          ostream& reporter)
{
    Document* pDoc = Injector::inject_Document(m_libScope, reporter);
    if (content != "")
        pDoc->from_string(content);
    else
        pDoc->create_empty();

    return Injector::inject_Presenter(m_libScope, viewType, pDoc);
}

//---------------------------------------------------------------------------------------
Presenter* PresenterBuilder::open_document(int viewType, const std::string& filename,
                                           ostream& reporter)
{
    Document* pDoc = Injector::inject_Document(m_libScope, reporter);
    int format = FileFormatFinder::determine_format(filename);
    pDoc->from_file(filename, format);

    return Injector::inject_Presenter(m_libScope, viewType, pDoc);
}

//---------------------------------------------------------------------------------------
Presenter* PresenterBuilder::open_document(int viewType, LdpReader& reader,
                                           ostream& reporter)
{
    Document* pDoc = Injector::inject_Document(m_libScope, reporter);
    pDoc->from_input(reader);

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
    m_pDoc->add_event_handler(k_doc_modified_event, pIntor);
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
        throw std::runtime_error("[Presenter::get_interactor] invalid index");
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
