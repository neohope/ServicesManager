//=====================================================================================================================================================================
#include "stdafx.h"
#include "ServiceManager.h"
#include "ServiceManagerDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//=====================================================================================================================================================================
//消息映射
BEGIN_MESSAGE_MAP(CServiceManagerApp, CWinAppEx)
END_MESSAGE_MAP()

//=====================================================================================================================================================================
//CServiceManagerApp构造函数
CServiceManagerApp::CServiceManagerApp()
{
}

//=====================================================================================================================================================================
//CServiceManagerApp 初始化
BOOL CServiceManagerApp::InitInstance()
{
	//只运行单一实例
	HANDLE hMutex=CreateMutex(NULL,FALSE,TEXT("ServicesManagerV1.0byHansen"));
	DWORD dw=WaitForSingleObject(hMutex,0);
	if(dw==WAIT_TIMEOUT)
	{
		//显示上一实例
		HWND myHwnd=FindWindow(NULL,TEXT("ServiceManager"));
		if(myHwnd)
		{
			if(IsIconic(myHwnd))
			{
				ShowWindow(myHwnd,SW_RESTORE);
			}

			ShowWindow(myHwnd,SW_SHOW);
			SetForegroundWindow(myHwnd);

			return FALSE;
		}
	}

	//基类初始化
	CWinAppEx::InitInstance();

	//使用控件容器
	AfxEnableControlContainer();

	//将配置信息储存在注册表中
	SetRegistryKey(TEXT("ServiceManager"));

	//=============================================================================================================================================================
	//判断是否有参数
	CString strCmdLine = m_lpCmdLine;
	bool bHide = false;

	//没有参数返回
	if(strCmdLine.GetLength()>0)
	{
		//全部大写
		strCmdLine.MakeUpper();
		//删除引号
		strCmdLine.Replace(TEXT("\""),TEXT(""));

		if(strCmdLine.Find(TEXT("TRAYICON"))>=0)
		{
			bHide = true;
		}
	}

	//创建对话框，并将其设为主窗体
	CServiceManagerDlg dlg;
	if(bHide)dlg.m_bInitHide=TRUE;
	m_pMainWnd=&dlg;
	return dlg.DoModal();
}

//=====================================================================================================================================================================
//唯一的CServiceManagerApp对象
CServiceManagerApp theApp;

//=====================================================================================================================================================================
