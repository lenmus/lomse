//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_MXL_COMPILER_H__
#define __LOMSE_MXL_COMPILER_H__

#include "lomse_compiler.h"
#include "lomse_xml_parser.h"

using namespace std;

namespace lomse
{

//forward declarations
class XmlParser;
class MxlAnalyser;
class LibraryScope;
class ImoDocument;
class Document;


//---------------------------------------------------------------------------------------
// MxlCompiler: builds the tree for a document
class MxlCompiler : public Compiler
{
protected:
    XmlParser* m_pXmlParser;
    MxlAnalyser* m_pMxlAnalyser;

public:
    MxlCompiler(XmlParser* p, MxlAnalyser* a, ModelBuilder* mb, Document* pDoc);
    ~MxlCompiler();

    //constructor for testing: direct construction
    MxlCompiler(LibraryScope& libraryScope, Document* pDoc);

    //compilation
    ImoDocument* compile_file(const std::string& filename) override;
    ImoDocument* compile_string(const std::string& source) override;
    ImoDocument* compile_buffer(const void* buffer, size_t size);

protected:
    ImoDocument* compile_parsed_tree(XmlNode* root);

};


}   //namespace lomse

#endif      //__LOMSE_MXL_COMPILER_H__
