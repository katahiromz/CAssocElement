// CAssocShellElement.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>
#include "CAssocApplicationElement.h"

static const QUERYKEYVAL g_shellAppKeyVals[] =
{
    { 0x2170008, NULL, L"FriendlyAppName" },
};

static BOOL _PathAppend(LPCWSTR key1, LPCWSTR key2, LPWSTR pszDest, size_t cchDest)
{
    return SUCCEEDED(StringCchCopyW(pszDest, cchDest, key1)) &&
           SUCCEEDED(StringCchCatW(pszDest, cchDest, L"\\")) &&
           SUCCEEDED(StringCchCatW(pszDest, cchDest, key2));
}

static void _MakeApplicationsKey(PCWSTR pszPath, PWSTR pszDest, UINT cchDest)
{
    if (_PathAppend(L"Applications", pszPath, pszDest, cchDest))
    {
        if (!*PathFindExtensionW(pszPath))
            StringCchCatW(pszDest, cchDest, L".exe");
    }
}

static HRESULT
_AllocValueString(
    HKEY hkey,
    PCWSTR pszSubKey,
    PCWSTR pszValue,
    PWSTR* ppszOut)
{
    *ppszOut = NULL;

    DWORD cbData;
    LSTATUS error = SHGetValueW(hkey, pszSubKey, pszValue, NULL, NULL, &cbData);
    if (error != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(error);

    PWSTR pszData = (PWSTR)LocalAlloc(LPTR, cbData);
    if (!pszData)
        return HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);

    error = SHGetValueW(hkey, pszSubKey, pszValue, NULL, pszData, &cbData);
    if (error != ERROR_SUCCESS)
    {
        LocalFree(pszData);
        return HRESULT_FROM_WIN32(error);
    }

    *ppszOut = pszData;
    return S_OK;
}

/*************************************************************************
 * PrettifyFileDescriptionW [SHLWAPI.492]
 *
 * @see SHGetFileDescriptionW
 */
VOID WINAPI PrettifyFileDescriptionW(_Out_ PWSTR pszTarget, _In_ PCWSTR pszCutList)
{
    if (!pszTarget || !*pszTarget)
        return;

    PCWSTR pszFreeList = NULL, pszList = pszCutList;
    PCWSTR pszAssoc = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileAssociation";
    if (_AllocValueString(HKEY_LOCAL_MACHINE, pszAssoc, L"CutList", &pszFreeList))
        pszList = pszFreeList;

    if (pszList && *pszList)
    {
        for (PCWSTR pszEntry = pszList; *pszEntry; pszEntry += lstrlenW(pszEntry) + 1)
        {
            PWSTR pszMatch = StrRStrIW(pszTarget, NULL, pszEntry);
            if (!pszMatch)
                continue;

            if (pszMatch[lstrlenW(pszEntry)])
                continue;

            *pszMatch-- = UNICODE_NULL;
            while (pszMatch >= pszTarget && *pszMatch == L' ')
                *pszMatch-- = UNICODE_NULL;

            break;
        }
    }

    if (pszFreeList)
        LocalFree(pszFreeList);
}

/*************************************************************************
 * SHGetFileDescriptionW [SHLWAPI.348]
 *
 * @see PrettifyFileDescriptionW
 */
