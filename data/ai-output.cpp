PWSTR _PathGetArgsLikeCreateProcess(PCWSTR lpString)
{
    PWSTR pch;
    if (*lpString == L'"')
        pch = StrChrW(lpString + 1, L'"');
    else
        pch = StrChrW(lpString, L' ');
    if (pch)
        return pch + 1;
    return (PWSTR)&lpString[lstrlenW(lpString)];
}

HRESULT _PathCopyAndTrim(PWSTR pszBuff, size_t cchBuff, PCWSTR pszSrc, size_t cchSrc)
{
    *pszBuff = UNICODE_NULL;
    HRESULT hr = StringCchCatNW(pszBuff, cchBuff, pszSrc, cchSrc);
    if (SUCCEEDED(hr))
        StrTrimW(pszBuff, L" \t");
    return hr;
}

BOOL _PathMatchesSuspicious(PCWSTR lpString)
{
    WCHAR pszPath[MAX_PATH];
    INT cch = lstrlenW(lpString);
    SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, 0, pszPath);
    return StrCmpNIW(lpString, pszPath, cch) == 0;
}

// This function attempts to find where the "arguments" portion of a command-line path string
PWSTR _PathGuessNextBestArgs(PWSTR pszPath)
{
    PWSTR pSpaceStart = NULL;
    BOOL bValid = TRUE;
    const DWORD PATH_VALID_CHARS = (
        PATH_CHAR_CLASS_DOT | PATH_CHAR_CLASS_SEMICOLON | PATH_CHAR_CLASS_COMMA |
        PATH_CHAR_CLASS_SPACE | PATH_CHAR_CLASS_OTHER_VALID);

    for (;;)
    {
        const WCHAR ch = *pszPath;
        if (!ch)
            break;

        switch (ch)
        {
            case L' ':
                if (!pSpaceStart)
                    pSpaceStart = pszPath;
                break;

            case L'"':
            case L'%':
                bValid = FALSE;
                break;

            case L'\\':
                bValid = !PathIsUNCW(pszPath);
                if (bValid)
                    pSpaceStart = NULL;
                break;

            default:
                bValid = PathIsValidCharW(ch, PATH_VALID_CHARS);
                break;
        }

        if (!bValid)
            break;

        ++pszPath;
    }

    if (pSpaceStart)
    {
        while (*pSpaceStart == L' ')
            ++pSpaceStart;
        return pSpaceStart;
    }

    return bValid ? pszPath : NULL;
}

DWORD SHWindowsPolicy(REFGUID rpolid, DWORD dwDefaultValue)
{
    DWORD dwData, cbData = sizeof(dwData);
    HRESULT hr = SHWindowsPolicyGetValue(rpolid, &dwData, &cbData);
    if (FAILED(hr))
        return dwDefaultValue;
    return dwData;
}

HRESULT SHCoAlloc(SIZE_T cb, PVOID* ppData)
{
    PVOID pData = CoTaskMemAlloc(cb);
    *ppData = pData;
    return pData ? S_OK : E_OUTOFMEMORY;
}

BOOL _PathAppend(PCWSTR key1, PCWSTR key2, PWSTR pszDest, size_t cchDest)
{
    return SUCCEEDED(StringCchCopyW(pszDest, cchDest, key1)) &&
           SUCCEEDED(StringCchCatW(pszDest, cchDest, L"\\")) &&
           SUCCEEDED(StringCchCatW(pszDest, cchDest, key2));
}

VOID _MakeAppPathKey(PCWSTR pszPath, PWSTR pszDest, UINT cchDest)
{
    if (_PathAppend(L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths",
                    pszPath, pszDest, cchDest))
    {
        if (!*PathFindExtensionW(pszPath))
            StringCchCatW(pszDest, cchDest, L".exe");
    }
}

BOOL _GetAppPath(PCWSTR pszPath, PWSTR pszValue, DWORD cchValue)
{
    WCHAR szSubKey[MAX_PATH];
    _MakeAppPathKey(pszPath, szSubKey, _countof(szSubKey));
    DWORD cbData = cchValue * sizeof(WCHAR);
    LSTATUS error = SHGetValueW(HKEY_LOCAL_MACHINE, szSubKey, NULL, NULL, pszValue, &cbData);
    return error == ERROR_SUCCESS;
}

HRESULT _PathExeExists(LPWSTR pszPath)
{
    DWORD dwWhich = WHICH_PIF | WHICH_COM | WHICH_EXE | WHICH_BAT | WHICH_CMD | WHICH_OPTIONAL;
    DWORD attrs;
    if (!PathFileExistsDefExtAndAttributesW(pszPath, dwWhich, &attrs) ||
        (attrs & FILE_ATTRIBUTE_DIRECTORY))
    {
        return CO_E_APPNOTFOUND;
    }
    return S_OK;
}

HRESULT _PathFindInFolder(int csidl, STRSAFE_LPCWSTR pszSrc, LPWSTR pszPath, size_t cchDest)
{
    HRESULT hr = SHGetFolderPathW(0, csidl, 0, 0, pszPath);
    if (FAILED(hr))
        return hr;

    StringCchCatW(pszPath, cchDest, L"\\");
    hr = StringCchCatW(pszPath, cchDest, pszSrc);
    if (FAILED(hr))
        return hr;
    return _PathExeExists(pszPath);
}

