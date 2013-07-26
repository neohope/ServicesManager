//=====================================================================================================================================================================
#include "stdafx.h"
#include "ServiceManager.h"
#include "ServiceManagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ID_TRAY	(WM_USER+100)
#define ERROR_MSGBOX_TITLE (TEXT("We have a big trouble here"))

//=====================================================================================================================================================================
//消息映射
BEGIN_MESSAGE_MAP(CServiceManagerDlg, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BT_START, &CServiceManagerDlg::OnBnClickedBtStart)
	ON_BN_CLICKED(IDC_BT_SHUTDOWN, &CServiceManagerDlg::OnBnClickedBtShutdown)
	ON_BN_CLICKED(IDC_BT_RESTART, &CServiceManagerDlg::OnBnClickedBtRestart)
	ON_BN_CLICKED(IDC_BT_DEL, &CServiceManagerDlg::OnBnClickedBtDel)
	ON_BN_CLICKED(IDC_BT_ADD, &CServiceManagerDlg::OnBnClickedBtAdd)
	ON_BN_CLICKED(IDC_BT_SAVE, &CServiceManagerDlg::OnBnClickedBtSave)
	ON_BN_CLICKED(IDC_BT_UPDATE, &CServiceManagerDlg::OnBnClickedBtUpdate)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_TASK, &CServiceManagerDlg::OnLvnItemchangedListTask)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(ID_TRAY,&CServiceManagerDlg::OnTrayNotify)
	ON_COMMAND(ID_POPMENU_EXIT, &CServiceManagerDlg::OnPopmenuExit)
	ON_COMMAND(ID_POPMENU_SHOW, &CServiceManagerDlg::OnPopmenuShow)
	ON_COMMAND(ID_POPMENU_HIDE, &CServiceManagerDlg::OnPopmenuHide)
	ON_WM_INITMENUPOPUP()
	ON_UPDATE_COMMAND_UI(ID_POPMENU_SHOW, &CServiceManagerDlg::OnUpdatePopmenuShow)
	ON_UPDATE_COMMAND_UI(ID_POPMENU_HIDE, &CServiceManagerDlg::OnUpdatePopmenuHide)
	ON_WM_NCPAINT()
	ON_COMMAND(ID_POPMENU_AUTOSTART, &CServiceManagerDlg::OnPopmenuAutostart)
	ON_UPDATE_COMMAND_UI(ID_POPMENU_AUTOSTART, &CServiceManagerDlg::OnUpdatePopmenuAutostart)
END_MESSAGE_MAP()

//=====================================================================================================================================================================
//CServiceManagerDlg构造函数
CServiceManagerDlg::CServiceManagerDlg(CWnd* pParent) : CDialog(CServiceManagerDlg::IDD, pParent)
{
	//载入图标
	m_hIcon=AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_pPopMenu=new CMenu();
	m_pPopMenu->LoadMenu(IDR_POPMENU);
	m_pPopMenu=m_pPopMenu->GetSubMenu(0);
}

CServiceManagerDlg* CServiceManagerDlg::m_pThis=NULL;
BOOL CServiceManagerDlg::m_bIsBussy=FALSE;

//=====================================================================================================================================================================
//初始化控件
void CServiceManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_TASK, m_TaskList);
}

//=====================================================================================================================================================================
//CServiceManagerDlg窗体初始化
BOOL CServiceManagerDlg::OnInitDialog()
{
	//基类初始化
	CDialog::OnInitDialog();

	//设置此对话框的图标。
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	//获取配置文件路径
	CString	szPath;   
	GetModuleFileName(NULL,szPath.GetBuffer(MAX_PATH),MAX_PATH);
	szPath.ReleaseBuffer();
	int nPos=szPath.ReverseFind('\\');   
	szPath=szPath.Left(nPos +1);
	m_szXML = szPath+TEXT("TaskList.xml");

	//调整列表框
	int nRet = m_TaskList.InsertColumn(0, TEXT("任务名"));
	nRet = m_TaskList.InsertColumn(1, TEXT("服务1"));
	nRet = m_TaskList.InsertColumn(2, TEXT("服务2"));
	nRet = m_TaskList.InsertColumn(3, TEXT("服务3"));
	nRet = m_TaskList.InsertColumn(4, TEXT("服务4"));
	nRet = m_TaskList.InsertColumn(5, TEXT("服务5"));
	nRet = m_TaskList.InsertColumn(6, TEXT("监视"));
	nRet = m_TaskList.InsertColumn(7, TEXT("扫描间隔"));
	nRet = m_TaskList.InsertColumn(8, TEXT("下次扫描"));
	
	CRect rect;
	m_TaskList.GetClientRect(&rect);
	int nColumnWidth = (rect.right - rect.left)/3;
	m_TaskList.SetColumnWidth(0, nColumnWidth);
	m_TaskList.SetColumnWidth(1, nColumnWidth);
	m_TaskList.SetColumnWidth(2, nColumnWidth);
	m_TaskList.SetColumnWidth(3, nColumnWidth);
	m_TaskList.SetColumnWidth(4, nColumnWidth);
	m_TaskList.SetColumnWidth(5, nColumnWidth);
	m_TaskList.SetColumnWidth(6, 40);
	m_TaskList.SetColumnWidth(7, 60);
	m_TaskList.SetColumnWidth(8, 60);

	//全行选取
	m_TaskList.SetExtendedStyle(m_TaskList.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	//添加托盘图标
	AddTrayIcon();

	//查询是否自动运行
	m_bAutoStart=QueryAutoRun();

	//初始化COM
	::CoInitialize(NULL);

	m_pThis=this;
	m_nRefreshServicesStatis=REFRESH_INTERVAL;

	//读入数据
	LoadConfig();

	//开启计时器
	SetTimer(WM_USER+1024,1000,(TIMERPROC)TimerMonitor);

	return TRUE;
}

//=====================================================================================================================================================================
//程序结束
void CServiceManagerDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	// CDialog::OnClose();

	//隐藏窗体
	ShowWindow(SW_HIDE);
}

