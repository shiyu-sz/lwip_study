
#include "def.h"
#include "sys.h"
#include "err.h"
//ucosii的内存管理结构，我们将所有邮箱空间通过内存管理结构来管理
//static OS_MEM *MboxMem = NULL;
//static char MboxMemoryArea[TOTAL_MBOX_NUM * sizeof(struct LWIP_MBOX_STRUCT)];
const u32_t NullMessage;//解决空指针投递的问题
//定义系统使用的超时链表首指针结构
//struct sys_timeouts global_timeouts;
//与系统任务新建函数相关的变量定义

#define LWIP_MAX_TASKS 4 	//允许内核最多创建的任务个数
#define LWIP_STK_SIZE  512	//每个任务的堆栈空间
OS_STK  LWIP_STK_AREA[LWIP_MAX_TASKS][LWIP_STK_SIZE];

//函数功能：操作模拟层初始化，完成邮箱需要的内存空间的初始化
//以及系统超时链表首指针的初始化，在函数初始化时必须调用这个函数
void sys_init(void)
{
    u8_t   Err;
  //  MboxMem = OSMemCreate( (void*)MboxMemoryArea, 
  //                      TOTAL_MBOX_NUM, sizeof(sys_mbox_t), &Err );
    //global_timeouts.next =NULL;
}
//函数功能：创建一个信号量，信号量初始值为count
//直接调用系统中的函数完成信号量创建
//返回值：指向信号量的指针，若创建失败，返回为0
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
  LWIP_ASSERT("[Sys_arch]sem != NULL", sem != NULL);

  *sem = OSSemCreate((u16_t)count);
  LWIP_ASSERT("[Sys_arch]Error creating sem", *sem != NULL);
  if(*sem != NULL) {
    return ERR_OK;
  }
   
  return ERR_MEM;
}

//函数功能：删除一个信号量
void sys_sem_free(sys_sem_t *sem)
{
  u8_t Err;
  // parameter check 
  LWIP_ASSERT("sem != NULL", sem != NULL);

  OSSemDel(*sem, OS_DEL_ALWAYS, &Err);
	
  if(Err != OS_ERR_NONE)
  {
    //add error log here
    Printf("[Sys_arch]free sem fail\n");
  }

  *sem = NULL;
}




//函数功能：释放一个信号量
void sys_sem_signal(sys_sem_t *sem)
{
  u8_t Err;
  LWIP_ASSERT("sem != NULL", sem != NULL);

  Err = OSSemPost(*sem);
  if(Err != OS_ERR_NONE)
  {
        //add error log here
        Printf("[Sys_arch]:signal sem fail\n");
  }
  
  LWIP_ASSERT("Error releasing semaphore", Err == OS_ERR_NONE);
}

//函数功能：阻塞进程，等待一个信号量的到来。如果timeout不为0，则进程阻塞的时间最多为
//相关的毫秒数，否则进程一直阻塞，直到收到信号量
//返回值：如果timeout不为0，则返回值为等待该信号量的毫秒数，如果函数在规定的时间内
//没有等到信号量，则返回值为SYS_ARCH_TIMEOUT，如果信号量在调用函数时已经可用，则函数
//不会发生任何阻塞操作，返回值这时可以是0
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  u8_t Err;
  u32_t wait_ticks;
  u32_t start, end;
  LWIP_ASSERT("sem != NULL", sem != NULL);

  if (OSSemAccept(*sem))		  // 如果已经收到, 则返回0 
  {
	  //Printf("debug:sem accept ok\n");
	  return 0;
  }   
  
  wait_ticks = 0;
  if(timeout!=0){
	 wait_ticks = (timeout * OS_TICKS_PER_SEC)/1000;
	 if(wait_ticks < 1)
		wait_ticks = 1;
	 else if(wait_ticks > 65535)
			wait_ticks = 65535;
  }

  start = sys_now();
  OSSemPend(*sem, (u16_t)wait_ticks, &Err);
  end = sys_now();
  
  if (Err == OS_NO_ERR)
		return (u32_t)(end - start);		//将等待时间设置为timeout/2
  else
		return SYS_ARCH_TIMEOUT;
  
}


//函数功能：创建一个邮箱，邮箱能容纳的消息数为size，邮箱内消息的本质是一个指针，
//它指向了消息的具体位置，此时用户需要定义内核中具有如下形式的宏：_MBOX_SIZE
//这些宏的默认值为0，表示了各种邮箱的大小；当然用户可以忽略size，直接使用自己的
//默认大小，此时没有必要再定义_MBOX_SIZE形式的宏
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  LWIP_UNUSED_ARG(size);

  mbox->ucos_queue = OSQCreate( &(mbox->mbox_msg_entris[0]), MAX_MSG_IN_MBOX );
  LWIP_ASSERT("Error creating queue", mbox->ucos_queue != NULL);
  if(mbox->ucos_queue == NULL) {
    return ERR_MEM;
  }
  memset(mbox->mbox_msg_entris, 0, sizeof(void *)*MAX_MSG_IN_MBOX);
  
  return ERR_OK;
}

