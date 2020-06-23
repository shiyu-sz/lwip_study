/****************************************************************************
* Copyright (C), 2011 奋斗嵌入式工作室 www.ourstm.net
*
* 本例程在 奋斗版STM32开发板V3上调试通过           
* QQ: 9191274, 旺旺：sun68, Email: sun68@163.com 
* 淘宝店铺：ourstm.taobao.com  
*
* 文件名: bsp.c
* 内容简述:
*       本例程提供了硬件平台的初始化
		
*
* 文件历史:
* 版本号  日期       作者    说明
* v0.2   2011-09-06 sun68  创建该文件
*
*/
#include "includes.h"
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "demo.h"
//#include "BUTTON.h"
//#include "uip.h"
//#include "uip_arp.h"
//#include "tapdev.h"	   
#include "enc28j60.h"	
#include  <stdarg.h>
void InitNet(void);	
void GPIO_Configuration(void);
void RCC_Configuration(void);
//void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,...);
void Printf(unsigned char *Data,...);
char *itoa(int value, char *string, int radix);
char *itoh(unsigned int value, char *string, int radix);
//void tp_Config(void);
//u16 TPReadX(void);
//u16 TPReadY(void);
void NVIC_Configuration(void);
//extern void FSMC_LCD_Init(void); 
//extern void SPI_Flash_Init(void);
extern void Delay(__IO uint32_t nCount);
extern void SPI1_Init(void);

/****************************************************************************
* 名    称：void RCC_Configuration(void)
* 功    能：系统时钟配置为72MHZ， 外设时钟配置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/ 
void RCC_Configuration(void){
  SystemInit();
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);			   //复用功能使能
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
}

/****************************************************************************
* 名    称：void USART_Config(USART_TypeDef* USARTx,u32 baud)
* 功    能：串口配置
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void USART_Config(USART_TypeDef* USARTx,u32 baud){
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
* 名    称：void BSP_Init(void)
* 功    能：奋斗板初始化函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/  
void BSP_Init(void)
{  																		   
  RCC_Configuration();  	       //系统时钟初始化	
  NVIC_Configuration();		   //中断源配置   
  GPIO_Configuration();			   //GPIO配置
  USART_Config(USART1,115200);     //初始化串口1
 
  SPI1_Init();                     //ENC28J60 SPI 初始化
  //tp_Config();					   //SPI1 触摸电路初始化    
  //FSMC_LCD_Init();				   //FSMC TFT接口初始化  

  /* 显示例程Logo */
  //USART_OUT(USART1,"**** (C) COPYRIGHT 2011 奋斗嵌入式开发工作室 *******\r\n");    	  //向串口1发送开机字符。
  //USART_OUT(USART1,"*                                                 *\r\n");    	
  //USART_OUT(USART1,"*     奋斗版STM32开发板基于uIP1.0的以太网实验     *\r\n");  
  //USART_OUT(USART1,"*                                                 *\r\n");    	  	
  //USART_OUT(USART1,"*     MCU平台:STM32F103VET6                       *\r\n");    	
  //USART_OUT(USART1,"*     以太网硬件:ENC28J60                         *\r\n");    	  	
  //USART_OUT(USART1,"*     固件库：3.5                                 *\r\n");    	
  //USART_OUT(USART1,"*     例程版本: 0.2                               *\r\n");  
  //USART_OUT(USART1,"*     奋斗STM32店铺：ourstm.taobao.com            *\r\n");     	   
  //USART_OUT(USART1,"*     奋斗STM32论坛：www.ourstm.net  QQ：9191274  *\r\n");   
  //USART_OUT(USART1,"*                                                 *\r\n");    	 	
  //USART_OUT(USART1,"***************************************************\r\n");  
    
 // InitNet();		/* 初始化网络设备以及UIP协议栈，配置IP地址 */

  /* 创建一个TCP监听端口和http监听端口，端口号为1200，80 */
  //uip_listen(HTONS(1200));
  //uip_listen(HTONS(80));
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
  //NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 //使能
  NVIC_Init(&NVIC_InitStructure);

  //用于配置AFIO外部中断配置寄存器AFIO_EXTICR1，用于选择EXTI2外部中断的输入源是PE2。
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);     //外部中断配置AFIO--ETXI2

  EXTI_InitStructure.EXTI_Line = EXTI_Line1;					  
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			  //中断模式
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		  //下降沿触发
  //EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}

