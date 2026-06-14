// CAssocClientElement.h
// Author: katahiromz
// License: MIT

#pragma once

#include "CAssocShellElement.h"

/******************************************************************************
 * CAssocClientElement
 */
class CAssocClientElement
    : public CAssocShellElement
{
public:
    /*** IAssociationElement ***/
    STDMETHODIMP QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppszValue) override;
    /*** IPersist ***/
    STDMETHODIMP GetClassID(CLSID *pClassID) override;

    HRESULT _InitSource() override;
    HRESULT _FixNetscapeRegistration();
    BOOL _CreateRepairedNetscapeRegistration(HKEY hKey);
    HRESULT _InitSourceFromKey(HKEY hKey, PCWSTR lpSubKey, DWORD dwFlags);
};
