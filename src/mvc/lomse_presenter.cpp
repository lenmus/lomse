//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#define LOMSE_INTERNAL_API
#include "lomse_presenter.h"

#include "lomse_injectors.h"
#include "private/lomse_document_p.h"
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
        else if (ext == "mxl")
            return Document::k_format_mxl_compressed;
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
Presenter* PresenterBuilder::new_document(int viewType, Drawer* screenDrawer,
                                          Drawer* printDrawer, const std::string& content,
                                          ostream& reporter, int format)
{
    Document* pDoc = Injector::inject_Document(m_libScope, reporter);
    if (content != "")
        pDoc->from_string(content, format);
    else
        pDoc->create_empty();

    return Injector::inject_Presenter(m_libScope, viewType, pDoc, screenDrawer,
                                      printDrawer);
}

//---------------------------------------------------------------------------------------
Presenter* PresenterBuilder::open_document(int viewType, const std::string& filename,
                                           Drawer* screenDrawer, Drawer* printDrawer,
                                           ostream& reporter)
{
    Document* pDoc = Injector::inject_Document(m_libScope, reporter);
    int format = FileFormatFinder::determine_format(filename);
    pDoc->from_file(filename, format);

    return Injector::inject_Presenter(m_libScope, viewType, pDoc, screenDrawer,
                                      printDrawer);
}

//---------------------------------------------------------------------------------------
Presenter* PresenterBuilder::open_document(int viewType, LdpReader& reader,
                                           Drawer* screenDrawer, Drawer* printDrawer,
                                           ostream& reporter)
{
    Document* pDoc = Injector::inject_Document(m_libScope, reporter);
    pDoc->from_input(reader);

    return Injector::inject_Presenter(m_libScope, viewType, pDoc, screenDrawer,
                                      printDrawer);
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
