//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_COMPRESSED_MXL_COMPILER_H__
#define __LOMSE_COMPRESSED_MXL_COMPILER_H__

#include "lomse_compiler.h"

namespace lomse
{

//forward declarations
class MxlCompiler;
class ZipInputStream;

//---------------------------------------------------------------------------------------
// CompressedMxlCompiler: builds the tree for a document
class CompressedMxlCompiler : public Compiler
{
protected:
    MxlCompiler* m_pMxlCompiler;

public:
    explicit CompressedMxlCompiler(MxlCompiler* pMxlCompiler);
    ~CompressedMxlCompiler();

    //compilation
    ImoDocument* compile_file(const std::string& filename) override;
    ImoDocument* compile_string(const std::string& source) override;

    //info
    int get_num_errors() const override;

protected:
    std::string get_rootfile_path(ZipInputStream&);
    std::vector<unsigned char> read_rootfile(ZipInputStream&);
};


}   //namespace lomse

#endif      //__LOMSE_COMPRESSED_MXL_COMPILER_H__
