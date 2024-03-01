
#include <stdio.h>
#include <stdlib.h>
#include "cdlib.h"
#include "cgi.h"

cg_stat_t STDCALL CM_connect(ip_address_t xi_ip, mac_address_t *xi_dev_mac,UINT16 xi_etype, eth_handle_t *xo_handle, bool bSnifferMode, bool bAllow_Incoming_Broadcast)
{
    cg_stat_t status = CG_STAT_SUCCESS;
    void      *padapter=NULL;
    mac_address_t local_mac;
    connection_t  *pconn;
    
    *xo_handle=NULL;

	/* get nic's MAC address */
    if ((status=NIC_get_mac_by_ip(xi_ip,&local_mac)) != CG_STAT_SUCCESS)
        fexit(status);
    
    /* bind nic by its IP */
	if (bSnifferMode == FALSE)
	{
		if ((status = ETH_bind_adapter(xi_ip,xi_dev_mac,&local_mac,xi_etype,&padapter, bAllow_Incoming_Broadcast)) != CG_STAT_SUCCESS)
	        fexit(status);
	}
	else // (bSnifferMode == TRUE)
	{
		if ((status = ETH_bind_adapter(xi_ip,xi_dev_mac,NULL,xi_etype,&padapter, bAllow_Incoming_Broadcast)) != CG_STAT_SUCCESS)
			fexit(status);
	}

    pconn = (connection_t*)malloc(sizeof(connection_t));
    if (!pconn)
        fexit(CG_STAT_INSUFFICIENT_MEMORY);

    /* fill connection properties */
    memset(pconn,0,sizeof(connection_t));
    
    pconn->type = type_e_ethernet;

    pconn->adapter = padapter;
    pconn->adapter_address = xi_ip;
    memcpy(pconn->host,local_mac,sizeof(mac_address_t));

    *xo_handle = pconn;
    if (xi_dev_mac) {
        memcpy(pconn->device,*xi_dev_mac,sizeof(mac_address_t));
    } else {

        /* if destination is NULL, assume broadcast address */
        memset(pconn->device,0xFF,sizeof(mac_address_t));
    }
    
exit:
    if (status != CG_STAT_SUCCESS) {
        if (padapter)
            ETH_release_adapter(padapter);
    }
    return status;
}


void STDCALL CM_disconnect(eth_handle_t xi_con)
{
    connection_t  *pconn;
    cg_stat_t     status;

    pconn = (connection_t*)xi_con;

	/* release adapter */
    if ((status=ETH_release_adapter(pconn->adapter)) != CG_STAT_SUCCESS) {
        printf("Failed to release device "MAC_ADDR_FMT" %lu\n",MAC_ADDR(pconn->device),status);
    }

	free(pconn);
}

#ifdef _WIN32
cg_stat_t STDCALL CM_Query_adapter(	ip_address_t xi_ip,
									bool*		xo_MediaConnectStatus,
									int*		xo_LinkSpeed)
{
	cg_stat_t status;

	status = ETH_Query_adapter(xi_ip, xo_MediaConnectStatus, xo_LinkSpeed);

	return status;
}
#endif
