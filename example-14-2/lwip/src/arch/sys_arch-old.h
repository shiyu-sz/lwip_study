#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__

#include    "includes.h"

//#define TOTAL_MBOX_NUM  20	//定义内核能够使用的最多邮箱数目
#define MAX_MSG_IN_MBOX 100	//每个邮箱最多能够存放的消息数目

//定义内核使用的邮箱的结构
struct LWIP_MBOX_STRUCT{
	OS_EVENT * ucos_queue;						//借助ucos中的队列机制来实现
	void     *mbox_msg_entris[MAX_MSG_IN_MBOX];//邮箱中存放消息的指针
};


//定义LwIP内部使用的数据类型
typedef struct LWIP_MBOX_STRUCT sys_mbox_t;  //系统邮箱类型指针
typedef OS_EVENT * sys_sem_t;                  //系统信号量类型指针
typedef INT8U     sys_thread_t;				  //系统任务标识

/* 信号NULL, 邮箱NULL 定义 */ 
#define SYS_MBOX_NULL  (sys_mbox_t)NULL
#define SYS_SEM_NULL   (sys_sem_t)NULL

#define sys_sem_valid(sema) ((*sema) != NULL)
#define sys_sem_set_invalid(sema) (*sema = NULL)

/* let sys.h use binary semaphores for mutexes */
#define LWIP_COMPAT_MUTEX 1

#define sys_mbox_valid(mbox) ((mbox != NULL) && ((mbox)->ucos_queue != NULL))
#define sys_mbox_set_invalid(mbox) ((mbox)->ucos_queue = NULL)


#endif
