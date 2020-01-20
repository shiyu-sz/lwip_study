/**
  ******************************************************************************
  * @file    telnetserver.c 
  * @author  Forrest
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   A hello world example based on a Telnet connection
  *          The application works as a server which wait for the client request
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 
/* Includes ------------------------------------------------------------------*/
#include "telnetserver.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>

extern unsigned int sys_now(void);

#define MAX_INFO_LEN 30
#define LOGIN_INFO "Please input Password to login:"
#define PASSWORD "20140102"

char *command[] = {
    "greeting",
	"date",
	"systick",
	"version",
	"quit",
	"help",
};
enum TFTP_STATE 
{
    TFTP_SETUP,
	TFTP_CONNECTED,
};

typedef struct 
{
  int state;
  u16_t client_port;
  u16_t bytes_len;
  char bytes[MAX_INFO_LEN];
}telnet_conn_arg;

void telnet_server_close_conn(struct tcp_pcb *pcb)
{
    if(NULL != (void *)pcb->callback_arg)
    {
		mem_free((void *)pcb->callback_arg);
		tcp_arg(pcb, NULL);
	}
	
	tcp_close(pcb);
}
int process_tftp_command(struct tcp_pcb *pcb, char *req)
{
    char res_buffer[100] = {0,};
	int strlen = 0;
	int close_flag = 0;
	
    if(strcmp(req, command[0]) == 0)
    {
        strlen = sprintf(res_buffer, "Hi, I am a LwIP Telnet Server..\r\n");
	}else if(strcmp(req, command[1]) == 0)
	{
	    strlen = sprintf(res_buffer, "Time Now is: 2014/01/02 11:45..\r\n");
	}else if(strcmp(req, command[2]) == 0)
	{
	    strlen = sprintf(res_buffer, "Current Systick is [%u]..\r\n", sys_now());
	}else if(strcmp(req, command[3]) == 0)
	{
	    strlen = sprintf(res_buffer, "Stm32 demo board V0.2 by 制造者科技..\r\n");
	}else if(strcmp(req, command[4]) == 0)
	{
	    strlen = sprintf(res_buffer, "The Connection will shutdown..\r\n");
		close_flag = 1;
	}else if(strcmp(req, command[5]) == 0)
	{
	    strlen = sprintf(res_buffer, "Suppprted Command：date  greeting  systick  version help  quit..\r\n");
	}else
	{
        strlen = sprintf(res_buffer, "Command not support..\r\n");
	}

	tcp_write(pcb, res_buffer, strlen, TCP_WRITE_FLAG_COPY);
	strlen = sprintf(res_buffer, "LwIP Shell>");
	tcp_write(pcb, res_buffer, strlen, TCP_WRITE_FLAG_COPY);

	return close_flag;

}

int telnet_input(struct tcp_pcb *pcb, telnet_conn_arg * conn_args, struct pbuf *p)
{
	
	int strlen = 0;
	char buf[20];
	u16_t len=p->len;
	u8_t * datab = (unsigned char *)p->payload;
	printf("Connection error len = %d,char = %c\r\n", len, *datab);
	if((len == 2) && (*datab == 0x0d) && (*(datab+1) == 0x0a))
	{	
		conn_args->bytes[conn_args->bytes_len] = 0x00;
		return 1;
	}
	else if((len == 1) && (*datab >= 0x20) && (*datab <= 0x7e))
	{
		conn_args->bytes[conn_args->bytes_len] = *datab;
        if(conn_args->bytes_len < (MAX_INFO_LEN-1))
        {
			conn_args->bytes_len++;
        }
		
	}
	else if((len == 1) && (*datab == 0x08) && (conn_args->bytes_len > 0))
	{
		conn_args->bytes_len--;
		strlen = sprintf(buf," \b \b");
		tcp_write(pcb, buf, strlen, TCP_WRITE_FLAG_COPY);
	}
	else if((len == 1) && (*datab == 0x08))
	{
		conn_args->bytes_len = 0;
		strlen = sprintf(buf,">");
		tcp_write(pcb, buf, strlen, TCP_WRITE_FLAG_COPY);
	}
	return 0;
}

/**
  * @brief  Called when a data is received on the telnet connection
  * @param  arg	the user argument
  * @param  pcb	the tcp_pcb that has received the data
  * @param  p	the packet buffer
  * @param  err	the error value linked with the received data
  * @retval error value
  */
