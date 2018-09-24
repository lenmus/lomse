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

#define LOMSE_INTERNAL_API
#include "lomse_presenter.h"

#include "lomse_injectors.h"
#include "lomse_document.h"
#include "lomse_command.h"
#include "lomse_view.h"
#include "lomse_interactor.h"
#include "lomse_logger.h"

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

        size_t i = fullpath.rfind('.', length);
        if (i == string::npos)
            return Document::k_format_unknown;

        string ext = fullpath.substr(i+1, length - i);
        if (ext == "lms")
            return Document::k_format_ldp;
        else if (ext == "lmd")
            return Document::k_format_lmd;
        else if (ext == "xml" || ext == "musicxml")
            return Document::k_format_mxl;
        else if (ext == "mnx")
            return Document::k_format_mnx;
        else
            return Document::k_format_unknown;
    }

};


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
                                          ostream& reporter, int format)
{
    Document* pDoc = Injector::inject_Document(m_libScope, reporter);
    if (content != "")
        pDoc->from_string(content, format);
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
Presenter::Presenter(SpDocument spDoc, Interactor* pIntor, DocCommandExecuter* pExec)
    : m_spDoc(spDoc)
    , m_userData(nullptr)
    , m_pExec(pExec)
    , m_callback(nullptr)
{
    m_interactors.push_back( SpInteractor(pIntor) );
    m_spDoc->add_event_handler(k_doc_modified_event, pIntor);
}

//---------------------------------------------------------------------------------------
Presenter::~Presenter()
{
    m_interactors.clear();
    LOMSE_LOG_TRACE(Logger::k_mvc, "[Presenter::~Presenter] Presenter is deleted");
    delete m_pExec;
}

//---------------------------------------------------------------------------------------
SpInteractor Presenter::get_interactor_shared_ptr(int iIntor)
{
    std::list<SpInteractor>::iterator it;
    int i = 0;
    for (it=m_interactors.begin(); it != m_interactors.end() && i != iIntor; ++it, ++i);
    if (i == iIntor && it != m_interactors.end())
        return *it;
    else
    {
        LOMSE_LOG_ERROR("[Presenter::get_interactor] invalid index");
        throw runtime_error("[Presenter::get_interactor] invalid index");
    }
}

//---------------------------------------------------------------------------------------
Interactor* Presenter::get_interactor_raw_ptr(int iIntor)
{
    return get_interactor_shared_ptr(iIntor).get();
}

//---------------------------------------------------------------------------------------
WpInteractor Presenter::get_interactor(int iIntor)
{
    SpInteractor p = get_interactor_shared_ptr(iIntor);
    return WpInteractor(p);
}

//---------------------------------------------------------------------------------------
void Presenter::on_document_updated()
{
    std::list<SpInteractor>::iterator it;
    for (it=m_interactors.begin(); it != m_interactors.end(); ++it)
        (*it)->on_document_updated();
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

//---------------------------------------------------------------------------------------
WpDocument Presenter::get_document_weak_ptr()
{
    return WpDocument(m_spDoc);
}


}  //namespace lomse