//=====================================================================================================================================================================
//窗体销毁
void CServiceManagerDlg::OnDestroy()
{
	RemoveTrayIcon();
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}

//=====================================================================================================================================================================
//绘制非客户区
void CServiceManagerDlg::OnNcPaint()
{
	// TODO: Add your message handler code here
	CDialog::OnNcPaint();

	if(m_bInitHide==TRUE)
	{
		ShowWindow(SW_HIDE);
	}
}

//=====================================================================================================================================================================
//启动服务按钮
void CServiceManagerDlg::OnBnClickedBtStart()
{
	// TODO: Add your control notification handler code here
	int nIdex = m_TaskList.GetSelectionMark();
	if(nIdex<0)return;

	CString szValue;

	szValue=m_TaskList.GetItemText(nIdex,1);
	if(szValue.GetLength()>0)StartServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,2);
	if(szValue.GetLength()>0)StartServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,3);
	if(szValue.GetLength()>0)StartServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,4);
	if(szValue.GetLength()>0)StartServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,5);
	if(szValue.GetLength()>0)StartServiceByName(szValue);

	m_nRefreshServicesStatis=REFRESH_INTERVAL;
}

//=====================================================================================================================================================================
//停止服务按钮
void CServiceManagerDlg::OnBnClickedBtShutdown()
{
	// TODO: Add your control notification handler code here
	int nIdex = m_TaskList.GetSelectionMark();
	if(nIdex<0)return;

	CString szValue;

	szValue=m_TaskList.GetItemText(nIdex,1);
	if(szValue.GetLength()>0)StopServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,2);
	if(szValue.GetLength()>0)StopServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,3);
	if(szValue.GetLength()>0)StopServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,4);
	if(szValue.GetLength()>0)StopServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,5);
	if(szValue.GetLength()>0)StopServiceByName(szValue);

	m_nRefreshServicesStatis=REFRESH_INTERVAL;
}

//=====================================================================================================================================================================
//重启服务按钮
void CServiceManagerDlg::OnBnClickedBtRestart()
{
	// TODO: Add your control notification handler code here
	int nIdex = m_TaskList.GetSelectionMark();
	if(nIdex<0)return;

	CString szValue;

	//停止服务
	szValue=m_TaskList.GetItemText(nIdex,1);
	if(szValue.GetLength()>0)StopServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,2);
	if(szValue.GetLength()>0)StopServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,3);
	if(szValue.GetLength()>0)StopServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,4);
	if(szValue.GetLength()>0)StopServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,5);
	if(szValue.GetLength()>0)StopServiceByName(szValue);

	//开启服务
	szValue=m_TaskList.GetItemText(nIdex,1);
	if(szValue.GetLength()>0)StartServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,2);
	if(szValue.GetLength()>0)StartServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,3);
	if(szValue.GetLength()>0)StartServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,4);
	if(szValue.GetLength()>0)StartServiceByName(szValue);

	szValue=m_TaskList.GetItemText(nIdex,5);
	if(szValue.GetLength()>0)StartServiceByName(szValue);

	m_nRefreshServicesStatis=REFRESH_INTERVAL;
}

//=====================================================================================================================================================================
//添加配置按钮
void CServiceManagerDlg::OnBnClickedBtAdd()
{
	// TODO: Add your control notification handler code here
	CString szTaskName;
	CString szService1,szService2,szService3,szService4,szService5;
	GetDlgItemText(IDC_TASK_NAME,szTaskName);
	GetDlgItemText(IDC_SERVICE1,szService1);
	GetDlgItemText(IDC_SERVICE2,szService2);
	GetDlgItemText(IDC_SERVICE3,szService3);
	GetDlgItemText(IDC_SERVICE4,szService4);
	GetDlgItemText(IDC_SERVICE5,szService5);

	if(szTaskName.GetLength()<=0 || szService1.GetLength()+szService2.GetLength()+szService3.GetLength()+szService4.GetLength()+szService5.GetLength()<=0)
	{
		MessageBox(TEXT("请输入任务名和至少一个服务名"),TEXT("提示"),MB_OK);
	}
	else
	{
		int nIndex=m_TaskList.InsertItem(0, szTaskName);
		m_TaskList.SetItemText(nIndex,1,szService1);
		m_TaskList.SetItemText(nIndex,2,szService2);
		m_TaskList.SetItemText(nIndex,3,szService3);
		m_TaskList.SetItemText(nIndex,4,szService4);
		m_TaskList.SetItemText(nIndex,5,szService5);
	}

	m_nRefreshServicesStatis=REFRESH_INTERVAL;
}

//=====================================================================================================================================================================
//删除配置按钮
void CServiceManagerDlg::OnBnClickedBtDel()
{
	// TODO: Add your control notification handler code here
	int nIdex = m_TaskList.GetSelectionMark();
	if(nIdex<0)return;

	m_TaskList.DeleteItem(nIdex);

	m_nRefreshServicesStatis=REFRESH_INTERVAL;
}

