// CAssocStarElement.h
// Author: katahiromz
// License: MIT

#pragma once

#include "CAssocShellElement.h"

/******************************************************************************
 * CAssocFolderElement
 */
class CAssocStarElement
    : public CAssocShellElement
{
protected:

public:
    /*** IAssociationElement ***/
    STDMETHODIMP QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppszValue) override;
    /*** IPersist ***/
    STDMETHODIMP GetClassID(CLSID *pClassID) override;

    HRESULT _InitSource() override;
};
