#include "stdafx.h"
#include "localisation.h"
#include "multilang.h"

#include "HardLink.h"
#include "CopyHook.h"
#include "IconOverlay.h"
#include "ColumnProvider.h"
#include "PropertySheetPage.h"
#include "HardLinkMenu.h"



HINSTANCE g_hInstance = NULL;

UINT    g_cRefThisDll = 0;    // Reference count of this DLL.
int     gColourDepth = 8;

// gLSE_Flag contains various settings, which can be enabled
_LSESettings gLSESettings;


UINT    g_LockCount = 0;    // Reference count of this DLL.

PWCHAR  MenuEntries[eMenue__Free__ * 2];
PWCHAR  HelpTextW[eCommandType__Free__];
PCHAR   HelpTextA[eCommandType__Free__];
PWCHAR  TopMenuEntries[eTopMenu__Free__];

STDAPI
DllMain( HINSTANCE hInstance, DWORD ul_reason_for_call, LPVOID lpReserved )
{
  switch( ul_reason_for_call )
  {
    case DLL_PROCESS_ATTACH:
      // This needed because we use a non-msvcrt heap, which places the chunks so close
      // towards each other, that the crt-dbg would use its CRT secure fill pattern, and thus would
      // destroy memory during wcscpy_s() with _FILL_STRING
#if defined _DEBUG
      _CrtSetDebugFillThreshold(0);
#endif

      HTRACE (L"LSE::DLL_PROCESS_ATTACH\n");
      g_hInstance = hInstance;

      ZeroMemory((void*)&gLSESettings, sizeof(gLSESettings));
      GetLSESettings(gLSESettings);
      LoadMlgTexts(gLSESettings.LanguageID);

      // Initialize the call to CreateHardlink & CreateSymbolicLink. With Win10 this is needed
      // here, because the PropertyheetPageHandler is loaded in a different process than the other
      // Shell extension, and thus also needs initialization.
      InitCreateHardlink();
    break;

    case DLL_PROCESS_DETACH:
      HTRACE (L"LSE::DLL_PROCESS_DETTACH\n");
      FreeMlgTexts();
    break;
  }

  return TRUE;
}

void
LoadMlgTexts(
  int aLanguageID
)
{
  wchar_t tmpChar[MAX_PATH];

  // Load the menuetexts once and store them
  int   MenueTextIdx = IDS_STRING_eMenuHardlink;
  int i;
  for (i = 0; i < eMenue__Free__ * 2; i++)
  {
    int r = LoadStringEx(g_hInstance, MenueTextIdx++, tmpChar, MAX_PATH, aLanguageID);
    if (r > 0)
    {
      MenuEntries[i] = new wchar_t[wcslen(tmpChar) + 1];
      wcscpy( MenuEntries[i], tmpChar );
    }
    else
      MenuEntries[i] = NULL;
  }

  // Load the helptexts once and store them
  int   HelpTextIdx = IDS_STRING_HelpText01;
  for (i = 0; i < eCommandType__Free__; i++)
  {
    int r = LoadStringEx(g_hInstance, HelpTextIdx++, tmpChar, MAX_PATH, aLanguageID);
    if (r > 0)
    {
      HelpTextW[i] = new wchar_t[wcslen(tmpChar) + 1];
      wcscpy( HelpTextW[i], tmpChar );

      HelpTextA[i] = new char[wcslen(tmpChar) + 1];
      MakeAnsiString(tmpChar, HelpTextA[i]);
    }
    else
    {
      HelpTextW[i] = NULL;
      HelpTextA[i] = NULL;
    }
  }

  int   TopMenueTextIdx = IDS_STRING_eTopMenuPickLinkSource;
  for (i = 0; i < eTopMenu__Free__; i++)
  {
    int r = LoadStringEx(g_hInstance, TopMenueTextIdx++, tmpChar, MAX_PATH, aLanguageID);
    if (r > 0)
    {
      TopMenuEntries[i] = new wchar_t[wcslen(tmpChar) + 1];
      wcscpy( TopMenuEntries[i], tmpChar );
    }
    else
      TopMenuEntries[i] = NULL;
  }
}

void
FreeMlgTexts(
)
{
  int i;  
  for (i = 0; i < eMenue__Free__ * 2; i++)
    delete MenuEntries[i];

  for (i = 0; i < eCommandType__Free__; i++)
  {
    delete HelpTextW[i];
    delete HelpTextA[i];
  }

  for (i = 0; i < eTopMenu__Free__; i++)
    delete TopMenuEntries[i];
}



