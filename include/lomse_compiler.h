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
