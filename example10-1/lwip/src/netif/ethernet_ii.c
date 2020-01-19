
#include "lwip/opt.h"
#include "lwip/ip_addr.h"
#include "lwip/def.h"
#include "lwip/ip.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "netif/etharp.h"

#include <string.h>
#include "ethernet_ii.h"

err_t ethernet_ii_raw(struct netif *netif, const struct eth_addr *ethsrc_addr,
           const struct eth_addr *ethdst_addr, unsigned char *buff, int len)
{
  struct pbuf *p;
  err_t result = ERR_OK;
  struct eth_hdr *ethhdr;
    unsigned char *hdr;

#if LWIP_AUTOIP
    const u8_t * ethdst_hwaddr;
#endif /* LWIP_AUTOIP */


  LWIP_ASSERT("netif != NULL", netif != NULL);

  /* allocate a pbuf for the outgoing ARP request packet */
  p = pbuf_alloc(PBUF_RAW, SIZEOF_ETHERNET_II_PACKET, PBUF_RAM);
  /* could allocate a pbuf for an ARP request? */
  if (p == NULL) {
    LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
      ("etharp_raw: could not allocate pbuf for ARP request.\n"));
    ETHARP_STATS_INC(etharp.memerr);
    return ERR_MEM;
  }
  LWIP_ASSERT("check that first pbuf can hold struct etharp_hdr",
              (p->len >= SIZEOF_ETHARP_PACKET));

  ethhdr = (struct eth_hdr *)p->payload;
  hdr = ((u8_t*)ethhdr + ETHERNET_II_HEAD);
  memcpy(hdr, buff, len);

  LWIP_ASSERT("netif->hwaddr_len must be the same as ETHARP_HWADDR_LEN for etharp!",
              (netif->hwaddr_len == ETHARP_HWADDR_LEN));

#if LWIP_AUTOIP
    /* If we are using Link-Local, all ARP packets that contain a Link-Local
     * 'sender IP address' MUST be sent using link-layer broadcast instead of
     * link-layer unicast. (See RFC3927 Section 2.5, last paragraph) */
    ethdst_hwaddr = ip_addr_islinklocal(ipsrc_addr) ? (u8_t*)(ethbroadcast.addr) : ethdst_addr->addr;
#endif /* LWIP_AUTOIP */

#if LWIP_AUTOIP
  ETHADDR16_COPY(&ethhdr->dest, ethdst_hwaddr);
#else  /* LWIP_AUTOIP */
  ETHADDR16_COPY(&ethhdr->dest, ethdst_addr);
#endif /* LWIP_AUTOIP */
  ETHADDR16_COPY(&ethhdr->src, ethsrc_addr);


  ethhdr->type = PP_HTONS(ETHTYPE_ETHERNET_II);
  /* send ARP query */
  result = netif->linkoutput(netif, p);
  ETHARP_STATS_INC(etharp.xmit);
  /* free ARP query packet */
  pbuf_free(p);
  p = NULL;
  /* could not allocate pbuf for ARP request */

  return result;
}

