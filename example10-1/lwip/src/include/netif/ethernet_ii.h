
#ifndef __ETHERNET_II_H__
#define __ETHERNET_II_H__

#define ETHERNET_II_HEAD            (14)
#define ETHERNET_II_DATA_LEN        (50)
#define SIZEOF_ETHERNET_II_PACKET   (ETHERNET_II_HEAD+ETHERNET_II_DATA_LEN)
#define ETHTYPE_ETHERNET_II         (0x7002U)

err_t ethernet_ii_raw(struct netif *netif, const struct eth_addr *ethsrc_addr,
           const struct eth_addr *ethdst_addr, unsigned char *buff, int len);


#endif

