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

#include "lomse_compressed_mxl_compiler.h"

#include "lomse_mxl_compiler.h"
#include "lomse_xml_parser.h"

#if (LOMSE_ENABLE_COMPRESSION == 1)
	#include "lomse_zip_stream.h"
#endif

namespace lomse
{

//=======================================================================================
// CompressedMxlCompiler implementation
//=======================================================================================
CompressedMxlCompiler::CompressedMxlCompiler(MxlCompiler* pMxlCompiler)
    : Compiler()
    , m_pMxlCompiler(pMxlCompiler)
{
}

//---------------------------------------------------------------------------------------
CompressedMxlCompiler::~CompressedMxlCompiler()
{
    delete m_pMxlCompiler;
}

//---------------------------------------------------------------------------------------
ImoDocument* CompressedMxlCompiler::compile_file(const std::string& filename)
{
    m_fileLocator = filename;

#if (LOMSE_ENABLE_COMPRESSION == 1)
    ZipInputStream zip(filename);

    const std::string mxmlString = read_rootfile(zip);

    if (mxmlString.empty())
    {
        LOMSE_LOG_ERROR("[CompressedMxlCompiler::compile_file] Couldn't read rootfile");
        return nullptr;
    }

    return m_pMxlCompiler->compile_string(mxmlString);
#else
    throw runtime_error("Could not open compressed file: Lomse was compiled without compression support");
#endif
}

//---------------------------------------------------------------------------------------
ImoDocument* CompressedMxlCompiler::compile_string(const std::string& UNUSED(source))
{
    m_fileLocator = "string:";
    throw runtime_error("Could not open compressed .mxl string: reading compressed string is not supported currently");
}

//---------------------------------------------------------------------------------------
std::string CompressedMxlCompiler::get_rootfile_path(ZipInputStream& zip)
{
#if (LOMSE_ENABLE_COMPRESSION == 1)
    zip.move_to_entry("META-INF/container.xml");

    if (!zip.open_current_entry())
        return std::string();

    std::vector<unsigned char> metaInfBuffer = zip.get_as_vector();

    XmlParser xml;
    xml.parse_cstring(reinterpret_cast<char*>(metaInfBuffer.data()));

    XmlNode* root = xml.get_tree_root();

    if (!root || root->name() != "container")
        return std::string();

    // XmlNode methods would just return empty values in case of
    // errors so we won't do any special errors handling here.
    XmlNode rootfiles = root->child("rootfiles");
    XmlNode firstRootfile = rootfiles.child("rootfile");
    return firstRootfile.attribute_value("full-path");
#else
    return std::string();
#endif
}

//---------------------------------------------------------------------------------------
std::string CompressedMxlCompiler::read_rootfile(ZipInputStream& zip)
{
#if (LOMSE_ENABLE_COMPRESSION == 1)
    const std::string rootFilePath = get_rootfile_path(zip);

    if (rootFilePath.empty())
        return std::string();

    zip.move_to_entry(rootFilePath);

    if (!zip.open_current_entry())
        return std::string();

    std::vector<unsigned char> mxmlBuffer = zip.get_as_vector();

    return std::string(reinterpret_cast<char*>(mxmlBuffer.data()));
#else
    return std::string();
#endif
}

//---------------------------------------------------------------------------------------
int CompressedMxlCompiler::get_num_errors() const
{
    return m_pMxlCompiler->get_num_errors();
}

}  //namespace lomse
