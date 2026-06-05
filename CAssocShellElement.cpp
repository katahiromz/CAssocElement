// CAssocShellElement.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>
#include "CAssocShellElement.h"

HRESULT _QuerySourceCreateFromKey(HKEY hKey, LPCWSTR lpSubKey, BOOL bCreate, IQuerySource **ppSource)
{
    return QuerySourceCreateFromKey(hKey, lpSubKey, bCreate, IID_IQuerySource, (PVOID*)ppSource);
}

static const QUERYKEYVAL g_shellKeyVals[] =
{
    { 0x170000, NULL, L"FriendlyTypeName" },
    { 0x70001, L"DefaultIcon", NULL },
    { 0x70003, L"Clsid", NULL },
    { 0x70004, L"Progid", NULL },
    { 0x81070002, L"ShellEx\\%s", NULL },
};

static const QUERYKEYVAL g_shellVerbKeyVals[] =
{
    { 0x2070000, L"command", NULL },
    { 0x2070001, L"ddeexec", NULL },
    { 0x2070002, L"ddeexec\\ifexec", NULL },
    { 0x2070003, L"ddeexec\\application", NULL },
    { 0x2070004, L"ddeexec\\topic", NULL },
    { 0x2060005, L"ddeexec", L"NoActivateHandler" },
    { 0x2060006, L"command", L"command" },
    { 0x2170008, NULL, L"FriendlyAppName" },
};

BOOL _ParamIsApp(PCWSTR psz1)
{
    return !StrCmpNW(psz1, L"%1", 2) || !StrCmpNW(psz1, L"\"%1\"", 4);
}

HRESULT _CopyExe(PWSTR pszBuff, size_t cchBuff, PCWSTR pszSrc, size_t cchSrc)
{
    *pszBuff = UNICODE_NULL;
    HRESULT hr = StringCchCatNW(pszBuff, cchBuff, pszSrc, cchSrc);
    if (SUCCEEDED(hr))
        StrTrimW(pszBuff, L" \t");
    return hr;
}

HRESULT _PathFileExists(LPCWSTR lpFileName)
{
    DWORD attrs;
    if ( !PathFileExistsAndAttributesW(lpFileName, &attrs) || (attrs & FILE_ATTRIBUTE_DIRECTORY))
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    return S_OK;
}

BOOL PathIsAbsolute(PCWSTR pszPath)
{
    return PathIsUNCW(pszPath) || (PathGetDriveNumberW(pszPath) != -1 && pszPath[2] == L'\\');
}

HRESULT _PathExeExists(PWSTR pszPath)
{
    DWORD dwAttributes = 0;
    const DWORD dwWhich = WHICH_OPTIONAL | WHICH_CMD | WHICH_BAT | WHICH_EXE | WHICH_COM | WHICH_PIF;
    if (!PathFileExistsDefExtAndAttributesW(pszPath, dwWhich, &dwAttributes))
        return CO_E_APPNOTFOUND;

    if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return CO_E_APPNOTFOUND;

    return S_OK;
}

HRESULT PathFindInFolder(INT csidl, PCWSTR pszSrc, PWSTR pszDest, size_t cchDest)
{
    HRESULT hr = SHGetFolderPathW(NULL, csidl, NULL, SHGFP_TYPE_CURRENT, pszDest);
    if (FAILED(hr))
        return hr;

    hr = StringCchCatW(pszDest, cchDest, L"\\");
    if (FAILED(hr))
        return hr;

    hr = StringCchCatW(pszDest, cchDest, pszSrc);
    if (FAILED(hr))
        return hr;

    return _PathExeExists(pszDest);
}

HRESULT PathFindInSystem(PWSTR pszSrc, size_t cchSrc)
{
    WCHAR szPath[MAX_PATH];
    HRESULT hr = PathFindInFolder(CSIDL_SYSTEM, pszSrc, szPath, _countof(szPath));
    if (SUCCEEDED(hr))
        return StringCchCopyW(pszSrc, cchSrc, szPath);

    hr = PathFindInFolder(CSIDL_WINDOWS, pszSrc, szPath, _countof(szPath));
    if (SUCCEEDED(hr))
        hr = StringCchCopyW(pszSrc, cchSrc, szPath);

    return hr;
}

