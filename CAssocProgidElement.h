// CAssocProgidElement.h
// Author: katahiromz
// License: MIT

#pragma once

#include "CAssocShellElement.h"

/******************************************************************************
 * CAssocProgidElement
 */
class CAssocProgidElement
    : public CAssocShellElement
{
    IQuerySource *m_pSource2 = NULL;

public:
    /*** IAssociationElement ***/
    STDMETHODIMP QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppszValue) override;
    /*** IPersist ***/
    STDMETHODIMP GetClassID(CLSID *pClassID) override;

    BOOL _UseEnumForDefaultVerb() override
    HRESULT _InitSource() override;
};
