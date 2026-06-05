// CAssocApplicationElement.h
// Author: katahiromz
// License: MIT

#pragma once

#include "CAssocShellElement.h"

/******************************************************************************
 * CAssocApplicationElement
 */
class CAssocApplicationElement
    : public CAssocShellElement
{
protected:
    BOOL m_bHasDir = FALSE;

    UINT _GetQueryKeyVal(QUERYKEYVAL **ppItems) override;
    HRESULT _InitSource() override;
    HRESULT _GetAppDisplayName(LPWSTR *ppsz);

public:
    STDMETHODIMP GetClassID(GUID *pClsid) override;
    STDMETHODIMP QueryObject(ASSOCQUERY query, PCWSTR key, REFIID riid, PVOID* ppv) override;
    STDMETHODIMP QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppsz) override;
};
