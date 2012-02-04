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

#ifndef __LOMSE_COMPILER_H__
#define __LOMSE_COMPILER_H__


#include "lomse_ldp_elements.h"
#include "lomse_reader.h"

using namespace std;

namespace lomse
{

//forward declarations
class LdpParser;
class Analyser;
class ModelBuilder;
class DocumentScope;
class LibraryScope;
class IdAssigner;
class InternalModel;
class ImoDocument;
class Document;


//---------------------------------------------------------------------------------------
// LdpCompiler: builds the tree for a document
class LdpCompiler
{
protected:
    LdpParser*      m_pParser;
    Analyser*       m_pAnalyser;
    ModelBuilder*   m_pModelBuilder;
    IdAssigner*     m_pIdAssigner;
    Document*       m_pDoc;
    SpLdpTree       m_pFinalTree;
    string          m_fileLocator;

public:
    LdpCompiler(LdpParser* p, Analyser* a, ModelBuilder* mb, IdAssigner* ida,
                Document* pDoc);
    ~LdpCompiler();

    //constructor for testing: direct construction
    LdpCompiler(LibraryScope& libraryScope, Document* pDoc);

    //compilation
    InternalModel* compile_file(const std::string& filename);
    InternalModel* compile_string(const std::string& source);
    InternalModel* compile_input(LdpReader& reader);
    InternalModel* create_empty();
    InternalModel* create_with_empty_score();

    //info
    int get_num_errors();
    string get_file_locator() { return m_fileLocator; }

protected:
    InternalModel* compile(SpLdpTree pParseTree);
    SpLdpTree wrap_score_in_lenmusdoc(SpLdpTree pParseTree);
    SpLdpTree parse_empty_doc();

};


}   //namespace lomse

#endif      //__LOMSE_COMPILER_H__