BOOL WINAPI SHGetFileDescriptionW(
    _In_ PCWSTR pszSrc,
    _In_ PCWSTR pszVerKey,
    _In_ PCWSTR pszDisplayName,
    _Out_ PWSTR pszOut,
    _Out_ PUINT pcchOut)
{
    DWORD pdwAttrs = 0;
    if (!PathFileExistsAndAttributesW(pszSrc, &pdwAttrs))
        return FALSE;

    WCHAR szPath[MAX_PATH];
    StrCpyNW(szPath, pszSrc, _countof(szPath));

    PVOID pvDescription  = NULL;
    PWSTR pszDescription = NULL;
    UINT  cchDescription = 0;
    PVOID pvBlock = NULL;

    BOOL bIsFile = !(pdwAttrs & FILE_ATTRIBUTE_DIRECTORY) &&
                   !PathIsUNCServerW(pszSrc) &&
                   !PathIsUNCServerShareW(pszSrc);
    if (bIsFile)
    {
        DWORD dwHandle = 0;
        DWORD cbBlock  = GetFileVersionInfoSizeW(szPath, &dwHandle);
        if (cbBlock)
        {
            pvBlock = LocalAlloc(LPTR, cbBlock);
            if (pvBlock && GetFileVersionInfoW(szPath, dwHandle, cbBlock, pvBlock))
            {
                WCHAR szSubBlock[60];
                if (pszVerKey &&
                    SUCCEEDED(StringCchCopyW(szSubBlock, _countof(szSubBlock), pszVerKey)) &&
                    VerQueryValueW(pvBlock, szSubBlock, &pvDescription, &cchDescription))
                {
                    pszDescription = (PWSTR)pvDescription;
                }
                else
                {
                    LPVOID pTranslation = NULL;
                    UINT   cbTranslation = 0;
                    if (VerQueryValueW(pvBlock, L"\\VarFileInfo\\Translation", &pTranslation,
                                       &cbTranslation) && cbTranslation)
                    {
                        UINT langId   = ((PWORD)pTranslation)[0];
                        UINT codePage = ((PWORD)pTranslation)[1];
                        wnsprintfW(szSubBlock, _countof(szSubBlock),
                                   L"\\StringFileInfo\\%04X%04X\\FileDescription",
                                   langId, codePage);
                        VerQueryValueW(pvBlock, szSubBlock, pvDescription, &cchDescription);
                        pszDescription = (PWSTR)pvDescription;
                    }

                    HRESULT hr;
                    if (!pszDescription || !*pszDescription)
                    {
                        hr = StringCchCopyW(szSubBlock, _countof(szSubBlock),
                                            L"\\StringFileInfo\\040904B0\\FileDescription");
                        if (SUCCEEDED(hr))
                        {
                            VerQueryValueW(pvBlock, szSubBlock, pvDescription, &cchDescription);
                            pszDescription = (PWSTR)pvDescription;
                        }
                    }
                    if (!pszDescription || !*pszDescription)
                    {
                        hr = StringCchCopyW(szSubBlock, _countof(szSubBlock),
                                L"\\StringFileInfo\\040904E4\\FileDescription");
                        if (SUCCEEDED(hr))
                        {
                            VerQueryValueW(pvBlock, szSubBlock, pvDescription, &cchDescription);
                            pszDescription = (PWSTR)pvDescription;
                        }
                    }
                    if (!pszDescription || !*pszDescription)
                    {
                        hr = StringCchCopyW(szSubBlock, _countof(szSubBlock),
                                            L"\\StringFileInfo\\04090000\\FileDescription");
                        if (SUCCEEDED(hr))
                        {
                            VerQueryValueW(pvBlock, szSubBlock, pvDescription, &cchDescription);
                            pszDescription = (PWSTR)pvDescription;
                        }
                    }
                }
            }
        }
    }

    if (!pszDescription || !*pszDescription)
    {
        PathRemoveExtensionW(szPath);
        pszDescription = PathFindFileNameW(szPath);
        cchDescription = lstrlenW(pszDescription);
    }

    PrettifyFileDescriptionW(pszDescription, pszDisplayName);

    UINT cchResult = lstrlenW(pszDescription) + 1;
    if (pszOut)
    {
        UINT cchCopy = min(cchResult, *pcchOut);
        StrCpyNW(pszOut, pszDescription, cchCopy);
        *pcchOut = cchCopy;
    }
    else
    {
        *pcchOut = cchResult;
    }

    if (pvBlock)
        LocalFree(pvBlock);

    return TRUE;
}

/******************************************************************************
 * CAssocApplicationElement
 */

UINT CAssocApplicationElement::_GetQueryKeyVal(QUERYKEYVAL **ppItems)
{
    *ppItems = g_shellAppKeyVals;
    return _countof(g_shellAppKeyVals);
}

HRESULT CAssocApplicationElement::_InitSource()
{
    LPWSTR pchFileName = PathFindpchFileName(m_pszName);

    WCHAR szSubKey[MAX_PATH];
    _MakeApplicationsKey(pchFileName, szSubKey, _countof(szSubKey));

    HRESULT hr = QuerySourceCreateFromKey(HKEY_CLASSES_ROOT, szSubKey, FALSE,
                                          IID_IQuerySource, (PVOID*)&m_pSource);
    m_bHasDir = (pchFileName != m_pszName);
    if (FAILED(hr) && m_bHasDir && PathFileExistsW(m_pszName))
        return S_FALSE;

    return hr;
}

STDMETHODIMP CAssocApplicationElement::GetClassID(GUID *pClsid)
{
    *pClsid = CLSID_AssocApplicationElement;
    return S_OK;
}

STDMETHODIMP CAssocApplicationElement::QueryObject(ASSOCQUERY query, PCWSTR key, REFIID riid, PVOID* ppv)
{
    if (query == 0x2200001)
        return QueryInterface(riid, ppv);
    return QueryObject(query, key, riid, ppv);
}

STDMETHODIMP CAssocApplicationElement::QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppsz)
{
    HRESULT hr = CAssocShellElement::QueryString(a1, query, key, ppsz);
    if (FAILED(hr) && query == 0x2170008)
        return _GetAppDisplayName(ppsz);
    return hr;
}

HRESULT CAssocApplicationElement::_GetAppDisplayName(LPWSTR* ppsz)
{
    LPWSTR pszName;
    if (!m_bHasDir)
    {
        HRESULT hr = QueryString(0x2010007, NULL, &pszName);
        if (FAILED(hr))
            return hr;
    }
    else
    {
        pszName = m_pszName;
    }

    WCHAR szData[MAX_PATH];
    DWORD cbData = sizeof(szData);
    DWORD flags = SHKEY_Subkey_MUICache | SHKEY_Key_ShellNoRoam | SHKEY_Root_HKCU;
    HRESULT hr = SKGetValueW(flags, NULL, pszName, 0, szData, &cbData);
    if (FAILED(hr))
    {
        UINT cchData = MAX_PATH;
        if (SHGetFileDescriptionW(pszName, NULL, NULL, szData, &cchData))
        {
            INT cchName = lstrlenW(szData);
            UINT cbName = (cchName + 1) * sizeof(WCHAR);
            SKSetValueW(flags, NULL, pszName, REG_SZ, (PBYTE)szData, cbName);
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr))
        hr = SHStrDupW(szData, ppsz);

    if (!m_bHasDir)
        CoTaskMemFree(pszName);

    return hr;
}
