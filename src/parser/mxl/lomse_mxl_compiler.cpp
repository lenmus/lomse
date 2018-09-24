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

#include "lomse_mxl_compiler.h"

#include <sstream>
#include "lomse_xml_parser.h"
#include "lomse_mxl_analyser.h"
#include "lomse_model_builder.h"
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_document.h"
#include "lomse_file_system.h"
#include "lomse_ldp_compiler.h"

#if (LOMSE_ENABLE_COMPRESSION == 1)
	#include "lomse_zip_stream.h"
#endif


using namespace std;

namespace lomse
{

//=======================================================================================
// MxlCompiler implementation
//=======================================================================================
MxlCompiler::MxlCompiler(XmlParser* p, MxlAnalyser* a, ModelBuilder* mb, Document* pDoc)
    : Compiler(p, a, mb, pDoc)
    , m_pXmlParser(p)
    , m_pMxlAnalyser(a)
{
}

//---------------------------------------------------------------------------------------
//constructor for testing: direct construction
MxlCompiler::MxlCompiler(LibraryScope& libraryScope, Document* pDoc)
    : Compiler()
{
    m_pXmlParser = Injector::inject_XmlParser(libraryScope, pDoc->get_scope());
    m_pMxlAnalyser = Injector::inject_MxlAnalyser(libraryScope, pDoc, m_pXmlParser);

    m_pParser = m_pXmlParser;
    m_pAnalyser = m_pMxlAnalyser;
    m_pModelBuilder = Injector::inject_ModelBuilder(pDoc->get_scope());
    m_pDoc = pDoc;
    m_fileLocator = "";
}

//---------------------------------------------------------------------------------------
MxlCompiler::~MxlCompiler()
{
}

//---------------------------------------------------------------------------------------
ImoDocument* MxlCompiler::compile_file(const std::string& filename)
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
		throw runtime_error("Could not open compressed file: Lomse was compiled without compression support");
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
ImoDocument* MxlCompiler::compile_string(const std::string& source)
{
    m_fileLocator = "string:";
    m_pXmlParser->parse_text(source);
    return compile_parsed_tree( m_pXmlParser->get_tree_root() );
}

//---------------------------------------------------------------------------------------
ImoDocument* MxlCompiler::compile_parsed_tree(XmlNode* root)
{
    ImoDocument* pDoc = dynamic_cast<ImoDocument*>(
                            m_pMxlAnalyser->analyse_tree(root, m_fileLocator));
    if (pDoc)
        m_pModelBuilder->build_model(pDoc);
    return pDoc;
}


}  //namespace lomse
