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
