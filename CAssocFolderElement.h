// CAssocFolderElement.h
// Author: katahiromz
// License: MIT

#pragma once

#include "CAssocFolderElement.h"

/******************************************************************************
 * CAssocFolderElement
 */
class CAssocFolderElement
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
