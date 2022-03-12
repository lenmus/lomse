//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_ldp_compiler.h"

#include <sstream>
#include "lomse_ldp_parser.h"
#include "lomse_ldp_analyser.h"
#include "lomse_model_builder.h"
#include "lomse_injectors.h"
#include "lomse_internal_model.h"
#include "private/lomse_document_p.h"


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
