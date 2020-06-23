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
#define NO_SYS                       0
//#define NO_SYS_NO_TIMERS             1

#define LWIP_SOCKET  1//(NO_SYS == 0)
#define LWIP_NETCONN 1//(NO_SYS == 0)
/* 4�ֽڶ��� */
#define MEM_ALIGNMENT                4
	
/* 4K���ڴ� */  
#define MEM_SIZE                     1024*10
#define TCP_SND_BUF                  1600 //4000   //����TCPЭ��ʹ�õ�����ͻ��峤��
#define TCP_MSS                      400//1000

/* */
#define ETH_PAD_SIZE				 0 
//#define ETHARP_DEBUG                LWIP_DBG_ON
//#define ICMP_DEBUG                  LWIP_DBG_ON
//#define MEM_DEBUG                       LWIP_DBG_ON
//#define MEMP_DEBUG                      LWIP_DBG_ON

#define MEMP_NUM_TCPIP_MSG_INPKT        100
#define TCP_OVERSIZE                    0
#define LWIP_DNS                        1

#define TCPIP_THREAD_PRIO               3 
//#define MEMP_MEM_MALLOC              1
//#define MEM_USE_POOLS 0
#endif /* __LWIPOPTS_H__ */

/*
;*****************************************************************************************************
;*                            			End Of File
;*****************************************************************************************************
;*/

