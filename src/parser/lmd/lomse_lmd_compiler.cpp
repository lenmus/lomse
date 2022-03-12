//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_lmd_compiler.h"

#include <sstream>
#include "lomse_xml_parser.h"
#include "lomse_lmd_analyser.h"
#include "lomse_model_builder.h"
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "private/lomse_document_p.h"
#include "lomse_file_system.h"

#if (LOMSE_ENABLE_COMPRESSION == 1)
	#include "lomse_zip_stream.h"
#endif

using namespace std;

namespace lomse
{

//=======================================================================================
// LmdCompiler implementation
//=======================================================================================
LmdCompiler::LmdCompiler(XmlParser* p, LmdAnalyser* a, ModelBuilder* mb, Document* pDoc)
    : Compiler(p, a, mb, pDoc)
    , m_pXmlParser(p)
    , m_pLmdAnalyser(a)
{
}

//---------------------------------------------------------------------------------------
//constructor for testing: direct construction
LmdCompiler::LmdCompiler(LibraryScope& libraryScope, Document* pDoc)
    : Compiler()
{
    m_pXmlParser = Injector::inject_XmlParser(libraryScope, pDoc->get_scope());
    m_pLmdAnalyser = Injector::inject_LmdAnalyser(libraryScope, pDoc, m_pXmlParser);

    m_pParser = m_pXmlParser;
    m_pAnalyser = m_pLmdAnalyser;
    m_pModelBuilder = Injector::inject_ModelBuilder(pDoc->get_scope());
    m_pDoc = pDoc;
    m_fileLocator = "";
}

//---------------------------------------------------------------------------------------
LmdCompiler::~LmdCompiler()
{
}

//---------------------------------------------------------------------------------------
ImoDocument* LmdCompiler::compile_file(const std::string& filename)
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
ImoDocument* LmdCompiler::compile_string(const std::string& source)
{
    m_fileLocator = "string:";
    m_pXmlParser->parse_text(source);
    return compile_parsed_tree( m_pXmlParser->get_tree_root() );
}

//---------------------------------------------------------------------------------------
ImoDocument* LmdCompiler::create_empty()
{
    m_pParser->parse_text("<lenmusdoc vers='0.0'><content/></lenmusdoc>");
    return compile_parsed_tree( m_pXmlParser->get_tree_root() );
}

//---------------------------------------------------------------------------------------
ImoDocument* LmdCompiler::create_with_empty_score()
{
//    m_pFinalTree = m_pParser->parse_text("(lenmusdoc (vers 0.0) (content (score (vers 1.6)(instrument (musicData)))))");
//    return compile_parsed_tree(m_pFinalTree);
    return nullptr;    //TODO: Probably this method is not needed
}

//---------------------------------------------------------------------------------------
ImoDocument* LmdCompiler::compile_parsed_tree(XmlNode* root)
{
    ImoDocument* pDoc = dynamic_cast<ImoDocument*>(
                                m_pLmdAnalyser->analyse_tree(root, m_fileLocator));
    if (pDoc)
        m_pModelBuilder->build_model(pDoc);
    return pDoc;
}


}  //namespace lomse
