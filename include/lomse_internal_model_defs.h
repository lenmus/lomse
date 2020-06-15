//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2020. All rights reserved.
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

#define LOMSE_DECLARE_IM_API_ROOT_CLASS(AXxxx, ImoXxxx) \
    protected: \
        friend class ImoXxxx; \
        friend class Document; \
        friend class ADocument; \
        friend class IChildren; \
        friend class ISiblings; \
        friend class RequestDynamic; \
        mutable ImoXxxx* m_pImpl; \
        Document* m_pDoc; \
        ImoId m_id; \
        mutable long m_imVersion; \
        AXxxx(ImoXxxx* impl, Document* doc, long imVers); \
        inline const ImoXxxx* pimpl() const { return m_pImpl; } \
        inline ImoXxxx* pimpl() { return m_pImpl; } \
        void ensure_validity() const; \
        struct Private; \
    public: \
        AXxxx(); \
        AXxxx(AXxxx &&) noexcept = default;             \
        AXxxx& operator=(AXxxx &&) noexcept = default;  \
        AXxxx(const AXxxx& other) = default;            \
        AXxxx& operator=(const AXxxx& other) = default; \
        virtual ~AXxxx() = default; \
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
        AXxxx(AXxxx &&) noexcept = default;             \
        AXxxx& operator=(AXxxx &&) noexcept = default;  \
        AXxxx(const AXxxx& other) = default;            \
        AXxxx& operator=(const AXxxx& other) = default; \
        virtual ~AXxxx() = default;


}   //namespace lomse

#endif    // __LOMSE_INTERNAL_MODEL_DEFS_H__




