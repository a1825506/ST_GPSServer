
// GPSServerDlg.cpp : 实现文件
#include "stdafx.h"
#include "GPSServer.h"
#include "WebPage.h"
#include "GPSServerDlg.h"
#include "afxdialogex.h"
#include <string>
#include<time.h>
#include <map>
#include  "winsock.h"
#include "mysql.h" 
#include <json.h>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
	
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	


END_MESSAGE_MAP()


// CGPSServerDlg 对话框




CGPSServerDlg::CGPSServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGPSServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_JD = _T("");
	m_WD = _T("");
}

void CGPSServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_SHOWINFO,mshowinfo);
	DDX_Control(pDX,IDC_EDITUsername,musername);
	DDX_Control(pDX,IDC_EDITPassword,mpassword);
   DDX_Control(pDX,IDC_EDITPort,mport);
}

BEGIN_MESSAGE_MAP(CGPSServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
		ON_WM_TIMER()  
	ON_BN_CLICKED(IDC_BUTTONStart,CGPSServerDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_RUNNINGSYSTEM,CGPSServerDlg::OnBnClickedRunSys)
	//添加消息映射
	ON_MESSAGE(WM_STATUSINFO,dealwith_statusmsg)
	ON_MESSAGE(WM_RECEIVEINFO,dealwith_parammsg)
	ON_MESSAGE(WM_DELETEINFO,dealwith_deletemsg)
	ON_MESSAGE(WM_SENDINFO,dealwith_sendmsg)
	ON_MESSAGE(WM_REBOOTINFO,dealwith_reboot)
	ON_MESSAGE(WM_RESTARTINFO,dealwith_restart)

	ON_MESSAGE(WM_SHOWTASK,onShowTask)       
END_MESSAGE_MAP()


// CGPSServerDlg 消息处理程序

BOOL CGPSServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	mport.SetWindowText("20086");
	musername.SetWindowText("root");
	mpassword.SetWindowText("");
    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGPSServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标  121.464325  31.19897
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CGPSServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/************************************************************************/
/* 启动服务                                                                     */
/************************************************************************/
 void CGPSServerDlg::OnBnClickedStart()
{
	 nport=0;
	 tts=0;
	 isrunning=true;
	 mport.GetWindowText(m_port);
	 musername.GetWindowText(m_username);
	 mpassword.GetWindowText(m_password);
	 nport = atoi(m_port);
	 dbHelper = new DBHelper();
	 if(!dbHelper->ConnectDatabase(m_username,m_password)){
		 return ;
	 }
	 //启动socket初始化线程
	 m_pThread_server=AfxBeginThread(ThreadFunction_server,(LPVOID)nport);//创建主线程
	 SetTimer(1,5000,NULL);
}
/************************************************************************/
/* 处理展示服务器以及数据库运行的状态 
*/
/************************************************************************/
LRESULT CGPSServerDlg::dealwith_statusmsg(WPARAM wParam,LPARAM lparam)
{

	CString pstr;
	pstr=(char *)lparam;
	GetDlgItem(IDC_EDIT1)->SetWindowText(pstr);
	if(pstr=="正在监听..."){
		GetDlgItem(IDC_BUTTONStart)->EnableWindow(FALSE);
	}else if(pstr=="数据库连接失败！"){

	}else if(pstr=="服务停止..."){

		GetDlgItem(IDC_BUTTONStart)->EnableWindow(TRUE);
	}
	return 0;
}

/************************************************************************/
/* 主线程启动socket 监听端口 ，接收和下发消息                                                                    */
/************************************************************************/
UINT ThreadFunction_server( LPVOID pParam )
{
	WSADATA wsa;
	SOCKET Listen_Sock;
	SOCKADDR_IN serverAddr;//创建一个SOCKADDR_IN结构的对象
	
	//初始化socket资源
	if (WSAStartup(MAKEWORD(1,1),&wsa) != 0)
	{
		::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"初始化失败！");
		WSACleanup();
	}
	//创建socket基于TCP的。
	Listen_Sock = socket(AF_INET, SOCK_STREAM, 0);
	//创建socket基于UDP的。
	//Listen_Sock = socket(AF_INET, SOCK_DGRAM, 0);

	if(Listen_Sock==INVALID_SOCKET)
	{
		::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"socket创建失败！");
	}
   
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((int)pParam); //本地监听端口:20086
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int retval;  
	retval=bind(Listen_Sock,(sockaddr*)&serverAddr,sizeof(serverAddr));
	//给套接字绑定地址
	if(retval==SOCKET_ERROR)
	{
		::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"绑定失败！");
		closesocket(Listen_Sock);

	}else{
		::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"服务开启成功");
	}

	     AfxBeginThread(ThreadFunction_listen,(LPVOID)Listen_Sock);//创建监听套接字的线程
	    WaitForSingleObject(hEvents_listen,INFINITE);//在线程执行完之前阻塞主线程

        AfxBeginThread(ThreadFunction_accept,(LPVOID)Listen_Sock);//创建accept线程
	   WaitForSingleObject(hEvents_accept,INFINITE);//在线程执行完之前阻塞主线程
	   return 0;
}
/************************************************************************/
/*       ThreadFunction_listen    :监听线程函数 */
/************************************************************************/
UINT ThreadFunction_listen( LPVOID pParam )
{
	if(listen((SOCKET)pParam,50)==SOCKET_ERROR)
	{
		::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"监听失败！");
		isrunning=false;
		closesocket((SOCKET)pParam);
		exit(0);
	}
	else
	{
		::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"正在监听...");
	    isrunning =true;
	}
	SetEvent(hEvents_listen);
	return 0;
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
UINT ThreadFunction_accept( LPVOID pParam )
{
	SOCKET  sockClient;
	SOCKADDR_IN  clientAddr;
	int sizeclient=sizeof(clientAddr);
	//循环accept，让多个客户端连接 使用while(1)则没有数量限制
	while(1)
	{
		sockClient = accept((SOCKET)pParam,(sockaddr *)&clientAddr,&sizeclient);//执行accept，等待客户端连接
		if(sockClient==INVALID_SOCKET)
		{
			//客户端连接失败
			continue;
		}
		//客户端连接成功
		AfxBeginThread(ThreadFunction_recv,(LPVOID)sockClient);//创建接受线程
		if(!isrunning){
			::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"服务停止...");
			::SendMessage(AfxGetMainWnd()->m_hWnd,WM_DELETEINFO,(SOCKET)pParam,0);
			closesocket((SOCKET)pParam);
			return 0;
		}
	}
	SetEvent(hEvents_accept);//线程同步
	return 0;
}

