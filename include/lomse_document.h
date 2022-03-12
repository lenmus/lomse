//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Copyright (c) 2010-present, Lomse Developers
//
// Licensed under the MIT license.
//
// See LICENSE and NOTICE.md files in the root directory of this source tree.
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_DOCUMENT_H__
#define __LOMSE_DOCUMENT_H__

#include "lomse_internal_model.h"
#include "private/lomse_document_p.h"


namespace lomse
{

class Document;

//---------------------------------------------------------------------------------------
// The ADocument represents the whole document and allows to access and modify the
// content of the document.
//
class LOMSE_EXPORT ADocument
{
private:
    Document* m_pImpl;
    struct Private;

    friend class Document;
    friend class AObject;
    friend class RequestDynamic;
    ADocument(Document* impl);

    inline const Document* pimpl() const { return m_pImpl; }
    inline Document* pimpl() { return m_pImpl; }

public:
    ADocument() { m_pImpl = nullptr; }
    ADocument(ADocument &&) noexcept = default;
    ADocument& operator=(ADocument &&) noexcept = default;
    ADocument(const ADocument& other) = default;
    ADocument& operator=(const ADocument& other) = default;

public:

    // Properties
    ImoId object_id() const;
    std::string& lmd_version() const;
    bool is_valid() const;

    //Access to content
    int num_children() const;
    AObject child_at(int iItem) const;
    AObject first_child() const;
    AObject last_child() const;
    AScore first_score() const;

    //Create new detached objects
    AObject create_object(EDocObject type);

    //attach content
    bool append_child(const AObject& detachedObj);

    //Page content scale
    float page_content_scale() const;
    void set_page_content_scale(float scale);

    //Document page size and margins
        //odd pages
    LUnits page_left_margin_odd() const;
    LUnits page_right_margin_odd() const;
    LUnits page_top_margin_odd() const;
    LUnits page_bottom_margin_odd() const;
    void set_page_left_margin_odd(LUnits value);
    void set_page_right_margin_odd(LUnits value);
    void set_page_top_margin_odd(LUnits value);
    void set_page_bottom_margin_odd(LUnits value);
        //even pages
    LUnits page_left_margin_even() const;
    LUnits page_right_margin_even() const;
    LUnits page_top_margin_even() const;
    LUnits page_bottom_margin_even() const;
    void set_page_left_margin_even(LUnits value);
    void set_page_right_margin_even(LUnits value);
    void set_page_top_margin_even(LUnits value);
    void set_page_bottom_margin_even(LUnits value);

    USize page_size() const;
    LUnits page_width() const;
    LUnits page_height() const;
    void set_page_size(USize uPageSize);
    void set_page_width(LUnits value);
    void set_page_height(LUnits value);
    std::unique_ptr<ImoPageInfo> page_info() const;

    void end_of_changes();

    // Transitional, to facilitate migration to the new public API.
    // Notice that this method will be removed in future so, please, if you need to
    // use this method open an issue at https://github.com/lenmus/lomse/issues
    // explaining the need, so that the public API could be fixed and your app. would
    // not be affected in future when this method is removed.
    Document* internal_object();

};



}   //namespace lomse

#endif      //__LOMSE_DOCUMENT_H__