//=====================================================================================================================================================================
//更新配置按钮
void CServiceManagerDlg::OnBnClickedBtUpdate()
{
	// TODO: Add your control notification handler code here
	int nIndex = m_TaskList.GetSelectionMark();
	if(nIndex<0)return;

	CString szTaskName;
	CString szService1,szService2,szService3,szService4,szService5,szInterval;
	GetDlgItemText(IDC_TASK_NAME,szTaskName);
	GetDlgItemText(IDC_SERVICE1,szService1);
	GetDlgItemText(IDC_SERVICE2,szService2);
	GetDlgItemText(IDC_SERVICE3,szService3);
	GetDlgItemText(IDC_SERVICE4,szService4);
	GetDlgItemText(IDC_SERVICE5,szService5);
	GetDlgItemText(IDC_MONITOR_INTERVAL,szInterval);

	if(szTaskName.GetLength()>0 || szService1.GetLength()+szService2.GetLength()+szService3.GetLength()+szService4.GetLength()+szService5.GetLength()>0)
	{
		m_TaskList.SetItemText(nIndex,1,szService1);
		m_TaskList.SetItemText(nIndex,2,szService2);
		m_TaskList.SetItemText(nIndex,3,szService3);
		m_TaskList.SetItemText(nIndex,4,szService4);
		m_TaskList.SetItemText(nIndex,5,szService5);
		if(IsDlgButtonChecked(IDC_CHECK_MONITORON))
		{
			m_TaskList.SetItemText(nIndex,6,TEXT("*"));
		}
		else
		{
			m_TaskList.SetItemText(nIndex,6,TEXT(""));
		}
		m_TaskList.SetItemText(nIndex,7,szInterval);
	}

	m_nRefreshServicesStatis=REFRESH_INTERVAL;
}


//=====================================================================================================================================================================
//保存配置按钮
void CServiceManagerDlg::OnBnClickedBtSave()
{
	// TODO: Add your control notification handler code here
	try
	{
		//XML实例
		MSXML2::IXMLDOMDocumentPtr pXml;
		HRESULT hr = pXml.CreateInstance(__uuidof(MSXML2::DOMDocument30));
		if(!SUCCEEDED(hr))
		{
			MessageBox(TEXT("MSXML2::DOMDocument30 Init Error"),ERROR_MSGBOX_TITLE,MB_OK);
			return;
		}

		MSXML2::IXMLDOMElementPtr xmlRoot;
		MSXML2::IXMLDOMElementPtr xmlNode;

		//创立根节点
		pXml->raw_createElement((_bstr_t)(char*)"ServiceManager", &xmlRoot);
		pXml->raw_appendChild(xmlRoot, NULL);

		//写入所有配置信息
		int nItemCount = m_TaskList.GetItemCount();
		for(int i=0;i<nItemCount;i++)
		{
			//建立并添加
			CString szValue;
			pXml->raw_createElement((_bstr_t)(char*)"TaskItem", &xmlNode);
			szValue=m_TaskList.GetItemText(i,0);
			xmlNode->Puttext(szValue.GetBuffer(szValue.GetLength()));
			szValue=m_TaskList.GetItemText(i,1);
			xmlNode->setAttribute("Service1", szValue.GetBuffer(szValue.GetLength()));
			szValue=m_TaskList.GetItemText(i,2);
			xmlNode->setAttribute("Service2", szValue.GetBuffer(szValue.GetLength()));
			szValue=m_TaskList.GetItemText(i,3);
			xmlNode->setAttribute("Service3", szValue.GetBuffer(szValue.GetLength()));
			szValue=m_TaskList.GetItemText(i,4);
			xmlNode->setAttribute("Service4", szValue.GetBuffer(szValue.GetLength()));
			szValue=m_TaskList.GetItemText(i,5);
			xmlNode->setAttribute("Service5", szValue.GetBuffer(szValue.GetLength()));
			szValue=m_TaskList.GetItemText(i,6);
			if(szValue==TEXT("*"))
			{
				xmlNode->setAttribute("MonitorOn", TEXT("1"));
			}
			else
			{
				xmlNode->setAttribute("MonitorOn", TEXT("0"));
			}
			szValue=m_TaskList.GetItemText(i,7);
			xmlNode->setAttribute("MonitorInterval", szValue.GetBuffer(szValue.GetLength()));
			xmlRoot->appendChild(xmlNode);
		}

		//保存文件
		pXml->save(m_szXML.GetBuffer(m_szXML.GetLength()));
	}
	catch(_com_error &e)
	{
		MessageBox(e.ErrorMessage(),ERROR_MSGBOX_TITLE,MB_OK);
	}
}

//=====================================================================================================================================================================
//改变CListControl选择
void CServiceManagerDlg::OnLvnItemchangedListTask(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	//改变右侧信息
	int nIndex=pNMLV->iItem;
	if(nIndex>=0)
	{
		CString szValue;
		szValue=m_TaskList.GetItemText(nIndex,0);
		SetDlgItemText(IDC_TASK_NAME,szValue);

		szValue=m_TaskList.GetItemText(nIndex,1);
		SetDlgItemText(IDC_SERVICE1,szValue);

		szValue=m_TaskList.GetItemText(nIndex,2);
		SetDlgItemText(IDC_SERVICE2,szValue);

		szValue=m_TaskList.GetItemText(nIndex,3);
		SetDlgItemText(IDC_SERVICE3,szValue);

		szValue=m_TaskList.GetItemText(nIndex,4);
		SetDlgItemText(IDC_SERVICE4,szValue);

		szValue=m_TaskList.GetItemText(nIndex,5);
		SetDlgItemText(IDC_SERVICE5,szValue);

		szValue=m_TaskList.GetItemText(nIndex,6);
		if(szValue==TEXT("*"))
		{
			CheckDlgButton(IDC_CHECK_MONITORON,1);
		}
		else
		{
			CheckDlgButton(IDC_CHECK_MONITORON,0);
		}

		szValue=m_TaskList.GetItemText(nIndex,7);
		SetDlgItemText(IDC_MONITOR_INTERVAL,szValue);
	}
}