HRESULT _ExeFromCmd(PCWSTR pszCmdTemplate, PWSTR* ppszApplication)
{
    if (_ParamIsApp(pszCmdTemplate))
        return SHStrDupW(L"%1", ppszApplication);

    PWSTR pszParameters = NULL;
    HRESULT hr = SHEvaluateSystemCommandTemplate(pszCmdTemplate, ppszApplication,
                                                 NULL, &pszParameters);
    if (FAILED(hr))
        return hr;

    if (hr != S_OK || StrCmpIW(PathFindFileNameW(*ppszApplication), L"rundll32.exe") != 0)
    {
        CoTaskMemFree(pszParameters);
        return hr;
    }

    CoTaskMemFree(*ppszApplication);
    *ppszApplication = NULL;

    PWSTR pchComma = StrChrW(pszParameters, L',');
    if (!pchComma)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        CoTaskMemFree(pszParameters);
        return hr;
    }

    WCHAR szExe[MAX_PATH];
    hr = _CopyExe(szExe, _countof(szExe), pszParameters, pchComma - pszParameters);
    if (FAILED(hr))
    {
        CoTaskMemFree(pszParameters);
        return hr;
    }

    PathUnquoteSpacesW(szExe);

    if (!*PathFindExtensionW(szExe))
        StringCchCatW(szExe, _countof(szExe), L".dll");

    if (PathIsAbsolute(szExe))
    {
        hr = _PathFileExists(szExe);
    }
    else if (PathIsFileSpecW(szExe))
    {
        hr = _PathFindInSystem(szExe, _countof(szExe));
    }
    else
    {
        CoTaskMemFree(pszParameters);
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    if (SUCCEEDED(hr))
        hr = SHStrDupW(szExe, ppszApplication);

    CoTaskMemFree(pszParameters);
    return hr;
}

/******************************************************************************
 * CAssocShellElement
 */

CAssocShellElement::~CAssocShellElement()
{
    if (m_pszName && m_pszName != m_szBuff)
        LocalFree(m_pszName);
}

HRESULT CAssocShellElement::_InitSource()
{
    return _QuerySourceCreateFromKey(HKEY_CLASSES_ROOT, m_pszName, 0, &m_pSource);
}

UINT CAssocShellElement::_GetQueryKeyVal(QUERYKEYVAL **ppItems)
{
    *ppItems = g_shellKeyVals;
    return _countof(g_shellKeyVals);
}

BOOL CAssocShellElement::_UseEnumForDefaultVerb()
{
    return FALSE;
}

BOOL CAssocShellElement::_IsAppSource()
{
    return FALSE;
}

HRESULT CAssocShellElement::_GetVerbDelegate(PCWSTR pszSrc, IAssociationElement **ppElement)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAssocShellElement::QueryString(CAssocShellElementShift4* this, ASSOCQUERY query, PCWSTR key, PWSTR *ppszValue)
{
    if (!(query & 0x2000000))
        return CAssocElement::QueryString(query, key, ppszValue);

    IAssociationElement *pDelegate = NULL;
    HRESULT hr = _GetVerbDelegate(key, &pDelegate);
    if (FAILED(hr))
        return hr;

    hr = pDelegate->QueryString(query, NULL, ppszValue);
    pDelegate->Release();
    return hr;
}

STDMETHODIMP CAssocShellElement::QueryDword(CAssocShellElementShift4* this, ASSOCQUERY query, PCWSTR key, DWORD *pdwValue)
{
    if (!(query & 0x2000000))
        return CAssocElement::QueryDword(query, key, pdwValue);

    IAssociationElement *pDelegate = NULL;
    HRESULT hr = _GetVerbDelegate(key, &pDelegate);
    if (FAILED(hr))
        return hr;

    hr = pDelegate->QueryDword(query, NULL, pdwValue);
    pDelegate->Release();
    return hr;
}

STDMETHODIMP CAssocShellElement::QueryExists(CAssocShellElementShift4* this, ASSOCQUERY query, PCWSTR key)
{
    if (!(query & 0x2000000))
        return CAssocElement::QueryExists(query, valueName);

    IAssociationElement *pDelegate = NULL;
    HRESULT hr = _GetVerbDelegate(key, &pDelegate);
    if (FAILED(hr))
        return hr;

    hr = pDelegate->QueryExists(query, NULL);
    pDelegate->Release();
    return hr;
}

STDMETHODIMP CAssocShellElement::QueryDirect(ASSOCQUERY query, PCWSTR key, FLAGGED_BYTE_BLOB **ppBlob)
{
    if (!(query & 0x2000000))
        return CAssocElement::QueryDirect(query, key, ppBlob);

    IAssociationElement *pDelegate = NULL;
    HRESULT hr = _GetVerbDelegate(key, &pDelegate);
    if (FAILED(hr))
        return hr;
    hr = pDelegate->_QueryDirect(key, query, 0, ppBlob);
    pDelegate->Release();
    return hr;
}

STDMETHODIMP CAssocShellElement::QueryObject(ASSOCQUERY query, PCWSTR key, REFIID riid, PVOID *ppvObj)
{
    if (!(query & 0x2000000))
    {
        *ppvObj = NULL;
        return E_INVALIDARG;
    }

    IAssociationElement *pDelegate = NULL;
    HRESULT hr = _GetVerbDelegate(key, &pDelegate);
    if (FAILED(hr))
        return hr;

    if (query == 0x2200000)
        hr = pDelegate->QueryInterface(riid, ppvObj);
    else
        hr = pDelegate->QueryObject(query, NULL, riid, ppvObj);

    pDelegate->Release();
    return hr;
}

STDMETHODIMP CAssocShellElement::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_AssocShellElement;
    return S_OK;
}

