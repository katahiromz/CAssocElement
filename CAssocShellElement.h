// CAssocShellElement.h
// Author: katahiromz
// License: MIT

#pragma once

#include "CAssocElement.h"

HRESULT _QuerySourceCreateFromKey(HKEY hKey, LPCWSTR lpSubKey, BOOL bCreate, IQuerySource **ppSource);

/******************************************************************************
 * CAssocShellElement
 */
class CAssocShellElement
    : public CAssocElement
    , public IPersistString2
{
protected:
    PWSTR m_pszName;
    WCHAR m_szBuff[64];

    UINT _GetQueryKeyVal(QUERYKEYVAL **ppItems) override;
    HRESULT _DefaultVerbSource(IQuerySource** ppSource);

public:
    ~CAssocShellElement();

    /*** IAssociationElement ***/
    STDMETHODIMP QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppszValue) override;
    STDMETHODIMP QueryDword(ASSOCQUERY query, PCWSTR key, DWORD *pdwValue) override;
    STDMETHODIMP QueryExists(ASSOCQUERY query, PCWSTR key) override;
    STDMETHODIMP QueryDirect(ASSOCQUERY query, PCWSTR key, FLAGGED_BYTE_BLOB **ppBlob) override;
    STDMETHODIMP QueryObject(ASSOCQUERY query, PCWSTR key, REFIID riid, PVOID *ppvObj) override;
    /*** IPersist ***/
    STDMETHODIMP GetClassID(CLSID *pClassID) override;
    /*** IPersistString2 ***/
    STDMETHODIMP SetString(PCWSTR psz) override;
    STDMETHODIMP GetString(PWSTR *ppsz) override;

    virtual BOOL _UseEnumForDefaultVerb();
    virtual HRESULT _InitSource();
    virtual BOOL _IsAppSource();
    virtual HRESULT _GetVerbDelegate(PCWSTR pszSrc, IAssociationElement **ppElement);
};

/******************************************************************************
 * CAssocShellVerbElement
 */
class CAssocShellVerbElement
    : public CAssocElement
{
protected:
    BOOL m_bAppSource = FALSE;

    UINT _GetQueryKeyVal(QUERYKEYVAL **ppItems) override;
    HRESULT _GetAppDelegate(REFIID riid, PVOID *ppv) override;

public:
    CAssocShellVerbElement(BOOL bAppSource);

    STDMETHODIMP QueryString(ASSOCQUERY query, PCWSTR key, LPWSTR *ppwsz) override;
    STDMETHODIMP QueryObject(ASSOCQUERY query, PCWSTR key, REFIID riid, PVOID *ppvObj) override;
};
