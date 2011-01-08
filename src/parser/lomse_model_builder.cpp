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

#include "lomse_model_builder.h"

#include "lomse_document.h"
#include "lomse_internal_model.h"
#include "lomse_staffobjs_table.h"
#include "lomse_basic_model.h"

using namespace std;

namespace lomse
{

//-------------------------------------------------------------------------------------
// ImObjectsBuilder. Helper class to build the ImoObj model from the basic model
//-------------------------------------------------------------------------------------

ImObjectsBuilder::ImObjectsBuilder(ostream& reporter)
    : m_reporter(reporter)
    , m_IModel(NULL)
{
    if (m_IModel)
        delete m_IModel;
}

ImObjectsBuilder::~ImObjectsBuilder()
{
}

ImoDocument* ImObjectsBuilder::create_objects(InternalModel* IModel)
{
    m_IModel = IModel;
    return dynamic_cast<ImoDocument*>( IModel->get_root() );
}



//-------------------------------------------------------------------------------------
// ModelBuilder implementation
//-------------------------------------------------------------------------------------

ModelBuilder::ModelBuilder(ostream& reporter)
    : m_reporter(reporter)
{
}

ModelBuilder::~ModelBuilder()
{
}

ImoDocument* ModelBuilder::build_model(InternalModel* IModel)
{
    ImObjectsBuilder imb(m_reporter);
    ImoDocument* pDoc = imb.create_objects(IModel);
    int numContent = pDoc->get_num_content_items();
    for (int i = 0; i < numContent; i++)
        structurize( pDoc->get_content_item(i) );
    return pDoc;
}


//void ModelBuilder::update_model(InternalModel* IModel)
//{
//    m_pTree = pTree;
//    DocIterator it(m_pTree);
//    for (it.start_of_content(); *it != NULL; ++it)
//    {
//        //Factory method ?
//        if ((*it)->is_modified())
//        {
//            if((*it)->is_type(k_score))
//            {
//                ImoScore* pScore = dynamic_cast<ImoScore*>( (*it)->get_imobj() );
//                ColStaffObjsBuilder builder(m_pTree);
//                builder.update(pScore);
//            }
//        }
//    }
//}

void ModelBuilder::structurize(ImoObj* pImo)
{
    //in future this should invoke a factory object

    ImoScore* pScore = dynamic_cast<ImoScore*>(pImo);
    if (pScore)
    {
        ColStaffObjsBuilder builder;
        builder.build(pScore);
    }
}



}  //namespace lomse