STDMETHODIMP CAssocShellElement::SetString(PCWSTR psz)
{
    if (m_pszName)
        return E_UNEXPECTED;

    INT cchBuff, cchString = lstrlenW(lpString);
    PWSTR pszBuff;
    if (cchString >= _countof(m_szBuff))
    {
        cchBuff = cchString + 1;
        pszBuff = (PWSTR)LocalAlloc(LPTR, cchBuff * sizeof(WCHAR));
    }
    else
    {
        pszBuff = m_szBuff;
        cchBuff = _countof(m_szBuff);
    }

    m_pszName = pszBuff;
    if (!m_pszName)
        return E_UNEXPECTED;

    StringCchCopyW(psz, cchBuff, psz);
    return _InitSource();
}

STDMETHODIMP CAssocShellElement::GetString(PWSTR *ppsz)
{
    return SHStrDupW(m_pszName, ppwsz);
}

HRESULT CAssocShellElement::_DefaultVerbSource(IQuerySource** ppSource)
{
    IQuerySource* pShellSource = NULL;
    HRESULT hr = m_pSource->OpenSource(L"shell", FALSE, &pShellSource);
    if (FAILED(hr))
        return hr;

    PWSTR pszValue;
    PCWSTR pszVerb = L"open";
    if (SUCCEEDED(pShellSource->QueryValueString(NULL, NULL, &pszValue)))
        pszVerb = pszValue;

    hr = pShellSource->OpenSource(pszVerb, FALSE, ppSource);
    if (FAILED(hr))
    {
        if (pszValue)
        {
            INT nDelim = StrCSpnW(pszValue, L" ,");
            if (nDelim != lstrlenW(pszValue))
            {
                pszValue[nDelim] = UNICODE_NULL;
                hr = pShellSource->OpenSource(pszValue, FALSE, ppSource);
            }
        }
        else
        {
            IEnumString* pEnum;
            if (_UseEnumForDefaultVerb() && SUCCEEDED(pShellSource->EnumSources(&pEnum)))
            {
                PWSTR pszFirst = NULL;
                ULONG cFetched = 0;
                if (SUCCEEDED(pEnum->Next(1, &pszFirst, &cFetched)))
                {
                    hr = pShellSource->OpenSource(pszFirst, FALSE, ppSource);
                    CoTaskMemFree(pszFirst);
                }
                pEnum->Release();
            }
        }
    }

    if (pszValue)
        CoTaskMemFree(pszValue);

    pShellSource->Release();
    return hr;
}

/******************************************************************************
 * CAssocShellVerbElement
 */

CAssocShellVerbElement::CAssocShellVerbElement(BOOL bAppSource)
    : m_bAppSource(bAppSource)
{
}

STDMETHODIMP CAssocShellVerbElement::QueryString(
    ASSOCQUERY query,
    PCWSTR key,
    LPWSTR *ppwsz)
{
    HRESULT hr = CAssocElement::QueryString(query, key, ppwsz);
    if (SUCCEEDED(hr))
        return hr;

    if (query == 0x2010007)
    {
        LPWSTR psz;
        hr = CAssocElement::QueryString(0x2070000, 0, (PWSTR*)&psz);
        if (SUCCEEDED(hr))
            hr = _ExeFromCmd(psz, ppwsz);
        if (psz)
            CoTaskMemFree(psz);
        return hr;
    }

    if (query == 0x2070003)
    {
        hr = QueryString(0x2010007, NULL, &psz);
        if (SUCCEEDED(hr))
        {
            PathStripPathW(psz);
            PathRemoveExtensionW(psz);
        }
        *ppwsz = psz;
        return hr;
    }

    if (query == 0x2070004)
        return SHStrDupW(L"System", ppwsz);

    if (query == 0x2170008 && !m_bAppSource)
    {
        IAssociationElement *pElement;
        hr = _GetAppDelegate(IID_IAssociationElement, (void **)&pElement);
        if (SUCCEEDED(hr))
        {
            hr = pElement->QueryString(0x2170008, NULL, ppwsz);
            pElement->Release();
        }
    }

    return hr;
}

UINT CAssocShellVerbElement::_GetQueryKeyVal(QUERYKEYVAL **ppItems)
{
    *ppItems = g_shellVerbKeyVals;
    return _countof(g_shellVerbKeyVals);
}

HRESULT CAssocShellVerbElement::_GetAppDelegate(REFIID riid, PVOID *ppv)
{
    LPWSTR psz = NULL;
    HRESULT hr = CAssocElement::QueryString(0x2010007, NULL, &psz);
    if (FAILED(hr))
        return hr;

    IPersistString2 *pPS2;
    hr = AssocCreateElement(CLSID_AssocApplicationElement, IID_IPersistString2, (PVOID*)&pPS2);
    if (SUCCEEDED(hr))
    {
        hr = pPS2->SetString(psz);
        if (SUCCEEDED(hr))
            hr = pPS2->QueryInterface(riid, ppv);
        pPS2->Release();
    }

    if (psz)
        CoTaskMemFree(psz);

    return hr;
}

STDMETHODIMP CAssocShellVerbElement::QueryObject(ASSOCQUERY query, PCWSTR key, REFIID riid, PVOID *ppvObj)
{
    if (query != 0x2200001)
        return E_INVALIDARG;
    return _GetAppDelegate(riid, ppvObj);
}
