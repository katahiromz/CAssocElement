// CAssocClsidElement.h
// Author: katahiromz
// License: MIT

#pragma once

#include "CAssocShellElement.h"

/******************************************************************************
 * CAssocClsidElement
 */
class CAssocClsidElement
    : public CAssocShellElement
{
public:
    /*** IPersist ***/
    STDMETHODIMP GetClassID(CLSID *pClassID) override;

    HRESULT _InitSource() override;
};
