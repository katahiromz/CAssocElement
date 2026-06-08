// CAssocSystemExtElement.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>
#include "CAssocSystemExtElement.h"

/******************************************************************************
 * CAssocSystemExtElement
 */

STDMETHODIMP CAssocSystemExtElement::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_AssocSystemElement;
    return S_OK;
}

HRESULT CAssocSystemExtElement::_InitSource()
{
    return _QuerySourceCreateFromKey2(HKEY_CLASSES_ROOT, L"SystemFileAssociations", m_pszName, &m_pSource);
}
