// CAssocProgidElement.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>
#include "CAssocProgidElement.h"

static const QUERYKEYVAL g_progidKeyVals[2] =
{
    { 0x81070002, "ShellEx\\%s", NULL },
    { 0x80070002, NULL, L"Content Type" },
};

HRESULT _AssocOpenRegKey(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult, BOOL bCreate)
{
    *phkResult = NULL;
    if (!hKey)
        return HRESULT_FROM_WIN32(ERROR_NO_ASSOCIATION);

    LSTATUS error;
    if ( bCreate )
        error = RegCreateKeyExW(hKey, lpSubKey, 0, NULL, 0, MAXIMUM_ALLOWED, NULL, phkResult, NULL);
    else
        error = RegOpenKeyExW(hKey, lpSubKey, 0, MAXIMUM_ALLOWED, phkResult);

    if (error == ERROR_SUCCESS)
        return S_OK;

    return HRESULT_FROM_WIN32(error);
}

HKEY _OpenProgidKey(PCWSTR psz2)
{
    HKEY hKey0, hKey1;
    HRESULT hr = _AssocOpenRegKey(HKEY_CLASSES_ROOT, psz2, &hKey0, 0);
    if (FAILED(hr))
        return NULL;

    WCHAR szData[64];
    DWORD cbData = sizeof(szData);
    if (!StrCmpIW(L"Excel.Sheet.8", psz2))
        return hKey0;

    LSTATUS error = SHGetValueW(hKey0, L"CurVer", 0, 0, szData, &cbData);
    if (error != ERROR_SUCCESS || cbData <= sizeof(WCHAR))
        return hKey0;

    HKEY hKey2;
    hr = _AssocOpenRegKey(HKEY_CLASSES_ROOT, szData, &hKey2, 0);
    if (FAILED(hr))
        return hKey0;

    hr = _AssocOpenRegKey(hKey2, L"shell", &hKey1, 0);
    if (SUCCEEDED(hr))
    {
        RegCloseKey(hKey1);
        RegCloseKey(hKey0);
        return hKey2;
    }

    hr = _AssocOpenRegKey(hKey0, L"shell", &hKey1, 0);
    if (FAILED(hr))
    {
        RegCloseKey(hKey0);
        return hKey2;
    }

    RegCloseKey(hKey1);
    RegCloseKey(hKey2);
    return hKey0;
}

/******************************************************************************
 * CAssocProgidElement
 */

STDMETHODIMP CAssocProgidElement::QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppszValue)
{
    HRESULT hr = CAssocShellElement::QueryString(query, key, ppszValue);
    if (SUCCEEDED(hr))
        return hr;

    if ((query & 0x80000000))
    {
        return _QueryKeyValAny(_QuerySourceString,
                               g_progidKeyVals, _countof(g_progidKeyVals),
                               lpVtbl,
                               query,
                               key,
                               ppszValue);
    }

    if (m_pSource && query == 0x170000)
        return m_pSource->QueryValueString(NULL, NULL, ppszValue);

    return hr;
}

STDMETHODIMP CAssocProgidElement::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_AssocProgidElement;
    return S_OK;
}

BOOL CAssocProgidElement::_UseEnumForDefaultVerb()
{
    return TRUE;
}

HRESULT CAssocProgidElement::_InitSource()
{
    HRESULT hr = S_OK;
    PWSTR pszName;
    HKEY hKey;

    if (*m_pszName == L'.')
    {
        hr = _QuerySourceCreateFromKey(HKEY_CLASSES_ROOT, m_pszName, 0, &m_pSource2);
        if (FAILED(hr))
            goto Exit;
        hr = m_pSource2->QueryValueString(NULL, NULL, &pszName);
    }
    else
    {
        pszName = m_pszName;
    }

    if (FAILED(hr))
        goto Exit;

    HKEY hKey = _OpenProgidKey(pszName);
    if (hKey)
    {
        hr = _QuerySourceCreateFromKey(hKey, NULL, NULL, &m_pSource);
        RegCloseKey(hKey);
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    if (pszName != m_pszName)
        CoTaskMemFree(pszName);

Exit:
    if (FAILED(hr))
    {
        if (m_pSource2)
        {
            m_pSource2 = NULL;
            m_pSource = m_pSource2;
            return S_FALSE;
        }
    }

    return hr;
}
