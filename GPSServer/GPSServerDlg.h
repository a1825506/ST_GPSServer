
// GPSServerDlg.h : 头文件
//
#pragma once
#include <Windows.h>
#include "afxwin.h"
#include "afxcmn.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include "DBHelper.h"
#include <cstring>

UINT ThreadFunction_server( LPVOID pParam );
UINT ThreadFunction_listen( LPVOID pParam );
UINT ThreadFunction_accept( LPVOID pParam );
UINT ThreadFunction_recv( LPVOID pParam);
//线程同步用到的变量
static CEvent hEvents_listen;
static CEvent hEvents_accept;
//保存连接上服务器的设备信息。
static map<SOCKET,string> map_socket;
//保存连接上服务器的手机控制端信息
static map<SOCKET,string> STmap_socket;
//定义一张转发表，用于主控端设备请求到指定设备的定位数据时，服务器返回给请求的主控端
//使用multimap能实现多对多的关系。
static multimap<SOCKET,SOCKET>Forwarding_map;


// CGPSServerDlg 对话框
class CGPSServerDlg : public CDialogEx
{
// 构造
public:
	CGPSServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_GPSSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CWinThread *m_pThread_server;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg  bool ConnectDatabase();//连接数据库
    afx_msg vector <string>split(string& str, string separate_character);
	afx_msg LRESULT dealwith_statusmsg(WPARAM wParam,LPARAM lparam);
	afx_msg LRESULT dealwith_parammsg(WPARAM wParam,LPARAM lparam);
	afx_msg LRESULT dealwith_deletemsg(WPARAM wParam,LPARAM lparam);
	afx_msg LRESULT dealwith_sendmsg(WPARAM wParam,LPARAM lparam);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	
	 int show_num;
	CString	m_JD;
	CString	m_WD;
	CString    m_date;
	CString    m_time;
	CString    m_DiviceIMEI;
	DBHelper* dbHelper;
	CEdit mshowinfo;
	 void dealwith_showmsg(string user,string show_msg);
};
