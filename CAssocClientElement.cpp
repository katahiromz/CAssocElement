// CAssocClientElement.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>
#include "CAssocClientElement.h"

/******************************************************************************
 * CAssocClsidElement
 */

STDMETHODIMP CAssocClientElement::QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppszValue)
{
    HRESULT hr;

    if (query == 0x70001)
    {
        hr = CAssocElement::QueryString(0x70001, key, ppszValue);
        if (SUCCEEDED(hr))
            return hr;

        IAssociationElement* pDelegate;
        HRESULT hr = _GetVerbDelegate(key, &pDelegate);
        if (FAILED(hr))
            return hr;

        hr = pDelegate->QueryString(0x2010007, L"open", ppszValue);
        pDelegate->Release();
        return hr;
    }

    if (query == 0x170000)
    {
        hr = CAssocElement::QueryString(0x1170001, L"LocalizedString", ppszValue);
        if (SUCCEEDED(hr))
            return hr;
        return CAssocElement::QueryString(0x10F0000, NULL, ppszValue);
    }

    return CAssocShellElement::QueryString(query, key, ppszValue);
}

STDMETHODIMP CAssocClientElement::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_AssocClientElement;
    return S_OK;
}

HRESULT CAssocClientElement::_InitSource()
{
    WCHAR szBuff[MAX_PATH];
    StringCchPrintfW(szBuff, _countof(szBuff), L"SOFTWARE\\Clients\\%s", m_pszName);

    HRESULT hr = _InitSourceFromKey(HKEY_CURRENT_USER, szBuff, 0);
    if (SUCCEEDED(hr))
        return hr;
    return _InitSourceFromKey(HKEY_LOCAL_MACHINE, szBuff, 0);
}

HRESULT CAssocClientElement::_FixNetscapeRegistration()
{
    HKEY hKey;
    LSTATUS error = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Clients\\Mail", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    if (error)
        return E_FAIL;

    DWORD dwDisposition;
    error = RegCreateKeyExW(hKey, L"Netscape Messenger", 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
    if (error)
    {
        SHDeleteKeyW(hKey, L"Netscape Messenger");
        RegCloseKey(hKey);
        return E_FAIL;
    }

    HRESULT hr = E_FAIL;
    if (dwDisposition == REG_OPENED_EXISTING_KEY || _CreateRepairedNetscapeRegistration(hKey))
    {
        m_pSource->Release();
        hr = _QuerySourceCreateFromKey(hKey, NULL, NULL, &m_pSource);
    }

    if (FAILED(hr))
        SHDeleteKeyW(hKey, L"Netscape Messenger");
    RegCloseKey(hKey);
    return hr;
}

BOOL CAssocClientElement::_CreateRepairedNetscapeRegistration(HKEY hKey)
{
    HKEY hKey;
    LSTATUS error = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Clients\\Mail\\Netscape Messenger", 0, KEY_READ, &hKey);
    if (error)
        return FALSE;

    WCHAR szIconFile[MAX_PATH];
    error = _RegQueryString(hKey, L"Protocols\\mailto\\DefaultIcon", szIconFile, _countof(szIconFile));
    if (error == ERROR_SUCCESS)
    {
        PathParseIconLocationW(szIconFile);
        StringCchCatW(szIconFile, _countof(szIconFile), L",-1349");
        _RegSetVolatileString(hKey, L"DefaultIcon", szIconFile);
    }

    BOOL ret = FALSE;
    PCWSTR lpString2;
    if (_RegQueryString(hKey, 0, szIconFile, _countof(szIconFile)) == ERROR_SUCCESS &&
        _RegSetVolatileString(hKey, 0, szIconFile) == ERROR_SUCCESS &&
        _RegQueryString(hKey, L"Protocols\\mailto\\shell\\open\\command",
                        szIconFile, _countof(szIconFile)) == ERROR_SUCCESS &&
        SUCCESS(_ExeFromCmd(szIconFile, (PWSTR *)&lpString2)))
    {
        StringCchCopyW(szIconFile, _countof(szIconFile), lpString2);
        SHFree(lpString2);
        PathQuoteSpacesW(szIconFile);
        StringCchCatW(szIconFile, _countof(szIconFile) L" -mail");
        if (_RegSetVolatileString(hKey, L"shell\\open\\command", szIconFile) == ERROR_SUCCESS)
            ret = TRUE;
    }

    RegCloseKey(hKey);
    return ret;
}

HRESULT CAssocClientElement::_InitSourceFromKey(HKEY hKey, PCWSTR lpSubKey, DWORD dwFlags)
{
    HKEY hKey2;
    LSTATUS error = RegOpenKeyExW(hKey, lpSubKey, 0, KEY_READ | dwFlags, &hKey2)
    if (error)
        return E_INVALIDARG;

    HRESULT hr = E_INVALIDARG;
    HKEY hkeySrc = NULL;
    WCHAR szData[80];
    DWORD dwType, cbData = sizeof(szData);
    error = RegQueryValueExW(hKey2, NULL, NULL, &dwType, (PBYTE)szData, &cbData);
    if (error == ERROR_SUCCESS && dwType == REG_SZ && szData[0] &&
        RegOpenKeyExW(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, &hkeySrc) == ERROR_SUCCESS)
    {
        hr = _QuerySourceCreateFromKey2(hkeySrc, NULL, szData, &this2->m_pSource);
        RegCloseKey(hkeySrc);
    }

    RegCloseKey(hKey2);

    if (FAILED(hr) ||
        StrCmpICW(this3->m_pszName, L"mail") != 0 ||
        StrCmpICW((LPCWSTR)szData, L"Netscape Messenger") != 0 ||
        SUCCEEDED(QueryExists(0x2070000, L"open")))
    {
        return hr;
    }

    return _FixNetscapeRegistration();
}
