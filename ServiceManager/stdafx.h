//=====================================================================================================================================================================
#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

//从 Windows 头中排除极少使用的资料
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

//操作系统版本
#include "targetver.h"

//某些 CString 构造函数将是显式的
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

//关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

//MFC 核心组件和标准组件
#include <afxwin.h>

//MFC 扩展
#include <afxext.h>

//MFC 自动化类
#include <afxdisp.h>

//MFC 对 Internet Explorer 4 公共控件的支持
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>
#endif

//MFC 对 Windows 公共控件的支持
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif

//功能区和控件条的 MFC 支持
#include <afxcontrolbars.h>

//=====================================================================================================================================================================
//XML支持
#import "msxml3.dll"
using namespace MSXML2;

#include <comdef.h>
#include <winsvc.h>

//=====================================================================================================================================================================
//Link参数
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

//=====================================================================================================================================================================
