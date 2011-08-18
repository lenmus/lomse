//--------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------

#ifndef __LOMSE_MODEL_BUILDER_H__
#define __LOMSE_MODEL_BUILDER_H__

#include <ostream>

using namespace std;

namespace lomse
{

//forward declarations
class InternalModel;
class ImoDocument;
class ImoObj;

//-------------------------------------------------------------------------------------
// ModelBuilder. Implements the final step of LDP compiler: code generation.
// Traverses the parse tree and creates the internal model
//-------------------------------------------------------------------------------------
class ModelBuilder
{
protected:
    ostream&    m_reporter;
    //LdpTree*    m_pTree;

public:
    ModelBuilder(ostream& reporter);
    virtual ~ModelBuilder();

    ImoDocument* build_model(InternalModel* IModel);


protected:
    void structurize(ImoObj* pImo);

};


}   //namespace lomse

#endif      //__LOMSE_MODEL_BUILDER_H__