//=====================================================================================================================================================================
//托盘图标消息
LRESULT CServiceManagerDlg::OnTrayNotify(WPARAM wParam,LPARAM lParam)
{
	UINT uMsg = (UINT)lParam; 
	switch(uMsg)
	{
	case WM_LBUTTONDBLCLK:
		//显示窗口
		if(m_bInitHide==TRUE)
		{
			m_bInitHide=FALSE;
		}
		ShowWindow(SW_SHOW);
		break;
	case WM_RBUTTONDOWN:
		//右键菜单
		if(m_pPopMenu->m_hMenu)
		{
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow();
			m_pPopMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_BOTTOMALIGN,pt.x,pt.y,this,NULL);
		}
		break;
	}

	return TRUE;
}

//=====================================================================================================================================================================
//托盘菜单启动
void CServiceManagerDlg::OnPopmenuAutostart()
{
	// TODO: Add your command handler code here
	if(m_bAutoStart==TRUE)
	{
		RemoveAutoRun();
		m_bAutoStart = FALSE;
	}
	else
	{
		SetAutoRun();
		m_bAutoStart = TRUE;
	}
}

//=====================================================================================================================================================================
//托盘图标退出
void CServiceManagerDlg::OnPopmenuExit()
{
	// TODO: Add your command handler code here
	RemoveTrayIcon();
	PostQuitMessage(0);
}

//=====================================================================================================================================================================
//托盘图标显示
void CServiceManagerDlg::OnPopmenuShow()
{
	// TODO: Add your command handler code here
	if(m_bInitHide==TRUE)
	{
		m_bInitHide=FALSE;
	}
	ShowWindow(SW_SHOW);
}

//=====================================================================================================================================================================
//托盘图标隐藏
void CServiceManagerDlg::OnPopmenuHide()
{
	// TODO: Add your command handler code here
	ShowWindow(SW_HIDE);
}

