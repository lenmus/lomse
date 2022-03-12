//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_LDP_COMPILER_H__
#define __LOMSE_LDP_COMPILER_H__

#include "lomse_compiler.h"
#include "lomse_ldp_elements.h"
#include "lomse_reader.h"

using namespace std;

namespace lomse
{

//forward declarations
class LdpParser;
class LdpAnalyser;
class ModelBuilder;
class DocumentScope;
class LibraryScope;
class ImoDocument;
class Document;


//---------------------------------------------------------------------------------------
// LdpCompiler: builds the tree for a document
class LdpCompiler : public Compiler
{
protected:
    LdpParser* m_pLdpParser;
    LdpAnalyser* m_pLdpAnalyser;

public:
    LdpCompiler(LdpParser* p, LdpAnalyser* a, ModelBuilder* mb, Document* pDoc);
    virtual ~LdpCompiler();

    //constructor for testing: direct construction
    LdpCompiler(LibraryScope& libraryScope, Document* pDoc);

    //compilation
    ImoDocument* compile_file(const std::string& filename) override;
    ImoDocument* compile_string(const std::string& source) override;
    ImoDocument* compile_input(LdpReader& reader);
    ImoDocument* create_empty();
    ImoDocument* create_with_empty_score();

    ////info
    //int get_num_errors();
    //string get_file_locator() { return m_fileLocator; }

protected:
    ImoDocument* compile_parsed_tree(LdpTree* tree);
    LdpTree* wrap_score_in_lenmusdoc(LdpTree* pParseTree);
    void parse_empty_doc();

};


}   //namespace lomse

#endif      //__LOMSE_LDP_COMPILER_H__