/*********************************************************/
/*   ThreadFunction_recv接收消息线程
       接收消息分为接收信标消息和接收
	   来自控制手机的消息
*/
/*******************************************************/
UINT ThreadFunction_recv( LPVOID pParam )
{
	//循环接收
	while(1)
	{
		char recvbuf[1024]={0};
		int recvd=recv((SOCKET)pParam,recvbuf,sizeof(recvbuf)+1,0);
		string str(recvbuf);
		if(!isrunning){
			return 0;
		}
		if(recvd<=0){
			//调出循环。
			break;
		}else{
			//对接收到的消息分别处理
			//处理客户端发送实时参数的消息
			::SendMessage(AfxGetMainWnd()->m_hWnd,WM_RECEIVEINFO,(SOCKET)pParam,(LPARAM)recvbuf);
		}
	}
	//客户端连接断开。删除map_socket表和GPSlist列表控件里的该项（包括连接的信标设备和手机控制端）
		::SendMessage(AfxGetMainWnd()->m_hWnd,WM_DELETEINFO,(SOCKET)pParam,0);

	return 0;
}

/************************************************************************/
/* 处理接收到的消息*/
/************************************************************************/
LRESULT  CGPSServerDlg::dealwith_parammsg(WPARAM wParam,LPARAM lparam)
{
	string receivemsg="";
	receivemsg = (char *)lparam;
	string ordertype=receivemsg.substr(1,2);//命令类型"ST"表示来时手机控制端的命令。"3G"表示来自终端定位设备的命令
	if(ordertype=="ST"){//
		string imei = receivemsg.substr(4,15);
		map<SOCKET,string>::iterator iter=STmap_socket.find((SOCKET)wParam);
		 vector <string> master_msg = split(receivemsg,"*,");//解析手机控制端发送的消息代码
		if(iter!=STmap_socket.end()){//在STmap_socket中存在该连接,根据命令格式做操作。
		  	if(master_msg[2]=="GetDivice]"){
				//查询数据库中设备列表
				dbHelper->GetDivice((SOCKET)wParam,master_msg[1].c_str());
			}else if(master_msg[2]=="ADDDivice"){//
				dbHelper->ADDDivice((SOCKET)wParam,master_msg[1].c_str(),master_msg[3].c_str(),master_msg[4].c_str(),master_msg[5].c_str());
			}else if(master_msg[2]=="DeleteDivice"){
				dbHelper->DeleteDivice((SOCKET)wParam,master_msg[1].c_str(),master_msg[3].c_str());
			}else if(master_msg[3]=="UPLOAD"){
					string uoload = "";
					for(map <SOCKET,string>::iterator it=map_socket.begin();it!=map_socket.end();it++)
					{
						if(it->second==master_msg[1]){
					
							map<SOCKET,SOCKET>::iterator Forwarding_iter=Forwarding_map.find((SOCKET)wParam);

							if(Forwarding_iter!=Forwarding_map.end()){

							}else{
								//
								Forwarding_map[(SOCKET)wParam] = it->first;
							}
							uoload = "[3G*"+master_msg[1]+"*0002*UPLOAD,"+master_msg[4];
							::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)it->first,(LPARAM)uoload.c_str());
						}
					}

			}else if(master_msg[3]=="DeleteLocus]"){
				dbHelper->DeleteLocus(master_msg[1],master_msg[2]);
			}else if(master_msg[3]=="GetlatLng]"){
				string getLatlng = "";
				//手机控制端请求某一设备的实时位置。需要找出该设备保存在map_socket中的连接。
				//再向该设备发送请求实时定位命令。
				for(map <SOCKET,string>::iterator it=map_socket.begin();it!=map_socket.end();it++)
				{
					if(it->second==master_msg[2]){
						//在map_socket设备连接表中找到value值为当前手机请求设备的IMEI的连接。该连接即为手机请求的设备
						//的连接。向该设备发送请求位置信息
						getLatlng = "[3G*"+master_msg[2]+"*0002*CR]";
						dbHelper->GetlatLng((SOCKET)it->first,getLatlng);
						//再将该手机连接和设备连接写入转发表，保证转发表中每次都是只有一条转发记录。每次使用前清空之前的记录
						if(Forwarding_map.size()!=0){
							map <SOCKET,SOCKET>::iterator it=Forwarding_map.begin();
							Forwarding_map.erase(it);
						}
						map<SOCKET,SOCKET>::iterator Forwarding_iter=Forwarding_map.find((SOCKET)wParam);
						if(Forwarding_iter!=Forwarding_map.end()){
							//如果在Forwarding_map转发表中有该转发记录则不做操作
						}else{
							//如果转发表中没有该转发记录则在转发表中增加该记录,
							Forwarding_map[(SOCKET)wParam]=it->first;
						}
					}
			   }
			}else if(master_msg[3]=="GetLocus]"){
				//手机控制端请求数据库中轨迹数据。
				dbHelper->GetLocus((SOCKET)wParam,master_msg[2].c_str());
			}else if(master_msg[4]=="LocusDetails]"){
				//请求具体的轨迹数据
				dbHelper->GetLocusDetails((SOCKET)wParam,master_msg[2].c_str(),master_msg[3].c_str());
			}
		}else{//在STmap_socket中不存在该连接
		     STmap_socket[(SOCKET)wParam] = imei;
			 if(master_msg[2]=="Connect]"){
				 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)"[ST*Login*OK]");
			 }
	}
	}else{
		map<SOCKET,string>::iterator iter=map_socket.find((SOCKET)wParam);
		 vector <string> divice_msg = split(receivemsg,"*,");//解析手机控制端发送的消息代码
		if(iter!=map_socket.end()){//在map_socket中存在该连接,根据命令格式做操作。
			if(divice_msg[3]=="UD"){//终端设备发送位置上报消息，保存位置信息到数据库，并更新当前位置信息m_JD，mm_WD;
			string n_str="";//
			 n_str = "[ST*Divice*GetLatLng,";
			vector <string> location_msg = split(receivemsg,",");	
			m_JD = location_msg[6].c_str();
	    	m_WD = location_msg[4].c_str();
			m_date= location_msg[1].c_str();
			m_DiviceIMEI = divice_msg[1].c_str();
			dbHelper->InsertData(m_WD,m_JD,m_DiviceIMEI,m_date);

			//检查转发表中是否有该设备转发的连接存在如果存在就进行转发到手机控制端的操作
			for(map <SOCKET,SOCKET>::iterator it=Forwarding_map.begin();it!=Forwarding_map.end();it++)
			{
				if(it->second==((SOCKET)wParam))
				{
					n_str+=receivemsg;
					 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,it->first,(LPARAM)n_str.c_str());
				}
			}
			}else if(divice_msg[3]=="UD2"){
				string n_str="";
				n_str = "[ST*Divice*GetLatLng,";
				vector <string> location_msg = split(receivemsg,",");	
				m_JD = location_msg[6].c_str();
				m_WD = location_msg[4].c_str();
				m_date= location_msg[1].c_str();
				m_DiviceIMEI = divice_msg[1].c_str();
				dbHelper->InsertData(m_WD,m_JD,m_DiviceIMEI,m_date);
			}else if(divice_msg[3]=="LK"){
			  //链路保持
			  ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)receivemsg.c_str());
			}
		}else{//在map_socket中不存在该连接
			map_socket[(SOCKET)wParam] = divice_msg[1];
			dbHelper->UpdateOnLine1(divice_msg[1].c_str());
			::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)receivemsg.c_str());
		}
	}
	return 0;
}
/************************************************************************/
/* 客户端连接断开处理：包括数据库里面信标设备的在线离、线处理 */
/************************************************************************/

