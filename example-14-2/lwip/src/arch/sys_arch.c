/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: sys_arch.c,v 1.1.1.1 2003/05/17 05:06:56 chenyu Exp $
 */
#if !NO_SYS

#include "def.h"
#include "sys.h"
#include "err.h"
//ucosii的内存管理结构，我们将所有邮箱空间通过内存管理结构来管理

/*
static OS_MEM *MboxMem;
static char MboxMemoryArea[TOTAL_MBOX_NUM * sizeof(struct LWIP_MBOX_STRUCT)];
const u32_t NullMessage;//解决空指针投递的问题
*/

//定义系统使用的超时链表首指针结构
//struct sys_timeouts global_timeouts;
//与系统任务新建函数相关的变量定义

#define LWIP_MAX_TASKS 4 	//允许内核最多创建的任务个数
#define LWIP_STK_SIZE  512	//每个任务的堆栈空间
OS_STK  LWIP_STK_AREA[LWIP_MAX_TASKS][LWIP_STK_SIZE];


void sys_init()
{
  //currently do nothing
  Printf("[Sys_arch] init ok");
}

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
  OS_EVENT *new_sem = NULL;

  LWIP_ASSERT("[Sys_arch]sem != NULL", sem != NULL);

  new_sem = OSSemCreate((u16_t)count);
  LWIP_ASSERT("[Sys_arch]Error creating sem", new_sem != NULL);
  if(new_sem != NULL) {
    sem->sem = (void *)new_sem;
    return ERR_OK;
  }
   
  sem->sem = SYS_SEM_NULL;
  return ERR_MEM;
}

void sys_sem_free(sys_sem_t *sem)
{
  u8_t Err;
  // parameter check 
  LWIP_ASSERT("sem != NULL", sem != NULL);
  LWIP_ASSERT("sem->sem != SYS_SEM_NULL", sem->sem != SYS_SEM_NULL);

  OSSemDel(sem->sem, OS_DEL_ALWAYS, &Err);
	
  if(Err != OS_ERR_NONE)
  {
    //add error log here
    Printf("[Sys_arch]free sem fail\n");
  }

  sem->sem = NULL;
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  u8_t Err;
  u32_t wait_ticks;
  u32_t start, end;
  LWIP_ASSERT("sem != NULL", sem != NULL);

  if (OSSemAccept(sem->sem))		  // 如果已经收到, 则返回0 
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
  OSSemPend(sem->sem, (u16_t)wait_ticks, &Err);
  end = sys_now();
  
  if (Err == OS_NO_ERR)
		return (u32_t)(end - start);		//将等待时间设置为timeout/2
  else
		return SYS_ARCH_TIMEOUT;
  
}

void sys_sem_signal(sys_sem_t *sem)
{
  u8_t Err;
  LWIP_ASSERT("sem != NULL", sem != NULL);
  LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

  Err = OSSemPost(sem->sem);
  if(Err != OS_ERR_NONE)
  {
        //add error log here
        Printf("[Sys_arch]:signal sem fail\n");
  }
  
  LWIP_ASSERT("Error releasing semaphore", Err == OS_ERR_NONE);
}

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  LWIP_UNUSED_ARG(size);

  mbox->sem = OSSemCreate(0);
  LWIP_ASSERT("Error creating semaphore", mbox->sem != NULL);
  if(mbox->sem == NULL) {
    return ERR_MEM;
  }
  memset(&mbox->q_mem, 0, sizeof(void *)*MAX_QUEUE_ENTRIES);
  mbox->head = 0;
  mbox->tail = 0;
  mbox->msg_num = 0;
  
  return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
  /* parameter check */
  u8_t Err;
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  LWIP_ASSERT("mbox->sem != NULL", mbox->sem != NULL);

  OSSemDel(mbox->sem, OS_DEL_ALWAYS, &Err);
	
  if(Err != OS_ERR_NONE)
  {
    //add error log here
    Printf("[Sys_arch]free Mbox sem fail\n");
  }

  mbox->sem = NULL;
}

void sys_mbox_post(sys_mbox_t *q, void *msg)
{
  u8_t Err;
  SYS_ARCH_DECL_PROTECT(lev);

  /* parameter check */
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);

  //queue is full, we wait for some time
  while(q->msg_num >= MAX_QUEUE_ENTRIES)
  {
    OSTimeDly(1);
  }
  
  SYS_ARCH_PROTECT(lev);
  
  if(q->msg_num >= MAX_QUEUE_ENTRIES)
  {
    LWIP_ASSERT("mbox post error, we can not handle it now, Just drop msg!", 0);
	SYS_ARCH_UNPROTECT(lev);
	return;
  }
  q->q_mem[q->head] = msg;
  (q->head)++;
  if (q->head >= MAX_QUEUE_ENTRIES) {
    q->head = 0;
  }

  q->msg_num++;
  if(q->msg_num == MAX_QUEUE_ENTRIES)
  {
    Printf("mbox post, box full\n");
  }
  
  Err = OSSemPost(q->sem);
  
  if(Err != OS_ERR_NONE)
  {
    //add error log here
    Printf("[Sys_arch]:mbox post sem fail\n");
  }

  SYS_ARCH_UNPROTECT(lev);
}

