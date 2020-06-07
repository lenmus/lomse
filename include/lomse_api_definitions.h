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

#ifndef __LOMSE_API_DEFINITIONS_H__        //to avoid nested includes
#define __LOMSE_API_DEFINITIONS_H__

///@cond INTERNALS
namespace lomse
{
///@endcond


//---------------------------------------------------------------------------------------
// macro to simplify the declaration of internal model API objects
// to be moved to another place?

class Document;

#define LOMSE_DECLARE_IM_API_ROOT_CLASS(IXxxx, ImoXxxx) \
    protected: \
        friend class ImoXxxx; \
        friend class Document; \
        mutable ImoXxxx* m_pImpl; \
        Document* m_pDoc; \
        ImoId m_id; \
        mutable long m_imVersion; \
        IXxxx(ImoXxxx* impl, Document* doc, long imVers); \
        IXxxx(); \
        inline const ImoXxxx* pimpl() const { return m_pImpl; } \
        inline ImoXxxx* pimpl() { return m_pImpl; } \
        virtual void ensure_validity() const; \
    public: \
        IXxxx(IXxxx &&) noexcept = default;             \
        IXxxx& operator=(IXxxx &&) noexcept = default;  \
        IXxxx(const IXxxx& other) = default;            \
        IXxxx& operator=(const IXxxx& other) = default; \
        virtual ~IXxxx() = default; \


#define LOMSE_DECLARE_IM_API_CLASS(IXxxx, ImoXxxx) \
    protected: \
        friend class ImoXxxx; \
        friend class Document; \
        IXxxx(ImoXxxx* impl, Document* doc, long imVers); \
        IXxxx(); \
        inline const ImoXxxx* pimpl() const { return static_cast<ImoXxxx*>(m_pImpl); } \
        inline ImoXxxx* pimpl() { return static_cast<ImoXxxx*>(m_pImpl); } \
    public: \
        IXxxx(IXxxx &&) noexcept = default;             \
        IXxxx& operator=(IXxxx &&) noexcept = default;  \
        IXxxx(const IXxxx& other) = default;            \
        IXxxx& operator=(const IXxxx& other) = default; \
        virtual ~IXxxx() = default; \


//---------------------------------------------------------------------------------------
/** @ingroup enumerations
    Options to join barlines in instrument groups
*/
enum EJoinBarlines
{
    k_non_joined_barlines = 0,      ///< Do not join the barlines of all instruments in the group.
    k_joined_barlines = 1,	        ///< join the barlines of all instruments in the group.
    k_mensurstrich_barlines = 2,    ///< join the barlines of all instruments in the group.
};

//---------------------------------------------------------------------------------------
/** @ingroup enumerations
    Symbol to use for joining all instruments in an instruments group.
*/
enum EGroupSymbol
{
    k_group_symbol_none = 0,    ///< Do not display a symbol.
    k_group_symbol_brace,       ///< Use a brace.
    k_group_symbol_bracket,     ///< Use a bracket.
    k_group_symbol_line,        ///< Use a vertical a line.
};

//---------------------------------------------------------------------------------------
/** @ingroup enumerations

    This enum defines the duration of one beat, for metronome and for methods that use
    measure/beat to define a location.
*/
enum EBeatDuration
{
    k_beat_implied = 0,     ///< Implied by the time signature; e.g. 4/4 = four
                            ///< beats, 6/8 = two beats, 3/8 = one beat.
                            ///< The number of implied beats for a time signature is
                            ///< provided by method ImoTimeSignature::get_num_pulses().
                            ///< Basically, for simple time signatures, such as 4/4,
                            ///< 3/4, 2/4, 3/8, and 2/2, the number of beats is given by
                            ///< the time signature top number, with the exception of
                            ///< 3/8 which is normally conducted in one beat. In compound
                            ///< time signatures (6/x, 12/x, and 9/x) the number of beats
                            ///< is given by dividing the top number by three.

    k_beat_bottom_ts,       ///< Use the note duration implied by the time signature
                            ///< bottom number; e.g. 3/8 = use eighth notes. Notice
                            ///< that the number of beats will coincide with the
                            ///< time signature top number, e.g. 3 beats for 3/8.

    k_beat_specified,       ///< Use specified note value for beat duration.
};


}   //namespace lomse

#endif    // __LOMSE_API_DEFINITIONS_H__

