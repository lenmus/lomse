//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_mnx_compiler.h"

#include <sstream>
#include "lomse_xml_parser.h"
#include "lomse_mxl_analyser.h"
#include "lomse_mnx_analyser.h"
#include "lomse_model_builder.h"
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "private/lomse_document_p.h"
#include "lomse_file_system.h"
#include "lomse_ldp_compiler.h"

#if (LOMSE_ENABLE_COMPRESSION == 1)
	#include "lomse_zip_stream.h"
#endif


using namespace std;

namespace lomse
{

//=======================================================================================
// MnxCompiler implementation
//=======================================================================================
MnxCompiler::MnxCompiler(XmlParser* p, MnxAnalyser* a, ModelBuilder* mb, Document* pDoc)
    : Compiler(p, a, mb, pDoc)
    , m_pXmlParser(p)
    , m_pMnxAnalyser(a)
{
}

//---------------------------------------------------------------------------------------
//constructor for testing: direct construction
MnxCompiler::MnxCompiler(LibraryScope& libraryScope, Document* pDoc)
    : Compiler()
{
    m_pXmlParser = Injector::inject_XmlParser(libraryScope, pDoc->get_scope());
    m_pMnxAnalyser = Injector::inject_MnxAnalyser(libraryScope, pDoc, m_pXmlParser);

    m_pParser = m_pXmlParser;
    m_pAnalyser = m_pMnxAnalyser;
    m_pModelBuilder = Injector::inject_ModelBuilder(pDoc->get_scope());
    m_pDoc = pDoc;
    m_fileLocator = "";
}

//---------------------------------------------------------------------------------------
MnxCompiler::~MnxCompiler()
{
}

//---------------------------------------------------------------------------------------
ImoDocument* MnxCompiler::compile_file(const std::string& filename)
{
    m_fileLocator = filename;
    DocLocator locator(m_fileLocator);
    if (locator.get_inner_protocol() == DocLocator::k_zip)
    {
#if (LOMSE_ENABLE_COMPRESSION == 1)
        InputStream* pFile = FileSystem::open_input_stream(m_fileLocator);
        ZipInputStream* zip  = static_cast<ZipInputStream*>(pFile);

        unsigned char* buffer = zip->get_as_string();
        m_pXmlParser->parse_cstring( (char *)buffer );

        delete pFile;
        delete[] buffer;
#else
        LOMSE_LOG_ERROR("Could not open compressed file '%s'. Lomse was "
                        "compiled without compression support.", filename.c_str());
        return nullptr;
#endif
    }
    else //k_file
        m_pParser->parse_file(filename);

    XmlNode* root = m_pXmlParser->get_tree_root();
    if (root)
        return compile_parsed_tree(root);
    else
        return nullptr;
}

//---------------------------------------------------------------------------------------
ImoDocument* MnxCompiler::compile_string(const std::string& source)
{
    m_fileLocator = "string:";
    m_pXmlParser->parse_text(source);
    return compile_parsed_tree( m_pXmlParser->get_tree_root() );
}

//---------------------------------------------------------------------------------------
ImoDocument* MnxCompiler::compile_parsed_tree(XmlNode* root)
{
    ImoDocument* pDoc = dynamic_cast<ImoDocument*>(
                            m_pMnxAnalyser->analyse_tree(root, m_fileLocator));
    if (pDoc)
        m_pModelBuilder->build_model(pDoc);
    return pDoc;
}


}  //namespace lomse
