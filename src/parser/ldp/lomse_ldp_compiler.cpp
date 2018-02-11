//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
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

#include "lomse_ldp_compiler.h"

#include <sstream>
#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_model_builder.h"
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "lomse_document.h"


using namespace std;

namespace lomse
{

//=======================================================================================
// LdpCompiler implementation
//=======================================================================================
LdpCompiler::LdpCompiler(LibraryScope& libraryScope, Document* pDoc)
    : Compiler()
{
    m_pLdpParser = Injector::inject_LdpParser(libraryScope, pDoc->get_scope());
    m_pLdpAnalyser = Injector::inject_LdpAnalyser(libraryScope, pDoc);

    m_pParser = m_pLdpParser;
    m_pAnalyser = m_pLdpAnalyser;
    m_pModelBuilder = Injector::inject_ModelBuilder(pDoc->get_scope());
    m_pDoc = pDoc;
    m_fileLocator = "";
}

//---------------------------------------------------------------------------------------
// constructor for unit tests: direct injection of components
LdpCompiler::LdpCompiler(LdpParser* p, LdpAnalyser* a, ModelBuilder* mb, Document* pDoc)
    : Compiler(p, a, mb, pDoc)
    , m_pLdpParser(p)
    , m_pLdpAnalyser(a)
{
}

//---------------------------------------------------------------------------------------
LdpCompiler::~LdpCompiler()
{
}

//---------------------------------------------------------------------------------------
ImoDocument* LdpCompiler::compile_file(const std::string& filename)
{
    m_fileLocator = filename;
    m_pParser->parse_file(filename);
    LdpTree* tree = m_pLdpParser->get_ldp_tree();
    return compile_parsed_tree(tree);
}

//---------------------------------------------------------------------------------------
ImoDocument* LdpCompiler::compile_string(const std::string& source)
{
    m_fileLocator = "string:";
    m_pParser->parse_text(source);
    LdpTree* tree = m_pLdpParser->get_ldp_tree();
    return compile_parsed_tree(tree);
}

//---------------------------------------------------------------------------------------
ImoDocument* LdpCompiler::compile_input(LdpReader& reader)
{
    m_fileLocator = reader.get_locator();
    m_pLdpParser->parse_input(reader);
    LdpTree* tree = m_pLdpParser->get_ldp_tree();
    return compile_parsed_tree(tree);
}

//---------------------------------------------------------------------------------------
ImoDocument* LdpCompiler::create_empty()
{
    parse_empty_doc();
    LdpTree* tree = m_pLdpParser->get_ldp_tree();
    return compile_parsed_tree(tree);
}

//---------------------------------------------------------------------------------------
ImoDocument* LdpCompiler::create_with_empty_score()
{
    m_pParser->parse_text("(lenmusdoc (vers 0.0) (content (score (vers 1.6)(instrument (musicData)))))");
    LdpTree* tree = m_pLdpParser->get_ldp_tree();
    return compile_parsed_tree(tree);
}

//---------------------------------------------------------------------------------------
ImoDocument* LdpCompiler::compile_parsed_tree(LdpTree* tree)
{
    if (!tree)
        return create_empty();

    if (tree->get_root()->is_type(k_score))
        tree = wrap_score_in_lenmusdoc(tree);

    ImoDocument* pRoot = dynamic_cast<ImoDocument*>(
                                m_pLdpAnalyser->analyse_tree(tree, m_fileLocator));
    m_pModelBuilder->build_model(pRoot);
    delete tree->get_root();
    return pRoot;
}

//---------------------------------------------------------------------------------------
LdpTree* LdpCompiler::wrap_score_in_lenmusdoc(LdpTree* pParseTree)
{
    //when parsing the empty document, current tree gets deleted. Therefore, we
    //need to save its root and create an auxiliary tree
    LdpTree auxTree( pParseTree->get_root() );
    m_pLdpParser->release_last_tree_ownership();

    //now we can safely parse the empty document
    parse_empty_doc();
    LdpTree* pFinalTree = m_pLdpParser->get_ldp_tree();

    LdpTree::depth_first_iterator it = pFinalTree->begin();
    while (it != pFinalTree->end() && !(*it)->is_type(k_content))
        ++it;
    (*it)->append_child(auxTree.get_root());

    auxTree.set_root(nullptr);
    return pFinalTree;
}

//---------------------------------------------------------------------------------------
void LdpCompiler::parse_empty_doc()
{
    m_pParser->parse_text("(lenmusdoc (vers 0.0) (content ))");
}


}  //namespace lomse