///////////////////////////////////////////////////////////////
//
STDAPI DllCanUnloadNow(void)
{
    return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
  *ppvOut = NULL;

  if (IsEqualIID(rclsid, CLSID_HardLinkExtension) )
  {
    HardLinkExtClassFactory *pcf = new HardLinkExtClassFactory();
    return pcf->QueryInterface(riid, ppvOut);
  }

  if ( IsEqualIID(rclsid, CLSID_HardLinkCopyhook) )
  {
    CopyHookClassFactory *pcf = new CopyHookClassFactory();
    return pcf->QueryInterface(riid, ppvOut);
  }

  if ( IsEqualIID(rclsid, CLSID_HardLinkPropertySheetPage) )
  {
    PropertySheetPageClassFactory *pcf = new PropertySheetPageClassFactory();
    return pcf->QueryInterface(riid, ppvOut);
  }

  if ( IsEqualIID(rclsid, CLSID_IconOverlayHardLink) )
  {
    IconOverlayHardlinkClassFactory *pcf = new IconOverlayHardlinkClassFactory();
    return pcf->QueryInterface(riid, ppvOut);
  }

  if ( IsEqualIID(rclsid, CLSID_IconOverlaySymbolicLink) )
  {
    IconOverlaySymbolicLinkClassFactory *pcf = new IconOverlaySymbolicLinkClassFactory();
    return pcf->QueryInterface(riid, ppvOut);
  }

  if ( IsEqualIID(rclsid, CLSID_IconOverlayJunction) )
  {
    IconOverlayJunctionClassFactory *pcf = new IconOverlayJunctionClassFactory();
    return pcf->QueryInterface(riid, ppvOut);
  }

  return CLASS_E_CLASSNOTAVAILABLE;
}



///////////////////////////////////////////////////////////////
// HardLinkExtClassFactory

HardLinkExtClassFactory::HardLinkExtClassFactory() : m_cRef(0L)
{
    g_cRefThisDll++;
}

HardLinkExtClassFactory::~HardLinkExtClassFactory()
{
    g_cRefThisDll--;
}

STDMETHODIMP HardLinkExtClassFactory::QueryInterface(REFIID riid,
                                                   LPVOID FAR *ppv)
{
    *ppv = NULL;
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;
        AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) HardLinkExtClassFactory::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) HardLinkExtClassFactory::Release()
{
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP
HardLinkExtClassFactory::
CreateInstance(
	LPUNKNOWN pUnkOuter,
	REFIID riid,
	LPVOID *ppvObj
)
{
  *ppvObj = NULL;

  // Shell extensions typically don't support aggregation (inheritance)
  if (pUnkOuter)
  	return CLASS_E_NOAGGREGATION;

  GetLSESettings(gLSESettings);

  // Get the Colrodepth so that we load the proper progress bar animation
  HDC hDC = GetDC(0);
  gColourDepth = GetDeviceCaps(hDC, BITSPIXEL);
  ReleaseDC(0, hDC);

	HRESULT r;
	ColumnProvider* pColProvider = new ColumnProvider();
  r = pColProvider->QueryInterface(riid, ppvObj);
  if (NULL != *ppvObj)
  {
    GetLSESettings(gLSESettings);
    return r;
  }
	delete pColProvider;

  HardLinkExt* pShellExt = new HardLinkExt();
  r = pShellExt->QueryInterface(riid, ppvObj);
  if (NULL != *ppvObj)
  {
    GetLSESettings(gLSESettings);
    return r;
  }
  delete pShellExt;

	return E_OUTOFMEMORY;
/*
	TRACE(L"LSE: IID %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n", 
			riid.Data1, 
			riid.Data2, 
			riid.Data3, 
			riid.Data4[0],
			riid.Data4[1],
			riid.Data4[2],
			riid.Data4[3],
			riid.Data4[4],
			riid.Data4[5],
			riid.Data4[6],
			riid.Data4[7]
	);
*/

}

STDMETHODIMP HardLinkExtClassFactory::LockServer(BOOL fLock)
{
    return NOERROR;
}

///////////////////////////////////////////////////////////////
// HardLinkExt

HardLinkExt::HardLinkExt() : m_cRef(0L),
	m_nTargets (0),
	m_pTargets (NULL),
	m_bTargetsFlag (0),
	m_ClipboardSize (0),
	m_DragDrop (false),
	m_ForceContext (false),
  m_DropTarget()
{
	g_cRefThisDll++;

	m_ClipFormat = RegisterClipboardFormat(CFSTR_HARDLINK);
	m_Command[0] = ePickLinkSource;
}

HardLinkExt::~HardLinkExt()
{
/*
Filename: HardLinkMenu.h
Line: 40
Severity: 2
Rule ID: (MRM.33)
Message: Class HardLinkExt contains pointer members not obviously deleted in destructor: m_pTargets
*/
	g_cRefThisDll--;
}

STDMETHODIMP HardLinkExt::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
	*ppv = NULL;

    if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
    {
    	*ppv = (IShellExtInit*)this;
    }
	else 
		if (IsEqualIID(riid, IID_IContextMenu))
		{
			*ppv = (IContextMenu*)this;
		}

	if (*ppv)
	{
		AddRef();
		return NOERROR;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) HardLinkExt::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) HardLinkExt::Release()
{
	if (--m_cRef)
		return m_cRef;

	FreeShellSelection();
	delete this;
	return 0L;
}

