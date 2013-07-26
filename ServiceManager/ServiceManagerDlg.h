//=====================================================================================================================================================================
#pragma once

#define REFRESH_INTERVAL (5)

#include "NeoListCtrl.h"

//=====================================================================================================================================================================
//CServiceManagerDlg 对话框
class CServiceManagerDlg : public CDialog
{
public:
	//构造函数
	CServiceManagerDlg(CWnd* pParent = NULL);

	//对话框数据
	enum { IDD = IDD_ServiceManager_DIALOG };
	BOOL m_bInitHide;

protected:
	 //DDX/DDV support
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	//图标
	HICON			m_hIcon;
	CNeoListCtrl	m_TaskList;
	CString			m_szXML;
	CMenu			*m_pPopMenu;
	BOOL			m_bAutoStart;

	//消息映射函数
	virtual BOOL OnInitDialog();
	BOOL LoadConfig();
	static BOOL StartServiceByName(CString szServiceName);
	static BOOL StopServiceByName(CString szServiceName);
	static BOOL IsServiceOnByName(CString szServiceName);
	void AddTrayIcon();
	void RemoveTrayIcon();
	void ShowMeError();
	BOOL SetAutoRun();
	BOOL RemoveAutoRun();
	BOOL QueryAutoRun();
	static VOID CALLBACK TimerMonitor(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);

	static BOOL m_bIsBussy;
	static CServiceManagerDlg*	m_pThis;
	int m_nRefreshServicesStatis;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtStart();
	afx_msg void OnBnClickedBtShutdown();
	afx_msg void OnBnClickedBtRestart();
	afx_msg void OnBnClickedBtDel();
	afx_msg void OnBnClickedBtAdd();
	afx_msg void OnBnClickedBtSave();
	afx_msg void OnBnClickedBtUpdate();
	LRESULT OnTrayNotify(WPARAM wParam,LPARAM lParam);
	afx_msg void OnLvnItemchangedListTask(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnPopmenuExit();
	afx_msg void OnPopmenuShow();
	afx_msg void OnPopmenuHide();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnUpdatePopmenuShow(CCmdUI *pCmdUI);
	afx_msg void OnUpdatePopmenuHide(CCmdUI *pCmdUI);
	afx_msg void OnNcPaint();
	afx_msg void OnPopmenuAutostart();
	afx_msg void OnUpdatePopmenuAutostart(CCmdUI *pCmdUI);
};

//=====================================================================================================================================================================