LRESULT CGPSServerDlg::dealwith_deletemsg(WPARAM wParam,LPARAM lparam)
{
	string delete_IMEI="";
	//删除设备连接表
	for(map <SOCKET,string>::iterator it=map_socket.begin();it!=map_socket.end();)
	{
		if(it->first==((SOCKET)wParam))
		{
			delete_IMEI = it->second;		
			map_socket.erase(it);
			break;
		}else 
			it++;
	}
	dbHelper->UpdateOnLine(delete_IMEI.c_str());
   //删除转发表中的记录
	for(map <SOCKET,SOCKET>::iterator it=Forwarding_map.begin();it!=Forwarding_map.end();)
	{
		if(it->first==((SOCKET)wParam))
		{
			Forwarding_map.erase(it);
			break;
		}else 
			it++;
	}
	//删除手机控制端连接表
	for(map <SOCKET,string>::iterator it=STmap_socket.begin();it!=STmap_socket.end();)
	{

		if(it->first==((SOCKET)wParam))
		{
			STmap_socket.erase(it);
			break;
		}else 
			it++;
	}
	dealwith_showmsg("","设备"+delete_IMEI+"断开连接");
	return 0;
}
/************************************************************************/
/*     处理发送消息   
       wParam ：socket连接
	   lparam  ：需要发送的消息
*/                             
/************************************************************************/
LRESULT CGPSServerDlg::dealwith_sendmsg(WPARAM wParam,LPARAM lparam)
{
	char sendBuf[1024*10]={0};
	strcpy(sendBuf,(char *)lparam);
	dealwith_showmsg("",sendBuf);
	int byte = 0;
	byte= send((SOCKET)wParam,sendBuf,strlen(sendBuf),0);
	if(byte<=0){
	}
	return 0;
}
/************************************************************************/
/*       处理服务器重新启动相关操作                                                               */
/************************************************************************/
LRESULT CGPSServerDlg::dealwith_reboot(WPARAM wParam,LPARAM lparam)
{
	isrunning=false;
	SOCKET sockClient;  
	//2.初始化socket  
	WSAData wsaData;  
	WSAStartup(MAKEWORD(2, 2), &wsaData);  
	//3.设置socket  
	sockClient = socket(AF_INET, SOCK_STREAM, 0);   //TCP,字节流  
	//4.设置连接信息  
	sockaddr_in address;  

	address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  

	address.sin_family = AF_INET;  

	address.sin_port = htons(nport);  //确认该端口未被占用  
	//5.连接  

	connect(sockClient, (const struct sockaddr *)&address, sizeof(address));  

	KillTimer(1);
	//Sleep(2000);
	 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_RESTARTINFO,0,0);

	return 0;
}
/************************************************************************/
/*         重新启动                                                            */
/************************************************************************/

