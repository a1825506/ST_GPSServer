
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

}

BEGIN_MESSAGE_MAP(CGPSServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	//添加消息映射
	ON_MESSAGE(WM_STATUSINFO,dealwith_statusmsg)
	ON_MESSAGE(WM_RECEIVEINFO,dealwith_parammsg)
	ON_MESSAGE(WM_DELETEINFO,dealwith_deletemsg)
	ON_MESSAGE(WM_SENDINFO,dealwith_sendmsg)
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
	dbHelper = new DBHelper();
	if(!dbHelper->ConnectDatabase()){
				return FALSE;
	}
	//启动socket初始化线程
	m_pThread_server=AfxBeginThread(ThreadFunction_server,NULL);//创建主线程

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


/************************************************************************/
/* 处理展示服务器以及数据库运行的状态                                                                     */
/************************************************************************/
LRESULT CGPSServerDlg::dealwith_statusmsg(WPARAM wParam,LPARAM lparam)
{
	CString pstr;
	pstr=(char *)lparam;
	GetDlgItem(IDC_EDIT1)->SetWindowText(pstr);
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
	//创建socket
	Listen_Sock = socket(AF_INET, SOCK_STREAM, 0);
	if(Listen_Sock==INVALID_SOCKET)
	{

		::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"socket创建失败！");
	}


	//ZeroMemory((char *)&serverAddr,sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(20086); //本地监听端口:20086
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//给套接字绑定地址
	if(bind(Listen_Sock,(sockaddr *)&serverAddr,sizeof(serverAddr))==SOCKET_ERROR)
	{
		//printf("%d",WSAGetLastError()); 
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
/*       ThreadFunction_listen    :监听线程函数                                              */
/************************************************************************/
UINT ThreadFunction_listen( LPVOID pParam )
{
	if(listen((SOCKET)pParam,50)==SOCKET_ERROR)
	{
		::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"监听失败！");
		closesocket((SOCKET)pParam);
		exit(0);
	}
	else
	{
		::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"正在监听...");
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
	}
	SetEvent(hEvents_accept);//线程同步
	return 0;

}

/************************************************************************/
/*   ThreadFunction_recv接收消息线程
       接收消息分为接收信标消息和接收
	   来自控制手机的消息
*/
/************************************************************************/
UINT ThreadFunction_recv( LPVOID pParam )
{
	//循环接收
	while(1)
	{
		char recvbuf[1024]={0};
		int recvd=recv((SOCKET)pParam,recvbuf,sizeof(recvbuf)+1,0);
		string str(recvbuf);
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
/* 处理接收到的消息。     
[3G*3916377609*0173*UD2,240916,044545,V,31.200533,N,121.4615883,E,0.00,0.0,0.0,0,100,98,0,0,00000010,7,0,460,0,6243,53825,160,6243,53826,167,6243,55377,154,6243,53827,150,6243,54178,146,6243,53362,144,6243,53363,143,5,jamestplink,c0:61:18:b:95:ce,-45,STWIFI005,0:c:1b:2d:1e:5a,-49,HP-Print-99-LaserJet 1025,d8:5d:e2:2b:1b:99,-61,GLEXER_004C88,cc:a3:74:0:4c:88,-63,,ca:d7:19:f0:ad:9a,-82,41.5]

*/
/************************************************************************/
LRESULT  CGPSServerDlg::dealwith_parammsg(WPARAM wParam,LPARAM lparam)
{
	string receivemsg="";
	receivemsg = (char *)lparam;
	string ordertype=receivemsg.substr(1,2);//命令类型"ST"表示来时手机控制端的命令。"3G"表示来自终端定位设备的命令
	if(ordertype=="ST"){//[ST*358584059950478*Connect]
		string imei = receivemsg.substr(4,15);
		map<SOCKET,string>::iterator iter=STmap_socket.find((SOCKET)wParam);
		  vector <string> master_msg = split(receivemsg,"*,");//解析手机控制端发送的消息代码
		if(iter!=STmap_socket.end()){//在STmap_socket中存在该连接,根据命令格式做操作。
		  	if(master_msg[2]=="GetDivice]"){
				//查询数据库中设备列表
				dbHelper->GetDivice((SOCKET)wParam,master_msg[1].c_str());
			}else if(master_msg[2]=="ADDDivice"){
				dbHelper->ADDDivice((SOCKET)wParam,master_msg[1].c_str(),master_msg[3].c_str(),master_msg[4].c_str(),master_msg[5].c_str());
			}else if(master_msg[2]=="DeleteDivice"){
				dbHelper->DeleteDivice((SOCKET)wParam,master_msg[1].c_str(),master_msg[3].c_str());
			}else if(master_msg[3]=="GetlatLng]"){
				string getLatlng = "";
				//手机控制端请求某一设备的实时位置。需要找出该设备保存在map_socket中的连接。
				//再向该设备发送请求实时定位命令。
				for(map <SOCKET,string>::iterator it=map_socket.begin();it!=map_socket.end();it++)
				{
					if(it->second==master_msg[2]){

						multimap<SOCKET,SOCKET>::iterator Forwarding_iter=Forwarding_map.find((SOCKET)wParam);
						
						if(Forwarding_iter!=Forwarding_map.end()){

						}else{

							Forwarding_map.insert(pair<SOCKET,SOCKET>(wParam,it->first));
						}

						getLatlng = "[3G*"+master_msg[2]+"*0002*CR]";
						dbHelper->GetlatLng((SOCKET)it->first,getLatlng);
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
				 //查询数据库验证登录消息 [ST*123547896354781*Login,admin,123456]
				 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)"[ST*Login*OK]");
			 }
	}
	}else{
		string mimei = receivemsg.substr(4,10);
		string ordermsg = receivemsg.substr(20,2);
		map<SOCKET,string>::iterator iter=map_socket.find((SOCKET)wParam);
		if(iter!=map_socket.end()){//在map_socket中存在该连接,根据命令格式做操作。
			if(ordermsg=="UD"){//终端设备发送位置上报消息，保存位置信息到数据库，并更新当前位置信息m_JD，mm_WD;
			string n_str="";
			 n_str = "[ST*Divice*GetLatLng,";
			vector <string> location_msg = split(receivemsg,",");	
			m_JD = location_msg[6].c_str();
	    	m_WD = location_msg[4].c_str();
			m_date= location_msg[1].c_str();
			m_DiviceIMEI = mimei.c_str();
			dbHelper->InsertData(m_WD,m_JD,m_DiviceIMEI,m_date);
	
			//检查转发表中是否有该设备转发的连接存在如果存在就进行转发到手机控制端的操作
			for(multimap <SOCKET,SOCKET>::iterator it=Forwarding_map.begin();it!=Forwarding_map.end();it++)
			{
		
				if(it->second==((SOCKET)wParam))
				{
					
					n_str+=receivemsg;
					 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,it->first,(LPARAM)n_str.c_str());
					
				}
			}
		  }else if(ordermsg=="LK"){
			  //链路保持
			  ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)receivemsg.c_str());

			
			}
		}else{//在map_socket中不存在该连接
		
			map_socket[(SOCKET)wParam] = mimei;
			dbHelper->UpdateOnLine1(mimei.c_str());
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
	for(multimap <SOCKET,SOCKET>::iterator it=Forwarding_map.begin();it!=Forwarding_map.end();)
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

