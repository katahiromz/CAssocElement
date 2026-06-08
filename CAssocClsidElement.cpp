// CAssocClsidElement.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>
#include "CAssocClsidElement.h"

HRESULT _QuerySourceCreateFromKey2(
    HKEY    hKey,
    PCWSTR  key1,
    PCWSTR  key2,
    IQuerySource **ppSource)
{
    WCHAR szBuff[MAX_PATH];
    if (!key1)
        szBuff[0] = UNICODE_NULL;
    else
        _PathAppend(key1, key2, szBuff, 0x104);

    return _QuerySourceCreateFromKey(hKey, szBuff, FALSE, ppSource);
}

/******************************************************************************
 * CAssocClsidElement
 */

STDMETHODIMP CAssocClsidElement::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_AssocClsidElement;
    return S_OK;
}

HRESULT CAssocClsidElement::_InitSource()
{
    return _QuerySourceCreateFromKey2(HKEY_CLASSES_ROOT, L"CLSID", m_pszName, &m_pSource);
}
