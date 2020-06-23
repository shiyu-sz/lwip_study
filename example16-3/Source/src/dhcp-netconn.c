
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/dhcp.h"
#include "lwip/inet.h"

#include "dhcp-netconn.h"

#if LWIP_NETCONN
#define MAX_BUFFER_LEN 256
char sendbuf[MAX_BUFFER_LEN];

extern struct netif enc28j60_netif;

static void dhcp_netconn_thread(void *arg)
{
	struct netconn *conn;
	struct ip_addr serveraddr;
	u32_t err,wr_err;
	int strlen = 0;
   
	while(enc28j60_netif.dhcp->state != DHCP_BOUND)	//DHCP是否获得有效IP地址
				OSTimeDly(10);					//等待，直至IP地址有效

	Printf("ip_addr = %d \n", enc28j60_netif.ip_addr);
	
	IP4_ADDR(&serveraddr,192,168,2,10); 			//构造服务器IP地址
	
	while(1)
	{
	   conn=netconn_new(NETCONN_TCP);			//申请TCP连接结构
	   err=netconn_connect(conn,&serveraddr,8080);	//连接服务器，端口号8080
	 
	   if(err==ERR_OK) {							//连接成功
		   Printf("Connection OK \n"); 		//打印信息
		   do
		   {
		      strlen = sprintf(sendbuf,"A LwIP client Using DHCP Address: %s\r\n", \
			  	ipaddr_ntoa((ip_addr_t *)&(enc28j60_netif.ip_addr)));
			  
			  wr_err=netconn_write(conn,sendbuf, strlen, NETCONN_NOCOPY);
			  OSTimeDly(100);
		   }while(wr_err==ERR_OK);
	   }
	   Printf("Connection failed \n");
	   netconn_close(conn); 						//关闭连接
	   netconn_delete(conn);						//删除连接结构
	}

}

void dhcp_netconn_init()
{
  sys_thread_new("dhcp_netconn_thread", dhcp_netconn_thread, NULL, DEFAULT_THREAD_STACKSIZE, TCPIP_THREAD_PRIO+1);
}

#endif /* LWIP_NETCONN*/
