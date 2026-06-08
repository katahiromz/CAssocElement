// assoc2.cpp
// Author: katahiromz
// License: MIT

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shlobj_undoc.h>
#include <shlguid_undoc.h>
#include <shlwapi_undoc.h>
#include "CAssocElement.h"
#include "CAssocShellElement.h"
#include "CAssocApplicationElement.h"
#include "CAssocProgidElement.h"

HRESULT AssocCreateElement(REFCLSID rclsid, REFIID riid, PVOID *ppvObj)
{
    if (rclsid == CLSID_AssocShellElement)
    {
        CAssocShellElement* pElement = new CAssocShellElement();
        HRESULT hr = pElement->QueryInterface(riid, ppvObj);
        pElement->Release();
        return hr;
    }
    if (rclsid == CLSID_AssocApplicationElement)
    {
        CAssocApplicationElement* pElement = new CAssocApplicationElement();
        HRESULT hr = pElement->QueryInterface(riid, ppvObj);
        pElement->Release();
        return hr;
    }
    if (rclsid == CLSID_AssocProgidElement)
    {
        CAssocProgidElement * pElement = new CAssocProgidElement();
        HRESULT hr = pElement->QueryInterface(riid, ppvObj);
        pElement->Release();
        return hr;
    }
    // TODO: CLSID_AssocClsidElement, CLSID_AssocSystemElement
    // TODO: CLSID_AssocPerceivedElement, CLSID_AssocFolderElement, CLSID_AssocStarElement
    // TODO: CLSID_AssocClientElement
    *ppvObj = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}