/****************************************************************************
* 名    称：void  OS_CPU_SysTickInit(void)
* 功    能：ucos 系统节拍时钟初始化  初始设置为10ms一次节拍
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
void  OS_CPU_SysTickInit(void)
{
    RCC_ClocksTypeDef  rcc_clocks;
    INT32U         cnts;
    RCC_GetClocksFreq(&rcc_clocks);		                        //获得系统时钟的值	 
    cnts = (INT32U)rcc_clocks.HCLK_Frequency/OS_TICKS_PER_SEC;	//算出时钟节拍的值	
	SysTick_Config(cnts);										//设置时钟节拍	     
}

/****************************************************************************
* 名    称：void tp_Config(void)
* 功    能：TFT 触摸屏控制初始化
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：无 
****************************************************************************/
//void tp_Config(void) 
//{ 
//  GPIO_InitTypeDef  GPIO_InitStructure;     
//
//  /* SPI1 时钟使能 */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE); 
// 
//  /* SPI1 SCK(PA5)、MISO(PA6)、MOSI(PA7) 设置 */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//口线速度50MHZ
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	        //复用模式
//  GPIO_Init(GPIOA, &GPIO_InitStructure);
//
//  /* SPI1 触摸芯片的片选控制设置 PB7 */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//口线速度50MHZ 
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;			//推挽输出模式
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
//  
//  
//  /* 由于SPI1总线上挂接了4个外设，所以在使用触摸屏时，需要禁止其余3个SPI1 外设， 才能正常工作 */  
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;           		//SPI1 SST25VF016B片选 
//  GPIO_Init(GPIOC, &GPIO_InitStructure);
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;                //SPI1 VS1003片选 
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;                 //SPI1 网络模块片选  
//  GPIO_Init(GPIOA, &GPIO_InitStructure);
//  GPIO_SetBits(GPIOC, GPIO_Pin_4);							//SPI1 SST25VF016B片选置高 
//  GPIO_SetBits(GPIOB, GPIO_Pin_12);							//SPI1 VS1003片选置高  
//  GPIO_SetBits(GPIOA, GPIO_Pin_4);							//SPI1 网络模块片选置高 	 
//																		 
//}

/****************************************************************************
* 名    称：unsigned char SPI_WriteByte(unsigned char data) 
* 功    能：SPI1 写函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/  
//unsigned char SPI_WriteByte(unsigned char data) 
//{ 
// unsigned char Data = 0; 
//
//  //等待发送缓冲区空
//  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET); 
//  // 发送一个字节  
//  SPI_I2S_SendData(SPI1,data); 
//
//   //等待是否接收到一个字节 
//  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET); 
//  // 获得该字节
//  Data = SPI_I2S_ReceiveData(SPI1); 
//
//  // 返回收到的字节 
//  return Data; 
//}  

/****************************************************************************
* 名    称：void SpiDelay(unsigned int DelayCnt) 
* 功    能：SPI1 写延时函数
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/  
//void SpiDelay(unsigned int DelayCnt)
//{
// unsigned int i;
// for(i=0;i<DelayCnt;i++);
//}

/****************************************************************************
* 名    称：u16 TPReadX(void) 
* 功    能：触摸屏X轴数据读出
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/  
//u16 TPReadX(void)
//{ 
//   u16 x=0;
//   TP_CS();	                        //选择XPT2046 
//   SpiDelay(10);					//延时
//   SPI_WriteByte(0xd0);				//设置X轴读取标志
//   SpiDelay(10);					//延时
//   x=SPI_WriteByte(0x00);			//连续读取16位的数据 
//   x<<=8;
//   x+=SPI_WriteByte(0x00);
//   SpiDelay(10);					//禁止XPT2046
//   TP_DCS(); 					    								  
//   x = x>>3;						//移位换算成12位的有效数据0-4095
//   return (x);
//}
/****************************************************************************
* 名    称：u16 TPReadY(void)
* 功    能：触摸屏Y轴数据读出
* 入口参数：无
* 出口参数：无
* 说    明：
* 调用方法：
****************************************************************************/
//u16 TPReadY(void)
//{
//   u16 y=0;
//   TP_CS();	                        //选择XPT2046 
//   SpiDelay(10);					//延时
//   SPI_WriteByte(0x90);				//设置Y轴读取标志
//   SpiDelay(10);					//延时
//   y=SPI_WriteByte(0x00);			//连续读取16位的数据 
//   y<<=8;
//   y+=SPI_WriteByte(0x00);
//   SpiDelay(10);					//禁止XPT2046
//   TP_DCS(); 					    								  
//   y = y>>3;						//移位换算成12位的有效数据0-4095
//   return (y);
//}


