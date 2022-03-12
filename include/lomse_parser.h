//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_PARSER_H__
#define __LOMSE_PARSER_H__

#include "lomse_basic.h"

#include <ostream>
using namespace std;

namespace lomse
{

//---------------------------------------------------------------------------------------
// Parser: base class for any parser
class Parser
{
protected:
    ostream& m_reporter;
    int m_numErrors;        //number of errors found during parsing
    ImoId m_nMaxId;         //maximum ID found

    Parser(ostream& reporter)
        : m_reporter(reporter)
        , m_numErrors(0)
        , m_nMaxId(k_no_imoid)
    {
    }

public:
    virtual ~Parser() {}

//    //setings and options
//    inline void SetIgnoreList(std::set<long>* pSet) { m_pIgnoreSet = pSet; }

    virtual void parse_file(const std::string& filename, bool fErrorMsg = true) = 0;
    virtual void parse_text(const std::string& sourceText) = 0;
    //virtual void parse_input(LdpReader& reader) = 0;

    inline int get_num_errors() const { return m_numErrors; }

};


} //namespace lomse

#endif    //__LOMSE_PARSER_H__
