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
#if 0
#include "../include/lwip/debug.h"

#include "../include/lwip/def.h"
#include "../include/lwip/sys.h"
#include "../include/lwip/mem.h"
#include "err.h"
//yangye 2003-1-22
#include "sys_arch.h" 
//yangye 2003-1-27
//notice: we use OSTaskQuery in sys_arch_timeouts() !
//#include "ucos_ii.h"
//#include "os_cfg.h"


const void * const pvNullPointer;

static OS_MEM *pQueueMem;

static char pcQueueMemoryPool[MAX_QUEUES * sizeof(TQ_DESCR) ];

//yangye 2003-1-27
struct sys_timeouts lwip_timeouts[LWIP_TASK_MAX];
struct sys_timeouts null_timeouts;

static OS_TCB curr_task_pcb;

OS_STK LWIP_TASK_STK[LWIP_TASK_MAX][LWIP_STK_SIZE];
u8_t curr_prio_offset;

/*-----------------------------------------------------------------------------------*/
sys_mbox_t sys_mbox_new(int size)
{
    u8_t       ucErr;
    PQ_DESCR    pQDesc;
    
    pQDesc = OSMemGet( pQueueMem, &ucErr );
    if( ucErr == OS_NO_ERR ) {   
        pQDesc->pQ = OSQCreate( &(pQDesc->pvQEntries[0]), MAX_QUEUE_ENTRIES );       
        if( pQDesc->pQ != NULL ) {
            return pQDesc;
        }
    } 
    return SYS_MBOX_NULL;
}

/*-----------------------------------------------------------------------------------*/
void
sys_mbox_free(sys_mbox_t mbox)
{
    u8_t     ucErr;
    
    //clear OSQ EVENT
    OSQFlush( mbox->pQ );
    //del OSQ EVENT
    (void)OSQDel( mbox->pQ, OS_DEL_NO_PEND, &ucErr);
    //put mem back to mem queue
    ucErr = OSMemPut( pQueueMem, mbox );
}

/*-----------------------------------------------------------------------------------*/
void
sys_mbox_post(sys_mbox_t mbox, void *data)
{
    if( !data ) 
	data = (void*)&pvNullPointer;
    (void)OSQPost( mbox->pQ, data);
}
err_t sys_mbox_trypost(sys_mbox_t mbox, void *msg)
{
  sys_mbox_post(mbox,msg);
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
u32_t
sys_arch_mbox_fetch(sys_mbox_t mbox, void **data, u32_t timeout)
{
    u8_t     ucErr;
    u16_t ucos_timeout;
  //yangye 2003-1-27
  //in lwip ,timeout is  millisecond 
  //in ucosII ,timeout is timer  tick! 
  //chang timeout from millisecond to ucos tick
  ucos_timeout = 0;
  if(timeout != 0){
  ucos_timeout = (timeout * OS_TICKS_PER_SEC)/1000;
  if(ucos_timeout < 1)
  	ucos_timeout = 1;
  else if(ucos_timeout > 65535)
  	ucos_timeout = 65535;
  }  
    
  //yangye 2003-1-29
  //it is very importent!!!! 
  //sometimes lwip calls sys_mbox_fetch(mbox,NULL)
  //it is dangrous to return (void *) to NULL ! (from OSQPend())
  if(data != NULL){
    *data = OSQPend( mbox->pQ, (u16_t)ucos_timeout, &ucErr );        
  }else{
    //just discard return value if data==NULL
    OSQPend(mbox->pQ,(u16_t)ucos_timeout,&ucErr);
  }
    
  if( ucErr == OS_TIMEOUT ) {
        timeout = SYS_ARCH_TIMEOUT;
    } else {
      if(*data == (void*)&pvNullPointer ) 
	  *data = NULL;
      timeout = 1;
    }
  return timeout;
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t mbox, void **msg)
{
 return sys_arch_mbox_fetch(mbox,msg,1);
}
/*-----------------------------------------------------------------------------------*/

sys_sem_t sys_sem_new(u8_t count)
{
    sys_sem_t pSem;
    pSem = OSSemCreate((u16_t)count );
    return pSem;
}

/*-----------------------------------------------------------------------------------*/

void sys_sem_signal(sys_sem_t sem)
{
    u8_t     ucErr;
    ucErr = OSSemPost((OS_EVENT *)sem );
}

/*-----------------------------------------------------------------------------------*/
//yangye 2003-1-25
u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout)
{
  u8_t err;
  u32_t ucos_timeout;
  //yangye 2003-1-27
  //in lwip ,timeout is  millisecond 
  //in ucosII ,timeout is timer  tick! 
  //chang timeout from millisecond to ucos tick
  ucos_timeout = 0;
  if(timeout != 0){
  ucos_timeout = (timeout * OS_TICKS_PER_SEC)/1000;
  if(ucos_timeout < 1)
  	ucos_timeout = 1;
  else if(ucos_timeout > 65535)
  	ucos_timeout = 65535;
  }
  	
  OSSemPend ((OS_EVENT *)sem,(u16_t)ucos_timeout, (u8_t *)&err);
  //should not return 0 when wait time is 0, only when timeout!
  //see sys_arch.txt in lwip/doc 
  if(err == OS_TIMEOUT)
  	return SYS_ARCH_TIMEOUT;
  else
  	return 1;
}

/*-----------------------------------------------------------------------------------*/

void sys_sem_free(sys_sem_t sem)
{
    u8_t     ucErr;
    (void)OSSemDel((OS_EVENT *)sem, OS_DEL_NO_PEND, &ucErr );
}

/*-----------------------------------------------------------------------------------*/
void sys_init(void)
{
    u8_t i;
    //this func is called first in lwip task!
    u8_t   ucErr;
    //init mem used by sys_mbox_t
    //use ucosII functions
    pQueueMem = OSMemCreate( (void*)pcQueueMemoryPool, MAX_QUEUES, sizeof(TQ_DESCR), &ucErr );
    //init lwip task prio offset
    curr_prio_offset = 0;
    //init lwip_timeouts for every lwip task
    for(i=0;i<LWIP_TASK_MAX;i++){
    	lwip_timeouts[i].next = NULL;
    }
}
/*-----------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
struct sys_timeouts * sys_arch_timeouts(void)
{
  u8_t curr_prio;
  s16_t err,offset;
 
    
  null_timeouts.next = NULL;
  //get current task prio
  err = OSTaskQuery(OS_PRIO_SELF,&curr_task_pcb);
  curr_prio = curr_task_pcb.OSTCBPrio;
  
  offset = curr_prio - LWIP_START_PRIO;
  //not called by a lwip task ,return timeouts->NULL
  if(offset < 0 || offset >= LWIP_TASK_MAX)
  {
    return &null_timeouts;
  }

  return &lwip_timeouts[offset];
}
/*------------------------------------------------------------------------*/


sys_thread_t sys_thread_new(char *name, void (* thread)(void *arg), void *arg, int stacksize, int prio)
//sys_thread_t sys_thread_new(void (* function)(void *arg), void *arg, int prio)
{

  if(curr_prio_offset < LWIP_TASK_MAX){
  
    OSTaskCreate(thread, (void *)arg, &LWIP_TASK_STK[curr_prio_offset][LWIP_STK_SIZE-1],LWIP_START_PRIO+curr_prio_offset );
  //  OSTaskCreate(function, (void *)0x1111, &LWIP_TASK_STK[curr_prio_offset][LWIP_STK_SIZE-1],prio );
    curr_prio_offset++; 
  } else {
   // Uart_Printf(" lwip task prio out of range ! error! ");
  }
  return prio;
}
#endif






