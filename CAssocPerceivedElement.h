// CAssocPerceivedElement.h
// Author: katahiromz
// License: MIT

#pragma once

#include "CAssocShellElement.h"

/******************************************************************************
 * CAssocPerceivedElement
 */
class CAssocPerceivedElement
    : public CAssocShellElement
{
protected:
    BOOL m_bNoPerceived = FALSE;

    UINT _GetQueryKeyVal(QUERYKEYVAL **ppItems) override;

public:
    /*** IPersist ***/
    STDMETHODIMP GetClassID(CLSID *pClassID) override;
    /*** IObjectWithQuerySource ***/
    STDMETHODIMP GetSource(REFIID riid, PVOID* ppSource) override;

    HRESULT _InitSource() override;
    HRESULT _GetVerbDelegate(PCWSTR pszSrc, IAssociationElement **ppElement) override;
};
