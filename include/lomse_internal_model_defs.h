//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_INTERNAL_MODEL_DEFS_H__        //to avoid nested includes
#define __LOMSE_INTERNAL_MODEL_DEFS_H__

#include "lomse_basic.h"
#include "lomse_build_options.h"
#include "private/lomse_internal_model_p.h"


namespace lomse
{

//---------------------------------------------------------------------------------------
// macro to simplify the declaration of internal model API objects

class Document;

#define LOMSE_DECLARE_IM_API_ROOT_CLASS \
    protected: \
        friend class ImoObj; \
        friend class Document; \
        friend class ADocument; \
        friend class IChildren; \
        friend class ISiblings; \
        friend class RequestDynamic; \
        mutable ImoObj* m_pImpl; \
        Document* m_pDoc; \
        ImoId m_id; \
        mutable long m_imVersion; \
        AObject(ImoObj* impl, Document* doc, long imVers); \
        inline const ImoObj* pimpl() const { return m_pImpl; } \
        inline ImoObj* pimpl() { return m_pImpl; } \
        void ensure_validity() const; \
        struct Private; \
    public: \
        AObject(); \
        AObject(const AObject& other) = default;            \
        AObject& operator=(const AObject& other) = default; \
        virtual ~AObject() = default; \
        bool is_valid() const;


#define LOMSE_DECLARE_IM_API_CLASS(AXxxx, ImoXxxx) \
    protected: \
        friend class ImoXxxx; \
        friend class Document; \
        friend class ADocument; \
        friend class AObject; \
        AXxxx(ImoObj* impl, Document* doc, long imVers); \
        inline const ImoXxxx* pimpl() const { return static_cast<ImoXxxx*>(m_pImpl); } \
        inline ImoXxxx* pimpl() { return static_cast<ImoXxxx*>(m_pImpl); } \
        struct Private; \
    public: \
        AXxxx(); \
        AXxxx(const AXxxx& other) = default;            \
        AXxxx& operator=(const AXxxx& other) = default; \
        virtual ~AXxxx() = default;


}   //namespace lomse

#endif    // __LOMSE_INTERNAL_MODEL_DEFS_H__




