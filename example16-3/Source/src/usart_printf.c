/****************************************************************************

*
* 文件名: usart_printf.c
* 内容简述: 本模块实现printf和scanf函数重定向到串口1
*	实现重定向，只需要添加2个函数
		int fputc(int ch, FILE *f);
		int fgetc(FILE *f);
*


*
*/
#if 0
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include <stdio.h>

/*******************************************************************************
	函数名：PrintfLogo
	输  入: 例程名称和例程最后更新日期
	输  出:
	功能说明：
*/
//void PrintfLogo(char *strName, char *strDate)
//{
//	USART_OUT(USART1,"*************************************************************\r\n");
//	USART_OUT(USART1,"* Example Name : %s\r\n", strName);
//	USART_OUT(USART1,"* Update Date  : %s\r\n", strDate);
//	USART_OUT(USART1,"* StdPeriph_Lib Version : V3.1.2\r\n");
//	USART_OUT(USART1,"* \r\n");
//	USART_OUT(USART1,"* Copyright ourstm.5d6d.com \r\n");
//	USART_OUT(USART1,"* QQ    : 9191274 \r\n");
//	USART_OUT(USART1,"* Email : sun68@qq.com \r\n");
//	USART_OUT(USART1,"*************************************************************\r\n");
//}



/*******************************************************************************
	函数名：fputc
	输  入:
	输  出:
	功能说明：
	重定义putc函数，这样可以使用printf函数从串口1打印输出
*/
int fputc(int ch, FILE *f)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	USART_SendData(USART1, (uint8_t) ch);

	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}

	return ch;
}

/*******************************************************************************
	函数名：fputc
	输  入:
	输  出:
	功能说明：
	重定义getc函数，这样可以使用scanff函数从串口1输入数据
*/
int fgetc(FILE *f)
{
	/* 等待串口1输入数据 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
	{}

	return (int)USART_ReceiveData(USART1);
}

#endif