//=====================================================================================================================================================================
//托盘图标菜单更新通知（Dialog必须）
void CServiceManagerDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialog::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: Add your message handler code here
	if(pPopupMenu->m_hMenu==NULL)return;

	CCmdUI cmdUI;
	cmdUI.m_pOther=NULL;
	cmdUI.m_pMenu=pPopupMenu;
	cmdUI.m_pSubMenu=NULL;

	//判断是否为顶级Menu
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu==pPopupMenu->m_hMenu)
	{
		//为顶级菜单Menu
		cmdUI.m_pParentMenu=pPopupMenu;
	}
	else if((hParentMenu=::GetMenu(m_hWnd))!=NULL)
	{
		//不是顶级菜单
		CWnd* pParent = this;
		if (pParent!=NULL && (hParentMenu=::GetMenu(pParent->m_hWnd))!=NULL)
		{
			//在hParentMenu中查找对应子菜单
			int nIndexMax=::GetMenuItemCount(hParentMenu);
			for(int nIndex=0; nIndex<nIndexMax; nIndex++)
			{
				if(::GetSubMenu(hParentMenu,nIndex)==pPopupMenu->m_hMenu)
				{
					cmdUI.m_pParentMenu=CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	//对于菜单中每一项进行更新
	cmdUI.m_nIndexMax=pPopupMenu->GetMenuItemCount();
	for(cmdUI.m_nIndex=0; cmdUI.m_nIndex<cmdUI.m_nIndexMax; cmdUI.m_nIndex++)
	{
		cmdUI.m_nID=pPopupMenu->GetMenuItemID(cmdUI.m_nIndex);
		if (cmdUI.m_nID==0)continue;

		//更新菜单
		if (cmdUI.m_nID==(UINT)-1)
		{
			//如果有子菜单
			cmdUI.m_pSubMenu=pPopupMenu->GetSubMenu(cmdUI.m_nIndex);
			
			//跳过无效菜单ID和Sepeter
			if (cmdUI.m_pSubMenu==NULL || (cmdUI.m_nID=cmdUI.m_pSubMenu->GetMenuItemID(0))==0 || cmdUI.m_nID==(UINT)-1)
			{
				continue;
			}

			//更新菜单项
			cmdUI.DoUpdate(this, TRUE);
		}
		else
		{
			//没有子菜单
			cmdUI.m_pSubMenu=NULL;
			cmdUI.DoUpdate(this, FALSE);
		}

		//由于可能增删MenuItem，需要调整cmdUI的m_nIndex及m_nIndexMax
		UINT nCount=pPopupMenu->GetMenuItemCount();
		if (nCount<cmdUI.m_nIndexMax)
		{
			cmdUI.m_nIndex-=(cmdUI.m_nIndexMax-nCount);
			while (cmdUI.m_nIndex<nCount &&  pPopupMenu->GetMenuItemID(cmdUI.m_nIndex)==cmdUI.m_nID)
			{
				cmdUI.m_nIndex++;
			}
		}
		cmdUI.m_nIndexMax = nCount;
	}
}

//=====================================================================================================================================================================
//更新托盘图标菜单
void CServiceManagerDlg::OnUpdatePopmenuAutostart(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if(m_bAutoStart==TRUE)
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

//=====================================================================================================================================================================
//更新托盘图标菜单
void CServiceManagerDlg::OnUpdatePopmenuShow(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if(IsWindowVisible()==TRUE)
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

//=====================================================================================================================================================================
//更新托盘图标菜单
void CServiceManagerDlg::OnUpdatePopmenuHide(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
		if(IsWindowVisible()==TRUE)
	{
		pCmdUI->SetCheck(0);
	}
	else
	{
		pCmdUI->SetCheck(1);
	}
}

//=====================================================================================================================================================================
//自定义函数
//=====================================================================================================================================================================
//添加托盘图标
void CServiceManagerDlg::AddTrayIcon()
{
	//创建托盘图标
	NOTIFYICONDATA nd;
	nd.cbSize=sizeof(NOTIFYICONDATA);
	nd.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nd.hWnd=m_hWnd;
	nd.hIcon=m_hIcon;
	nd.uID=IDR_MAINFRAME;
	nd.uCallbackMessage=ID_TRAY;
	lstrcpy(nd.szTip,TEXT("双击显示窗口"));
	if(Shell_NotifyIcon(NIM_ADD,&nd)==FALSE)
	{
		ShowMeError();
	}

}

//=====================================================================================================================================================================
//删除托盘图标
void CServiceManagerDlg::RemoveTrayIcon()
{
	//创建托盘图标
	NOTIFYICONDATA nd;
	nd.cbSize=sizeof(NOTIFYICONDATA);
	nd.hWnd=m_hWnd;
	nd.uID=IDR_MAINFRAME;
	
	if(Shell_NotifyIcon(NIM_DELETE,&nd)==FALSE)
	{
		//ShowMeError();
	}
}

//=====================================================================================================================================================================
//添加自动运行
BOOL CServiceManagerDlg::SetAutoRun()
{
	HKEY hKey;
	CString	szPath;
	GetModuleFileName(NULL,szPath.GetBuffer(MAX_PATH),MAX_PATH);
	szPath.ReleaseBuffer();
	szPath = TEXT("\"") + szPath +  TEXT("\" TrayIcon");

	BOOL bRet=FALSE;
	if(RegOpenKey(HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\RUN"),&hKey)==ERROR_SUCCESS)
	{
		if(RegSetValueEx(hKey,TEXT("ServiceManager"),0,REG_SZ,(CONST BYTE*)szPath.GetBuffer(szPath.GetLength()),szPath.GetLength()*sizeof(TCHAR))==ERROR_SUCCESS)
		{
			bRet=TRUE;
		}
	}
	RegCloseKey(hKey);

	return bRet;
}

//=====================================================================================================================================================================
//删除自动运行
BOOL CServiceManagerDlg::RemoveAutoRun()
{
	HKEY hKey;
	BOOL bRet=FALSE;
	if(RegOpenKey(HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\RUN"),&hKey)==ERROR_SUCCESS)
	{
		if(RegDeleteValue(hKey,TEXT("ServiceManager"))==ERROR_SUCCESS)
		{
			bRet=TRUE;
		}
	}
	RegCloseKey(hKey);

	return bRet;
}

//=====================================================================================================================================================================
//查询自动运行
BOOL CServiceManagerDlg::QueryAutoRun()
{
	HKEY hKey;
	BOOL bRet=FALSE;
	TCHAR *tcValue = NULL;
	if(RegOpenKey(HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\RUN"),&hKey)==ERROR_SUCCESS)
	{
		DWORD dwCount;
		DWORD dwType;
		LONG lResult=RegQueryValueEx(hKey,TEXT("ServiceManager"),NULL,&dwType,(LPBYTE)tcValue,&dwCount);
		
		tcValue = new TCHAR[dwCount];
		if(RegQueryValueEx(hKey,TEXT("ServiceManager"),NULL,&dwType,(LPBYTE)tcValue,&dwCount)==ERROR_SUCCESS)
		{
			bRet=TRUE;
		}
	}
	RegCloseKey(hKey);

	return bRet;
}

//=====================================================================================================================================================================
//显示操作错误
void CServiceManagerDlg::ShowMeError()
{
	DWORD	nErrorNo;
	LPTSTR	lptBuffer;

	nErrorNo=GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,nErrorNo,LANG_NEUTRAL,(LPTSTR)&lptBuffer,0,NULL);
	MessageBox(lptBuffer,TEXT("Error"),MB_OK);
	LocalFree(lptBuffer);
}

//=====================================================================================================================================================================
//从XML中载入配置信息
BOOL CServiceManagerDlg::LoadConfig()
{
	try
	{
		//创建COM实例
		MSXML2::IXMLDOMDocumentPtr pXml;
		HRESULT hr = pXml.CreateInstance(__uuidof(MSXML2::DOMDocument30));
		if(!SUCCEEDED(hr))
		{
			MessageBox(TEXT("MSXML2::DOMDocument30 Init Error"),ERROR_MSGBOX_TITLE,MB_OK);
			return FALSE;
		}

		MSXML2::IXMLDOMElementPtr	pXmlRoot;
		MSXML2::IXMLDOMNodeListPtr	pXmlNodeList; 
		MSXML2::IXMLDOMNodePtr		pXmlNode;
		MSXML2::DOMNodeType		nodeType;
		MSXML2::IXMLDOMNamedNodeMapPtr	pAttrs;
		MSXML2::IXMLDOMNodePtr		pAttrItem;

		//载入XML
		pXml->load(m_szXML.GetBuffer(m_szXML.GetLength()));
		
		//获取TaskList数量
		pXmlRoot = (MSXML2::IXMLDOMElementPtr)(pXml->selectSingleNode("ServiceManager"));
		if(pXmlRoot == NULL)return TRUE;

		//获取所有TaskList
		pXmlNodeList=pXmlRoot->GetchildNodes();
		long lNodeCount=0l;
		pXmlNodeList->get_length(&lNodeCount);

		//遍历节点
		for(long i=0l;i<lNodeCount;i++)
		{
			pXmlNodeList->get_item(i,&pXmlNode);

			long lAttributeCount=0l;
			BSTR bstrNodeName;
			pXmlNode->get_text(&bstrNodeName);
			pXmlNode->get_nodeType(&nodeType);
			
			if(MSXML2::NODE_ELEMENT==nodeType)
			{
				pXmlNode->get_attributes(&pAttrs);
				pAttrs->get_length(&lAttributeCount);
				m_TaskList.InsertItem(i, (LPCTSTR)bstrNodeName);

				//遍历属性
				for(long j = 0; j < lAttributeCount; j++)
				{
					pAttrs->get_item(j, &pAttrItem);
					CString strAttrName = pAttrItem->nodeName;

					if(strAttrName.CompareNoCase(TEXT("Service1"))==0)
					{
						CString strAttrValue = (_bstr_t)pAttrItem->nodeTypedValue;
						m_TaskList.SetItemText(i, 1, strAttrValue);
					}
					else if(strAttrName.CompareNoCase(TEXT("Service2"))==0)
					{
						CString strAttrValue = (_bstr_t)pAttrItem->nodeTypedValue;
						m_TaskList.SetItemText(i, 2, strAttrValue);
					}
					else if(strAttrName.CompareNoCase(TEXT("Service3"))==0)
					{
						CString strAttrValue = (_bstr_t)pAttrItem->nodeTypedValue;
						m_TaskList.SetItemText(i, 3, strAttrValue);
					}
					else if(strAttrName.CompareNoCase(TEXT("Service4"))==0)
					{
						CString strAttrValue = (_bstr_t)pAttrItem->nodeTypedValue;
						m_TaskList.SetItemText(i, 4, strAttrValue);
					}
					else if(strAttrName.CompareNoCase(TEXT("Service5"))==0)
					{
						CString strAttrValue = (_bstr_t)pAttrItem->nodeTypedValue;
						m_TaskList.SetItemText(i, 5, strAttrValue);
					}
					else if(strAttrName.CompareNoCase(TEXT("MonitorOn"))==0)
					{
						CString strAttrValue = (_bstr_t)pAttrItem->nodeTypedValue;
						if(strAttrValue==TEXT("1"))
						{
							m_TaskList.SetItemText(i, 6, TEXT("*"));
						}
						else
						{
							m_TaskList.SetItemText(i, 6, TEXT(""));
						}
					}
					else if(strAttrName.CompareNoCase(TEXT("MonitorInterval"))==0)
					{
						CString strAttrValue = (_bstr_t)pAttrItem->nodeTypedValue;
						m_TaskList.SetItemText(i, 7, strAttrValue);
					}
				}
			}
		}
	}
	catch(_com_error &e)
	{
		MessageBox(e.ErrorMessage(),ERROR_MSGBOX_TITLE,MB_OK);
		return FALSE;
	}

	return TRUE;
}

//=====================================================================================================================================================================
//根据服务名启动服务
BOOL CServiceManagerDlg::StartServiceByName(CString szServiceName)
{
	//SCMD与Service的句柄
	SC_HANDLE	hSCManager=NULL;
	SC_HANDLE	hService=NULL;
	
	//服务状态
	SERVICE_STATUS_PROCESS	sspServiceStatus; 
	DWORD	dwBytesNeeded;
	
	//等待时间
	DWORD	dwOldCheckPoint; 
	DWORD	dwStartTickCount;
	DWORD	dwWaitTime;
 
	//打开服务管理器
	hSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(NULL == hSCManager) 
	{
		//MessageBox(TEXT("OpenSCManager Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		return FALSE;
	}

	//获取服务句柄
	hService = OpenService(hSCManager,szServiceName,SERVICE_ALL_ACCESS);
	if(hService == NULL)
	{ 
		//MessageBox(TEXT("OpenService Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	//查询服务状态
	if (!QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE) &sspServiceStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded))
	{
		//MessageBox(TEXT("QueryServiceStatusEx Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		CloseServiceHandle(hService); 
		CloseServiceHandle(hSCManager);
		return FALSE; 
	}

	//如果服务已经运行
	if(sspServiceStatus.dwCurrentState != SERVICE_STOPPED && sspServiceStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		CloseServiceHandle(hService); 
		CloseServiceHandle(hSCManager);
		return TRUE; 
	}

	//如果服务正在停止，等待其停止
	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = sspServiceStatus.dwCheckPoint;
	if(dwOldCheckPoint!=0)
	{
		while (sspServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
		{
			//等待一段时间
			dwWaitTime = sspServiceStatus.dwWaitHint/10;
			if(dwWaitTime<1000)
			{
				dwWaitTime = 1000;
			}
			else if (dwWaitTime>10000)
			{
				dwWaitTime = 10000;
			}
			Sleep( dwWaitTime );

			//继续查询服务状态
			if (!QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE) &sspServiceStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded))
			{
				//MessageBox(TEXT("QueryServiceStatusEx Failed"),ERROR_MSGBOX_TITLE,MB_OK);
				CloseServiceHandle(hService); 
				CloseServiceHandle(hSCManager);
				return FALSE; 
			}

			if(sspServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
			{
				if(sspServiceStatus.dwCheckPoint>dwOldCheckPoint)
				{
					//继续等待
					dwStartTickCount = GetTickCount();
					dwOldCheckPoint = sspServiceStatus.dwCheckPoint;
				}
				else
				{
					//等待超时
					if(GetTickCount()-dwStartTickCount>sspServiceStatus.dwWaitHint)
					{
						//MessageBox(TEXT("Timeout waiting for service to stop"),ERROR_MSGBOX_TITLE,MB_OK);
						CloseServiceHandle(hService); 
						CloseServiceHandle(hSCManager);
						return FALSE; 
					}
				}
			}
		}
	}

	//启动服务
	if (!StartService(hService,0,NULL))
	{
		//MessageBox(TEXT("StartService Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		CloseServiceHandle(hService); 
		CloseServiceHandle(hSCManager);
		return FALSE; 
	}

	//检查服务状态
	if (!QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE) &sspServiceStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded))
	{
		//MessageBox(TEXT("QueryServiceStatusEx Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		CloseServiceHandle(hService); 
		CloseServiceHandle(hSCManager);
		return FALSE; 
	}
 
	//等待服务启动完毕
	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = sspServiceStatus.dwCheckPoint;
	if(dwOldCheckPoint!=0)
	{
		while (sspServiceStatus.dwCurrentState == SERVICE_START_PENDING) 
		{
			//等待一段时间
 			dwWaitTime = sspServiceStatus.dwWaitHint/10;
			if( dwWaitTime < 1000 )
			{
				dwWaitTime = 1000;
			}
			else if ( dwWaitTime > 10000 )
			{
				dwWaitTime = 10000;
			}
			Sleep( dwWaitTime );

			//检查服务状态
			if (!QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE) &sspServiceStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded ) )			  // if buffer too small
			{
				//MessageBox(TEXT("QueryServiceStatusEx Failed"),ERROR_MSGBOX_TITLE,MB_OK);
				CloseServiceHandle(hService); 
				CloseServiceHandle(hSCManager);
				return FALSE; 
			}
	 
			if(sspServiceStatus.dwCurrentState == SERVICE_START_PENDING)
			{
				if ( sspServiceStatus.dwCheckPoint > dwOldCheckPoint )
				{
					//继续等待
					dwStartTickCount = GetTickCount();
					dwOldCheckPoint = sspServiceStatus.dwCheckPoint;
				}
				else
				{
					//等待超时
					if(GetTickCount()-dwStartTickCount > sspServiceStatus.dwWaitHint)
					{
						//MessageBox(TEXT("Timeout waiting for service to start up"),ERROR_MSGBOX_TITLE,MB_OK);
						CloseServiceHandle(hService); 
						CloseServiceHandle(hSCManager);
						return FALSE;
					}
				}
			}
		}
	}

	//关闭句柄
	CloseServiceHandle(hService); 
	CloseServiceHandle(hSCManager);
	return TRUE;
}

//=====================================================================================================================================================================
//根据服务名停止服务
BOOL CServiceManagerDlg::StopServiceByName(CString szServiceName)
{
	//SCMD与Service的句柄
	SC_HANDLE	hSCManager=NULL;
	SC_HANDLE	hService=NULL;
	
	//服务状态
	SERVICE_STATUS_PROCESS	sspServiceStatus; 
	DWORD	dwBytesNeeded;
	
	//等待时间
	DWORD	dwOldCheckPoint; 
	DWORD	dwStartTickCount;
	DWORD	dwWaitTime;
 
	//打开服务管理器
	hSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(NULL == hSCManager) 
	{
		//MessageBox(TEXT("OpenSCManager Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		return FALSE;
	}

	//获取服务句柄
	hService = OpenService(hSCManager,szServiceName,SERVICE_ALL_ACCESS);
	if(hService == NULL)
	{ 
		//MessageBox(TEXT("OpenService Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	//查询服务状态
	if (!QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE) &sspServiceStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded))
	{
		//MessageBox(TEXT("QueryServiceStatusEx Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		CloseServiceHandle(hService); 
		CloseServiceHandle(hSCManager);
		return FALSE; 
	}

	//如果服务已经停止
	if(sspServiceStatus.dwCurrentState != SERVICE_RUNNING && sspServiceStatus.dwCurrentState != SERVICE_START_PENDING)
	{
		CloseServiceHandle(hService); 
		CloseServiceHandle(hSCManager);
		return TRUE; 
	}

	//等待服务启动完毕
	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = sspServiceStatus.dwCheckPoint;
	if(dwOldCheckPoint!=0)
	{
		while (sspServiceStatus.dwCurrentState == SERVICE_START_PENDING) 
		{
			//等待一段时间
 			dwWaitTime = sspServiceStatus.dwWaitHint/10;
			if( dwWaitTime < 1000 )
			{
				dwWaitTime = 1000;
			}
			else if ( dwWaitTime > 10000 )
			{
				dwWaitTime = 10000;
			}
			Sleep( dwWaitTime );

			//检查服务状态
			if (!QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE) &sspServiceStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded ) )			  // if buffer too small
			{
				//MessageBox(TEXT("QueryServiceStatusEx Failed"),ERROR_MSGBOX_TITLE,MB_OK);
				CloseServiceHandle(hService); 
				CloseServiceHandle(hSCManager);
				return FALSE; 
			}
	 
			if(sspServiceStatus.dwCurrentState == SERVICE_START_PENDING)
			{
				if ( sspServiceStatus.dwCheckPoint > dwOldCheckPoint )
				{
					//继续等待
					dwStartTickCount = GetTickCount();
					dwOldCheckPoint = sspServiceStatus.dwCheckPoint;
				}
				else
				{
					//等待超时
					if(GetTickCount()-dwStartTickCount > sspServiceStatus.dwWaitHint)
					{
						//MessageBox(TEXT("Timeout waiting for service to start up"),ERROR_MSGBOX_TITLE,MB_OK);
						CloseServiceHandle(hService); 
						CloseServiceHandle(hSCManager);
						return FALSE;
					}
				}
			}
		}
	}

	//停止服务
	SERVICE_STATUS SvcStatus;
	if (!ControlService(hService, SERVICE_CONTROL_STOP, &SvcStatus))
	{
		//MessageBox(TEXT("StartService Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		CloseServiceHandle(hService); 
		CloseServiceHandle(hSCManager);
		return FALSE; 
	}

	//检查服务状态
	if (!QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE) &sspServiceStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded))
	{
		//MessageBox(TEXT("QueryServiceStatusEx Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		CloseServiceHandle(hService); 
		CloseServiceHandle(hSCManager);
		return FALSE; 
	}
 
	//如果服务正在停止，等待其停止
	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = sspServiceStatus.dwCheckPoint;
	if(dwOldCheckPoint!=0)
	{
		while (sspServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
		{
			//等待一段时间
			dwWaitTime = sspServiceStatus.dwWaitHint/10;
			if(dwWaitTime<1000)
			{
				dwWaitTime = 1000;
			}
			else if (dwWaitTime>10000)
			{
				dwWaitTime = 10000;
			}
			Sleep( dwWaitTime );

			//继续查询服务状态
			if (!QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE) &sspServiceStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded))
			{
				//MessageBox(TEXT("QueryServiceStatusEx Failed"),ERROR_MSGBOX_TITLE,MB_OK);
				CloseServiceHandle(hService); 
				CloseServiceHandle(hSCManager);
				return FALSE; 
			}

			if(sspServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
			{
				if(sspServiceStatus.dwCheckPoint>dwOldCheckPoint)
				{
					//继续等待
					dwStartTickCount = GetTickCount();
					dwOldCheckPoint = sspServiceStatus.dwCheckPoint;
				}
				else
				{
					//等待超时
					if(GetTickCount()-dwStartTickCount>sspServiceStatus.dwWaitHint)
					{
						//MessageBox(TEXT("Timeout waiting for service to stop"),ERROR_MSGBOX_TITLE,MB_OK);
						CloseServiceHandle(hService); 
						CloseServiceHandle(hSCManager);
						return FALSE; 
					}
				}
			}
		}
	}

	//关闭句柄
	CloseServiceHandle(hService); 
	CloseServiceHandle(hSCManager);
	return TRUE;
}

