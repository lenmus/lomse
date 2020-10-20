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

#include "lomse_internal_model.h"
#include "private/lomse_internal_model_p.h"

#include "lomse_document.h"
#include "lomse_model_builder.h"
#include "lomse_im_factory.h"
#include "lomse_score_algorithms.h"


using namespace std;

namespace lomse
{

//=======================================================================================
// helper macro to simplify the implementation of internal model API objects

#define LOMSE_IMPLEMENT_IM_API_CLASS(AXxxx, ImoXxxx, AParentClass) \
    AXxxx::AXxxx(ImoObj* impl, Document* doc, long imVers) \
        : AParentClass(impl, doc, imVers) \
    { \
    } \
    AXxxx::AXxxx() \
        : AParentClass() \
    { \
    }



//=======================================================================================
// Documentation for API enums

////---------------------------------------------------------------------------------------
// /* * @enum EJoinBarlines
//    @ingroup enumerations
//
//    Options to join barlines in instrument groups.
//    k_non_joined_barlines = 0,      ///< Do not join the barlines of all instruments in the group.
//    k_joined_barlines = 1,	        ///< join the barlines of all instruments in the group.
//    k_mensurstrich_barlines = 2,    ///< join the barlines of all instruments in the group.
////
////    Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod
////    tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam,
////    quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.
////
////    Option 'k_truncate_barline_final' is the default behavior and it can be useful
////    for score editors: staff lines will run always to right margin until a barline of
////    type final is entered.
////
//    Option 'k_truncate_always' truncates staff lines after last staff object. It can
//    be useful for creating score samples (e.g., for ebooks).
//        k_isolated=0,       Independent barlines for each score part
//        k_joined,           Barlines joined across all parts
//        k_mensurstrich,     Barlines only in the gaps between parts, but not on
//                            the staves of each part
//
//    @#include <lomse_internal_model.h>
//*/


////---------------------------------------------------------------------------------------
// /* * @enum EGroupSymbol
//    @ingroup enumerations
//
//    When several instruments form a group this enum indicates if a symbol for
//    the group should be displayed in the score and what symbol to use.
//
//    @#include <lomse_internal_model.h>
//*/
////Doxygen: this does not work, but there are neither warnings nor errors !
// /* * @enum EGroupSymbol
//    @var k_group_symbol_brace
//    @brief Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod
//           tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam,
//           quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo
//           consequat.
//*/
// /* * @enum EGroupSymbol
//    @var EGroupSymbol::k_group_symbol_brace
//    @brief Lorem ipsum dolor sit amet, consectetur adipiscing elit,
//*/
// /* * @enum EGroupSymbol
//    @var EGroupSymbol::k_group_symbol_brace
//    @brief Lorem ipsum dolor sit amet,
//*/


//-----------------------------------------------------------------------------
/** @enum EDocObject
    @ingroup enumerations

    The values from this
    enum are normally used in factory methods, to define the class of the object to
    be created. For instance, in ADocument::create_object().

    @#include <lomse_internal_model.h>
*/
//Doxygen: this does not work, but there are neither warnings nor errors !
/** @var EDocObject::k_obj_content
    @brief Lorem ipsum dolor sit amet,
*/
/** @enum EDocObject
    @var EDocObject::k_obj_heading
    Lorem ipsum dolor sit amet,
*/
/** @var k_obj_image
    @brief Foo it with a C

    When you use C... etc.
 */


//=======================================================================================
// API objects implementation
//=======================================================================================


//=======================================================================================
/** @class AObject
    Abstract base class for all objects composing the document

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/
//---------------------------------------------------------------------------------------
/** @memberof AObject
    Default constructor. Creates an empty (invalid) %AObject. Useful for methods that
    returns an %AObject when a valid object cannot be returned.
*/
AObject::AObject()
    : m_pImpl(nullptr)
    , m_pDoc(nullptr)
    , m_id(-1L)
    , m_imVersion(-1L)
{
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @FALSE when the object is not in a usable state. It is the equivalent to
    checking if a pointer contains value @nullptr.

    What happens if you tries to use it? The same than if you tries to use a @nullptr
    pointer: a crash!
*/
bool AObject::is_valid() const
{
    if (m_pImpl == nullptr)
        return false;
    ensure_validity();
    return m_pImpl != nullptr;
}


////---------------------------------------------------------------------------------------
// /* * @memberof AObject
//    Default constructor. Creates an empty (invalid) AObject. Useful for methods that
//    returns an AObject when a valid object cannot be returned.
//*/

///@cond INTERNALS

//---------------------------------------------------------------------------------------
AObject::AObject(ImoObj* impl, Document* doc, long imVers)
    : m_pImpl(impl)
    , m_pDoc(doc)
    , m_id(impl->get_id())
    , m_imVersion(imVers)
{
}

//---------------------------------------------------------------------------------------
void AObject::ensure_validity() const
{
    if (!m_pDoc->is_valid_model(m_imVersion))
    {
        m_imVersion = m_pDoc->get_model_ref();
        m_pImpl = static_cast<ImoObj*>(m_pDoc->get_pointer_to_imo(m_id));
    }
}

//---------------------------------------------------------------------------------------
struct AObject::Private
{
    //-----------------------------------------------------------------------------------
    static AObject downcast_content_obj(ImoObj* pImo, Document* pDoc)
    {
        if (pImo == nullptr)
            return AObject();

        if (pImo->is_score())
        {
            ImoScore* pObj = static_cast<ImoScore*>(pImo);
            return AScore(pObj, pDoc, pDoc->get_model_ref());
        }
//        else if (pImo->is_content())
//        {
//            ImoContent* pObj = static_cast<ImoContent*>(pImo);
//            return AContent>(new AContent(pObj, pDoc, pDoc->get_model_ref()) );
//        }
        else if (pImo->is_dynamic())
        {
            ImoDynamic* pObj = static_cast<ImoDynamic*>(pImo);
            return ADynamic(pObj, pDoc, pDoc->get_model_ref());
        }
//        else if (pImo->is_multi_column())
//        {
//            ImoMultiColumn* pObj = static_cast<ImoMultiColumn*>(pImo);
//            return AMultiColumn>(new AMultiColumn(pObj, pDoc, pDoc->get_model_ref()) );
//        }
//        else if (pImo->is_table())
//        {
//            ImoTable* pObj = static_cast<ImoTable*>(pImo);
//            return ATable>(new ATable(pObj, pDoc, pDoc->get_model_ref()) );
//        }
        else if (pImo->is_list())
        {
            ImoList* pObj = static_cast<ImoList*>(pImo);
            return AList(pObj, pDoc, pDoc->get_model_ref());
        }
//        else if (pImo->is_table_row())
//        {
//            ImoTableRow* pObj = static_cast<ImoTableRow*>(pImo);
//            return ATableRow>(new ATableRow(pObj, pDoc, pDoc->get_model_ref()) );
//        }
//        else if (pImo->is_list_item())
//        {
//            ImoListItem* pObj = static_cast<ImoListItem*>(pImo);
//            return AListItem>(new AListItem(pObj, pDoc, pDoc->get_model_ref()) );
//        }
//        else if (pImo->is_table_cell())
//        {
//            ImoTableCell* pObj = static_cast<ImoTableCell*>(pImo);
//            return ATableCell>(new ATableCell(pObj, pDoc, pDoc->get_model_ref()) );
//        }
//        else if (pImo->is_anonimous_block())
//        {
//            ImoAnonymousBlock* pObj = static_cast<ImoAnonymousBlock*>(pImo);
//            return AAnonymousBlock>(new AAnonymousBlock(pObj, pDoc, pDoc->get_model_ref()) );
//        }
        else if (pImo->is_paragraph())
        {
            ImoParagraph* pObj = static_cast<ImoParagraph*>(pImo);
            return AParagraph(pObj, pDoc, pDoc->get_model_ref());
        }
//        else if (pImo->is_heading())
//        {
//            ImoHeading* pObj = static_cast<ImoHeading*>(pImo);
//            return AHeading>(new AHeading(pObj, pDoc, pDoc->get_model_ref()) );
//        }
        else
        {
            return AObject(pImo, pDoc, pDoc->get_model_ref());
        }
    }


    //Document content traversal

    //-----------------------------------------------------------------------------------
    static AObject previous_sibling(ImoObj* pImo, Document* pDoc)
    {
        if (pImo == nullptr)
            return AObject();

        ImoObj* pSibling = pImo->get_prev_sibling();
        if (pSibling)
            return downcast_content_obj(pSibling, pDoc);
        else
            return AObject();
    }

    //-----------------------------------------------------------------------------------
    static AObject next_sibling(ImoObj* pImo, Document* pDoc)
    {
        if (pImo == nullptr)
            return AObject();

        ImoObj* pSibling = pImo->get_next_sibling();
        if (pSibling)
            return downcast_content_obj(pSibling, pDoc);
        else
            return AObject();
    }

};

///@endcond


/// @name Object properties
//@{

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns the internal unique identifier (ID) for this object. It could be required
    by some methods in the libray.
*/
ImoId AObject::object_id() const
{
    return m_id;
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns the name of this object class. It is an string.
*/
const std::string& AObject::object_name() const
{
    ensure_validity();
    return m_pImpl->get_name();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns the ADocument owning this object.
*/
ADocument AObject::owner_document() const
{
    return ADocument(m_pDoc);
}

//@}    //Access to group properties


/** @name Downcast objects
    All API classes derive from base class %AObject. These downcasting methods
    convert the %AObject to the derived class referenced by it.
*/
//@{

///@cond INTERNALS
//protected class, for ADocument
//---------------------------------------------------------------------------------------
AObject AObject::downcast_to_content_obj()
{
    return Private::downcast_content_obj(m_pImpl, m_pDoc);
}
///@endcond


//---------------------------------------------------------------------------------------
/** @memberof AObject
    Downcasts this %AObject to an ADynamic. That is, if this %AObject references an
    ADynamic object, this method returns the referenced ADynamic object. Otherwise,
    it returns an invalid (nullptr) ADynamic.
*/
ADynamic AObject::downcast_to_dynamic() const
{
    ensure_validity();
    if (m_pImpl && m_pImpl->is_dynamic())
    {
        ImoDynamic* pObj = static_cast<ImoDynamic*>(m_pImpl);
        return ADynamic(pObj, m_pDoc, m_imVersion);
    }
    else
        return ADynamic();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Downcasts this %AObject to an AInstrument. That is, if this %AObject references an
    AInstrument object, this method returns the referenced AInstrument object. Otherwise,
    it returns an invalid (nullptr) AInstrument.
*/
AInstrument AObject::downcast_to_instrument() const
{
    ensure_validity();
    if (m_pImpl && m_pImpl->is_instrument())
    {
        ImoInstrument* pObj = static_cast<ImoInstrument*>(m_pImpl);
        return AInstrument(pObj, m_pDoc, m_imVersion);
    }
    else
        return AInstrument();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Downcasts this %AObject to an AInstrument. That is, if this %AObject references an
    AInstrument object, this method returns the referenced AInstrument object. Otherwise,
    it returns an invalid (nullptr) AInstrument.
*/
AInstrGroup AObject::downcast_to_instr_group() const
{
    ensure_validity();
    if (m_pImpl && m_pImpl->is_instr_group())
    {
        ImoInstrGroup* pObj = static_cast<ImoInstrGroup*>(m_pImpl);
        return AInstrGroup(pObj, m_pDoc, m_imVersion);
    }
    else
        return AInstrGroup();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Downcasts this %AObject to an ALink. That is, if this %AObject references an
    ALink object, this method returns the referenced ALink object. Otherwise,
    it returns an invalid (nullptr) ALink.
*/
ALink AObject::downcast_to_link() const
{
    ensure_validity();
    if (m_pImpl && m_pImpl->is_link())
    {
        ImoLink* pObj = static_cast<ImoLink*>(m_pImpl);
        return ALink(pObj, m_pDoc, m_imVersion);
    }
    else
        return ALink();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Downcasts this %AObject to an AList. That is, if this %AObject references an
    AList object, this method returns the referenced AList object. Otherwise,
    it returns an invalid (nullptr) AList.
*/
AList AObject::downcast_to_list() const
{
    ensure_validity();
    if (m_pImpl && m_pImpl->is_list())
    {
        ImoList* pObj = static_cast<ImoList*>(m_pImpl);
        return AList(pObj, m_pDoc, m_imVersion);
    }
    else
        return AList();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Downcasts this %AObject to an AParagraph. That is, if this %AObject references an
    AParagraph object, this method returns the referenced AParagraph object. Otherwise,
    it returns an invalid (nullptr) AParagraph.
*/
AParagraph AObject::downcast_to_paragraph() const
{
    ensure_validity();
    if (m_pImpl && m_pImpl->is_paragraph())
    {
        ImoParagraph* pObj = static_cast<ImoParagraph*>(m_pImpl);
        return AParagraph(pObj, m_pDoc, m_imVersion);
    }
    else
        return AParagraph();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Downcasts this %AObject to an AScore. That is, if this %AObject references an
    AScore object, this method returns the referenced AScore object. Otherwise,
    it returns an invalid (nullptr) AScore.
*/
AScore AObject::downcast_to_score() const
{
    ensure_validity();
    if (m_pImpl && m_pImpl->is_score())
    {
        ImoScore* pObj = static_cast<ImoScore*>(m_pImpl);
        return AScore(pObj, m_pDoc, m_imVersion);
    }
    else
        return AScore();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Downcasts this %AObject to a ATextItem. That is, if this %AObject references an
    ATextItem object, this method returns the referenced ATextItem object. Otherwise,
    it returns an invalid (nullptr) ATextItem.
*/
ATextItem AObject::downcast_to_text_item() const
{
    ensure_validity();
    if (m_pImpl && m_pImpl->is_text_item())
    {
        ImoTextItem* pObj = static_cast<ImoTextItem*>(m_pImpl);
        return ATextItem(pObj, m_pDoc, m_imVersion);
    }
    else
        return ATextItem();
}

//@}    //Downcast objects


/** @name Check referenced object type
    All API classes derive from base class %AObject. These testing methods
    allow to check the type of derived object referenced by this %AObject.
*/
//@{

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is an anonymous block
    (AAnonymousBlock class)
*/
bool AObject::is_anonymous_block() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_anonymous_block();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is an button control
    (AButton class)
*/
bool AObject::is_button() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_button();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is an content block
    (a \<div\> block), represented by the AContent class.
*/
bool AObject::is_content() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_content();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is an control object
    (AControl class)
*/
bool AObject::is_control() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_control();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a dynamically generated
    content block (ADynamic class)
*/
bool AObject::is_dynamic() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_dynamic();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a heading
    (AHeading class)
*/
bool AObject::is_heading() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_heading();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is an image
    (AImage class)
*/
bool AObject::is_image() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_image();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is an generic inline-box
    container (similar to the \<spam\> html element) represented by the
    AInlineWrapper class.
*/
bool AObject::is_inline_wrapper() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_inline_wrapper();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is an score instrument
    (AInstrument class)
*/
bool AObject::is_instrument() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_instrument();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is an score instruments group
    (AInstrGroup class)
*/
bool AObject::is_instr_group() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_instr_group();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is an hyperlink
    (ALink class)
*/
bool AObject::is_link() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_link();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a list
    (AList class)
*/
bool AObject::is_list() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_list();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a list item
    (AListItem class)
*/
bool AObject::is_list_item() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_listitem();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a MIDI properties object
    (AMidiInfo class)
*/
bool AObject::is_midi_info() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_midi_info();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a multi-column container
    object (IMultiColumn class)
*/
bool AObject::is_multicolumn() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_multicolumn();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a container describing the
    musical content for an instrument
    (IMusicData class)
*/
bool AObject::is_music_data() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_music_data();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a paragraph
    (AParagraph class)
*/
bool AObject::is_paragraph() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_paragraph();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a music score
    (AScore class)
*/
bool AObject::is_score() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_score();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a sound properties object
    (ASoundInfo class)
*/
bool AObject::is_sound_info() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_sound_info();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a table
    (ATable class)
*/
bool AObject::is_table() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_table();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a cell for a table
    (ATableCell class)
*/
bool AObject::is_table_cell() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_table_cell();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is the body for a table
    (ATableBody class)
*/
bool AObject::is_table_body() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_table_body();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is the head for a table
    (ATableHead class)
*/
bool AObject::is_table_head() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_table_head();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a row for a table
    (ATableRow class)
*/
bool AObject::is_table_row() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_table_row();
}

//---------------------------------------------------------------------------------------
/** @memberof AObject
    Returns @TRUE if the object referenced by this %AObject is a chunk of text
    (ATextItem class)
*/
bool AObject::is_text_item() const
{
    ensure_validity();
    return const_cast<ImoObj*>(pimpl())->is_text_item();
}

//@}    //Check referenced object type


//---------------------------------------------------------------------------------------
/** @memberof AObject
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoObj* AObject::internal_object() const
{
    return const_cast<ImoObj*>(pimpl());
}



//=======================================================================================
#if (LOMSE_BYPASS_ISSUE_253 == 1)
LOMSE_IMPLEMENT_IM_API_CLASS(AObjectWithSiblings, ImoObj, AObject)

//---------------------------------------------------------------------------------------
AObject AObjectWithSiblings::previous_sibling() const
{
    ensure_validity();
    return AObject::Private::previous_sibling(m_pImpl, m_pDoc);
}

//---------------------------------------------------------------------------------------
AObject AObjectWithSiblings::next_sibling() const
{
    ensure_validity();
    return AObject::Private::next_sibling(m_pImpl, m_pDoc);
}

#else
//=======================================================================================
/** @class ISiblings
    @extends AObject
    ISiblings class provides sibling traversal methods for objects supporting them
*/
LOMSE_IMPLEMENT_IM_API_CLASS(ISiblings, ImoObj, AObject)


/// @name Document content traversal
//@{

//---------------------------------------------------------------------------------------
/** @memberof ISiblings
*/
AObject ISiblings::previous_sibling() const
{
    ensure_validity();
    return AObject::Private::previous_sibling(m_pImpl, m_pDoc);
}

//---------------------------------------------------------------------------------------
/** @memberof ISiblings
*/
AObject ISiblings::next_sibling() const
{
    ensure_validity();
    return AObject::Private::next_sibling(m_pImpl, m_pDoc);
}
#endif      //if (LOMSE_BYPASS_ISSUE_253 == 1)

//@}    //Document content traversal



//=======================================================================================
#if (LOMSE_BYPASS_ISSUE_253 == 1)
LOMSE_IMPLEMENT_IM_API_CLASS(AObjectWithSiblingsAndChildren, ImoObj, AObjectWithSiblings)

//---------------------------------------------------------------------------------------
int AObjectWithSiblingsAndChildren::num_children() const
{
    ensure_validity();
    if (m_pImpl->is_blocks_container())
    {
        ImoBlocksContainer* pBlock = static_cast<ImoBlocksContainer*>(m_pImpl);
        return pBlock->get_num_content_items();
    }
    else if (m_pImpl->is_inlines_container())
    {
        ImoInlinesContainer* pBlock = static_cast<ImoInlinesContainer*>(m_pImpl);
        return pBlock->get_num_items();
    }
    else if (m_pImpl->is_box_inline())
    {
        ImoBoxInline* pBlock = static_cast<ImoBoxInline*>(m_pImpl);
        return pBlock->get_num_items();
    }
    return 0;
}

//---------------------------------------------------------------------------------------
AObject AObjectWithSiblingsAndChildren::child_at(int iItem) const
{
    ensure_validity();
    if (m_pImpl->is_blocks_container())
    {
        ImoBlocksContainer* pBlock = static_cast<ImoBlocksContainer*>(m_pImpl);
        ImoContentObj* pImo = pBlock->get_content_item(iItem);
        if (pImo)
            return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    else if (m_pImpl->is_inlines_container())
    {
        ImoInlinesContainer* pBlock = static_cast<ImoInlinesContainer*>(m_pImpl);
        if (iItem < pBlock->get_num_children())
        {
            ImoObj* pImo = pBlock->get_child(iItem);
            return AObject::Private::downcast_content_obj(pImo, m_pDoc);
        }
    }
    else if (m_pImpl->is_box_inline())
    {
        ImoBoxInline* pBlock = static_cast<ImoBoxInline*>(m_pImpl);
        ImoInlineLevelObj* pImo = pBlock->get_item(iItem);
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    return AObject();
}

//---------------------------------------------------------------------------------------
AObject AObjectWithSiblingsAndChildren::first_child() const
{
    ensure_validity();
    if (m_pImpl->is_blocks_container())
    {
        ImoBlocksContainer* pBlock = static_cast<ImoBlocksContainer*>(m_pImpl);
        ImoContentObj* pImo = pBlock->get_first_content_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    else if (m_pImpl->is_inlines_container())
    {
        ImoInlinesContainer* pBlock = static_cast<ImoInlinesContainer*>(m_pImpl);
        ImoContentObj* pImo = pBlock->get_first_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    else if (m_pImpl->is_box_inline())
    {
        ImoBoxInline* pBlock = static_cast<ImoBoxInline*>(m_pImpl);
        ImoInlineLevelObj* pImo = pBlock->get_first_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    return AObject();
}

//---------------------------------------------------------------------------------------
AObject AObjectWithSiblingsAndChildren::last_child() const
{
    ensure_validity();
    if (m_pImpl->is_blocks_container())
    {
        ImoBlocksContainer* pBlock = static_cast<ImoBlocksContainer*>(m_pImpl);
        ImoContentObj* pImo = pBlock->get_last_content_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    else if (m_pImpl->is_inlines_container())
    {
        ImoInlinesContainer* pBlock = static_cast<ImoInlinesContainer*>(m_pImpl);
        ImoContentObj* pImo = pBlock->get_last_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    else if (m_pImpl->is_box_inline())
    {
        ImoBoxInline* pBlock = static_cast<ImoBoxInline*>(m_pImpl);
        ImoInlineLevelObj* pImo = pBlock->get_last_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    return AObject();
}

#else
//=======================================================================================
/** @class IChildren
    @extends AObject
    Some API clases present a virtual structure similar to a tree. For
    instance, AParagraph class can be seen as a container for text with different styles.
    Thus, the following LDP source code:
    @code
        (para (txt (style bold) "Hello") (txt " world! ")(txt "It is a nice day!"))
    @endcode

    will produce the following structure of API classes:

    @verbatim
                                        AParagraph
                                            |
                       +--------------------+--------------------+
                       |                    |                    |
                   ATextItem (bold)     ATextItem (normal)   ATextItem (normal)
                   "Hello"              " world! "           "It is a nice day!"
    @endverbatim

    For objects, such as AParagraph, that organizes its content in a tree, class
    %IChildren provides the methods for child content traversal. %IChildren is just an
    interface class and thus, you will never directly manage IChildren objects but
    objects derived from this class.
*/
LOMSE_IMPLEMENT_IM_API_CLASS(IChildren, ImoObj, AObject)


/// @name Document content traversal
//@{

//---------------------------------------------------------------------------------------
/** @memberof IChildren
*/
int IChildren::num_children() const
{
    ensure_validity();
    if (m_pImpl->is_blocks_container())
    {
        ImoBlocksContainer* pBlock = static_cast<ImoBlocksContainer*>(m_pImpl);
        return pBlock->get_num_content_items();
    }
    else if (m_pImpl->is_inlines_container())
    {
        ImoInlinesContainer* pBlock = static_cast<ImoInlinesContainer*>(m_pImpl);
        return pBlock->get_num_items();
    }
    else if (m_pImpl->is_box_inline())
    {
        ImoBoxInline* pBlock = static_cast<ImoBoxInline*>(m_pImpl);
        return pBlock->get_num_items();
    }
    return 0;
}

//---------------------------------------------------------------------------------------
/** @memberof IChildren
*/
AObject IChildren::child_at(int iItem) const
{
    ensure_validity();
    if (m_pImpl->is_blocks_container())
    {
        ImoBlocksContainer* pBlock = static_cast<ImoBlocksContainer*>(m_pImpl);
        ImoContentObj* pImo = pBlock->get_content_item(iItem);
        if (pImo)
            return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    else if (m_pImpl->is_inlines_container())
    {
        ImoInlinesContainer* pBlock = static_cast<ImoInlinesContainer*>(m_pImpl);
        if (iItem < pBlock->get_num_children())
        {
            ImoObj* pImo = pBlock->get_child(iItem);
            return AObject::Private::downcast_content_obj(pImo, m_pDoc);
        }
    }
    else if (m_pImpl->is_box_inline())
    {
        ImoBoxInline* pBlock = static_cast<ImoBoxInline*>(m_pImpl);
        ImoInlineLevelObj* pImo = pBlock->get_item(iItem);
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    return AObject();
}

//---------------------------------------------------------------------------------------
/** @memberof IChildren
*/
AObject IChildren::first_child() const
{
    ensure_validity();
    if (m_pImpl->is_blocks_container())
    {
        ImoBlocksContainer* pBlock = static_cast<ImoBlocksContainer*>(m_pImpl);
        ImoContentObj* pImo = pBlock->get_first_content_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    else if (m_pImpl->is_inlines_container())
    {
        ImoInlinesContainer* pBlock = static_cast<ImoInlinesContainer*>(m_pImpl);
        ImoContentObj* pImo = pBlock->get_first_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    else if (m_pImpl->is_box_inline())
    {
        ImoBoxInline* pBlock = static_cast<ImoBoxInline*>(m_pImpl);
        ImoInlineLevelObj* pImo = pBlock->get_first_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    return AObject();
}

//---------------------------------------------------------------------------------------
/** @memberof IChildren
*/
AObject IChildren::last_child() const
{
    ensure_validity();
    if (m_pImpl->is_blocks_container())
    {
        ImoBlocksContainer* pBlock = static_cast<ImoBlocksContainer*>(m_pImpl);
        ImoContentObj* pImo = pBlock->get_last_content_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    else if (m_pImpl->is_inlines_container())
    {
        ImoInlinesContainer* pBlock = static_cast<ImoInlinesContainer*>(m_pImpl);
        ImoContentObj* pImo = pBlock->get_last_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    else if (m_pImpl->is_box_inline())
    {
        ImoBoxInline* pBlock = static_cast<ImoBoxInline*>(m_pImpl);
        ImoInlineLevelObj* pImo = pBlock->get_last_item();
        return AObject::Private::downcast_content_obj(pImo, m_pDoc);
    }
    return AObject();
}
#endif      //if (LOMSE_BYPASS_ISSUE_253 == 1)

//@}    //Document content traversal



//=======================================================================================
/** @class ADynamic
    @extends AObject
    @extends ISiblings
    @extends IChildren
    %ADynamic represents external content that is injected dynamically into the document
    by the user application. It is equivalent to the HTML \<object\> element.

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/
#if (LOMSE_BYPASS_ISSUE_253 == 1)
LOMSE_IMPLEMENT_IM_API_CLASS(ADynamic, ImoDynamic, AObjectWithSiblingsAndChildren)
#else
LOMSE_IMPLEMENT_IM_API_CLASS(ADynamic, ImoDynamic, AObject)
#endif

//---------------------------------------------------------------------------------------
/** @memberof ADynamic
    Returns the value of the <i>classid</i> attribute. It defines the type of dynamic
    content that this object represents.
*/
std::string& ADynamic::classid()
{
    ensure_validity();
    return const_cast<ImoDynamic*>(pimpl())->get_classid();
}

//---------------------------------------------------------------------------------------
/** @memberof ADynamic
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoDynamic* ADynamic::internal_object() const
{
    return const_cast<ImoDynamic*>(pimpl());
}



//=======================================================================================
/** @class AInstrument
    @extends AObject
    %AInstrument represents an instrument on the score. For lomse, an instrument refers
    to a physical musical instrument, such as a violin, a flute, or a piano. It is
    represented by one or more staves and is modeled as the collection of all aspects
    pertaining to the visual display of the staff/staves as they appear on the printed
    page (name, transposition, musical content, etc) as well as the required information
    for audio playback (e.g. MIDI channel, MIDI program, etc.).

    But there are cases in which the staves for an instrument contain the music for
    more than one real instrument. For instance the bass and tenor voices in a choral
    can be placed in a single staff, or the different percussion instruments can share
    an staff.

    In theory, sharing  the same staff between several instruments should be forbidden,
    and each instrument should be modeled in its own staff. And for presentation, the
    user should be able to decide which instruments are going to form an score part,
    and how that part should be displayed: all instruments merged in a single staff or
    other layout. But for compatibility with MusicXML, lomse allows that several
    instruments share the same staves. As a consequence, a lomse instrument could
    represent several real instruments and, thus, could have several sound information
    objects (ASoundInfo objects) each of them containing the sound information for each
    real physical instrument that is sharing the staff.

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/
LOMSE_IMPLEMENT_IM_API_CLASS(AInstrument, ImoInstrument, AObject)


/// @name Name and abbreviation
//@{

//---------------------------------------------------------------------------------------
/** @memberof AInstrument
    Returns the name of the instrument, that is the string that is placed at the start of
    the first system in the score.
*/
std::string& AInstrument::name_string() const
{
    ensure_validity();
    ImoScoreText& text = const_cast<ImoInstrument*>(pimpl())->get_name();
    return text.get_text();
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrument
    Returns the short, abbreviated name that appears on every system other than
    the first system.
*/
std::string& AInstrument::abbreviation_string() const
{
    ensure_validity();
    ImoScoreText& text = const_cast<ImoInstrument*>(pimpl())->get_abbrev();
    return text.get_text();
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrument
    Sets the name of the instrument, that is the string that is placed at the start of
    the first system in the score.

    It’s important to note that changing the name of an instrument doesn’t actually
    change any other instrument properties, such as transposition information or
    number of staves. For example, renaming "Flute" to "Piano" does not creates a
    second staff for the instrument. Or renaming  “Trumpets in Bb” to “Trumpets in C”
    does not change the transposition information.
*/
void AInstrument::set_name_string(const string& name)
{
    ensure_validity();
    pimpl()->set_name(name);
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrument
    Sets the short, abbreviated name that appears on every system other than
    the first system.
*/
void AInstrument::set_abbreviation_string(const string& abbrev)
{
    ensure_validity();
    pimpl()->set_abbrev(abbrev);
}

//@}    //Name and abbreviation


/// @name Sound information
//@{

//---------------------------------------------------------------------------------------
/** @memberof AInstrument
    Returns the number of ASoundInfo elements contained in this instrument, at least one.

    An %AInstruments always have at least one sound, represented by an ASoundInfo object.
    But there are cases in which the staves for an instrument contain the music for
    more than one real instrument. For instance the bass and tenor voices in a choral
    can be placed in a single staff, or the different percussion instruments can share
    an staff. In these cases in which a lomse instrument represents several real
    instruments there is aISoundInfo object for each real physical instrument that is
    sharing the staff.

    Method AInstrument::num_sounds() informs about the number of ASoundInfo objects
    that this instrument contains. Always at least one.
*/
int AInstrument::num_sounds() const
{
    ensure_validity();
    return const_cast<ImoInstrument*>(pimpl())->get_num_sounds();
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrument
    Returns the ASoundInfo object at a given position in the collection of sounds
    for this instrument.

    In cases in which a lomse instrument represents several real
    instruments there is a collection of ASoundInfo objects, one for each real physical
    instrument that is sharing the staff.

    AInstrument::sound_info_at(0) is always valid and returns the only ASoundInfo
    object for normal cases and the first ASoundInfo object when several real instruments
    are represented by a single AInstrument object.

    @param iSound   The index (0..n-1) to the requested sound info object.

*/
ASoundInfo AInstrument::sound_info_at(int iSound) const
{
    ensure_validity();
    return ASoundInfo(const_cast<ImoInstrument*>(pimpl())->get_sound_info(iSound),
                      m_pDoc, m_imVersion);
}

//@}    //Sound information


//---------------------------------------------------------------------------------------
/** @memberof AInstrument
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoInstrument* AInstrument::internal_object() const
{
    return const_cast<ImoInstrument*>(pimpl());
}


//=======================================================================================
/** @class AInstrGroup
    @extends AObject
    %AInstrGroup is the API object for interacting with the internal model Bla,bla bla...

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/
LOMSE_IMPLEMENT_IM_API_CLASS(AInstrGroup, ImoInstrGroup, AObject)

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoInstrGroup* AInstrGroup::internal_object() const
{
    return const_cast<ImoInstrGroup*>(pimpl());
}


/// @name Access to group properties
//@{

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Returns a value from enum EJoinBarlines indicating how the barlines for the
    instruments in the group will be displayed.
*/
EJoinBarlines AInstrGroup::barlines_mode() const
{
    ensure_validity();
    return static_cast<EJoinBarlines>(
        const_cast<ImoInstrGroup*>(pimpl())->join_barlines() );
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Returns a value from enum EGroupSymbol indicating what symbol will be displayed for
    marking the instruments that form the group.
*/
EGroupSymbol AInstrGroup::symbol() const
{
    ensure_validity();
    return static_cast<EGroupSymbol>(
        const_cast<ImoInstrGroup*>(pimpl())->get_symbol() );
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Returns the name of the group, that is the string for the group that is placed at
    the start of the first system in the score.
*/
const std::string& AInstrGroup::name_string() const
{
    ensure_validity();
    ImoScoreText& text = const_cast<ImoInstrGroup*>(pimpl())->get_name();
    return text.get_text();
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Returns the short, abbreviated name of the group. This is the string for the group
    that appears on every system other than the first system.
*/
const std::string& AInstrGroup::abbreviation_string() const
{
    ensure_validity();
    ImoScoreText& text = const_cast<ImoInstrGroup*>(pimpl())->get_abbrev();
    return text.get_text();
}

//@}    //Access to group properties


/// @name Group properties modification
//@{

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Sets the symbol that will be displayed for marking the instruments that form the
    group. Must be a value from enum EGroupSymbol.
*/
void AInstrGroup::set_symbol(EGroupSymbol symbol)
{
    ensure_validity();
    pimpl()->set_symbol(symbol);
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Defines how the barlines for the instruments in the group will be displayed.
    Parameter must be a value from enum EJoinBarlines.
*/
void AInstrGroup::set_barlines_mode(EJoinBarlines value)
{
    ensure_validity();
    pimpl()->set_join_barlines(value);
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Sets the name of the group, that is the string for the group that is placed at the
    start of the first system in the score.
*/
void AInstrGroup::set_name_string(const std::string& name)
{
    ensure_validity();
    pimpl()->set_name(name);
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Sets the short, abbreviated name for the group that appears on every system other
    than the first one.
*/
void AInstrGroup::set_abbreviation_string(const std::string& abbrev)
{
    ensure_validity();
    pimpl()->set_abbrev(abbrev);
}

//@}    //Group properties modification


/// @name Instruments in the group
//@{

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Returns the number of instruments included in the group.
*/
int AInstrGroup::num_instruments() const
{
    ensure_validity();
    return const_cast<ImoInstrGroup*>(pimpl())->get_num_instruments();
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Returns the instrument at position <i>pos</i> in the group. First instrument in the
    group is position 0.
*/
AInstrument AInstrGroup::instrument_at(int pos) const
{
    ensure_validity();
    ImoInstrGroup* pGrp = const_cast<ImoInstrGroup*>(pimpl());
    if (pos < 0 || pos >= pGrp->get_num_instruments())
        return AInstrument();

    return AInstrument(pGrp->get_instrument(pos), m_pDoc, m_imVersion);
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Returns the first instrument included in the group.
*/
AInstrument AInstrGroup::first_instrument() const
{
    ensure_validity();
    return AInstrument(const_cast<ImoInstrGroup*>(pimpl())->get_first_instrument(),
                       m_pDoc, m_imVersion);
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Returns the last instrument included in the group.
*/
AInstrument AInstrGroup::last_instrument() const
{
    ensure_validity();
    return AInstrument(const_cast<ImoInstrGroup*>(pimpl())->get_last_instrument(),
                       m_pDoc, m_imVersion);
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Returns the index, referred to the score, of the first instrument included in the
    group. The returned index is the position occupied by this instrument in the score
    (0 based: 0 .. num.instrs - 1)
*/
int AInstrGroup::index_to_first_instrument() const
{
    ensure_validity();
    return const_cast<ImoInstrGroup*>(pimpl())->get_index_to_first_instrument();
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Returns the index, referred to the score, of the last instrument included in the
    group. The returned index is the position occupied by this instrument in the score
    (0 based: 0 .. num.instrs - 1)
*/
int AInstrGroup::index_to_last_instrument() const
{
    ensure_validity();
    return const_cast<ImoInstrGroup*>(pimpl())->get_index_to_last_instrument();
}

//---------------------------------------------------------------------------------------
/** @memberof AInstrGroup
    Defines the instruments that will be included in the group. Returns @FALSE if
    any error.
    @param iFirstInstr  Position in the score of the first instrument to be included in
        the group: 0 .. num.instrs - 1
    @param iLastInstr  Position in the score of the last instrument to be included in
        the group: 0 .. num.instrs - 1

    Notice that <i>iFirstInstr</i> must be lower than <i>iLastInstr</i>. Otherwise
    this method will do nothing.
*/
bool AInstrGroup::set_range(int iFirstInstr, int iLastInstr)
{
    ensure_validity();
    ImoScore* pScore = pimpl()->get_score();
    int maxInstr = pScore->get_num_instruments();
    if ((iFirstInstr >= 0 && iFirstInstr < maxInstr)
        && (iLastInstr > iFirstInstr && iLastInstr < maxInstr) )
    {
        pimpl()->set_range(iFirstInstr, iLastInstr);
        return true;
    }
    else
    {
        return false;
    }
}

//@}    //Instruments in the group



//=======================================================================================
/** @class ALink
    @extends AObject
    @extends ISiblings
    @extends IChildren
    %ALink is a container for inline objects, and reprensents a clickable 'link'
    object that creates hyperlinks. It is similar to the HTML \<a\> element.

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/
#if (LOMSE_BYPASS_ISSUE_253 == 1)
LOMSE_IMPLEMENT_IM_API_CLASS(ALink, ImoLink, AObjectWithSiblingsAndChildren)
#else
LOMSE_IMPLEMENT_IM_API_CLASS(ALink, ImoLink, AObject)
#endif

//---------------------------------------------------------------------------------------
/** @memberof ALink
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoLink* ALink::internal_object() const
{
    return const_cast<ImoLink*>(pimpl());
}



//=======================================================================================
/** @class AList
    @extends AObject
    @extends ISiblings
    @extends IChildren
    %AList represents a list of items and it is a container for AListItem objects.
    It is equivalent to the HTML \<ol\> and \<ul\> elements. The type of list, ordered
    or unordered, is an attribute of the AList object.

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/
#if (LOMSE_BYPASS_ISSUE_253 == 1)
LOMSE_IMPLEMENT_IM_API_CLASS(AList, ImoList, AObjectWithSiblingsAndChildren)
#else
LOMSE_IMPLEMENT_IM_API_CLASS(AList, ImoList, AObject)
#endif

//---------------------------------------------------------------------------------------
/** @memberof AList
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoList* AList::internal_object() const
{
    return const_cast<ImoList*>(pimpl());
}



//=======================================================================================
/** @class AMidiInfo
    @extends AObject
    %AMidiInfo provides access to the MIDI information associated to a ASoundInfo object
    for an instrument. MIDI info always exists in the ASoundInfo object.
    By default, when no MIDI information is provided in the source file or when
    programatically building a score, MIDI information is initialized as follows:

        device name = ""
        program name = ""
        bank = 1
        port = 0        port 0 is invalid. Means "not initialized"
        channel = 0     channel 0 is invalid. Means "not initialized"
        program = 1
        unpitched = 0
        volume = 1.0    maximum volume
        pan = 0         in front, centered
        elevation = 0   at listener head level

    When MIDI information has been specified when building the score, notice that
    'port', 'channel' or both could have not been specified and, thus, any of them can
    still in "not initialized". In these cases, port and channel are automatically
    assigned right values when the score is finished, that is, when invoking
    AScore::end_of_changes() method. The algorithm, in method
    MidiAssigner::assign_port_and_channel(), ensures that
    each instrument is assigned a unique combination (port, channel) for all instruments
    with port or channel containig the "not initialized" value.

    Important. Please notice that currently, lomse sound API for playback generate play
    events that only uses the MIDI channel, program and volume information. All other
    MIDI information, such as bank, port or elevation is currently ignored.
*/
LOMSE_IMPLEMENT_IM_API_CLASS(AMidiInfo, ImoMidiInfo, AObject)

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoMidiInfo* AMidiInfo::internal_object() const
{
    return const_cast<ImoMidiInfo*>(pimpl());
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Returns the MIDI port assigned to this sound.
    It is a number from 1 to 16 that can be used with the unofficial MIDI port
    (or cable) meta event.
*/
int AMidiInfo::port() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_port();
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Returns the MIDI device name assigned to this sound.
    It will be used in the DeviceName meta-event when exporting the score as a
    Standard MIDI File (not yet implemented).
*/
std::string& AMidiInfo::device_name() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_device_name();
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Returns the MIDI program name assigned to this sound.
    It will be used in the ProgramName meta-events when exporting the score as
    a Standard MIDI File (not yet implemented).
*/
std::string& AMidiInfo::program_name() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_name();
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Returns the MIDI bank assigned to this sound. MIDI 1.0 bank numbers range
    from 1 to 16,384.
*/
int AMidiInfo::bank() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_bank() + 1;
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Returns the MIDI channel assigned to this sound. MIDI 1.0 channel numbers range
    from 1 to 16.
*/
int AMidiInfo::channel() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_channel() + 1;
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Returns the MIDI program number assigned to this sound. MIDI 1.0 program numbers
    range from 1 to 128.
*/
int AMidiInfo::program() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_program() + 1;
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Returns the unpitched note number assigned to this sound. It is only meaningfull
    for unpitched instruments and, for them, it specifies a MIDI 1.0 note number
    ranging from 0 to 127. It is usually used with MIDI banks for percussion.
*/
int AMidiInfo::unpitched() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_unpitched();
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Returns the MIDI volume assigned to this sound.
    The volume value is a percentage of the maximum, ranging from 0.0 to 1.0, with
    decimal values allowed.
    This corresponds to a scaling value for the MIDI 1.0
    channel volume controller.
*/
float AMidiInfo::volume() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_volume();
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Returns the MIDI pan value assigned to this sound.
    Pan and elevation allow placing of sound in a 3-D space relative to the listener.
    Pan refers to the horizontal position around the listener, expressed
    in degrees, ranging from -180 to 180. Some values:
          0 is straight ahead, in front of the listener, centered.
        -90 is hard left, 90 is hard right, and
       -180 or 180 are directly behind the listener, centered.
*/
int AMidiInfo::pan() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_pan();
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Returns the MIDI elevation value assigned to this sound.
    Pan and elevation allow placing of sound in a 3-D space relative to the listener.
    Elevation refers to the vertical position around the listener, expressed
    in degrees, ranging from -180 to 180. Some values:
         0 is level with the listener head,
        90 is directly above, and -90 is directly below.
*/
int AMidiInfo::elevation() const
{
    ensure_validity();
    return const_cast<ImoMidiInfo*>(pimpl())->get_midi_elevation();
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Sets the MIDI port to use for this sound.
    It is a number from 1 to 16 that can be used with the unofficial MIDI port
    (or cable) meta event.
*/
void AMidiInfo::set_port(int value)
{
    ensure_validity();
    pimpl()->set_midi_port(value - 1);
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Sets the MIDI device name to use for this sound.
    It will be used in the DeviceName meta-event when exporting the score as a
    Standard MIDI File (not yet implemented).
*/
void AMidiInfo::set_device_name(const std::string& value)
{
    ensure_validity();
    pimpl()->set_midi_device_name(value);
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Sets the MIDI program name to use for this sound.
    It will be used in the ProgramName meta-events when exporting the score as
    a Standard MIDI File (not yet implemented).
*/
void AMidiInfo::set_program_name(const std::string& value)
{
    ensure_validity();
    pimpl()->set_midi_name(value);
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Sets the MIDI bank to use for this sound. MIDI 1.0 bank numbers range
    from 1 to 16,384.
*/
void AMidiInfo::set_bank(int value)
{
    ensure_validity();
    pimpl()->set_midi_bank(value - 1);
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Sets the MIDI channel to use for this sound. MIDI 1.0 channel numbers range
    from 1 to 16.
*/
void AMidiInfo::set_channel(int value)
{
    ensure_validity();
    pimpl()->set_midi_channel(value - 1);
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Sets the MIDI program number to use for this sound. MIDI 1.0 program numbers
    range from 1 to 128.
*/
void AMidiInfo::set_program(int value)
{
    ensure_validity();
    pimpl()->set_midi_program(value - 1);
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Sets the unpitched note number to use for this sound. It is only meaningfull
    for unpitched instruments and, for them, it specifies a MIDI 1.0 note number
    ranging from 0 to 127. It is usually used with MIDI banks for percussion.
*/
void AMidiInfo::set_unpitched(int value)
{
    ensure_validity();
    pimpl()->set_midi_unpitched(value);
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Sets the MIDI volume to use for this sound.
    The volume value is a percentage of the maximum, ranging from 0.0 to 1.0, with
    decimal values allowed.
    This corresponds to a scaling value for the MIDI 1.0
    channel volume controller.
*/
void AMidiInfo::set_volume(float value)
{
    ensure_validity();
    pimpl()->set_midi_volume(value);
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Sets the MIDI pan value to use for this sound.
    Pan and elevation allow placing of sound in a 3-D space relative to the listener.
    Pan refers to the horizontal position around the listener, expressed
    in degrees, ranging from -180 to 180. Some values:
          0 is straight ahead, in front of the listener, centered.
        -90 is hard left, 90 is hard right, and
       -180 or 180 are directly behind the listener, centered.
*/
void AMidiInfo::set_pan(int value)
{
    ensure_validity();
    pimpl()->set_midi_pan(value);
}

//---------------------------------------------------------------------------------------
/** @memberof AMidiInfo
    Sets the MIDI pan value to use for this sound.
    Pan and elevation allow placing of sound in a 3-D space relative to the listener.
    Elevation refers to the vertical position around the listener, expressed
    in degrees, ranging from -180 to 180. Some values:
         0 is level with the listener head,
        90 is directly above, and -90 is directly below.
*/
void AMidiInfo::set_elevation(int value)
{
    ensure_validity();
    pimpl()->set_midi_elevation(value);
}



//=======================================================================================
/** @class AParagraph
    @extends AObject
    @extends IChildren
    @extends ISiblings

    %AParagraph represents a paragraph. It is a block-level container, similar to the
    HTML <p> element. Paragraphs are usually blocks of text separated from adjacent
    blocks by blank lines and/or first-line indentation, but appart from text, a
    paragraph can also contain other elements, e.g. images (AImage), links (ALink),
    buttons (AButton), etc.

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/
#if (LOMSE_BYPASS_ISSUE_253 == 1)
LOMSE_IMPLEMENT_IM_API_CLASS(AParagraph, ImoParagraph, AObjectWithSiblingsAndChildren)
#else
LOMSE_IMPLEMENT_IM_API_CLASS(AParagraph, ImoParagraph, AObject)
#endif



//=======================================================================================
/** @class AScore
    @extends AObject
    @extends ISiblings

    %AScore represents a full music score, that is, an object comprising all of the
    music for all of the players and their instruments, typically laid out in a specific
    order. In lomse, an score is, basically, a collection instruments (AInstrument
    objects) and some information common to all them, such as score titles.

    In a full score, related instruments are usually grouped by sharing barlines and
    having some visual clues, such as a brace or bracket, and a group name. In lomse, a
    group of instruments is represented by an AInstrGroup object, and the AScore object
    is also responsible for managing the collection of all defined instrument groups.

    See @ref api-internal-model-scores.

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/
#if (LOMSE_BYPASS_ISSUE_253 == 1)
LOMSE_IMPLEMENT_IM_API_CLASS(AScore, ImoScore, AObjectWithSiblings)
#else
LOMSE_IMPLEMENT_IM_API_CLASS(AScore, ImoScore, AObject)
#endif

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoScore* AScore::internal_object() const
{
    return const_cast<ImoScore*>(pimpl());
}

//exclude from API documentation all private members
///@cond INTERNALS

struct AScore::Private
{
    //-----------------------------------------------------------------------------------
    static void delete_instrument(ImoScore* pScore, ImoInstrument* pInstr)
    {
        //remove the instrument from the score
        ImoInstruments* pColInstr = pScore->get_instruments();
        pColInstr->remove_child_imo(pInstr);    //AWARE: this does not delete ImoInstrument object

        //trim groups if necessary, and delete any group containing only one instrument
        int numInstrs = pScore->get_num_instruments();
        ImoInstrGroups* pGroups = pScore->get_instrument_groups();
        if (pGroups)
        {
            list<ImoInstrGroup*> toDelete;
            ImoObj::children_iterator itG;
            for (itG= pGroups->begin(); itG != pGroups->end(); ++itG)
            {
                ImoInstrGroup* pGroup = static_cast<ImoInstrGroup*>(*itG);
                int iFirst = pGroup->get_index_to_first_instrument();
                if ((iFirst + pGroup->get_num_instruments()) > numInstrs)
                    pGroup->set_range(iFirst, numInstrs-1);

                if (pGroup->get_num_instruments() < 2)
                    toDelete.push_back(pGroup);
            }

            for (auto group : toDelete)
            {
                pGroups->remove_child_imo(group);
                delete group;
            }

            //If no groups left, remove ImoInstrGroups child
            if (pGroups->get_num_items() == 0)
            {
                pScore->remove_child_imo(pGroups);
                delete pGroups;
            }
        }

        //finally, delete the instrument object
        delete pInstr;
    }

    //-----------------------------------------------------------------------------------
    static void move_up_instrument(ImoScore* pScore, ImoInstrument* pInstr)
    {
        ImoInstrument* pInstrPrev = static_cast<ImoInstrument*>(pInstr->get_prev_sibling());
        if (pInstrPrev == nullptr)
            return;

        //remove the instrument from the score and insert it before the previous instr.
        ImoInstruments* pColInstr = pScore->get_instruments();
        pColInstr->remove_child_imo(pInstr);    //AWARE: this does not delete ImoInstrument object
        pColInstr->insert(pInstrPrev, pInstr);
    }

    //-----------------------------------------------------------------------------------
    static void move_down_instrument(ImoScore* pScore, ImoInstrument* pInstr)
    {
        ImoInstrument* pInstrNext = static_cast<ImoInstrument*>(pInstr->get_next_sibling());
        if (pInstrNext == nullptr)
            return;

        //remove the instrument from the score and insert it after the next one
        ImoInstruments* pColInstr = pScore->get_instruments();
        pColInstr->remove_child_imo(pInstr);    //AWARE: this does not delete ImoInstrument object
        ImoInstrument* pAt = static_cast<ImoInstrument*>(pInstrNext->get_next_sibling());
        if (pAt)
            pColInstr->insert(pAt, pInstr);
        else
            pColInstr->append_child(pInstr);
    }

};

///@endcond

/// @name Instruments management. Access to information
//@{

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Returns the requested instrument.
    @param iInstr    Is the index to the requested instrument (0 ... num_instruments - 1).
*/
AInstrument AScore::instrument_at(int iInstr) const
{
    ensure_validity();
    ImoInstrument* pInstr = const_cast<ImoScore*>(pimpl())->get_instrument(iInstr);
    return AInstrument(pInstr, m_pDoc, m_imVersion);
}

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Returns the number of instruments that this score contains.
*/
int AScore::num_instruments() const
{
    ensure_validity();
    return const_cast<ImoScore*>(pimpl())->get_num_instruments();
}


//@}    //Instruments management. Access to information


/// @name Instruments management. Add/delete instruments
//@{

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Appends a new empty instrument to the score, instantiated with default values:
    - One empty staff, staff size 7.2 mm (rastral size between 2 (7.4mm) and 3 (7.0mm)).
    - No name, no abbreviation
    - Not included in any existing instruments group.
    - Has default MIDI info (see AMidiInfo for default values)

    Returns the created instrument.
*/
AInstrument AScore::append_new_instrument()
{
    ensure_validity();
    ImoInstrument* pInstr = pimpl()->add_instrument();

    PartIdAssigner assigner;
    assigner.assign_parts_id(pimpl());

    return AInstrument(pInstr, m_pDoc, m_imVersion);
}

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Removes the specified instrument form the score and deletes it.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a delete instrument operation. A remove operation
    will not affect existing groups unless the group contained the last instrument in
    the score, as that instrument position will no longer exist after deleting an
    instrument. In that case, the group will have one less instrument or will be deleted
    if only containing one instrument.

    @param instrId    The ID of the instrument to delete.
*/
void AScore::delete_instrument(ImoId instrId)
{
    ensure_validity();
    ImoInstrument* pInstr = static_cast<ImoInstrument*>(m_pDoc->get_pointer_to_imo(instrId));
    AScore::Private::delete_instrument(pimpl(), pInstr);
}

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Removes the specified instrument form the score and deletes it.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a delete instrument operation. A remove operation
    will not affect existing groups unless the group contained the last instrument in
    the score, as that instrument position will no longer exist after deleting an
    instrument. In that case, the group will have one less instrument or will be deleted
    if only containing one instrument.

    @param instr    A reference to the instrument to delete.
*/
void AScore::delete_instrument(AInstrument& instr)
{
    ensure_validity();
    ImoInstrument* pInstr = instr.pimpl();
    AScore::Private::delete_instrument(pimpl(), pInstr);
}


//@}    //Instruments management. Add/delete instruments


/// @name Instruments management. Reordering instruments
//@{

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Changes the order of the instruments in the score by moving the referenced
    instrument before the one immediately above it.

    @param instrId    The ID of the instrument to move up.

    Trying to move up the first instrument is a valid operation but nothing
    will be changed.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a move operation.

    @see move_down_instrument()
*/
void AScore::move_up_instrument(ImoId instrId)
{
    ensure_validity();
    ImoInstrument* pInstr = static_cast<ImoInstrument*>(m_pDoc->get_pointer_to_imo(instrId));
    AScore::Private::move_up_instrument(pimpl(), pInstr);
}

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Changes the order of the instruments in the score by moving the referenced
    instrument before the one immediately above it.

    @param instr    A reference to the instrument to move up.

    Trying to move up the first instrument is a valid operation but nothing
    will be changed.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a move operation.

    @see move_down_instrument()
*/
void AScore::move_up_instrument(AInstrument& instr)
{
    ensure_validity();
    ImoInstrument* pInstr = instr.pimpl();
    AScore::Private::move_up_instrument(pimpl(), pInstr);
}

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Changes the order of the instruments in the score by moving the referenced
    instrument after the one immediately below it.

    @param instrId    The ID of the instrument to move down.

    Trying to move down the last instrument is a valid operation but nothing
    will be changed.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a move operation.

    @see move_up_instrument()
*/
void AScore::move_down_instrument(ImoId instrId)
{
    ensure_validity();
    ImoInstrument* pInstr = static_cast<ImoInstrument*>(m_pDoc->get_pointer_to_imo(instrId));
    AScore::Private::move_down_instrument(pimpl(), pInstr);
}

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Changes the order of the instruments in the score by moving the referenced
    instrument after the one immediately below it.

    @param instr    A reference to the instrument to move down.

    Trying to move down the last instrument is a valid operation but nothing
    will be changed.

    Instrument groups in the score are not affected, as they are not tied to specific
    instruments but to instruments positions. That is, a group joining instruments two to
    five will always join the instruments occupying those positions, whatever they
    are there after a move operation.

    @see move_up_instrument()
*/
void AScore::move_down_instrument(AInstrument& instr)
{
    ensure_validity();
    ImoInstrument* pInstr = instr.pimpl();
    AScore::Private::move_down_instrument(pimpl(), pInstr);
}


//@}    //Instruments management. Reordering instruments


/// @name Groups management. Access to information
//@{

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Returns the number of instrument groups that this score contains.
*/
int AScore::num_instruments_groups() const
{
    ensure_validity();
    ImoInstrGroups* pGroups = const_cast<ImoScore*>(pimpl())->get_instrument_groups();
    if (!pGroups)
        return 0;

    return pGroups->get_num_children();
}

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Returns the requested instruments group.
    @param iGroup   Is the index to the requested instruments
                    group (0 ... num.groups - 1).
*/
AInstrGroup AScore::instruments_group_at(int iGroup) const
{
    ensure_validity();
    ImoInstrGroups* pGroups = const_cast<ImoScore*>(pimpl())->get_instrument_groups();
    if (!pGroups)
        return AInstrGroup();

    ImoObj::children_iterator itG;
    int i = 0;
    for (itG= pGroups->begin(); itG != pGroups->end() && i < iGroup; ++itG, ++i);
    if (i == iGroup && itG != pGroups->end())
    {
        ImoInstrGroup* pGroup = static_cast<ImoInstrGroup*>(*itG);
        return AInstrGroup(pGroup, m_pDoc, m_imVersion);
    }
    else
        return AInstrGroup();
}


//@}    //Groups management. Access to information


/// @name Groups management. Create/remove groups
//@{

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Creates a group whose first instrument will be instrument at position
    <i>iFirstInstr</i> and the last in the group will be instrument at position
    <i>iLastInstr</i>.

    @param iFirstInstr    Position (0..n-1) of first instrument to include in the group.

    @param iLastInstr     Position (0..n-1) of last instrument to include in the group.
        Notice that <i>iFirstInstr</i> must be lower than <i>iLastInstr</i>. Otherwise
        this method will do nothing and will return an invalid AInstrGroup.

    The created group will not have any symbol (k_group_symbol_none),
    the instruments will have their barlines joined (k_joined_barlines), and the
    group will have neither name nor abbreviation. After the group is created,
    you can change all these default settings.
*/
AInstrGroup AScore::group_instruments(int iFirstInstr, int iLastInstr)
{
    ensure_validity();
    int maxInstr = pimpl()->get_num_instruments();
    if ((iFirstInstr >= 0 && iFirstInstr < maxInstr)
        && (iLastInstr > iFirstInstr && iLastInstr < maxInstr) )
    {
        ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>(
                                        ImFactory::inject(k_imo_instr_group, m_pDoc));

        pGrp->set_owner_score(pimpl());
        pGrp->set_range(iFirstInstr, iLastInstr);
        pimpl()->add_instruments_group(pGrp);
        return AInstrGroup(pGrp, m_pDoc, m_imVersion);
    }
    else
    {
        return AInstrGroup();
    }
}

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Deletes the instruments group at position <i>iGroup</i>.
    Returns @FALSE if error (invalid value for param iGroup).

    @param iGroup    Position (0..num.groups-1) of group to delete. Please, notice
        that groups are ordered as they are created: first created one has index 0.
*/
bool AScore::delete_instruments_group_at(int iGroup)
{
    if (iGroup < 0)
        return false;    //no success: invalid index, too low

    ensure_validity();
    ImoInstrGroups* pGroups = pimpl()->get_instrument_groups();
    if (!pGroups)
        return false;    //no success: no groups

    //find and remove group
    ImoObj::children_iterator itG;
    int i = 0;
    for (itG= pGroups->begin(); itG != pGroups->end() && i < iGroup; ++itG, ++i);
    if (i == iGroup && itG != pGroups->end())
    {
        pGroups->remove_child_imo(*itG);
        delete *itG;
        return true;    //success
    }
    return false;    //no success: invalid index, too high
}

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Deletes the instruments group <i>group</i>.
    Returns @FALSE if any error.

    @param group    The group to delete.
*/
bool AScore::delete_instruments_group(const AInstrGroup& group)
{
    ensure_validity();
    ImoInstrGroups* pGroups = pimpl()->get_instrument_groups();
    if (!pGroups)
        return false;    //no success: no groups

    ImoInstrGroup* pGrp = static_cast<ImoInstrGroup*>(group.m_pImpl);
    pGroups->remove_child_imo(pGrp);
    delete pGrp;
    return true;    //success
}

    void delete_all_instruments_groups();

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Deletes all instruments groups defined in the score, if any.
*/
void AScore::delete_all_instruments_groups()
{
    ensure_validity();
    ImoInstrGroups* pGroups = pimpl()->get_instrument_groups();
    if (!pGroups)
        return;

    pimpl()->remove_child_imo(pGroups);
    delete pGroups;
}

//@}    //Groups management. Create/remove groups


/// @name Algorithms
//@{

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Returns a measure locator for the specified instrument and timepos.
    @param timepos
    @param iInstr Number of the instrument (0..m) to which the measures refer to.
        Take into account that for polymetric music (music in which not all
        instruments have the same time signature), the measure number is not an
        absolute value, common to all the score instruments, but it
        is relative to an instrument. For normal scores, just providing measure
        number and location will do the job.
*/
MeasureLocator AScore::locator_for(TimeUnits timepos, int iInstr)
{
    ensure_validity();
    return ScoreAlgorithms::get_locator_for(pimpl(), timepos, iInstr);
}

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Returns the time position for the specified measure and beat.
    @param iMeasure Measure number (0..n) in instrument iInstr.
    @param iBeat Beat number (0..m) relative to the measure.
    @param iInstr Number of the instrument (0..m) to which the measures refer to.
        Take into account that for polymetric music (music in which not all
        instruments have the same time signature), the measure number is not an
        absolute value, common to all the score instruments, but it
        is relative to an instrument. For normal scores, just providing measure
        number and location will do the job.

    @warning For scores without time signature this method is useless as there are no
        measures, and beats are not defined. Therefore, if this method is invoked in an
        score without time signature, this method will always return time position 0.
*/
TimeUnits AScore::timepos_for(int iMeasure, int iBeat, int iInstr)
{
    ensure_validity();
    return ScoreAlgorithms::get_timepos_for(pimpl(), iMeasure, iBeat, iInstr);
}

//---------------------------------------------------------------------------------------
/** @memberof AScore
    Returns the time position for the specified measure locator.
    @param ml The measure locator to convert.
*/
TimeUnits AScore::timepos_for(const MeasureLocator& ml)
{
    ensure_validity();
    return ScoreAlgorithms::get_timepos_for(pimpl(), ml);
}

//@}    //Algorithms


//---------------------------------------------------------------------------------------
/** @memberof AScore
    When you finish modifying the content of an score it is necessary to inform
    lomse for updating all internal structures associated to the score.
    For this it is mandatory to invoke this method. Alternatively, you can invoke
    ADocument::end_of_changes(), that will invoke this method on all scores.
*/
void AScore::end_of_changes()
{
    ensure_validity();
    pimpl()->end_of_changes();
}



//=======================================================================================
/** @class ASoundInfo
    @extends AObject
    %ASoundInfo class contains and manages the information for one sound, such as its
    MIDI values. It always contains a AMidiInfo object.
    An AInstrument always have at least one sound but can have more. For each sound there
    is a ASoundInfo object and its associated AMidiInfo object.

    MusicXML files and other can contain additional information about the sound for an
    instrument, such as performance data (a solo instrument or an ensemble?), the
    virtual instrument used for the sound, or the play technique to use for all notes
    played in the associated instrument. This information is stored in the ASoundInfo
    object when the score is imported from MusicXML files. But this information is not
    yet used in lomse sound API.

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/
LOMSE_IMPLEMENT_IM_API_CLASS(ASoundInfo, ImoSoundInfo, AObject)

//---------------------------------------------------------------------------------------
/** @memberof ASoundInfo
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoSoundInfo* ASoundInfo::internal_object() const
{
    return const_cast<ImoSoundInfo*>(pimpl());
}

//---------------------------------------------------------------------------------------
/** @memberof ASoundInfo
    Provides access to the MIDI information for this instrument. MIDI info always exists.
    By default, when no MIDI information is provided in the source file or when
    programatically building a score, MIDI information is initialized as follows:

        device name = ""
        program name = ""
        bank = 1
        port = 0        port 0 is invalid. Means "not initialized"
        channel = 0     channel 0 is invalid. Means "not initialized"
        program = 1
        unpitched = 0
        volume = 1.0    maximum volume
        pan = 0         in front, centered
        elevation = 0   at listener head level

    When MIDI information has been specified when building the score, notice that
    'port', 'channel' or both could have not been specified and, thus, any of them can
    still in "not initialized". In these cases, port and channel are automatically
    assigned right values when the score is finished, that is, when invoking
    AScore::end_of_changes() method. The algorithm, in method
    MidiAssigner::assign_port_and_channel(), ensures that
    each instrument is assigned a unique combination (port, channel) for all instruments
    with port or channel containig the "not initialized" value.
*/
AMidiInfo ASoundInfo::midi_info() const
{
    ensure_validity();
    return AMidiInfo(const_cast<ImoSoundInfo*>(pimpl())->get_midi_info(),
                     m_pDoc, m_imVersion);
}



//=======================================================================================
/** @class ATextItem
    @extends AObject
    @extends ISiblings

    %ATextItem is an inline-level object containing a chunk of text with the same style.

    @warning This documentation is incomplete. The user API for the document
        internal model is currently being defined and, thus, for this class, only some
        methods have been defined.
*/
#if (LOMSE_BYPASS_ISSUE_253 == 1)
LOMSE_IMPLEMENT_IM_API_CLASS(ATextItem, ImoTextItem, AObjectWithSiblings)
#else
LOMSE_IMPLEMENT_IM_API_CLASS(ATextItem, ImoTextItem, AObject)
#endif

//---------------------------------------------------------------------------------------
/** @memberof ATextItem
    Transitional, to facilitate migration to the new public API.
    Notice that this method will be removed in future so, please, if you need to
    use this method open an issue at https://github.com/lenmus/lomse/issues
    explaining the need, so that the public API
    could be fixed and your app. would not be affected in future when this method
    is removed.
*/
ImoTextItem* ATextItem::internal_object() const
{
    return const_cast<ImoTextItem*>(pimpl());
}



////=======================================================================================
// /* * @class AAnonymousBlock
//    %AAnonymousBlock represents an structural block-level container that is not explicitly
//    present in the source document, but that was created by lomse to satisfy an internal
//    model constrain. For instance, if a block level container, such as a list item, has
//    some inline-level content inside it, such as some text, it is necessary to enclose
//    the inline content in an inlines container to satisfy the constrain that block
//    containers only contain other containers. The %AAnonymousBlock object represents
//    a container to be used in these cases.
//
//    This model would apply in the following example for this LMD content:
//
//    @code
//        <listitem>This is some text.</listitem>
//    @endcode
//
//    The \<listitem\> element contains a chunk text and will originate an AListItem
//    object, a type of blocks container object.
//    And the text string will originate an ATextItem object containing the string. But
//    ATextItem is an inline object and, thus, can not be included in a blocks container.
//    The solution for situations like this one is to generate a blocks container without
//    name, an anonymous container, to wrap the inlines content. The resulting model for
//    the previous example is an AListItem container, enclosing an %AAnonymousBlock with
//    the ATextItem object:
//
//    @verbatim
//                    AListItem (blocks container object)
//                        |
//                 AAnonymousBlock (inlines container object)
//                        |
//                    ATextItem (inline content object)
//                "This is some text."
//    @endverbatim
//
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(AAnonymousBlock, ImoAnonymousBlock, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof AAnonymousBlock
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoAnonymousBlock* AAnonymousBlock::internal_object() const
//{
//    return const_cast<ImoAnonymousBlock*>(pimpl());
//}

////=======================================================================================
// /* * @class AContent
//    %AContent is a generic block-level container, similar to the HTML \<div\> element. It
//    is used for grouping content but has no effect on the content or its layout.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(AContent, ImoContent, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof AContent
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoContent* AContent::internal_object() const
//{
//    return const_cast<ImoContent*>(pimpl());
//}

////=======================================================================================
// /* * @class AMultiColumn
//    %AMultiColumn is a blocks container subdivided in columns. It is an structural
//    container to display its content in conlumns instead of in a single block. There is
//    no equivalent in HTML, but you can consider it as a table with a single row and as
//    many columns as you need.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(AMultiColumn, ImoMultiColumn, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof AMultiColumn
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoMultiColumn* AMultiColumn::internal_object() const
//{
//    return const_cast<ImoMultiColumn*>(pimpl());
//}

////=======================================================================================
// /* * @class ATable
//    %ATable represents tabular data, that is, information presented in a two-dimensional
//    table comprised of rows and columns of cells containing data. It is equivalent to
//    the HTML \<table\> and can be considered as a container for the ATableRow,
//    ATableCell, ATableHead and ATableBoby objects.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(ATable, ImoTable, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof ATable
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoTable* ATable::internal_object() const
//{
//    return const_cast<ImoTable*>(pimpl());
//}

////=======================================================================================
// /* * @class ATableRow
//    %ATableRow defines a row of cells in a table. It is equivalent to the HTML \<tr\>
//    element. It is a container for the ATableCell objects that define the row's cells.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(ATableRow, ImoTableRow, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof ATableRow
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoTableRow* ATableRow::internal_object() const
//{
//    return const_cast<ImoTableRow*>(pimpl());
//}

////=======================================================================================
// /* * @class AListItem
//    %AListItem represents an item in a list. It is similar to the HTML \<li\> element.
//    %AListItem objects ar always contained in an AList parent object.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(AListItem, ImoListItem, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof AListItem
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoListItem* AListItem::internal_object() const
//{
//    return const_cast<ImoListItem*>(pimpl());
//}

////=======================================================================================
// /* * @class ATableCell
//    %ATableCell defines a cell of a table that contains data. It is similar to the
//    HTML \<td\> and \<th\> elements. %ATableCell objects are always contained in an
//    ATableRow object.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(ATableCell, ImoTableCell, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof ATableCell
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoTableCell* ATableCell::internal_object() const
//{
//    return const_cast<ImoTableCell*>(pimpl());
//}

////=======================================================================================
// /* * @class AHeading
//    %AHeading represents a section heading, similar to the HTML \<h1\> - \<h6\> elements.
//    The level of the heading is an attribute of the %AHeading object.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(AHeading, ImoHeading, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof AHeading
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoHeading* AHeading::internal_object() const
//{
//    return const_cast<ImoHeading*>(pimpl());
//}

////=======================================================================================
// /* * @class AInlineWrapper
//    %AInlineWrapper is a generic inline-box container similar to the HTML \<span\>
//    element. It does not inherently represent anything. It can be used to group
//    elements for styling purposes or because they share attribute values, such as
//    language. %AInlineWrapper is very much like the AContent object, but AContent is
//    a block-level object whereas the %AInlineWrapper is an inline object.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(AInlineWrapper, ImoInlineWrapper, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof AInlineWrapper
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoInlineWrapper* AInlineWrapper::internal_object() const
//{
//    return const_cast<ImoInlineWrapper*>(pimpl());
//}

////=======================================================================================
// /* * @class AButton
//    %AButton represents a clickable button, used to generate an action in the
//    application. It is similar to the HTML button element.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(AButton, ImoButton, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof AButton
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoButton* AButton::internal_object() const
//{
//    return const_cast<ImoButton*>(pimpl());
//}

////=======================================================================================
// /* * @class AControl
//    %AControl represents a user defined GUI control object, that is, an object that
//    can be clicked to produce an action on the user application. It is similar to
//    ALink, AButton or AScorePlayer, but ALink, AButton and AScorePlayer have a
//    pre-defined apearance and behaviour, whereas AControl content can be much more
//    complex, containing other controls, such as buttons and links, and its
//    appearance and behaviour is defined by the user application.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(AControl, ImoControl, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof AControl
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoControl* AControl::internal_object() const
//{
//    return const_cast<ImoControl*>(pimpl());
//}

////=======================================================================================
// /* * @class AScorePlayer
//    %AScorePlayer is a control for managing the playback of the associated AScore object.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(AScorePlayer, ImoScorePlayer, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof AScorePlayer
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoScorePlayer* AScorePlayer::internal_object() const
//{
//    return const_cast<ImoScorePlayer*>(pimpl());
//}

////=======================================================================================
// /* * @class AImage
//    %AImage is an inline object that represents a two-dimensional image. It is
//    equivalent to the HTML \<img\> element, that embeds an image into the document.
//
//    @warning This documentation is incomplete. The user API for the document
//        internal model is currently being defined and, thus, for this class, only some
//        methods have been defined.
//*/
//LOMSE_IMPLEMENT_IM_API_CLASS(AImage, ImoImage, AObject)
//
////---------------------------------------------------------------------------------------
// /* * @memberof AImage
//    Transitional, to facilitate migration to the new public API.
//    Notice that this method will be removed in future so, please, if you need to
//    use this method open an issue at https://github.com/lenmus/lomse/issues
//    explaining the need, so that the public API
//    could be fixed and your app. would not be affected in future when this method
//    is removed.
//*/
//ImoImage* AImage::internal_object() const
//{
//    return const_cast<ImoImage*>(pimpl());
//}
//


}  //namespace lomse

