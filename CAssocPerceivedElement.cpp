// CAssocShellElement.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>
#include "CAssocPerceivedElement.h"

QUERYKEYVAL g_perceivedKeyVals[] =
{
    { 0x170000, NULL, L"FriendlyTypeName" },
    { 0x70001, L"DefaultIcon", NULL },
};

/******************************************************************************
 * CAssocPerceivedElement
 */

UINT CAssocPerceivedElement::_GetQueryKeyVal(QUERYKEYVAL **ppItems)
{
    if (m_bNoPerceived)
    {
        *ppItems = g_perceivedKeyVals;
        return _countof(g_perceivedKeyVals);
    }
    else
    {
        *ppItems = g_shellKeyVal;
        return _countof(g_shellKeyVal);
    }
}

STDMETHODIMP CAssocPerceivedElement::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_AssocPerceivedElement;
    return S_OK;
}

STDMETHODIMP CAssocPerceivedElement::GetSource(REFIID riid, PVOID* ppSource)
{
    *ppv = NULL
    if (m_bNoPerceived)
        return E_FAIL;

    return CAssocElement::GetSource(riid, ppv);
}

HRESULT CAssocPerceivedElement::_InitSource()
{
    PERCEIVED types;
    PERCEIVEDFLAG flag;
    PWSTR ppszType;
    HRESULT hr = AssocGetPerceivedType(m_pszName, &types, &flag, &ppszType);
    if (FAILED(hr))
        return hr;

    m_bNoPerceived = (flag & 4) == 0;
    hr = _QuerySourceCreateFromKey2(HKEY_CLASSES_ROOT, L"SystemFileAssociations", ppszType, &m_pSource);
    CoTaskMemFree(ppszType);
    return hr;
}

HRESULT CAssocPerceivedElement::_GetVerbDelegate(PCWSTR pszSrc, IAssociationElement **ppElement)
{
    if (m_bNoPerceived)
        return E_FAIL;

    return CAssocShellElement::_GetVerbDelegate(pszSrc, ppElement);
}
