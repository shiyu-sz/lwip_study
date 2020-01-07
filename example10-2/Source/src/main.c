#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include <stdio.h>	 
#include  <stdarg.h>



//extern functions	
extern void SPI1_Init(void);
extern void lwip_demo(void *pdata);

void GPIO_Configuration(void);
void RCC_Configuration(void);

unsigned int system_tick_num = 0;
unsigned int sys_now(void)
{
    return system_tick_num;
}


/****************************************************************************
* 名    称：void NVIC_Configuration(void)
* 功    能：中断源配置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  /* Configure one bit for preemption priority */
  /* 优先级组 说明了抢占优先级所用的位数，和子优先级所用的位数   在这里是1， 7 */    
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  /* Enable the EXTI2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;				 //外部中断2
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	     //抢占优先级 0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			 //子优先级0  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 //使能
  NVIC_Init(&NVIC_InitStructure);

  																  //用于选择EXTI1外部中断的输入源是PA1。
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);     //外部中断配置,网卡中断

  EXTI_InitStructure.EXTI_Line = EXTI_Line1;					  //配置外部中断1
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			  //中断模式
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		  //下降沿触发
  //EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}
/****************************************************************************
* 名    称：void Usart1_Init(void)
* 功    能：串口1初始化函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void Usart1_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
 
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 , ENABLE);	 		//使能串口1时钟

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	         		 		//USART1 TX
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    		 		//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);		    		 		//A端口 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	         	 		//USART1 RX
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   	 		//复用开漏输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);		         	 		//A端口 

  USART_InitStructure.USART_BaudRate = 115200;						//速率115200bps
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//数据位8位
  USART_InitStructure.USART_StopBits = USART_StopBits_1;			//停止位1位
  USART_InitStructure.USART_Parity = USART_Parity_No;				//无校验位
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;   //无硬件流控
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式

  /* Configure USART1 */
  USART_Init(USART1, &USART_InitStructure);							//配置串口参数函数   
   /* Enable the USART1 */
  USART_Cmd(USART1, ENABLE);	
  
}

/****************************************************************************
* 名    称：void RCC_Configuration(void)
* 功    能：系统时钟配置为72MHZ， 外设时钟配置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void RCC_Configuration(void)
{
   SystemInit(); 
   RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO  , ENABLE);  
 
}


/****************************************************************************
* 名    称：void GPIO_Configuration(void)
* 功    能：通用IO口配置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/  
void GPIO_Configuration(void)
{
  
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 |RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                         RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                         RCC_APB2Periph_GPIOE, ENABLE);
  	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				     //LED1控制
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);					 

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_3;		 //LED2, LED3控制
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;					 //SST25VF016B SPI片选
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_7;		 //PB12---VS1003 SPI片选（V2.1) 
  GPIO_Init(GPIOB, &GPIO_InitStructure);					 //PB7---触摸屏芯片XPT2046 SPI 片选
  
  /* 禁止SPI1总线上的其他设备 */
  GPIO_SetBits(GPIOB, GPIO_Pin_7);						     //触摸屏芯片XPT2046 SPI 片选禁止  
  GPIO_SetBits(GPIOB, GPIO_Pin_12);						     //VS1003 SPI片选（V2.1)禁止 
  GPIO_SetBits(GPIOC, GPIO_Pin_4);						     //SST25VF016B SPI片选禁止  

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	         	 	//ENC28J60接收完成中断引脚 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   	 		//内部上拉输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);		 


//  GPIO_ResetBits(GPIOE, GPIO_Pin_1);						 //复位ENC28J60
//  Delay(0xAFFF);					   
//  GPIO_SetBits(GPIOE, GPIO_Pin_1 );		 	 	             
//  Delay(0xAFFF);	
}


/****************************************************************************
* 名    称：int main(void)
* 功    能：程序入口
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
 
int main(void)
{
	RCC_Configuration();				 //系统时钟设置
	NVIC_Configuration();
	GPIO_Configuration();                //IO口初始化
	Usart1_Init();                       //串口1初始化

	/* 配置systic作为20ms中断 */
	if (SysTick_Config(20*72000))		//时钟节拍中断，20ms一次
    { 
    	/* error handle*/ 
     	while (1);
    }
   		  
	/* 向串口1发送开机字符 */
    printf("****       (C) COPYRIGHT 2013 制造者科技    *******\r\n");    	  //。
    printf("*                                                 *\r\n");  
    printf("*                                                 *\r\n");    	  	
    printf("*     MCU平台:STM32F103VET6                       *\r\n");    	
    printf("*     以太网硬件:ENC28J60                         *\r\n");    	  	
    printf("*     固件库：3.5                                 *\r\n");    	
    printf("*     例程版本: 0.2                               *\r\n");     
    printf("*                                                 *\r\n");    	 	
    printf("***************************************************\r\n");  	
				
	SPI1_Init();	      //SPI口初始化，用于网卡间传输数据
	
	lwip_demo(NULL);	  //初始化内核，启动LwIP相关
	 
	while (1)
	{
	    //exit(0);	
	}
}

__asm void EnableInt(unsigned int data)
{

    MRS     R0, PRIMASK
    CPSID   I
    BX      LR

}
__asm unsigned int DisableInt(void)
{

    MSR     PRIMASK, R0;
    BX      LR

}
