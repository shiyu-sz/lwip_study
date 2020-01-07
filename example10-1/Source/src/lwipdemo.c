#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/init.h"
#include "netif/etharp.h"
#include "lwip/timers.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include <stdio.h>	

//extern functions
extern err_t ethernetif_init(struct netif *netif);
extern void process_mac(void);

//global data
struct netif enc28j60_netif;

void lwip_init_task(void)
{
	struct ip_addr ipaddr, netmask, gw;

    lwip_init();
	IP4_ADDR(&gw, 192,168,2,1);
	IP4_ADDR(&ipaddr, 192,168,2,37);
	IP4_ADDR(&netmask, 255,255,255,0);

    netif_add(&enc28j60_netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init,ethernet_input);
	netif_set_default(&enc28j60_netif);
	netif_set_up(&enc28j60_netif);
	
}

#define UDP_ECHO_PORT 1008
void udp_demo_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
   udp_sendto(upcb, p, addr, port);
   pbuf_free(p);
}

void udp_demo_callback1(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
   struct pbuf *q = NULL;
   const char* reply = "This is reply!\n";

	printf("data  %s\n", (char *)p->payload);
	
   if(arg)
   {
       printf("s1  %s\n",(char *)arg);
   }

   pbuf_free(p);
   
   q = pbuf_alloc(PBUF_TRANSPORT, strlen(reply)+1, PBUF_RAM);
   if(!q)
   {
   	   printf("out of PBUF_RAM\n");
	   return;
   }

   memset(q->payload, 0 , q->len);
   memcpy(q->payload, reply, strlen(reply));
   udp_sendto(upcb, q, addr, port);
   pbuf_free(q);
}

static char * st_buffer= "We get a data\n";
void udp_demo_init(void)
{
   struct udp_pcb *upcb;
 //  struct ip_addr ipaddr;
 //  IP4_ADDR(&ipaddr, 192,168,1,78);                              
   /* Create a new UDP control block  */
   upcb = udp_new();
   /* Bind the upcb to any IP address and the UDP_PORT port*/
   udp_bind(upcb, IP_ADDR_ANY, UDP_ECHO_PORT);
  // udp_connect(upcb,&ipaddr, 8080);
   /* Set a receive callback for the upcb */
   //udp_recv(upcb, udp_demo_callback, NULL);
   /* Free the p buffer */
   udp_recv(upcb, udp_demo_callback1, (void *)st_buffer);
}

void lwip_demo(void *pdata)
{
	//init LwIP
	lwip_init_task();

	//setup echo server
 	udp_demo_init();

	//for periodic handle
	while(1)
	{
		process_mac();
		
		//process LwIP timeout
		sys_check_timeouts();
		
		//todo: add your own user code here

	}
}


