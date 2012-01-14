//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
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
    LdpTree*        m_pFinalTree;
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
    InternalModel* compile(LdpTree* pParseTree);
    LdpTree* wrap_score_in_lenmusdoc(LdpTree* pParseTree);
    LdpTree* parse_empty_doc();

};


}   //namespace lomse

#endif      //__LOMSE_COMPILER_H__