/*******************************************************************************
*	函数名：InitNet
*	输  入:
*	输  出:
*	功能说明：初始化网络硬件、UIP协议栈、配置本机IP地址
************************************************************/
void InitNet(void)
{	//uip_ipaddr_t ipaddr;
//	myip[0]=192; myip[1]=168; myip[2]=1; myip[3]=15;
//	routeip[0]=192; routeip[1]=168; routeip[2]=1; routeip[3]=1;
//	netmask[0]=255; netmask[1]=255; netmask[2]=255; netmask[3]=0;
//	TCP_S_Link=0;                            //TCP服务器和客户端建立标志
//
//	tapdev_init();                     		 //ENC28J60初始化
//	
//	USART_OUT(USART1,"uip_init\n\r");
//	uip_init();								 //UIP协议栈初始化
//
//	USART_OUT(USART1,"uip ip address : %d,%d,%d,%d\r\n",myip[0],myip[1],myip[2],myip[3]);
//	uip_ipaddr(ipaddr, myip[0],myip[1],myip[2],myip[3]);		 //设置IP地址
//	uip_sethostaddr(ipaddr);
//
//	USART_OUT(USART1,"uip route address : %d,%d,%d,%d\r\n",routeip[0],routeip[1],routeip[2],routeip[3]);
//	uip_ipaddr(ipaddr, routeip[0],routeip[1],routeip[2],routeip[3]);		 //设置默认路由器IP地址
//	uip_setdraddr(ipaddr);
//
//	USART_OUT(USART1,"uip net mask : %d,%d,%d,%d\r\n",netmask[0],netmask[1],netmask[2],netmask[3]);
//	uip_ipaddr(ipaddr, netmask[0],netmask[1],netmask[2],netmask[3]);		 //设置网络掩码
//	uip_setnetmask(ipaddr);

}

/****************************************************************************
* 名    称：void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,...)
* 功    能：格式化串口输出函数
* 入口参数：USARTx:  指定串口
			Data：   发送数组
			...:     不定参数
* 出口参数：无
* 说    明：格式化串口输出函数
        	"\r"	回车符	   USART_OUT(USART1, "abcdefg\r")   
			"\n"	换行符	   USART_OUT(USART1, "abcdefg\r\n")
			"%s"	字符串	   USART_OUT(USART1, "字符串是：%s","abcdefg")
			"%d"	十进制	   USART_OUT(USART1, "a=%d",10)
* 调用方法：无 
****************************************************************************/
void Printf(unsigned char *Data,...){ 
   
	const char *s;
	unsigned char* temp;
    int d;
    char buf[16];
    va_list ap;
    va_start(ap, Data);
	while(*Data!=0){				                          //判断是否到达字符串结束符
		if(*Data==0x5c){									  //'\'
			switch (*++Data){
				case 'r':							          //回车符
					USART_SendData(USART1, 0x0d);	   

					Data++;
					break;
				case 'n':							          //换行符
					USART_SendData(USART1, 0x0a);	
					Data++;
					break;
				
				default:
					Data++;
				    break;
			}
			
			 
		}
		else if(*Data=='%'){									  //
	        //first, we escape length control
      	    temp = (Data + 1);
			while(*temp >= '0' && *temp <= '9') {temp++;};
			Data = temp - 1;

			switch (*++Data){				
				case 's':										  //字符串
                	s = va_arg(ap, const char *);
                	for ( ; *s; s++) {
                    	USART_SendData(USART1,*s);
						while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
                	}
					Data++;
                	break;
            	case 'd':										  //十进制
				case 'u':
                	d = va_arg(ap, int);
                	itoa(d, buf, 10);
                	for (s = buf; *s; s++) {
                    	USART_SendData(USART1,*s);
						while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
                	}
					Data++;
                	break;
				case 'x':										  //十六进制
                	d = va_arg(ap, int);
                	itoh(d, buf, 16);
					
                	for (s = buf; *s; s++) {
                    	USART_SendData(USART1,*s);
						while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
                	}
					Data++;
                	break;
				default:
					Data++;
				    break;
			}		 
		}
		else USART_SendData(USART1, *Data++);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
	}
}

/******************************************************
		整形数据转字符串函数
        char *itoa(int value, char *string, int radix)
		radix=10 表示是10进制	非十进制，转换结果为0;  

	    例：d=-379;
		执行	itoa(d, buf, 10); 后
		
		buf="-379"							   			  
**********************************************************/
char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 1000000000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */

/******************************************************
		整形数据转十六字符串函数
        char *itoh(unsigned int value, char *string, int radix)
		radix=16 表示是16进制	非十六进制，转换结果为0;  
							   			  
**********************************************************/
char *itoh(unsigned int value, char *string, int radix)
{
    unsigned int  i, d;
    int     flag = 0;
	int     index = 32;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 16)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
		*ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    for (i = 0xF0000000; i > 0; i = i >> 4)
    {
        d = value & i;
		index = index - 4;
        d = d >> index;

        if (d || flag)
        {
		    //for last and only last hex, we add a '0'
		    if(i == 0x0F && flag == 0)
			{
			    *ptr++ = 0x30;
			}
		
			if(d < 10)
		    {
                *ptr++ = (char)(d + 0x30);
			}else
			{
				*ptr++ = (char)(d - 10 + 'a');
			}
            //value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */
/******************* (C) COPYRIGHT 2011 奋斗STM32 *****END OF FILE****/

