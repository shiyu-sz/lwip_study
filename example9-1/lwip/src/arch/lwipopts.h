/*
;*****************************************************************************************************

;*	      
;*****************************************************************************************************
;*/

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__


/* ��app_cfg.h�ļ�ͳһ�����̵߳����ȼ� */
//#include "app_cfg.h"
/* �ٽ����ı��� */
	 
/* ��ϵͳ */  
#define NO_SYS                       1
//#define NO_SYS_NO_TIMERS             1

#define LWIP_SOCKET  0
#define LWIP_NETCONN 0
/* 4�ֽڶ��� */
#define MEM_ALIGNMENT                4
	
/* 4K���ڴ� */  
#define MEM_SIZE                     1024*10
#define TCP_SND_BUF                  4000    //����TCPЭ��ʹ�õ�����ͻ��峤��
#define TCP_MSS                      1000

/* */
#define ETH_PAD_SIZE				 0 
//#define ETHARP_DEBUG                LWIP_DBG_ON
//#define ICMP_DEBUG                  LWIP_DBG_ON

//#define LWIP_HAVE_LOOPIF             1

//#define MEM_DEBUG                       LWIP_DBG_ON
//#define MEMP_DEBUG                      LWIP_DBG_ON
//#define RAW_DEBUG                       LWIP_DBG_ON
//#define IP_DEBUG                        LWIP_DBG_ON
//#define TCP_DEBUG					  LWIP_DBG_ON
//#define UDP_DEBUG					  LWIP_DBG_ON
//#define MEMP_MEM_MALLOC              1
//#define MEM_USE_POOLS 0
#define MEMP_NUM_SYS_TIMEOUT      10

#endif /* __LWIPOPTS_H__ */

/*
;*****************************************************************************************************
;*                            			End Of File
;*****************************************************************************************************
;*/