err_t sys_mbox_trypost(sys_mbox_t *q, void *msg)
{
  u8_t Err;
  SYS_ARCH_DECL_PROTECT(lev);

  /* parameter check */
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);

  SYS_ARCH_PROTECT(lev);

  if (q->msg_num >= MAX_QUEUE_ENTRIES) {
    SYS_ARCH_UNPROTECT(lev);
	Printf("[Sys_arch]:mbox try post mbox full\n");
    return ERR_MEM;
  }

  q->q_mem[q->head] = msg;
  (q->head)++;
  if (q->head >= MAX_QUEUE_ENTRIES) {
    q->head = 0;
  }

  q->msg_num++;
  if(q->msg_num == MAX_QUEUE_ENTRIES)
  {
    Printf("mbox try post, box full\n");
  }

  Err = OSSemPost(q->sem);
  if(Err != OS_ERR_NONE)
  {
    //add error log here
    Printf("[Sys_arch]:mbox try post sem fail\n");
  }
  SYS_ARCH_UNPROTECT(lev);
  return ERR_OK;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, u32_t timeout)
{
  u8_t Err;
  u32_t wait_ticks;
  u32_t start, end;
  u32_t tmp_num;
  SYS_ARCH_DECL_PROTECT(lev);

  // parameter check 
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);

  wait_ticks = 0;
  if(timeout!=0){
	 wait_ticks = (timeout * OS_TICKS_PER_SEC)/1000;
	 if(wait_ticks < 1)
		wait_ticks = 1;
	 else if(wait_ticks > 65535)
			wait_ticks = 65535;
  }
  
  start = sys_now();
  OSSemPend(q->sem, (u16_t)wait_ticks, &Err);
  end = sys_now();

  if (Err == OS_NO_ERR)
  {
    SYS_ARCH_PROTECT(lev);
	
	if(q->head == q->tail)
	{
        Printf("mbox fetch queue abnormal [%u]\n", q->msg_num);
		if(msg != NULL) {
			*msg  = NULL;
	    }
		SYS_ARCH_UNPROTECT(lev);
		return SYS_ARCH_TIMEOUT;
	}
	
    if(msg != NULL) {
      *msg  = q->q_mem[q->tail];
    }

    (q->tail)++;
    if (q->tail >= MAX_QUEUE_ENTRIES) {
      q->tail = 0;
    }

	if(q->msg_num > 0)
	{
      q->msg_num--;
	}
	else
	{
      Printf("mbox fetch queue error [%u]\n", q->msg_num);
	}

	tmp_num = (q->head >= q->tail)?(q->head - q->tail):(MAX_QUEUE_ENTRIES + q->head - q->tail);

	if(tmp_num != q->msg_num)
	{
        Printf("mbox fetch error, umatch [%u] with tmp [%u]\n", q->msg_num, tmp_num);
	}
	
	SYS_ARCH_UNPROTECT(lev);
	//Printf("mbox fetch ok, match [%u] with tmp [%u] \n", q->msg_num, tmp_num);
	return (u32_t)(end - start);		//将等待时间设置为timeout/2;
  }
  else if(Err == OS_ERR_TIMEOUT)
  {
    //Printf("mbox fetch time out error");
    if(msg != NULL) {
      *msg  = NULL;
    }
	
	return SYS_ARCH_TIMEOUT;
  }
  else
  {
	  Printf("mbox fetch uknow error [%u]\n", Err);
	  if(msg != NULL) {
			*msg  = NULL;
	  }
	  
	  return SYS_ARCH_TIMEOUT;
  }

}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)
{
  u32_t tmp_num;
  SYS_ARCH_DECL_PROTECT(lev);

  /* parameter check */
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);

  if (OSSemAccept(q->sem)) {
    SYS_ARCH_PROTECT(lev);

	if(q->head == q->tail)
	{
        Printf("mbox tryfetch queue abnormal [%u]\n", q->msg_num);
		if(msg != NULL) {
			*msg  = NULL;
	    }
		SYS_ARCH_UNPROTECT(lev);
		return SYS_MBOX_EMPTY;
	}
		
    if(msg != NULL) {
      *msg  = q->q_mem[q->tail];
    }

    (q->tail)++;
    if (q->tail >= MAX_QUEUE_ENTRIES) {
      q->tail = 0;
    }

    if(q->msg_num > 0)
	{
      q->msg_num--;
	}

	tmp_num = (q->head >= q->tail)?(q->head - q->tail):(MAX_QUEUE_ENTRIES + q->head - q->tail);
    
	
	if(tmp_num != q->msg_num)
	{
        Printf("mbox try fetch error, umatch [%u] with tmp [%u]\n", q->msg_num, tmp_num);
	}
	
    SYS_ARCH_UNPROTECT(lev);
    return 0;
  }
  else
  {
    Printf("mbox try fetch uknow error\n");
    if(msg != NULL) {
      *msg  = NULL;
    }

    return SYS_MBOX_EMPTY;
  }
}

//函数功能：新建一个进程，在整个系统中只会被调用一次
//sys_thread_t sys_thread_new(char *name, void (* thread)(void *arg), void *arg, int stacksize, int prio);
//prio 1~10, is kept for network 
//TCPIP_THREAD_PRIO    1   -> lwip thead prio
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

#endif