//函数功能：释放邮箱，如果邮箱释放时邮箱中还包含有消息，这说明是用户应用程序出错，
//用户应该自行解决这样的问题
void sys_mbox_free(sys_mbox_t *mbox)
{
  /* parameter check */
  u8_t Err;
  LWIP_ASSERT("mbox != NULL", mbox != NULL);


  OSQFlush(mbox->ucos_queue);
	
  OSQDel(mbox->ucos_queue, OS_DEL_ALWAYS, &Err);
	
  if(Err != OS_ERR_NONE)
  {
    //add error log here
    Printf("[Sys_arch]free Mbox queue fail\n");
  }

  mbox->ucos_queue= NULL;
}


//函数功能：向邮箱发送一条消息，如果发送队列满，这个函数阻塞，直至发送成功
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
	
    if (msg == NULL)
    	msg = (void*)&NullMessage;//解决空指针投递的问题
	while (OSQPost(mbox->ucos_queue, msg) == OS_Q_FULL)
		   OSTimeDly(2);
}

//函数功能：尝试向邮箱发送一条消息，若成功则返回ERR_OK，如果邮箱满了，则返回ERR_MEM
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
	
    if (msg == NULL)
    	msg = (void*)&NullMessage;//解决空指针投递的问题

    if (OSQPost(mbox->ucos_queue, msg) != OS_NO_ERR)
    {
        return ERR_MEM;
    }
    
    return ERR_OK;
}

//函数功能：从一个邮箱等待消息，如果timeout不为0，则标识等待的最大毫秒数，此时
//若等待成功，则返回值标识等待的毫秒数，若为0，表示函数将一直等待，直至收到消息
//data可能为NULL，这时，标识邮箱中接收到的这条消息必须被丢弃掉
u32_t
sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	u8_t Err;
	u32_t wait_ticks;
	u32_t start, end;
    void *Data;
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("*msg != NULL", msg != NULL);
	
    Data = OSQAccept(mbox->ucos_queue, &Err);
	if (Data != NULL)		
	{
        if (Data == (void*)&NullMessage)
        {
            *msg = NULL;
        }
        else
        {
            *msg = Data;
        }
		return 0;
	}
	
	wait_ticks = 0;
	if(timeout!=0){
	   wait_ticks = (timeout * OS_TICKS_PER_SEC)/1000;
	   if(wait_ticks < 1)
  	      wait_ticks = 1;
       else if(wait_ticks > 65535)
  	          wait_ticks = 65535;
	}

    Data = OSQPend(mbox->ucos_queue, (u16_t)wait_ticks, &Err);

	if (Data != NULL)		
	{
        if (Data == (void*)&NullMessage)
        {
            *msg = NULL;
        }
        else
        {
            *msg = Data;
        }
	}
    
	if (Err == OS_NO_ERR)
		return timeout/2;       //将等待时间设置为timeout/2
	else
	    return SYS_ARCH_TIMEOUT;
}

//函数功能：尝试从一个邮箱读取消息，该函数不会阻塞进程，当邮箱中有数据时，
//读取成功，返回0，否则立即返回SYS_MBOX_EMPTY
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	INT8U err;
	void *Data;
	LWIP_ASSERT("mbox != NULL", mbox != NULL);
	LWIP_ASSERT("*msg != NULL", msg != NULL);
	
	Data = OSQAccept(mbox->ucos_queue, &err);
	
    if (Data != NULL)		
	{
        if (Data == (void*)&NullMessage)
        {
            *msg = NULL;
        }
        else
        {
            *msg = Data;
        }
		return 0;
	}
	else
	{
        return SYS_MBOX_EMPTY;
    }
}
//返回进程的超时时间结构链表，这里我们将系统所有的进程使用同一条超时链表

//函数功能：新建一个进程，在整个系统中只会被调用一次
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
  static u32_t TaskCreateFlag=0;
  u8_t i=0;
  name=name;
  stacksize=stacksize;
  
  while((TaskCreateFlag>>i)&0x01){
    if(i<LWIP_MAX_TASKS&&i<32)
          i++;
    else return 0;
  }
  if(OSTaskCreate(thread, (void *)arg, &LWIP_STK_AREA[i][LWIP_STK_SIZE-1],prio)==OS_NO_ERR){
       TaskCreateFlag |=(0x01<<i); 
	   
  };

  return prio;
}


