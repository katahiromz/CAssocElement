// CAssocElement.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>

static PQUERYKEYVAL _FindKeyVal(ASSOCQUERY query, PQUERYKEYVAL pItems, UINT cItems)
{
    for (UINT iItem = 0; iItem < cItems; ++iItem)
    {
        if (pItems[iItem].query == query)
            return &pItems[iItem];
    }
    return NULL;
}

HRESULT SHAllocMUI(LPWSTR *ppwsz)
{
    WCHAR szOutBuf[1024];
    HRESULT hr = SHLoadIndirectString(*ppwsz, szOutBuf, 0x400, 0);
    CoTaskMemFree(*ppwsz);
    if (FAILED(hr))
    {
        *ppwsz = NULL;
        return hr;
    }
    return SHStrDupW(szOutBuf, ppwsz);
}

static HRESULT CALLBACK _QuerySourceDirect(
    IQuerySource *pSource,
    ASSOCQUERY query,
    PCWSTR keyName,
    PCWSTR valueName,
    PVOID pValue)
{
    return pSource->QueryValueDirect(keyName, valueName, (FLAGGED_BYTE_BLOB **)pValue);
}

static HRESULT CALLBACK _QuerySourceDword(
    IQuerySource *pSource,
    ASSOCQUERY query,
    PCWSTR keyName,
    PCWSTR valueName,
    PVOID pValue)
{
    return pSource->QueryValueDword(keyName, valueName, (DWORD *)pValue);
}

static HRESULT CALLBACK _QuerySourceExists(
    IQuerySource *pSource,
    ASSOCQUERY query,
    PCWSTR keyName,
    PCWSTR valueName,
    PVOID pValue)
{
    return pSource->QueryValueExists(keyName, valueName);
}

static HRESULT CALLBACK _QuerySourceString(
    IQuerySource *pSource,
    ASSOCQUERY query,
    PCWSTR keyName,
    PCWSTR valueName,
    PVOID pValue)
{
    HRESULT hr = pSource->QueryValueString(keyName, valueName, (PWSTR *)pValue);
    if (FAILED(hr) || !(query & 0x100000))
        return hr;
    return SHAllocMUI((PWSTR*)pValue);
}

/******************************************************************************
 * CAssocElement
 */
CAssocElement::~CAssocElement()
{
    if (m_pSource)
    {
        m_pSource->Release();
        m_pSource = NULL;
    }
}

UINT CAssocElement::_GetQueryKeyVal(QUERYKEYVAL **ppItems)
{
    *ppQuery = NULL;
    return 0;
}

HRESULT
CAssocElement::_QueryKeyValAny(
    QUERY_CALLBACK callback,
    QUERYKEYVAL *pItems,
    UINT cItems,
    IQuerySource *pQS,
    ASSOCQUERY query,
    PCWSTR valueName,
    PVOID pValue)
{
    PQUERYKEYVAL pItem = _FindKeyVal(query, pItems, cItems);
    if (!pItem)
        return E_INVALIDARG;

    WCHAR szBuff[128];
    PWSTR key = pItem->key;
    if ((query & 0x1000000) && key)
    {
        wnsprintfW(szBuff, _countof(szBuff), key, valueName);
        key = szBuff;
    }
    return callback(pQS, query, key, pItem->value, pValue);
}

HRESULT
CAssocElement::_QuerySourceAny(
    QUERY_CALLBACK callback,
    IQuerySource *pSource,
    DWORD dwFlags,
    ASSOCQUERY query,
    PCWSTR valueName,
    PVOID pValue)
{
    if (!pSource)
        return E_INVALIDARG;

    if (query == 0x10F0000 || query == 0x1170001)
        return callback(pSource, query, NULL, valueName, pValue);

    if ((query & dwFlags) != dwFlags)
        return E_INVALIDARG;

    PQUERYKEYVAL pItemss = NULL;
    UINT cItems = _GetQueryKeyVal(&pItemss);
    if (!cItems)
        return E_INVALIDARG;

    return _QueryKeyValAny(callback, pItemss, cItems, pSource, query, valueName, pValue);
}

STDMETHODIMP CAssocElement::QueryInterface(REFIID riid, PVOID* ppv)
{
    if (riid == IID_IObjectWithQuerySource)
    {
        *ppv = static_cast<IObjectWithQuerySource*>(this);
        AddRef();
        return S_OK;
    }
    if (riid == IID_IAssociationElement)
    {
        *ppv = static_cast<IAssociationElement*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CAssocElement::AddRef()
{
    return InterlockedIncrement(&m_cRefs);
}

STDMETHODIMP_(ULONG) CAssocElement::Release()
{
    LONG refs = InterlockedDecrement(&m_cRefs);
    if (!refs)
        delete this;
    return refs;
}

STDMETHODIMP CAssocElement::SetSource(IQuerySource *pSource)
{
    if (m_pSource)
        return E_UNEXPECTED;
    m_pSource = pSource;
    m_pSource->AddRef();
    return S_OK;
}

STDMETHODIMP CAssocElement::GetSource(REFIID riid, PVOID* ppSource)
{
    if (!m_pSource)
    {
        *ppSource = NULL;
        return E_NOINTERFACE;
    }
    return m_pSource->QueryInterface(riid, ppSource);
}

STDMETHODIMP CAssocElement::QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppszValue)
{
    *ppszValue = NULL;
    return _QuerySourceAny(_QuerySourceString, m_pSource, 0x50000, query, key, ppszValue);
}

STDMETHODIMP CAssocElement::QueryDword(ASSOCQUERY query, PCWSTR key, DWORD *pdwValue)
{
    return _QuerySourceAny(_QuerySourceDword, m_pSource, 0xC0000, query, key, pdwValue);
}

STDMETHODIMP CAssocElement::QueryExists(ASSOCQUERY query, PCWSTR key)
{
    return _QuerySourceAny(_QuerySourceExists, m_pSource, 0x60000, query, key, NULL);
}

STDMETHODIMP CAssocElement::QueryDirect(ASSOCQUERY query, PCWSTR key, FLAGGED_BYTE_BLOB **ppBlob)
{
    *ppBlob = NULL;
    return _QuerySourceAny(_QuerySourceDirect, m_pSource, 0x40000, query, key, ppBlob);
}

STDMETHODIMP CAssocElement::QueryObject(ASSOCQUERY query, PCWSTR key, REFIID riid, PVOID *ppvObj)
{
    *ppvObj = NULL;
    return E_NOTIMPL;
}
