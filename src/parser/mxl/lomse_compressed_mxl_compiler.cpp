//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
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

    const std::vector<unsigned char> mxmlBuffer = read_rootfile(zip);

    if (mxmlBuffer.empty())
    {
        LOMSE_LOG_ERROR("[CompressedMxlCompiler::compile_file] Couldn't read rootfile");
        return nullptr;
    }

    return m_pMxlCompiler->compile_buffer(static_cast<const void*>(mxmlBuffer.data()), mxmlBuffer.size());
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
    xml.parse_buffer(static_cast<const void*>(metaInfBuffer.data()), metaInfBuffer.size());

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
std::vector<unsigned char> CompressedMxlCompiler::read_rootfile(ZipInputStream& zip)
{
#if (LOMSE_ENABLE_COMPRESSION == 1)
    const std::string rootFilePath = get_rootfile_path(zip);

    if (rootFilePath.empty())
        return {};

    zip.move_to_entry(rootFilePath);

    if (!zip.open_current_entry())
        return {};

    return zip.get_as_vector();
#else
    return {};
#endif
}

//---------------------------------------------------------------------------------------
int CompressedMxlCompiler::get_num_errors() const
{
    return m_pMxlCompiler->get_num_errors();
}

}  //namespace lomse
