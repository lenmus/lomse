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

#ifndef __LOMSE_INTERNAL_MODEL_H__        //to avoid nested includes
#define __LOMSE_INTERNAL_MODEL_H__

#include "lomse_basic.h"
#include "lomse_build_options.h"
#include "private/lomse_internal_model_p.h"

#include <string>
using namespace std;


namespace lomse
{

//forward declaration of the implementation classes and API classes
class ImoDocument;
class IDocument;
class ImoScore;
class IScore;


//---------------------------------------------------------------------------------------
// macro to simplify the declaration of internal model API objects
// to be moved to another place?

class Document;

#define LOMSE_DECLARE_IM_API_CLASS(IXxxx, ImoXxxx) \
    private: \
        friend class ImoXxxx; \
        friend class Document; \
        ImoXxxx* m_pImpl; \
        Document* m_pDoc; \
        ImoId m_id; \
        long m_imVersion; \
        IXxxx(ImoXxxx* impl, Document* doc, long imVers); \
        IXxxx(); \
        inline const ImoXxxx* pimpl() const { return m_pImpl; } \
        inline ImoXxxx* pimpl() { return m_pImpl; } \
        void ensure_validity(); \
    public: \
        IXxxx(IXxxx &&) noexcept = default;             \
        IXxxx& operator=(IXxxx &&) noexcept = default;  \
        IXxxx(const IXxxx& other) = default;            \
        IXxxx& operator=(const IXxxx& other) = default; \
        bool is_valid();





//---------------------------------------------------------------------------------------
/** %IScore is the API object for interacting with the internal model for a music score.
*/
class LOMSE_EXPORT IScore
{
    LOMSE_DECLARE_IM_API_CLASS(IScore, ImoScore)
    friend class IDocument;

    /** Returns an internal unique identifier for this score. It could be required
        by some methods in the libray, such as some %Interactor methods.
    */
    ImoId get_score_id();

    /** Transitional, to facilitate migration to the new public API.
        Notice that this method will be removed in future so, please, if you need to
        use this method open an issue at https://github.com/lenmus/lomse/issues
        explaining the need, so that the public API
        could be fixed and your app. would not be affected in future when this method
        is removed.
    */
    ImoScore* get_internal_object();


};


}   //namespace lomse

#endif    // __LOMSE_INTERNAL_MODEL_H__


