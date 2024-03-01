
#ifndef _CGI_H
#define _CGI_H

#include "cdlib.h"

/********************************************************************************
                                MACROS
********************************************************************************/
#define fexit(s)            {status=s;goto exit;}
#define SIZEOF(t)           ((BYTE*)(&((t*)0)[1])-(BYTE*)(&((t*)0)[0]))

#define MAC_ADDR_FMT        "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_ADDR(p)			(p)[0],(p)[1],(p)[2],(p)[3],(p)[4],(p)[5]

#define MAC_ADDR_NO_COLON	"%02x%02x%02x%02x%02x%02x"

#define NID_ADDR_FMT		"%02x:%02x:%02x:%02x:%02x:%02x:%02x"
#define NID_ADDR(p)			(p)[0],(p)[1],(p)[2],(p)[3],(p)[4],(p)[5],(p)[6]

#define HandleToConn(h)     ((connection_t*)h)

#define DEV_ERASE_TOTAL_TIMEOUT		60000		// Device NVM erase total timeout
#define DEV_ERASE_TIMEOUT			20000		// Device NVM erase timeout

#define DEV_QUERY_TIMEOUT		15000		// Query timeout
#define DEV_QUERY_FAST_TIMEOUT	  100		// Query Fast Timeout
#define DEV_DEFAULT_TIMEOUT		 8000		// Default timeout for request/reply
#define DEV_SHORT_TIMEOUT		 1000		// Sleep-Interval in Pkt_Send_Receive()
#define POLL_INTERVAL			   10		// Sleep-Interval in ETH_rx()


/* Handle type checking */
#define IsEthConnection(h)     ((HandleToConn(h)->type == type_e_ethernet) ? TRUE : FALSE)



/********************************************************************************
                               TYPE DEFINITIONS
********************************************************************************/
typedef enum {
    type_e_ethernet,
    type_e_mdio
} connection_type_t;

/* device I/O callback pointer type */
typedef cg_stat_t (STDCALL * devio_callback_t) (eth_handle_t xi_con, unsigned long xi_addr, unsigned char* xo_buf, UINT16 xi_len);


/*
 *  Connection data structure
 */
typedef struct {


    mac_address_t       device;          /* device MAC address */
    connection_type_t   type;

    /* ethernet specific details */
    mac_address_t       host;            /* host nic MAC address */
    ip_address_t        adapter_address; /* adapter's IPv4 address */
    void                *adapter;        /* adapter connection handle */
    
    /* mdio specific */
    int                 phy_address;     /* mdio phy address */

} connection_t;



/********************************************************************************
                          RAW ETHERNET ACCESS
********************************************************************************/
cg_stat_t STDCALL ETH_bind_adapter(ip_address_t xi_ip, 
                                   mac_address_t *xi_src, 
                                   mac_address_t *xi_dst, 
                                   UINT16 xi_etype, 
                                   void **xo_adapter,
								   bool xi_bAllow_Incoming_Broadcast);
cg_stat_t STDCALL ETH_release_adapter(void *xi_adapter);
cg_stat_t STDCALL ETH_set_filter_raw(void *xi_adapter, char *xi_filter);
cg_stat_t STDCALL ETH_set_filter(void *xi_adapter, 
                                 mac_address_t *xi_src, 
                                 mac_address_t *xi_dst, 
                                 UINT16 xi_etype);

#ifdef _WIN32
cg_stat_t STDCALL ETH_Query_adapter(ip_address_t xi_ip,
									bool*		xo_MediaConnectStatus,
									int*		xo_LinkSpeed);
#endif
/********************************************************************************
                                NIC API
********************************************************************************/
cg_stat_t STDCALL NIC_get_adapter(ip_address_t xi_ip, AdapterInfo *xo_adapter);
#endif /* _CGI_H */
