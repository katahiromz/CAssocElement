// CAssocClsidElement.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>
#include "CAssocClsidElement.h"

/******************************************************************************
 * CAssocClsidElement
 */

STDMETHODIMP CAssocClsidElement::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_AssocClsidElement;
    return S_OK;
}

HRESULT CAssocClsidElement::_InitSource()
{
    return _QuerySourceCreateFromKey2(HKEY_CLASSES_ROOT, L"CLSID", m_pszName, &m_pSource);
}
