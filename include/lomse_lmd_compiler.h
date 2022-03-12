//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_LMD_COMPILER_H__
#define __LOMSE_LMD_COMPILER_H__

#include "lomse_compiler.h"
#include "lomse_xml_parser.h"

using namespace std;

namespace lomse
{

//forward declarations
class XmlParser;
class LmdAnalyser;
class LibraryScope;
class Document;


//---------------------------------------------------------------------------------------
// LmdCompiler: builds the tree for a document
class LmdCompiler : public Compiler
{
protected:
    XmlParser* m_pXmlParser;
    LmdAnalyser* m_pLmdAnalyser;

public:
    LmdCompiler(XmlParser* p, LmdAnalyser* a, ModelBuilder* mb, Document* pDoc);
    ~LmdCompiler();

    //constructor for testing: direct construction
    LmdCompiler(LibraryScope& libraryScope, Document* pDoc);

    //compilation
    ImoDocument* compile_file(const std::string& filename) override;
    ImoDocument* compile_string(const std::string& source) override;
    ImoDocument* create_empty();
    ImoDocument* create_with_empty_score();

protected:
    ImoDocument* compile_parsed_tree(XmlNode* root);

};


}   //namespace lomse

#endif      //__LOMSE_LMD_COMPILER_H__