HRESULT _PathFindInSystem(PWSTR pszPath, UINT cchPath)
{
    WCHAR szPath[MAX_PATH];
    HRESULT hr = _PathFindInFolder(CSIDL_SYSTEM, pszPath, szPath, _countof(szPath));
    if (SUCCEEDED(hr))
        return StringCchCopyW(pszPath, cchPath, szPath);
    hr = _PathFindInFolder(CSIDL_WINDOWS, pszPath, szPath, 260u);
    if (FAILED(hr))
        return hr;
    return StringCchCopyW(pszPath, cchPath, szPath);
}

BOOL PathIsAbsolute(LPCWSTR pszPath)
{
    return PathIsUNCW(pszPath) || (PathGetDriveNumberW(pszPath) != -1 && pszPath[2] == L'\\');
}

/*************************************************************************
 * SHEvaluateSystemCommandTemplate [SHLWAPI.552] (Vista+)
 *
 * https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shevaluatesystemcommandtemplate
 */
HRESULT WINAPI SHEvaluateSystemCommandTemplate(
    _In_ PCWSTR pszCmdTemplate,
    _Outptr_opt_ PWSTR *ppszApplication,
    _Outptr_opt_ PWSTR *ppszCommandLine,
    _Outptr_opt_ PWSTR *ppszParameters)
{
    HRESULT hr;
    WCHAR szExe[MAX_PATH];
    PWSTR pszArgs = _PathGetArgsLikeCreateProcess(pszCmdTemplate);

    UINT cchArgs = (UINT)(pszArgs - pszCmdTemplate);
    hr = _PathCopyAndTrim(szExe, _countof(szExe), pszCmdTemplate, cchArgs);
    if (FAILED(hr))
        goto Exit;

    // Unquote if necessary
    BOOL bQuoted = (szExe[0] == L'"');
    if (bQuoted)
        PathUnquoteSpacesW(szExe);

    if (PathIsAbsolute(szExe))
    {
        if (bQuoted)
        {
            hr = _PathExeExists(szExe);
        }
        else // Not quoted
        {
            if (_PathMatchesSuspicious(szExe)) // ProgramFiles-likely?
                hr = E_ACCESSDENIED;
            else
                hr = _PathExeExists(szExe);
        }

        while (FAILED(hr))
        {
            if (bQuoted || !*pszArgs)
                break;

            pszArgs = _PathGuessNextBestArgs(pszArgs);
            if (!pszArgs)
                break;

            cchArgs = (UINT)(pszArgs - pszCmdTemplate);
            hr = _PathCopyAndTrim(szExe, _countof(szExe), pszCmdTemplate, cchArgs);
            if (FAILED(hr))
                break;

            hr = _PathExeExists(szExe);
        }
    }
    else
    {
        if (!PathIsFileSpecW(szExe))
        {
            hr = E_ACCESSDENIED;
            goto Exit;
        }

        if (!_GetAppPath(szExe, szExe, _countof(szExe)))
        {
            if (SHWindowsPolicy(POLID_UsePathEnvVarForCommandTemplates, FALSE))
            {
                const DWORD PATH_VALID_CHARS = (
                    PATH_CHAR_CLASS_DOT | PATH_CHAR_CLASS_SEMICOLON | PATH_CHAR_CLASS_COMMA |
                    PATH_CHAR_CLASS_SPACE | PATH_CHAR_CLASS_OTHER_VALID);
                hr = PathFindOnPathExW(szExe, NULL, PATH_VALID_CHARS) ? S_OK : CO_E_APPNOTFOUND;
            }
            else
            {
                hr = _PathFindInSystem(szExe, _countof(szExe));
            }
        }
        else
        {
            hr = S_OK;
        }

        pszArgs = PathFindFileNameW(szExe);
    }

Exit:
    if (ppszApplication)
        *ppszApplication = NULL;
    if (ppszCommandLine)
        *ppszCommandLine = NULL;
    if (ppszParameters)
        *ppszParameters = NULL;

    if (!pszArgs)
        pszArgs = L"";

    // Create output strings
    if (SUCCEEDED(hr) && ppszApplication)
        hr = SHStrDupW(szExe, ppszApplication);

    if (SUCCEEDED(hr) && ppszCommandLine)
    {
        size_t cch = lstrlenW(szExe) + lstrlenW(pszArgs) + 8;
        hr = SHCoAlloc(cch * sizeof(WCHAR), (PVOID*)ppszCommandLine);
        if (SUCCEEDED(hr))
            hr = StringCchPrintfW(*ppszCommandLine, cch, L"\"%s\" %s", szExe, pszArgs);
    }

    if (SUCCEEDED(hr) && ppszParameters)
        hr = SHStrDupW(pszArgs, ppszParameters);

    if (FAILED(hr))
    {
        // Clean up
        if (ppszApplication && *ppszApplication)
        {
            CoTaskMemFree(*ppszApplication);
            *ppszApplication = NULL;
        }
        if (ppszCommandLine && *ppszCommandLine)
        {
            CoTaskMemFree(*ppszCommandLine);
            *ppszCommandLine = NULL;
        }
    }

    return hr;
}