//=====================================================================================================================================================================
//根据服务名，判断服务是否已经运行
BOOL CServiceManagerDlg::IsServiceOnByName(CString szServiceName)
{
	//SCMD与Service的句柄
	SC_HANDLE	hSCManager=NULL;
	SC_HANDLE	hService=NULL;
	
	//服务状态
	SERVICE_STATUS_PROCESS	sspServiceStatus; 
	DWORD	dwBytesNeeded;
 
	//打开服务管理器
	hSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(NULL == hSCManager) 
	{
		//MessageBox(TEXT("OpenSCManager Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		return FALSE;
	}

	//获取服务句柄
	hService = OpenService(hSCManager,szServiceName,SERVICE_ALL_ACCESS);
	if(hService == NULL)
	{ 
		//MessageBox(TEXT("OpenService Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		CloseServiceHandle(hSCManager);
		return FALSE;
	}

	//查询服务状态
	if (!QueryServiceStatusEx(hService,SC_STATUS_PROCESS_INFO,(LPBYTE) &sspServiceStatus,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded))
	{
		//MessageBox(TEXT("QueryServiceStatusEx Failed"),ERROR_MSGBOX_TITLE,MB_OK);
		CloseServiceHandle(hService); 
		CloseServiceHandle(hSCManager);
		return FALSE; 
	}

	//如果服务已经运行
	if(sspServiceStatus.dwCurrentState != SERVICE_STOPPED && sspServiceStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		CloseServiceHandle(hService); 
		CloseServiceHandle(hSCManager);
		return TRUE; 
	}

	CloseServiceHandle(hService); 
	CloseServiceHandle(hSCManager);
	return FALSE;
}

