// CAssocElement.h
// Author: katahiromz
// License: MIT

#pragma once

typedef struct QUERYKEYVAL
{
    ASSOCQUERY query;
    PWSTR key;
    PWSTR value;
} QUERYKEYVAL, *PQUERYKEYVAL;

typedef HRESULT (CALLBACK* QUERY_CALLBACK)(IQuerySource*, ASSOCQUERY query, PCWSTR keyName, PCWSTR valueName, PVOID pValue);

/******************************************************************************
 * CAssocElement
 */

class CAssocElement
    : public IObjectWithQuerySource
    , public IAssociationElement
{
protected:
    LONG m_cRefs = 1;
    IQuerySource *m_pSource = NULL;

    HRESULT _QueryKeyValAny(
        QUERY_CALLBACK callback,
        QUERYKEYVAL *pItems,
        UINT cItems,
        IQuerySource *pQS,
        ASSOCQUERY query,
        PCWSTR valueName,
        PVOID pValue);

    HRESULT _QuerySourceAny(
        QUERY_CALLBACK callback,
        IQuerySource *pSource,
        DWORD dwFlags,
        ASSOCQUERY query,
        PCWSTR valueName,
        PVOID pValue);

public:
    virtual ~CAssocElement();

    /*** IUnknown ***/
    STDMETHODIMP QueryInterface(REFIID riid, PVOID* ppv) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;
    /*** IObjectWithQuerySource ***/
    STDMETHODIMP SetSource(IQuerySource *pSource) override;
    STDMETHODIMP GetSource(REFIID riid, PVOID* ppSource) override;
    /*** IAssociationElement ***/
    STDMETHODIMP QueryString(ASSOCQUERY query, PCWSTR key, PWSTR *ppszValue) override;
    STDMETHODIMP QueryDword(ASSOCQUERY query, PCWSTR key, DWORD *pdwValue) override;
    STDMETHODIMP QueryExists(ASSOCQUERY query, PCWSTR key) override;
    STDMETHODIMP QueryDirect(ASSOCQUERY query, PCWSTR key, FLAGGED_BYTE_BLOB **ppBlob) override;
    STDMETHODIMP QueryObject(ASSOCQUERY query, PCWSTR key, REFIID riid, PVOID *ppvObj) override;

    virtual UINT _GetQueryKeyVal(QUERYKEYVAL **ppItems);
};