LRESULT CGPSServerDlg::dealwith_restart(WPARAM wParam,LPARAM lparam)
{
	
	rebootnum++;
    nport=0;
	tts=0;
	isrunning=true;
		
		 	//启动socket初始化线程
	m_pThread_server=AfxBeginThread(ThreadFunction_server,(LPVOID)nport);//创建主线程
    SetTimer(1,5000,NULL);
    return 0;
}
/************************************************************************/
/* 分割字符串.c++没有现成的split函数需自己实现     */
/************************************************************************/
	vector<string> CGPSServerDlg::split(string& s, string seperator)  
	{  
		vector<string> result;
		typedef string::size_type string_size;
		string_size i = 0;

		while(i != s.size()){
			//找到字符串中首个不等于分隔符的字母；
			int flag = 0;
			while(i != s.size() && flag == 0){
				flag = 1;
				for(string_size x = 0; x < seperator.size(); ++x)
					if(s[i] == seperator[x]){
						++i;
						flag = 0;
						break;
					}
			}

			//找到又一个分隔符，将两个分隔符之间的字符串取出；
			flag = 0;
			string_size j = i;
			while(j != s.size() && flag == 0){
				for(string_size x = 0; x < seperator.size(); ++x)
					if(s[j] == seperator[x]){
						flag = 1;
						break;
					}
					if(flag == 0) 
						++j;
			}
			if(i != j){
				result.push_back(s.substr(i, j-i));
				i = j;
			}
		}
		return result;
	}  
	/************************************************************************/
	/* 消息显示区域                                                                     */
	/************************************************************************/
	void CGPSServerDlg:: dealwith_showmsg(string user,string  show_msg )
	{
		show_num++;
		if(show_num>20){
			show_num=0;
			mshowinfo.SetWindowText("");
		}
		time_t t = time(0); 
		char tmp[64]; 
		strftime( tmp, sizeof(tmp), "%Y-%m-%d %X",localtime(&t) ); 
		string str_time(tmp);
		string str =str_time+":"+show_msg+"\r\n";
		 mshowinfo.ReplaceSel(str.c_str());
	}
   void CGPSServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
   {
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
  }
   /************************************************************************/
   /*       定时器  软件长时间运行会出现不能接受手机端的消息问题
             暂时解决方案就是重启服务。
   */
   /************************************************************************/
   void CGPSServerDlg::OnTimer(UINT_PTR nIDEvent)
   {
	   switch (nIDEvent)
	   {
	      case 1:
			  tts=tts+5;
			  if(tts==20){
				  ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_REBOOTINFO,0,0);
			  }
				 break;
	   }
	   CDialogEx::OnTimer(nIDEvent);
   }

   /************************************************************************/
   /* 响应系统托盘                                                                     */
   /************************************************************************/
   LRESULT CGPSServerDlg::onShowTask(WPARAM wParam,LPARAM lParam)   
	   //wParam接收的是图标的ID，而lParam接收的是鼠标的行为   
   {   
	   if(wParam!=IDI_ICON1)   
		   return 1;   
	   switch(lParam)   
	   {      
	   case WM_RBUTTONUP://右键起来时弹出快捷菜单   
		   {   

			   LPPOINT lpoint=new tagPOINT;   
			   ::GetCursorPos(lpoint);//得到鼠标位置   
			   CMenu menu;   
			   menu.CreatePopupMenu();//声明一个弹出式菜单   
			   //增加菜单项“关闭”，点击则发送消息WM_DESTROY给主窗口（已   
			   //隐藏），将程序结束。   
			   menu.AppendMenu(MF_STRING,WM_DESTROY,"关闭");   
			   menu.AppendMenu(MF_STRING,WM_SHOWWINDOW,"打开");  
			   //确定弹出式菜单的位置   
			   menu.TrackPopupMenu(TPM_LEFTALIGN,lpoint->x,lpoint->y,this);   
			   //资源回收   
			   HMENU hmenu=menu.Detach();   
			   menu.DestroyMenu();           
			   delete lpoint;   
		   }   
		   break;   
	   case WM_LBUTTONDBLCLK://双击左键的处理   
		   {   
			   this->ShowWindow(SW_SHOW);//显示主窗口
		   }   
		   break;   
	   }   
	   return 0;   
   }   
   void CGPSServerDlg::OnBnClickedRunSys()
   {
	   NOTIFYICONDATA nid;   
	   nid.cbSize=(DWORD)sizeof(NOTIFYICONDATA);   
	   nid.hWnd=this->m_hWnd;   
	   nid.uID=IDI_ICON1;   
	   nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP ;   
	   nid.uCallbackMessage=WM_SHOWTASK;//自定义的消息名称   
	   nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_ICON1));   
	   strcpy(nid.szTip,"GPSServer");    //信息提示条  
	   Shell_NotifyIcon(NIM_ADD,&nid);    //在托盘区添加图标   
	   ShowWindow(SW_HIDE);    //隐藏主窗口   
   }


