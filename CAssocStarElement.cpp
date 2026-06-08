// CAssocFolderElement.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>
#include "CAssocStarElement.h"

HRESULT _GetFileTypeName(LPWSTR lpsz, LPWSTR *ppwsz)
{
    WCHAR szDest[MAX_PATH], szBuff[128];

    if (!lpsz || *lpsz != '.' || !lpsz[1])
        return _SHAllocLoadString(g_hinst, 0x1301, ppwsz);

    CharUpperW(lpsz);
    LoadStringW(g_hinst, 0x1300, szBuff, _countof(szBuff));
    wnsprintfW(szDest, _countof(szDest), szBuff, lpsz + 1);
    return SHStrDupW(szDest, ppwsz);
}

/******************************************************************************
 * CAssocStarElement
 */

STDMETHODIMP CAssocStarElement::QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppszValue)
{
    if (query == 0x170000)
        return _GetFileTypeName(m_pszName, ppszValue);

    return CAssocShellElement::QueryString(query, key, ppszValue);
}

STDMETHODIMP CAssocStarElement::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_AssocStarElement;
    return S_OK;
}

HRESULT CAssocStarElement::_InitSource()
{
    return QuerySourceCreateFromKey(HKEY_CLASSES_ROOT, L"*", FALSE, IID_IQuerySource, (PVOID*)&m_pSource);
}
