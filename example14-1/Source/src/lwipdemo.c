#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/init.h"
#include "netif/etharp.h"
#include "lwip/timers.h"
#include "lwip/tcpip.h"

#include "includes.h"
//extern functions
extern err_t ethernetif_init(struct netif *netif);
extern void process_mac(void);

//global data
struct netif enc28j60_netif;

extern void init_output(void);

void lwip_init_task(void)
{
	struct ip_addr ipaddr, netmask, gw;
    
    //lwip_init();
	tcpip_init(NULL, NULL);
	
	IP4_ADDR(&gw, 192,168,2,1);
	IP4_ADDR(&ipaddr, 192,168,2,37);
	IP4_ADDR(&netmask, 255,255,255,0);

    netif_add(&enc28j60_netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init,tcpip_input);

	netif_set_default(&enc28j60_netif);
	netif_set_up(&enc28j60_netif);
	
}

void tcpserver_init(void);
extern void output_thread(void *arg);

#include "socket_examples.h"

void lwip_demo(void *pdata)
{
    u32_t count = 0;
	//init LwIP
	lwip_init_task();
 	
	//init the tcp server
	//tcpecho_init();
	socket_examples_init();
	//for periodic handle
	while(1)
	{
		/*if use process_mac() here to query netif packages, Please disable eth INT in bsp.c*/
        process_mac();

		count++;
		if(count%10 == 0)
		{
		    OSTimeDly(1);    /*give other task a chance to run*/
		}
		//process LwIP timeout
		//sys_check_timeouts();
		//OSTimeDlyHMSM(0, 0, 0, 40);
		//todo: add your own user code here
		//printf("we are in loop");
	}
}
