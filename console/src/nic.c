#include <stdio.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#include <unistd.h>
#include <errno.h>

#include <cdlib.h>
#include "cgi.h"


/********************************************************************************
long nic_get_adapters()
	Description:
		
	Parameters:
		xio_adapters[10] = AdapterInfo
	Returns:
		unsigned
********************************************************************************/
cg_stat_t STDCALL NIC_get_adapters(AdapterInfo *xio_adapters, int *xio_size)
{
    struct ifreq  *pifreq;
    struct ifreq  ireq;
    struct ifconf conf;
    cg_stat_t status = CG_STAT_SUCCESS;
    int devfd;
    int i;
    int devices=0;
    
    devfd = socket(PF_INET,SOCK_STREAM,0);
    if (devfd < 0) {
        printf("(!) failed to open socket %d\n",errno);
        exit(-1);
    }
    
    i = *xio_size  * sizeof(struct ifreq); 
    pifreq = (struct ifreq*)malloc(i);
    if (pifreq == NULL)
        fexit(CG_STAT_INSUFFICIENT_MEMORY);
    
    conf.ifc_req = pifreq;
    conf.ifc_len = i;

    /* get interfaces list */
    if (ioctl(devfd,SIOCGIFCONF,&conf) < 0) {
        printf("(!) failed to query interfaces (stat=%d)\n",errno);
        fexit(CG_STAT_ADAPTERS_QUERY_FAILED);
    }
    
    /* fill adapters array */
    for (i=0;i<conf.ifc_len/sizeof(struct ifreq);i++)
	{
        xio_adapters[i].ip =  ((struct sockaddr_in*)&conf.ifc_req[i].ifr_addr)->sin_addr.s_addr;
        xio_adapters[i].idx = conf.ifc_req[i].ifr_ifindex;
        strcpy(xio_adapters[i].name, conf.ifc_req[i].ifr_name);
        
        /* get interface MAC address */
        strcpy(ireq.ifr_name,conf.ifc_req[i].ifr_name);
        if (ioctl(devfd,SIOCGIFHWADDR,&ireq) < 0)
		{
            printf("(!) failed to query MAC for interface %s (stat=%d)\n",conf.ifc_req[i].ifr_name,errno);
            fexit(CG_STAT_ADAPTERS_QUERY_FAILED);
        }
		else
		{
            memcpy(xio_adapters[i].mac,ireq.ifr_hwaddr.sa_data,sizeof(mac_address_t));
            devices++;
        }
        
        /* query interface index */
        if (ioctl(devfd,SIOCGIFINDEX,&ireq) < 0)
		{
            printf("(!) failed to query interface %s index (stat=%d)\n",conf.ifc_req[i].ifr_name,errno);
            fexit(CG_STAT_ADAPTERS_QUERY_FAILED);
        }
		else
		{
            xio_adapters[i].idx = ireq.ifr_ifindex;
        }

		/* Get Subnet Mask*/
		if (ioctl(devfd,SIOCGIFNETMASK,&ireq) < 0)
		{
			printf("(!) failed to query Subnet Mask for interface %s (stat=%d)\n",conf.ifc_req[i].ifr_name,errno);
			fexit(CG_STAT_ADAPTERS_QUERY_FAILED);
		}
		else
		{
			xio_adapters[i].SubnetMask = ((struct sockaddr_in *)(&ireq.ifr_netmask))->sin_addr.s_addr;
		}

    }     

exit:
    if (pifreq)
        free(pifreq);
    if (devfd)
        close(devfd);
    *xio_size = devices;
    return status;
}

/********************************************************************************
long nic_get_mac_by_ip()
	Description:
		
	Parameters:
		xi_ip = ip_address_t
		*xo_mac = mac_address_t
	Returns:
		cg_stat_t
********************************************************************************/
cg_stat_t STDCALL NIC_get_mac_by_ip(ip_address_t xi_ip, mac_address_t *xo_mac)
{
    AdapterInfo ifinfo[64];
    cg_stat_t status = CG_STAT_ADAPTERS_QUERY_FAILED;
    int size = 10;
    int i;

    status = NIC_get_adapters(ifinfo, &size);
    if (status != CG_STAT_SUCCESS) {
        return status;
    } else {

        for (i=0; i<size; i++) {
            if (ifinfo[i].ip == xi_ip) {
                memcpy(xo_mac,ifinfo[i].mac,sizeof(mac_address_t));
                fexit(CG_STAT_SUCCESS);
            }
        }
    }
exit:
    return status;
}

cg_stat_t STDCALL NIC_get_adapter(ip_address_t xi_ip, AdapterInfo *xo_adapter)
{
#define DEF_ARR_SIZE 64

    AdapterInfo ifinfo[DEF_ARR_SIZE];
    cg_stat_t status;
    int size = DEF_ARR_SIZE;
    int i;
    
    status = NIC_get_adapters(ifinfo, &size);
    if (status != CG_STAT_SUCCESS) {
        return status;
    } else {
        status = CG_STAT_ADAPTERS_QUERY_FAILED;
        for (i=0; i<size; i++) {
            if (ifinfo[i].ip == xi_ip) {
                memcpy(xo_adapter,&ifinfo[i],sizeof(AdapterInfo));
                fexit(CG_STAT_SUCCESS);
            }
        }
    }
exit:
    return status;
}

