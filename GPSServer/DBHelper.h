#include "stdafx.h"
#include  "winsock.h"
#include "GPSServer.h"
#include "mysql.h" 
#include <string>
using namespace std;

class DBHelper
{
	MYSQL mysql;
	MYSQL_RES *resultset;  
public:
	/************************************************************************/
	/* 连接数据库操作                                                                     */
	/************************************************************************/
     bool ConnectDatabase(){
		 try 
		 {  
			 mysql_init(&mysql);  //连接mysql，数据库

			 //返回false则连接失败，返回true则连接成功
			 if (!(mysql_real_connect(&mysql,"localhost", "root", "", "gps",0,NULL,0))) //中间分别是主机，用户名，密码，数据库名，端口号（可以写默认0或者3306等），可以先写成参数再传进去
			 {
				 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"数据库连接失败！");
				 return FALSE;  

			 }
			 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"数据库连接成功！");
			 return TRUE;
		 }  
		 catch (...)  
		 {  
			 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_STATUSINFO,0,(LPARAM)"数据库连接失败！");
			 return FALSE;  
		 } 

	 }
	 /************************************************************************/
	 /*             数据库插入操作*/
	 /************************************************************************/
	 void InsertData(CString Latitude,CString Longitude,CString diviceIMEI,CString Data )
	 {
		 CString sql;

		 sql.Format(("insert into locationdata(Latitude,Longitude,DiviceIMEI,Date)values(%s,%s,%s,%s)"), Latitude,Longitude,diviceIMEI,Data);

		 if(mysql_query(&mysql, sql))        //执行SQL语句
		 {
			 printf("插入数据失败");
		 }
		 else
		 {
			 printf("插入数据成功");
		 }
	 }
	 /************************************************************************/
	 /* 用户登录验证操作                                                                     */
	 /************************************************************************/
	 void LoginCheck(SOCKET  wParam,CString UserName,CString Password)
	 {
		 CString sql;

		 sql.Format(("select * from user where UserName='%s' and UserPassword = %s"), UserName,Password);

		 //select * from user where UserName='admin' and UserPassword = 123456]"	ATL::CStringT<char,StrTraitMFC_DLL<char,ATL::ChTraitsCRT<char> > >

		 if(mysql_query(&mysql,sql))        //执行SQL语句
		 {
			 printf("数据库查询失败");
		 }
		 else
		 {  
			 resultset = mysql_store_result(&mysql);// 获得结果集           
			 if (mysql_num_rows(resultset) != NULL)  
			 {//查询到该用户 ,向请求登录的手机控制端返回登录成功消息
				 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)"[ST*Login*OK]");
			 }  
			 else  
			 {  
				 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)"[ST*Login*Failure]");
			 }   
			 mysql_free_result(resultset);  // 释放结果集  
		 }  
	 }
	 /************************************************************************/
	 /* 查询在线设备 。查询群组表中IMEI项为该IMEI号的在信标设备                                                               */
	 /************************************************************************/
	 void GetDivice(SOCKET  wParam,CString IMEI)
	 {

		 CString sql;

		 CString diviceinfo;

		 MYSQL_ROW row;    

		 string diviceName,diviceNum,online,diviceIMEI;

		 string n_str = "[ST*SetDivice*OK,";

		 sql.Format(("select * from divicegroup where IMEI = %s"), IMEI);

	      if(mysql_query(&mysql,sql))  
			 {  
		
			 }  
			 else  
			 {                      
				 //检索数据           
				 resultset = mysql_store_result(&mysql);// 获得结果集           
				 if (mysql_num_rows(resultset) != NULL)  
				 {  
					 int numRows = mysql_num_rows(resultset); // 获得结果集中的记录数  
					 int numFields = mysql_num_fields(resultset);// 获得表中字段数  
				  
					 while (row = mysql_fetch_row(resultset))  
					 {  
							 diviceName = row[2];
							 diviceNum = row[3];
							 diviceIMEI = row[4];
							 online   = row[5];
							 n_str += diviceName+"#"+diviceNum+"#"+diviceIMEI+"#"+online+",";
				
					 }  
					 n_str+="]";
			
					 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)n_str.c_str());
				
				 }  
				 else  
				 {  
					 n_str+="]";
					 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)n_str.c_str());

				 }   
				 mysql_free_result(resultset);  // 释放结果集  
		 }

	 }

	 /************************************************************************/
	 /*      获取轨迹数据                                                                */
	 /************************************************************************/
	 void GetLocus(SOCKET  wParam,CString diviceIMEI)
	 {
		 CString sql;

		  MYSQL_ROW row;   

		   string Locus_str = "[ST*GetLocus*OK";

		  string Date;

		   sql.Format(("select * from locationdata where DiviceIMEI=%s"), diviceIMEI);

		   if(mysql_query(&mysql,sql))  
		   {  
			   //查询失败
		   }  
		   else  
		   {                
		      //检索数据
			   resultset = mysql_store_result(&mysql);// 获得结果集           
			   if (mysql_num_rows(resultset) != NULL)  
			   {  
				   while (row = mysql_fetch_row(resultset))  
				   {  
				       Date = row[4];

					   string::size_type idx = Locus_str.find( Date);

					   if(idx==string::npos){

						    Locus_str+=","+Date;
					   }
				   }
				   Locus_str+="]";
				    ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)Locus_str.c_str());
			   }else{
				   Locus_str+="]";
				   ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)Locus_str.c_str());
			   }
				 
			    mysql_free_result(resultset);  // 释放结果集  

		   }
	 }
	 /************************************************************************/
	 /*                                                                      */
	 /************************************************************************/
	 void GetLocusDetails(SOCKET  wParam,CString diviceIMEI,CString date)
	 {
		 CString sql;

		 MYSQL_ROW row;   

		 string LocusDetails_str = "[ST*LocusDetails*OK";

		 string Longitude,Latitude;

		 sql.Format(("select * from locationdata where DiviceIMEI=%s and Date = %s"), diviceIMEI,date);

		 if(mysql_query(&mysql,sql))  
		 {  
			 //查询失败
		 }  
		 else  
		 {
			 //开始检索数据
			 //检索数据
			 resultset = mysql_store_result(&mysql);// 获得结果集           
			 if (mysql_num_rows(resultset) != NULL)  
			 {  
				 while (row = mysql_fetch_row(resultset))  
				 { 
			    	 Longitude=row[1];
			    	 Latitude = row[2];
					 LocusDetails_str+=","+Longitude+"&"+Latitude;
				 }
				 LocusDetails_str+="]";
	
				 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)LocusDetails_str.c_str());
			 }
		 }
		 mysql_free_result(resultset);  // 释放结果集  
	 }


	 /************************************************************************/
	 /* 手机客户端添加可控的设备。                                                                     */
	 /************************************************************************/
	 void ADDDivice(SOCKET  wParam,CString IMEI,CString DiviceName,CString DiviceNum,CString DiviceIMEI)
	 {
		 CString sql;

		 sql.Format(("insert into divicegroup(IMEI,DiviceName,DiviceNum,DiviceIMEI)values(%s,'%s',%s,%s)"), IMEI,DiviceName,DiviceNum,DiviceIMEI);

		 if(mysql_query(&mysql, sql))        //执行SQL语句
		 {
			 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)"ST*ADDDivice*Failure");
			 printf("插入数据失败");
		 }
		 else
		 {
			  ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)"ST*ADDDivice*OK");
			 printf("插入数据成功");
		 }
	 }
	 /************************************************************************/
	 /* 手机客户端删除可控的设备                                                                     */
	 /************************************************************************/
	 void DeleteDivice(SOCKET  wParam,CString IMEI,CString DiviceIMEI)
	 {
		 CString sql;

		 sql.Format(("delete  from divicegroup where IMEI=%s and DiviceIMEI=%s"), IMEI,DiviceIMEI);
		 if(mysql_query(&mysql, sql))        //执行SQL语句
		 {
			 ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)"ST*DeleteDivice*Failure");
			  printf("删除数据失败");
		 }else {
			  ::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)"ST*DeleteDivice*OK");
			  printf("删除数据成功");
		 }

	 }
	 /************************************************************************/
	 /*       手机控制端请求指定设备的实时定位数据
	          向指定的设备发送请求实时定位的命令
	 */
	 /************************************************************************/
	 void GetlatLng(SOCKET  wParam,string IMEI)
	 {

	::SendMessage(AfxGetMainWnd()->m_hWnd,WM_SENDINFO,(SOCKET)wParam,(LPARAM)IMEI.c_str());
	 }
	  /************************************************************************/
	 /*     更新数据库信标设备离线状态
	 */
	 /************************************************************************/
	 void UpdateOnLine(CString IMEI){
		 CString sql;

		 sql.Format(("UPDATE divicegroup set DiviceOnLine=0 where DiviceIMEI = %s"), IMEI);

		 if(mysql_query(&mysql, sql))        //执行SQL语句
		 {
		 
		 }else{

		 }
	
	 }
	   /************************************************************************/
	 /*     更新数据库信标设备在线状态
	 */
	 /************************************************************************/
	 void UpdateOnLine1(CString IMEI){
		 CString sql;

		 sql.Format(("UPDATE divicegroup set DiviceOnLine=1 where DiviceIMEI = %s"), IMEI);

		 if(mysql_query(&mysql, sql))        //执行SQL语句
		 {
			

		 }else{


		 }

	 }

};