//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_MNX_COMPILER_H__
#define __LOMSE_MNX_COMPILER_H__

#include "lomse_compiler.h"
#include "lomse_xml_parser.h"

using namespace std;

namespace lomse
{

//forward declarations
class XmlParser;
class MnxAnalyser;
class LibraryScope;
class ImoDocument;
class Document;


//---------------------------------------------------------------------------------------
// MnxCompiler: builds the tree for a document
class MnxCompiler : public Compiler
{
protected:
    XmlParser* m_pXmlParser;
    MnxAnalyser* m_pMnxAnalyser;

public:
    MnxCompiler(XmlParser* p, MnxAnalyser* a, ModelBuilder* mb, Document* pDoc);
    ~MnxCompiler();

    //constructor for testing: direct construction
    MnxCompiler(LibraryScope& libraryScope, Document* pDoc);

    //compilation
    ImoDocument* compile_file(const std::string& filename) override;
    ImoDocument* compile_string(const std::string& source) override;

protected:
    ImoDocument* compile_parsed_tree(XmlNode* root);

};


}   //namespace lomse

#endif      //__LOMSE_MNX_COMPILER_H__
