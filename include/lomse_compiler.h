//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_COMPILER_H__
#define __LOMSE_COMPILER_H__

#include "lomse_parser.h"
#include "lomse_analyser.h"

#include <string>
using namespace std;

namespace lomse
{

//forward declarations
class ModelBuilder;
class DocumentScope;
class LibraryScope;
class ImoDocument;
class Document;


//---------------------------------------------------------------------------------------
// Compiler: base class for all compilers
class Compiler
{
protected:
    Parser*         m_pParser;
    Analyser*       m_pAnalyser;
    ModelBuilder*   m_pModelBuilder;
    Document*       m_pDoc;
    string         m_fileLocator;

    Compiler()
        : m_pParser(nullptr)
        , m_pAnalyser(nullptr)
        , m_pModelBuilder(nullptr)
        , m_pDoc(nullptr)
    {
    }
    Compiler(Parser* p, Analyser* a, ModelBuilder* mb, Document* pDoc);

public:
    virtual ~Compiler();

    //compilation
    virtual ImoDocument* compile_file(const std::string& filename)=0;
    virtual ImoDocument* compile_string(const std::string& source)=0;

    //info
    virtual int get_num_errors() const;
    string get_file_locator() { return m_fileLocator; }

};


}   //namespace lomse

#endif      //__LOMSE_COMPILER_H__
