//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#include "lomse_compiler.h"

#include "lomse_model_builder.h"

namespace lomse
{

//=======================================================================================
// Compiler implementation
//=======================================================================================
Compiler::Compiler(Parser* p, Analyser* a, ModelBuilder* mb, Document* pDoc)
    : m_pParser(p)
    , m_pAnalyser(a)
    , m_pModelBuilder(mb)
    , m_pDoc(pDoc)
    , m_fileLocator("")
{
}

//---------------------------------------------------------------------------------------
Compiler::~Compiler()
{
    delete m_pParser;
    delete m_pAnalyser;
    delete m_pModelBuilder;
}

//---------------------------------------------------------------------------------------
int Compiler::get_num_errors() const
{
    return m_pParser->get_num_errors();
}


}  //namespace lomse
