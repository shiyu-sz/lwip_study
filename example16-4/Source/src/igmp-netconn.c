
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"

#include "igmp-netconn.h"

#if LWIP_NETCONN

extern struct netif enc28j60_netif;

static void igmp_netconn_thread(void *arg)
{
	//struct netconn *conn;
	//struct ip_addr serveraddr;
	//u32_t err,wr_err;
	err_t err;
	//int strlen = 0;
	struct netconn *conn;
	struct ip_addr local_addr,group_addr,remote_addr;
   
	// while(enc28j60_netif.dhcp->state != DHCP_BOUND)	//DHCP是否获得有效IP地址
	// 			OSTimeDly(10);					//等待，直至IP地址有效

	
    local_addr = enc28j60_netif.ip_addr;        //本地IP地址
	
	IP4_ADDR(&group_addr,233,0,0,6);			//多播地址
	IP4_ADDR(&remote_addr,192,168,2,10);		//主机地址
	 
	conn=netconn_new(NETCONN_UDP);		//新建UDP类型的连接结构
	netconn_bind(conn,NULL,9090);			//绑定在本地端口9090上
	//加入多播组
	Printf("Join group\n");
	netconn_join_leave_group(conn,&group_addr,&local_addr,NETCONN_JOIN);
	Printf("Join group ok\n");
	while(1)
	{
	struct netbuf *inbuf = NULL;
	err = netconn_recv(conn, &inbuf); 		//连接上等待接收数据
	Printf("recv data group\n");
	if(err == ERR_OK) 				//数据有效
	{								//则将数据发往主机remote_addr的8080端口
		netconn_sendto(conn,inbuf,&remote_addr,8080);
		netbuf_delete(inbuf);			//删除数据
	 }
	}
	netconn_delete(conn);

}

void igmp_netconn_init()
{
  sys_thread_new("dhcp_netconn_thread", igmp_netconn_thread, NULL, DEFAULT_THREAD_STACKSIZE, TCPIP_THREAD_PRIO+1);
}

#endif /* LWIP_NETCONN*/
