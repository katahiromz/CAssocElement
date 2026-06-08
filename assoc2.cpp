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
#include "CAssocClsidElement.h"
#include "CAssocSystemExtElement.h"
#include "CAssocFolderElement.h"
#include "CAssocStarElement.h"

HRESULT AssocCreateElement(REFCLSID rclsid, REFIID riid, PVOID *ppvObj)
{
    IAssociationElement* pElement;
    if (rclsid == CLSID_AssocShellElement)
    {
        CAssocShellElement* pNewElement = new CAssocShellElement();
        pElement = static_cast<IAssociationElement*>(pNewElement);
    }
    else if (rclsid == CLSID_AssocApplicationElement)
    {
        CAssocApplicationElement* pNewElement = new CAssocApplicationElement();
        pElement = static_cast<IAssociationElement*>(pNewElement);
    }
    else if (rclsid == CLSID_AssocProgidElement)
    {
        CAssocProgidElement* pNewElement = new CAssocProgidElement();
        pElement = static_cast<IAssociationElement*>(pNewElement);
    }
    else if (rclsid == CLSID_AssocClsidElement)
    {
        CAssocClsidElement* pNewElement = new CAssocClsidElement();
        pElement = static_cast<IAssociationElement*>(pNewElement);
    }
    else if (rclsid == CLSID_AssocSystemElement)
    {
        CAssocSystemExtElement* pNewElement = new CAssocSystemExtElement();
        pElement = static_cast<IAssociationElement*>(pNewElement);
    }
    else if (rclsid == CLSID_AssocFolderElement)
    {
        CAssocFolderElement* pNewElement = new CAssocFolderElement();
        pElement = static_cast<IAssociationElement*>(pNewElement);
    }
    else if (rclsid == CLSID_AssocStarElement)
    {
        CAssocStarElement* pNewElement = new CAssocStarElement();
        pElement = static_cast<IAssociationElement*>(pNewElement);
    }
    else
    {
        *ppvObj = NULL;
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    // TODO: CLSID_AssocPerceivedElement
    // TODO: CLSID_AssocClientElement
    HRESULT hr = pElement->QueryInterface(riid, ppvObj);
    pElement->Release();
    return hr;
}
