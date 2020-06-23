// UdpClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock.h>
#pragma comment(lib,"Ws2_32.lib")//链接Ws2_32.lib库，包含了socket函数
#include<stdio.h>

int main(int argc, char* argv[])
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	struct sockaddr_in remote;

	wVersionRequested = MAKEWORD( 2, 2 );
	
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		printf("创建套接字初始化失败！\n");
		return -1;
	}
	if ( LOBYTE( wsaData.wVersion ) != 2 ||HIBYTE( wsaData.wVersion ) != 2 ) {
		WSACleanup( );
		return -1; 
	}
	printf("  **********************************************\n");
	printf("  ****   A UDP Client for LwIP Server Test  ****\n");
	printf("  ****           Created by Forrest         ****\n");
	printf("  **********************************************\n");
	
	SOCKET SockSrv=socket(AF_INET,SOCK_DGRAM,0);
	if(INVALID_SOCKET ==SockSrv){
		WSACleanup();
		return -1;
	}
	
	remote.sin_family=AF_INET;
	remote.sin_port=htons(8080);
	remote.sin_addr.s_addr=inet_addr("192.168.1.37");
	connect(SockSrv,(struct sockaddr*)&remote,sizeof(remote));
	while(1)
	{
		char  RecvBuff[50];                  // 用于接收bmp文件头
		char DataToSend[]="GET";
		int   ret;   
		Sleep(1000);
		if((ret=send(SockSrv,DataToSend,sizeof(DataToSend),0))==SOCKET_ERROR)
		       break;
		printf("Request--> GET\n");
        ret = recv(SockSrv,RecvBuff,50,0);
		RecvBuff[ret]='\0';
		if(ret>0)  printf("Reply  --> %s",RecvBuff);
	}
	closesocket(SockSrv);
	WSACleanup();
	return 0;
}

