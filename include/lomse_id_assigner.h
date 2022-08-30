//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_ID_ASSIGNER_H__
#define __LOMSE_ID_ASSIGNER_H__

#include "lomse_basic.h"

#include <map>
#include <unordered_map>
#include <string>

namespace lomse
{

//forward declarations
class ImoObj;
class ImoDocument;
class Control;

//---------------------------------------------------------------------------------------
//IdAssigner: responsible for assigning/re-assigning ids to ImoObj and Control
// objects and providing access to them by Id
class IdAssigner
{
protected:
    ImoId m_idCounter;
    std::unordered_map<ImoId, ImoObj*> m_idToImo;
    std::unordered_map<ImoId, Control*> m_idToControl;
    std::unordered_map<ImoId, std::string> m_idToXmlId;
    std::map<std::string, ImoId> m_xmlIdToId;

public:
    IdAssigner() : m_idCounter(k_no_imoid) {}

    void reset();

    void assign_id(ImoObj* pImo);
    void assign_id(Control* pControl);
    ImoId reserve_id(ImoId id);
    ImoObj* get_pointer_to_imo(ImoId id) const;
    ImoObj* get_pointer_to_imo(const std::string& xmlId) const;
    Control* get_pointer_to_control(ImoId id) const;
    void remove(ImoObj* pImo);
    void copy_ids_to(IdAssigner* assigner, ImoId idMin);
    std::string get_xml_id_for(ImoId id);
    void set_xml_id_for(ImoId id, const std::string& xmlId);

    //debug
    std::string dump() const;
    inline size_t size() const { return m_idToImo.size(); }
    bool check_ids(IdAssigner* pCopy, std::stringstream& reporter, const std::string& label);

protected:
    friend class FixModelVisitor;
    friend class DocModel;

    void add_id(ImoId id, ImoObj* pImo);
    void add_control_id(ImoId id, Control* pControl);
    void copy_strings_from(IdAssigner* pIdAssigner);
    void set_counter(ImoId value) { m_idCounter = value; }
    void set_control_id(ImoId id, Control* pControl);

};


} //namespace lomse

#endif    //__LOMSE_ID_ASSIGNER_H__
