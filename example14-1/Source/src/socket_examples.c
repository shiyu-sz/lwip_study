
#include "socket_examples.h"

#include "lwip/opt.h"

#if LWIP_SOCKET

#include "lwip/sockets.h"
#include "lwip/sys.h"

#include <string.h>
#include <stdio.h>

#ifndef SOCK_TARGET_HOST
#define SOCK_TARGET_HOST  "192.168.2.10"
#endif

#ifndef SOCK_TARGET_PORT
#define SOCK_TARGET_PORT  8080
#endif
char zrxbuf[1024];

/** This is an example function that tests
    blocking-connect and nonblocking--recv-write . */
static void socket_nonblocking(void *arg)
{
  int s;
  int ret;
  u32_t opt;
  struct sockaddr_in addr;
  int err;

  LWIP_UNUSED_ARG(arg);
  /* set up address to connect to */
  memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_port = PP_HTONS(SOCK_TARGET_PORT);
  addr.sin_addr.s_addr = inet_addr(SOCK_TARGET_HOST);
  
  /* create the socket */
  //s = socket(AF_INET, SOCK_STREAM, 0);
  //LWIP_ASSERT("s >= 0", s >= 0);

  /* connect */
  do
  {
      s = socket(AF_INET, SOCK_STREAM, 0);
      LWIP_ASSERT("s >= 0", s >= 0);
      ret = connect(s, (struct sockaddr*)&addr, sizeof(addr));
	  Printf("socket connect result [%d]\n", ret);
	  if(ret != 0)
      {
         close(s);
	  }
  }while(ret != 0);
  
  /* should have an error: "inprogress" */
  if(ret != 0)
  {
     ret = close(s);
     while(1)
     {
         Printf("socket connect error\n");
		 sys_msleep(1000);
     }
  }
  
  /* nonblocking */
  opt = 1;
  ret = ioctlsocket(s, FIONBIO, &opt);
  LWIP_ASSERT("ret == 0", ret == 0);

  /* write should fail, too */
  while(1)
  {
        ret = read(s, zrxbuf, 1024);
        if (ret > 0) {
        /* should return 0: closed */
        Printf("socket recv a data\n");        
		}
        Printf("socket recv [%d]\n", ret);
		
        ret = write(s, "test", 4);
        if(ret>0)
        {
            Printf("socket send %d data\n",ret);
		}
		else
		{
            ret = close(s);
			Printf("socket closed %d\n",ret);
			while(1) sys_msleep(1000);
		}

		sys_msleep(1000);
		
  }
}

/** This is an example function that tests
    the recv function (timeout etc.). */
char rxbuf[1024];
char sndbuf[64];
static void socket_timeoutrecv(void *arg)
{
  int s;
  int ret;
  int opt;
  struct sockaddr_in addr;
  size_t len;

  LWIP_UNUSED_ARG(arg);
  /* set up address to connect to */
  memset(&addr, 0, sizeof(addr));
  addr.sin_len = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_port = PP_HTONS(SOCK_TARGET_PORT);
  addr.sin_addr.s_addr = inet_addr(SOCK_TARGET_HOST);

  /* first try blocking: */

  /* create the socket */
  //s = socket(AF_INET, SOCK_STREAM, 0);
  //LWIP_ASSERT("s >= 0", s >= 0);

  /* connect */
  do
  {
      s = socket(AF_INET, SOCK_STREAM, 0);
      LWIP_ASSERT("s >= 0", s >= 0);
      ret = connect(s, (struct sockaddr*)&addr, sizeof(addr));
	  Printf("socket connect result [%d]\n", ret);
	  if(ret != 0)
      {
         close(s);
	  }
  }while(ret != 0);
  /* should succeed */
  if(ret != 0)
  {
     Printf("socket connect error %d\n", ret);
     ret = close(s);
     while(1) sys_msleep(1000);
  }

  /* set recv timeout (100 ms) */
  opt = 1000;
  ret = lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &opt, sizeof(int));

    opt = 1;
    ret = ioctlsocket(s, FIONBIO, &opt);

  while(1)
  {
      len = 0;
      ret = read(s, zrxbuf, 1024);
      if (ret > 0) {
          /* should return 0: closed */
          //Printf("socket recv a data\n"); 
          len = ret;
      }
      Printf("read [%d] data\n", ret); 

	  len = sprintf(sndbuf,"Client:I receive [%d] data\n", len);
      ret = write(s, sndbuf, len);
      if(ret>0)
      {
          Printf("socket send %d data\n",ret);
	  }
	  else
	  {
          ret = close(s);
	      Printf("socket closed %d\n",ret);
		  while(1) sys_msleep(1000);
	  }

	  //sys_msleep(1000);
		
  }
}

void socket_examples_init(void)
{
  //sys_thread_new("socket_nonblocking", socket_nonblocking, NULL, 0, TCPIP_THREAD_PRIO+2);
  sys_thread_new("socket_timeoutrecv", socket_timeoutrecv, NULL, 0, TCPIP_THREAD_PRIO+1);
}

#endif /* LWIP_SOCKETS */
