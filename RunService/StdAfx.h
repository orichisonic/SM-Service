// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__2C55F8C2_8DE6_4DF0_804E_60A3B0BA652C__INCLUDED_)
#define AFX_STDAFX_H__2C55F8C2_8DE6_4DF0_804E_60A3B0BA652C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#define	_WIN32_WINNT	0x0500
#include <afx.h>
#include <winsock2.h>
#include <stdio.h>
#include <string>
#include <cassert>
#include <conio.h>
#include <iostream>
#include <io.h> 
#include <fstream.h>
#include <afxwin.h>


#include <tlhelp32.h> 
#include <stdlib.h> 
#include <malloc.h> 
#include <winsock.h> 

#include <direct.h>



#include <winbase.h>
#include <accctrl.h>
#include <aclapi.h>



#include "PSAPI.H"
#pragma comment( lib, "PSAPI.LIB")
#pragma comment( lib, "WS2_32.lib")

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__2C55F8C2_8DE6_4DF0_804E_60A3B0BA652C__INCLUDED_)
