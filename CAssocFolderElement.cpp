// CAssocFolderElement.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>
#include "CAssocFolderElement.h"

HRESULT _SHAllocLoadString(HINSTANCE hInstance, UINT uID, LPWSTR *ppwsz)
{
    WCHAR szBuff[MAX_PATH];
    LoadStringW(hInstance, uID, szBuff, _countof(szBuff));
    return SHStrDupW(szBuff, ppwsz);
}
/******************************************************************************
 * CAssocFolderElement
 */

STDMETHODIMP CAssocFolderElement::QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppszValue)
{
    if (query == 0x170000)
        return _SHAllocLoadString(g_hinst, 0x1302u, ppszValue);
    return CAssocShellElement::QueryString(this, query, key, ppszValue);
}

STDMETHODIMP CAssocFolderElement::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_AssocFolderElement;
    return S_OK;
}

HRESULT CAssocFolderElement::_InitSource()
{
    return QuerySourceCreateFromKey(HKEY_CLASSES_ROOT, L"Folder", FALSE, IID_IQuerySource, (PVOID*)&m_pSource);
}