static err_t telnet_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  telnet_conn_arg *conn_args = (telnet_conn_arg *)arg;
  char sndbuf[50];
  int strlen = 0;
  int ret = 0;
  
  if(NULL == conn_args || pcb->remote_port != conn_args->client_port)
  {
	  if(p) pbuf_free(p);
	  return ERR_ARG;
  }
  /* We perform here any necessary processing on the pbuf */
  if (p != NULL) 
  {        
	/* We call this function to tell the LwIp that we have processed the data */
	/* This lets the stack advertise a larger window, so more data can be received*/
	tcp_recved(pcb, p->tot_len);

	//memset(conn_args->bytes, 0, MAX_INFO_LEN);
	//copy_len = p->len > (MAX_INFO_LEN - 1)?(MAX_INFO_LEN - 1):p->len;
	//strncpy(conn_args->bytes, p->payload, copy_len);
    ret = telnet_input(pcb, conn_args, p);
	if(ret == 1)
	{
		switch(conn_args->state)
		{
        case TFTP_SETUP:
			if(strcmp(conn_args->bytes, PASSWORD) == 0)
			{
				strlen = sprintf(sndbuf,"##Welcome to demo TELNET based on LwIP##\r\n");
                tcp_write(pcb, sndbuf, strlen,TCP_WRITE_FLAG_COPY); 
                strlen = sprintf(sndbuf,"##Created by Forrest...               ##\r\n");
                tcp_write(pcb, sndbuf, strlen,TCP_WRITE_FLAG_COPY);
                strlen = sprintf(sndbuf,"##quit:退出    help:帮助信息          ##\r\n");
                tcp_write(pcb, sndbuf, strlen,TCP_WRITE_FLAG_COPY);
                strlen = sprintf(sndbuf,"LwIP Shell>");
                tcp_write(pcb,sndbuf,strlen, 1);
				
				conn_args->state = TFTP_CONNECTED;
			}
			else
			{
			    strlen = sprintf(sndbuf,"##PASSWORD ERROR! Try again:##\r\n");
                tcp_write(pcb, sndbuf, strlen,TCP_WRITE_FLAG_COPY);
			}
			
			memset(conn_args->bytes, 0, MAX_INFO_LEN);
			conn_args->bytes_len = 0;
			
			break;
	    case TFTP_CONNECTED:

				if(process_tftp_command(pcb, conn_args->bytes) == 0)
				{
				    memset(conn_args->bytes, 0, MAX_INFO_LEN);
			        conn_args->bytes_len = 0;
				}else
	    		{
        			telnet_server_close_conn(pcb);
				}
			break;
		default:
			break;
		}
	/* End of processing, we free the pbuf */
	}
	
    pbuf_free(p);
  }  
  else if (err == ERR_OK) 
  {
    /* When the pbuf is NULL and the err is ERR_OK, the remote end is closing the connection. */
    /* We free the allocated memory and we close the connection */
	telnet_server_close_conn(pcb);
  }
  return ERR_OK;


}


/**
  * @brief  This function is called when an error occurs on the connection, pcb already dealloc
  * @param  arg
  * @parm   err
  * @retval None 
  */
static void telnet_server_conn_err(void *arg, err_t err)
{
  telnet_conn_arg *conn_arg = (telnet_conn_arg *)arg;
  if(!conn_arg) return;
  
  printf("Connection error [%d] for client [port:%d]", err, conn_arg->client_port);
  mem_free(arg);
}

/**
  * @brief  This function when the Telnet connection is established
  * @param  arg  user supplied argument 
  * @param  pcb	 the tcp_pcb which accepted the connection
  * @param  err	 error value
  * @retval ERR_OK
  */
static err_t telnet_server_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{     
  u32_t remote_ip;
  char greet[100];
  u8_t iptab[4];
  telnet_conn_arg *conn_arg = NULL;
  remote_ip = pcb->remote_ip.addr;

  iptab[0] = (u8_t)(remote_ip >> 24);
  iptab[1] = (u8_t)(remote_ip >> 16);
  iptab[2] = (u8_t)(remote_ip >> 8);
  iptab[3] = (u8_t)(remote_ip);

  sprintf(greet, "Welcome to Telnet! your Info --> [%d.%d.%d.%d:%d]\r\n", \
  	              iptab[3], iptab[2], iptab[1], iptab[0], pcb->remote_port);	

  conn_arg = mem_calloc(sizeof(telnet_conn_arg), 1);
  if(!conn_arg)
  {
    return ERR_MEM;
  }
  /* Tell LwIP to associate this structure with this connection. */
  conn_arg->state = TFTP_SETUP;
  //conn_arg->state = TFTP_CONNECTED;
  conn_arg->client_port = pcb->remote_port;
  conn_arg->bytes_len = 0;
  memset(conn_arg->bytes, 0, MAX_INFO_LEN);
  
  tcp_arg(pcb, conn_arg);
  
  /* Configure LwIP to use our call back functions. */
  tcp_err(pcb, telnet_server_conn_err);
  tcp_recv(pcb, telnet_server_recv);
  
  /* Send out the first message */  
  tcp_write(pcb, greet, strlen(greet), 1);
  tcp_write(pcb, LOGIN_INFO, strlen(LOGIN_INFO), 1); 
  
  return ERR_OK;
}

/**
  * @brief  Initialize the telnet application  
  * @param  None 
  * @retval None 
  */
#define TELNET_SERVER_PORT 23
void telnet_server_init(void)
{
  struct tcp_pcb *pcb;	            		
  
  /* Create a new TCP control block  */
  pcb = tcp_new();	                		 	

  /* Assign to the new pcb a local IP address and a port number */
  /* Using IP_ADDR_ANY allow the pcb to be used by any local interface */
  tcp_bind(pcb, IP_ADDR_ANY, TELNET_SERVER_PORT);       


  /* Set the connection to the LISTEN state */
  pcb = tcp_listen(pcb);				

  /* Specify the function to be called when a connection is established */	
  tcp_accept(pcb, telnet_server_accept); 						
}


