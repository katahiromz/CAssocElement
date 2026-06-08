// CAssocSystemExtElement.h
// Author: katahiromz
// License: MIT

#pragma once

#include "CAssocShellElement.h"

/******************************************************************************
 * CAssocSystemExtElement
 */
class CAssocSystemExtElement
    : public CAssocShellElement
{
public:
    /*** IPersist ***/
    STDMETHODIMP GetClassID(CLSID *pClassID) override;

    HRESULT _InitSource() override;
};