//=====================================================================================================================================================================
//计时器回调函数
VOID CALLBACK CServiceManagerDlg::TimerMonitor(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
	if(m_bIsBussy==TRUE || m_pThis==NULL)return;
	m_bIsBussy=TRUE;

	//监视服务运行
	int nItemCount = m_pThis->m_TaskList.GetItemCount();
	CString szMonitorOn;
	CString szMonitorInterval;
	CString szMonitorRefresh;
	CString szValue;
	int nMonitorInterval=0;
	int nMonitorRefresh=0;

	for(int i=0;i<nItemCount;i++)
	{
		szMonitorOn=m_pThis->m_TaskList.GetItemText(i,6);
		szMonitorInterval=m_pThis->m_TaskList.GetItemText(i,7);
		szMonitorRefresh=m_pThis->m_TaskList.GetItemText(i,8);

		nMonitorInterval=_ttoi(szMonitorInterval);
		nMonitorRefresh=_ttoi(szMonitorRefresh);

		if(szMonitorOn==TEXT("*") && nMonitorInterval>0)
		{
			if(nMonitorRefresh==0)
			{
				nMonitorRefresh=nMonitorInterval;
			}
			else if(nMonitorRefresh==1)
			{
				//依次启动程序组内全部服务
				szValue=m_pThis->m_TaskList.GetItemText(i,1);
				if(szValue.GetLength()>0)StartServiceByName(szValue);

				szValue=m_pThis->m_TaskList.GetItemText(i,2);
				if(szValue.GetLength()>0)StartServiceByName(szValue);

				szValue=m_pThis->m_TaskList.GetItemText(i,3);
				if(szValue.GetLength()>0)StartServiceByName(szValue);

				szValue=m_pThis->m_TaskList.GetItemText(i,4);
				if(szValue.GetLength()>0)StartServiceByName(szValue);

				szValue=m_pThis->m_TaskList.GetItemText(i,5);
				if(szValue.GetLength()>0)StartServiceByName(szValue);

				nMonitorRefresh=nMonitorInterval;
			}
			else
			{
				nMonitorRefresh--;
			}

			szValue.Format(TEXT("%d"),nMonitorRefresh);
			m_pThis->m_TaskList.SetItemText(i,8,szValue);
		}
	}

	//定时检查服务状态
	if(m_pThis->m_nRefreshServicesStatis>=REFRESH_INTERVAL)
	{
		for(int i=0;i<nItemCount;i++)
		{
			for(int j=1;j<6;j++)
			{
				CString szServicesName = m_pThis->m_TaskList.GetItemText(i,j);
				if(IsServiceOnByName(szServicesName))
				{
					m_pThis->m_TaskList.setSubItemColor(i,j,CNeoSubItemColor::GreenColor);
				}
				else
				{
					m_pThis->m_TaskList.setSubItemColor(i,j,CNeoSubItemColor::RedColor);
				}
			}
		}

		m_pThis->m_TaskList.RedrawWindow();
		m_pThis->m_nRefreshServicesStatis=0;
	}
	else
	{
		m_pThis->m_nRefreshServicesStatis++;
	}


	m_bIsBussy=FALSE;
}

//=====================================================================================================================================================================
