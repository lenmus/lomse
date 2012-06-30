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

#include "lomse_compiler.h"

//#include <sstream>
//#include "lomse_ldp_parser.h"
//#include "lomse_ldp_analyser.h"
#include "lomse_model_builder.h"
//#include "lomse_injectors.h"
//#include "lomse_internal_model.h"
//#include "lomse_document.h"
//
//
//using namespace std;

namespace lomse
{

//=======================================================================================
// Compiler implementation
//=======================================================================================
Compiler::Compiler(Parser* p, Analyser* a, ModelBuilder* mb, IdAssigner* ida,
                   Document* pDoc)
    : m_pParser(p)
    , m_pAnalyser(a)
    , m_pModelBuilder(mb)
    , m_pIdAssigner(ida)
    , m_pDoc(pDoc)
//    , m_pFinalTree()
    , m_fileLocator("")
{
}

//---------------------------------------------------------------------------------------
Compiler::~Compiler()
{
    delete m_pParser;
    delete m_pAnalyser;
    delete m_pModelBuilder;
//    if (m_pFinalTree)
//        delete m_pFinalTree->get_root();
}

////---------------------------------------------------------------------------------------
//InternalModel* Compiler::compile_file(const std::string& filename)
//{
//    m_fileLocator = filename;
//    m_pFinalTree = m_pParser->parse_file(filename);
//    m_pIdAssigner->set_last_id( m_pParser->get_max_id() );
//    return compile_parsed_tree(m_pFinalTree);
//}
//
////---------------------------------------------------------------------------------------
//InternalModel* Compiler::compile_string(const std::string& source)
//{
//    m_fileLocator = "string:";
//    m_pFinalTree = m_pParser->parse_text(source);
//    m_pIdAssigner->set_last_id( m_pParser->get_max_id() );
//    return compile_parsed_tree(m_pFinalTree);
//}
//
////---------------------------------------------------------------------------------------
//InternalModel* Compiler::compile_input(LdpReader& reader)
//{
//    m_fileLocator = reader.get_locator();
//    m_pFinalTree = m_pParser->parse_input(reader);
//    m_pIdAssigner->set_last_id( m_pParser->get_max_id() );
//    return compile_parsed_tree(m_pFinalTree);
//}
//
////---------------------------------------------------------------------------------------
//InternalModel* Compiler::create_empty()
//{
//    m_pFinalTree = parse_empty_doc();
//    m_pIdAssigner->set_last_id( m_pParser->get_max_id() );
//    return compile_parsed_tree(m_pFinalTree);
//}
//
////---------------------------------------------------------------------------------------
//InternalModel* Compiler::create_with_empty_score()
//{
//    m_pFinalTree = m_pParser->parse_text("(lenmusdoc (vers 0.0) (content (score (vers 1.6)(instrument (musicData)))))");
//    m_pIdAssigner->set_last_id( m_pParser->get_max_id() );
//    return compile_parsed_tree(m_pFinalTree);
//}
//
////---------------------------------------------------------------------------------------
//InternalModel* Compiler::compile_parsed_tree(SpLdpTree pParseTree)
//{
//    if (pParseTree->get_root()->is_type(k_score))
//        m_pFinalTree = wrap_score_in_lenmusdoc(pParseTree);
//    else
//        m_pFinalTree = pParseTree;
//
//    SpLdpTree finalTree( m_pFinalTree );
//    InternalModel* IModel = m_pAnalyser->analyse_tree(finalTree, m_fileLocator);
//    m_pModelBuilder->build_model(IModel);
//    return IModel;
//}
//
////---------------------------------------------------------------------------------------
//SpLdpTree Compiler::wrap_score_in_lenmusdoc(SpLdpTree pParseTree)
//{
//    SpLdpTree pFinalTree = parse_empty_doc();
//    m_pIdAssigner->reassign_ids(pParseTree);
//
//    LdpTree::depth_first_iterator it = pFinalTree->begin();
//    while (it != pFinalTree->end() && !(*it)->is_type(k_content))
//        ++it;
//    (*it)->append_child(pParseTree->get_root());
//
//    return pFinalTree;
//}
//
////---------------------------------------------------------------------------------------
//SpLdpTree Compiler::parse_empty_doc()
//{
//    SpLdpTree pTree = m_pParser->parse_text("(lenmusdoc (vers 0.0) (content ))");
//    m_pIdAssigner->set_last_id( m_pParser->get_max_id() );
//    return pTree;
//}

//---------------------------------------------------------------------------------------
int Compiler::get_num_errors()
{
    return m_pParser->get_num_errors();
}


}  //namespace lomse
