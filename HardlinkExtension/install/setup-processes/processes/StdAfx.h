// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__780690DC_E128_403D_BC07_780D1B2CC101__INCLUDED_)
#define AFX_STDAFX_H__780690DC_E128_403D_BC07_780D1B2CC101__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

//disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
//disable warnings on extern before template instantiation
#pragma warning (disable : 4231)

#include <vector>
#include <string>


#if defined _NTQUERYHANDLE
#include <map>
#include <vector>


#endif // _NTQUERYHANDLE

using namespace std;

typedef vector<wstring> _StringList;
typedef vector<DWORD> _LongList;


//To make it a NSIS Plug-In
#include "exdll.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__780690DC_E128_403D_BC07_780D1B2CC101__INCLUDED_)
