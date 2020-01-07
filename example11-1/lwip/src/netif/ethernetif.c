/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"

#if 1 /* don't build, this is only a skeleton, see previous comment */

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

#include "enc28j60.h"
#include "string.h"


/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};

//extern data and functions
extern struct netif enc28j60_netif;

//global data
static unsigned char MyMacID[6] = {0x04,0x02,0x35,0x00,0x00,0x01};

/* Forward declarations. */
static int  ethernetif_input(struct netif *netif);
/*
 * Read the specified number of bytes from the device DMA port into
 * the supplied buffer.
 */

/****************************************************************************
* 名    称：void PacketSend (struct pbuf *p)
* 功    能：发送一包数据	完成pbuf中数据的发送																	 
* 入口参数：
* 出口参数: 
* 说    明：基于uip的驱动程序完成LwIP的数据包发送和接收
* 调用方法：将pbuf中的数据拷贝到全局数组MyDatabuf中，然后调用上面的函数enc28j60PacketSend发送数据
****************************************************************************/ 
//以太网数据帧的最大长度1500，定义这个数组会增大内存开销，但是驱动程序变得简单
static unsigned char  MySendbuf[1500]; 
static err_t PacketSend (struct pbuf *p)
{
	struct pbuf *q = NULL;
	unsigned int templen = 0;

	for(q = p;q!=NULL;q = q->next)
	{
		memcpy(&MySendbuf[templen],q->payload,q->len);	 //将pbuf中的数据拷贝到全局数组MyDatabuf中
		templen += 	q->len ;

		if(templen > 1500 || templen > p->tot_len)	 	//有效性校验，防止数据溢出
		{
			LWIP_PLATFORM_DIAG(("PacketSend: error,tmplen=%"U32_F",tot_len=%"U32_F"\n\t", templen, p->tot_len));
			return ERR_BUF;
		}
	}
	
	//拷贝完毕，下面进行数据的发送工作
	if(templen == p->tot_len)
	{
		enc28j60PacketSend(templen, MySendbuf);		   //调用网卡发送函数
		return ERR_OK; 
	}
	
	LWIP_PLATFORM_DIAG(("PacketSend: length mismatch ,tmplen=%"U32_F",tot_len=%"U32_F"\n\t", templen, p->tot_len));
	return ERR_BUF;
}
/****************************************************************************
* 名    称：struct pbuf *PacketReceive(struct netif *netif)
* 功    能：完成LwIP需要的数据包接收																	 
* 入口参数：
* 出口参数: 
* 说    明：基于uip的驱动程序完成LwIP的数据包发送和接收
* 调用方法：网卡的数据拷贝到全局数组MyRecvbuf中，再组装成pbuf
****************************************************************************/ 
//以太网数据帧的最大长度1500，定义这个数组会增大内存开销，但是驱动程序变得简单
static unsigned char  MyRecvbuf[1500]; 
struct pbuf *PacketReceive(struct netif *netif)
{
	struct pbuf *p = NULL;	
	unsigned int recvlen = 0;
	unsigned int i = 0;
	struct pbuf *q = NULL;
    
	recvlen = enc28j60PacketReceive(1500, MyRecvbuf);

	if(!recvlen)	       //接收数据长度为0，直接返回空
	{
	    return NULL;
	}
	
	//申请内核pbuf空间，为RAM类型
	p = pbuf_alloc(PBUF_RAW, recvlen, PBUF_RAM);
	
	if(!p)			       //申请失败，则返回空
	{
	    LWIP_PLATFORM_DIAG(("PacketReceive: pbuf_alloc fail ,len=%"U32_F"\n\t", recvlen));
		return NULL;
	 }
    //申请成功，拷贝数据到pbuf中
	q = p;
		
	while(q != NULL)
	{   
		memcpy(q->payload,&MyRecvbuf[i],q->len);
		i += q->len;
		q = q->next;
		if(i >= recvlen)  break;
	}
		
	return p;
}

static void
low_level_init(struct netif *netif)
{
  struct ethernetif *ethernetif = netif->state;
  
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] = MyMacID[0];
  netif->hwaddr[1] = MyMacID[1];
  netif->hwaddr[2] = MyMacID[2];
  netif->hwaddr[3] = MyMacID[3];
  netif->hwaddr[4] = MyMacID[4];                
  netif->hwaddr[5] = MyMacID[5];

  /* maximum transfer unit */
  netif->mtu = 1500;
  
  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

  /* Do whatever else is needed to initialize interface. */
  //enc28j60Init(MyMacID); 
  mymacinit(MyMacID);
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
	//struct ethernetif *ethernetif = netif->state;
  /* Interrupts are disabled through this whole thing to support multi-threading
	   transmit calls. Also this function might be called from an ISR. */
	return PacketSend(p);	                // 调用网卡发送函数发送一帧数据
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
 
  //return NULL;
  return PacketReceive(netif);  
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */

s32_t ethernetif_input(struct netif *netif)
{
  struct ethernetif *ethernetif;
  struct eth_hdr *ethhdr;
  struct pbuf *p;

  ethernetif = netif->state;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);
  /* no packet could be read, silently ignore this */
  if (p == NULL) 
  {
  	return 0;
  }
  /* points to packet payload, which starts with an Ethernet header */
  ethhdr = p->payload;

  switch (htons(ethhdr->type)) {
  /* IP or ARP packet? */
  case ETHTYPE_IP:
  case ETHTYPE_ARP:
#if PPPOE_SUPPORT
  /* PPPoE packet? */
  case ETHTYPE_PPPOEDISC:
  case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
    /* full packet send to tcpip_thread to process */
    if (netif->input(p, netif)!=ERR_OK)
     { LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
       pbuf_free(p);
       p = NULL;
     }
    break;

  default:
    pbuf_free(p);
    p = NULL;
    break;
  }

  return 1;
}

void process_mac(void)
{
   s32_t ret = 0;
   do
   {
       ret = ethernetif_input(&enc28j60_netif);

   }while(ret);
}
/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));
    
  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;
  
  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
  
  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}
#endif /* 0 */
