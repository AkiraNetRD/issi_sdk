
#include "cdlib.h"

#ifdef _WIN32

#define HAVE_REMOTE
#include <pcap.h>
#include <Win32-Extensions.h>
#include <Packet32.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <NtDDNdis.h>
#endif /* WIN32 */

#include "cgi.h"

#include "console_typedefs.h"


/********************************************************************************
                          MACRO DEFINITIONS
********************************************************************************/
#define OPTIMIZATION_LIST_OF_INTERFACES 1			/* set to 0 to disable this optimization */


/********************************************************************************
                            INTERNAL TYPES
********************************************************************************/
typedef struct {
	pcap_if_t *iter;
	pcap_t    *netif;
} net_if_t;


/********************************************************************************
							DUMP PACKETS
 For debugging purpose Only !!!
********************************************************************************/
#if CONSOLE_DUMP_DEBUG_PACKETS

#define DUMP_PACKET_TX
#define DUMP_PACKET_RX

void DumpPacket(char *Operation,ETH_header_t* xi_pkt, UINT16 xi_len)
{
	static int Counter = 1;
	unsigned char* Payload = (unsigned char*)xi_pkt + sizeof(ETH_header_t);
	int i=0,j=0;
	int PayloadSize = xi_len - sizeof(ETH_header_t);

	fprintf(stdout,"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	fprintf(stdout,"Operation %d: %s\n",Counter++,Operation);
	fprintf(stdout,"EthernetII Source: " MAC_ADDR_FMT "\n",MAC_ADDR(xi_pkt->sa));
	fprintf(stdout,"EthernetII Destination: " MAC_ADDR_FMT "\n",MAC_ADDR(xi_pkt->da));
	fprintf(stdout,"EthernetII Type: 0x%04X\n",betohs(xi_pkt->etype));
	fprintf(stdout,"Length: %d\n\n",PayloadSize);

	for(i =0; i< PayloadSize; i++)
	{
		j++;
		fprintf(stdout,"%02x ", Payload[i]);
		if(j==8)
		{
			j = 0;
			fprintf(stdout,"\n");
		}

	}
	fprintf(stdout,"\n");

}

#endif
/********************************************************************************
                          EXTERNAL INTERFACE
********************************************************************************/
cg_stat_t STDCALL ETH_tx(eth_handle_t xi_con, void* xi_pkt, UINT16 xi_len)
{
    connection_t *con;
    net_if_t     *netif;
    int          len;
    /* sanity */
    if ((xi_pkt == NULL) || 
        (xi_con == NULL) || 
        (((connection_t*)xi_con)->adapter == NULL)) {
        return CG_STAT_INVALID_PARAMETER;
    }
    
    con = (connection_t*)xi_con;
    netif = (net_if_t*)con->adapter;
    
    /* more sanity */
    if (!netif->netif)
        return CG_STAT_INVALID_HANDLE;

    /* format ethernet header */
    //memcpy(xi_pkt->da,HandleToConn(xi_con)->device, sizeof(mac_address_t));
    //memcpy(xi_pkt->sa,HandleToConn(xi_con)->host, sizeof(mac_address_t));    

/* For debugging purpose Only !!! */ 
#ifdef DUMP_PACKET_TX
	DumpPacket("TX",(eth_handle_t*)xi_pkt,xi_len);
#endif

	// If the packet is smaller than "MIN_ETH_SIZE", add padding at the end
	if (xi_len < MIN_ETH_SIZE)
	{
		char Buffer[MIN_ETH_SIZE];

		memset(Buffer,0x00,MIN_ETH_SIZE);
		memcpy(Buffer,xi_pkt,xi_len);

		if ((len = pcap_sendpacket(netif->netif, (unsigned char*)Buffer, MIN_ETH_SIZE)) != 0)
		{
			printf("Failed to send packet of len %d\n",MIN_ETH_SIZE);
			return CG_STAT_SEND_FAILED;
		}

		return CG_STAT_SUCCESS;
	}

    len = min(xi_len,MAX_ETH_SIZE);
    if ((len = pcap_sendpacket(netif->netif, (unsigned char*)xi_pkt, len)) != 0)
	{
        printf("Failed to send packet of len %d\n",xi_len);
        return CG_STAT_SEND_FAILED;
    }

    return CG_STAT_SUCCESS;    
}

cg_stat_t STDCALL ETH_rx(eth_handle_t xi_con, unsigned char *xi_pkt, UINT16 *xio_len, int xi_timeout)
{
	struct pcap_pkthdr	header;
	const BYTE*			pkt=NULL;
	UINT16				len = 0;
	int					VLAN_size = 0;
	DWORD				start_time,elapsed_time;
	bool				forever = FALSE;

	/* sanity */
	if ((xi_con == NULL) || 
		(((connection_t*)xi_con)->adapter == NULL)) {
			return CG_STAT_INVALID_PARAMETER;
	}

	if (xi_timeout == WAIT_INFINITE)
	{
		forever = TRUE;
	}

	start_time = GetTickCount(); // Retrieves the number of milliseconds that have elapsed since the system was started

	do 
	{
		if (!((net_if_t*)((connection_t*)xi_con)->adapter)->netif)
			return CG_STAT_INVALID_HANDLE;
		if((pkt = pcap_next(((net_if_t*)((connection_t*)xi_con)->adapter)->netif, &header)) == NULL)
		{
			OS_Sleep(POLL_INTERVAL);

		}
		else
		{
			break;
		}
		
		elapsed_time = GetTickCount() - start_time;
		
	} while(( elapsed_time < (UINT32)xi_timeout) || (forever == TRUE));

	if (!pkt)
		return CG_STAT_TIMEOUT;
	else 
		len = header.caplen;


	/* Check if we got a packet with VLAN Tagging */
	if (len>13)
	{
		if ((pkt[12]==0x81) && (pkt[13]==0x00))
		{
			VLAN_size = 4;
			len-=VLAN_size;
		}
	}

	// Support Windows 8 (Fix a BUG - The reply packet is padding with extra 4 bytes with value 0x00000000)
	if ((xi_pkt) && ((*xio_len+4) == len))
	{
		if ((pkt[*xio_len] == 0x00) &&
			(pkt[*xio_len+1] == 0x00) &&
			(pkt[*xio_len+2] == 0x00) &&
			(pkt[*xio_len+3] == 0x00))
		{
			len = len - 4;
		}
	}

	if ((xi_pkt) && (*xio_len >= len))
	{
		/* handle vlan - in case of vlan jump over vlan 4 bytes */
		if (VLAN_size > 0)
		{ 
			/*vlan */
			memcpy(xi_pkt,pkt,12);
			memcpy(&xi_pkt[12],&pkt[16],len-12);
		}
		else
		{ 
			/* regular */
			memcpy(xi_pkt, pkt, len);
		}
	}
	else
	{
		printf("(!) receive buffer too short\n");
		return CG_STAT_BUFFER_TOO_SMALL;
	}

	/* For debugging purpose Only !!! */ 
#ifdef DUMP_PACKET_RX
	DumpPacket("RX",(eth_handle_t*)xi_pkt,len);
#endif

	*xio_len = len;

	return CG_STAT_SUCCESS;
}

cg_stat_t STDCALL ETH_rx2(eth_handle_t xi_con, struct timeval *ts,int *length,int *Protocol)
{
	struct pcap_pkthdr header;
	const BYTE  *pkt=NULL;

	if((pkt = pcap_next(((net_if_t*)((connection_t*)xi_con)->adapter)->netif, &header)) == NULL)
	{
		return CG_STAT_RECV_FAILED;
	}

	*length = header.caplen;
	*ts = header.ts;

	*Protocol = (pkt[12] << 8) + pkt[13];

	return CG_STAT_SUCCESS;    
}


void STDCALL ETH_flush_rx(eth_handle_t xi_con)
{
	UINT16 len = 0;

	while (ETH_rx(xi_con,NULL,&len,100) == CG_STAT_BUFFER_TOO_SMALL)
	{
		len = 0; /* reset length */
	}    
}


/********************************************************************************
                          INTERNAL INTERFACE
********************************************************************************/
cg_stat_t STDCALL ETH_set_filter_raw(void *xi_adapter, char *xi_filter)
{
    net_if_t *net = (net_if_t *)xi_adapter;
    struct bpf_program prog;
    int         netmask;
	
	if (!net)
        return CG_STAT_INVALID_HANDLE;
    
    /* get netmask */
    if(net->iter->addresses != NULL)
        /* Retrieve the mask of the first address of the interface */
        netmask=((struct sockaddr_in *)(net->iter->addresses->netmask))->sin_addr.S_un.S_addr;
    else
        /* If the interface is without addresses we suppose to be in a C class network */
        netmask=0xffffff;  
	
    if (pcap_compile(net->netif, &prog, xi_filter, 0, netmask) != 0)
        return CG_STAT_GENERAL_ERROR;
    
    /* install filter */
    if(pcap_setfilter(net->netif, &prog) != 0)
        return CG_STAT_GENERAL_ERROR;
    
    pcap_freecode(&prog);

    return CG_STAT_SUCCESS;
}

/* From tcptraceroute, convert a numeric IP address to a string */
#define IPTOSBUFFERS	12
char *iptos(u_long in)
{
	static char output[IPTOSBUFFERS][3*4+3+1];
	static short which;
	u_char *p;
	
	p = (u_char *)&in;
	which = (which + 1 == IPTOSBUFFERS ? 0 : which + 1);
	sprintf(output[which], "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
	return output[which];
}

#ifndef __MINGW32__ /* Cygnus doesn't have IPv6 */
char* ip6tos(struct sockaddr *sockaddr, char *address, int addrlen)
{
	socklen_t sockaddrlen;
	
#ifdef WIN32
	sockaddrlen = sizeof(struct sockaddr_in6);
#else
	sockaddrlen = sizeof(struct sockaddr_storage);
#endif
	
	
	if(getnameinfo(sockaddr, 
		sockaddrlen, 
		address, 
		addrlen, 
		NULL, 
		0, 
		NI_NUMERICHOST) != 0) address = NULL;
	
	return address;
}
#endif /* __MINGW32__ */



cg_stat_t STDCALL ETH_set_filter(void *xi_adapter, mac_address_t *xi_src, mac_address_t *xi_dst, UINT16 xi_etype, bool xi_bAllow_Incoming_Broadcast)
{
	char		Buffer[256] = "";
    char		filter[256] = "";
	macStruct	MultiCast_B1_Mac = { {0x01,0x19,0xA7,0x52,0x76,0x96} }; // Support messages from B1 (MAC-Address 01:19:A7:52:76:96)
	macStruct	Broadcast		 = { {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF} };

    if (xi_dst)
	{
		if (xi_bAllow_Incoming_Broadcast == FALSE)
		{
			sprintf(Buffer,"(ether dst "MAC_ADDR_FMT" or ether dst "MAC_ADDR_FMT")" ,MAC_ADDR(*xi_dst), MAC_ADDR(MultiCast_B1_Mac.macAddress));
		}
		else
		{
			sprintf(Buffer,"(ether dst "MAC_ADDR_FMT" or ether dst "MAC_ADDR_FMT" or ether dst "MAC_ADDR_FMT")" ,MAC_ADDR(*xi_dst), MAC_ADDR(MultiCast_B1_Mac.macAddress), MAC_ADDR(Broadcast.macAddress));
		}
	}

	if (strlen(Buffer) > 0)
	{
		strcat(filter, Buffer);
	}

    if (xi_src)
	{
		sprintf(Buffer,"(ether src " MAC_ADDR_FMT")", MAC_ADDR(*xi_src));

		if (strlen(filter) > 0)
		{
			strcat(filter, " and ");
		}
		strcat(filter, Buffer);
	}

	sprintf(Buffer, "((ether proto 0x%x) or (vlan and ether proto 0x%x))", xi_etype, xi_etype);
	
	if (strlen(filter) > 0)
	{
		strcat(filter, " and ");
	}
	strcat(filter, Buffer);

	return ETH_set_filter_raw(xi_adapter,filter);
}

cg_stat_t STDCALL ETH_bind_adapter(ip_address_t xi_ip,
								   mac_address_t *xi_src, 
								   mac_address_t *xi_dst, 
								   UINT16 xi_etype, 
								   void **xo_adapter,
								   bool xi_bAllow_Incoming_Broadcast)
{  
	char errbuf[PCAP_ERRBUF_SIZE];

	static pcap_if_t	*proxy_pnics = NULL;
	pcap_if_t			*pnics = NULL;
	pcap_if_t			*iter;

	pcap_t      *netif = NULL;
	cg_stat_t   status = CG_STAT_BIND_ADAPTER_FAILED;
	net_if_t    *net=NULL;
	bool found = FALSE;
	UINT32 family;
	UINT32 i=0 ;
	pcap_addr_t *a;

#if OPTIMIZATION_LIST_OF_INTERFACES
	/* For optimization purpose, we call the WinPcap function pcap_findalldevs() only once.  */
	/* We never call the WinPcap function pcap_freealldevs(). it will free when program ends */
	if (proxy_pnics == NULL)
	{
		if (pcap_findalldevs(&proxy_pnics, errbuf) == -1)
			return CG_STAT_ADAPTERS_QUERY_FAILED;
	}

	/* locate nic  */
	iter = proxy_pnics;

#else
	/* Without optimization, we call the WinPcap function pcap_findalldevs() on start */
	/* of this function, and call the WinPcap function pcap_freealldevs() on exit     */
	if (pcap_findalldevs(&pnics, errbuf) == -1)
		return CG_STAT_ADAPTERS_QUERY_FAILED;

	/* locate nic  */
	iter = pnics;

#endif

	/* in order to support Dual stack Ipv4/ Ipv6 we are looping over address */ 

	while (iter) 
	{
		found = FALSE;
		if (iter->addresses)
		{
			for(a=iter->addresses;a;a=a->next) 
			{
				family = (UINT32)(((pcap_if_t *)a->addr->sa_family));
				switch (family ) {
					case  AF_INET:
						/* Search only for IPv4 address */
						if	(((struct sockaddr_in *)a->addr)->sin_addr.S_un.S_addr == xi_ip)
							found = TRUE;
						break;
					case AF_INET6:
						/* TBD - Add support for IPv6 addresses */
						break;
				}
			}
		}
		if  (found) 
		{   
			netif = pcap_open(iter->name, 
				4194304,																							/* buffer size */
				PCAP_OPENFLAG_NOCAPTURE_LOCAL | PCAP_OPENFLAG_PROMISCUOUS | PCAP_OPENFLAG_MAX_RESPONSIVENESS,		/* Special Flags */
				1,																	/* read timeout [msec] */ 
				NULL,																/* remote authentication */
				errbuf);															/* error buffer */
			if (netif == NULL)
				fexit(CG_STAT_ADAPTERS_QUERY_FAILED);

			net = (net_if_t*)malloc(sizeof(net_if_t));
			if (!net)
				fexit(CG_STAT_ADAPTERS_QUERY_FAILED);

			net->iter  = iter;
			net->netif = netif;
			status = CG_STAT_SUCCESS;
			break;
		}
		iter = iter->next;

	}
	/* Set filters on connection */
	if ((status = ETH_set_filter(net,xi_src,xi_dst,xi_etype, xi_bAllow_Incoming_Broadcast)) != CG_STAT_SUCCESS) {
		fexit(CG_STAT_BIND_ADAPTER_FAILED);
	}

exit:
	if ((status != CG_STAT_SUCCESS) && (net))
		free(net);
	else
		*xo_adapter = net;

	if (pnics) /* When OPTIMIZATION_LIST_OF_INTERFACES == 0 */
		pcap_freealldevs(pnics);

	return status;   
}

cg_stat_t STDCALL ETH_release_adapter(void *xi_adapter)
{
	net_if_t *net;

	if (!xi_adapter)
		return CG_STAT_INVALID_HANDLE;

	net = (net_if_t*)xi_adapter;

	if (net->netif)
		pcap_close(net->netif);

	free(net);
	return CG_STAT_SUCCESS;
}

// Return xo_MediaConnectStatus as TRUE if NIC is connected to a device
// Return xo_LinkSpeed in Mbps (10/100/1000) (0 = Unknown)
bool GetAdapterInformation(	char*		xi_AdapterName,
							bool*		xo_MediaConnectStatus,
							int*		xo_LinkSpeed)
{
	LPADAPTER			AdapterObject;
	PPACKET_OID_DATA	OidData;
	int					MediaState= NdisMediaStateDisconnected;
	int					LinkSpeed = 0;

	AdapterObject = PacketOpenAdapter(xi_AdapterName);

	if (AdapterObject == NULL)
	{
		return FALSE;
	}
	
	OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA)+sizeof(ULONG)-1);

	// Get OID_GEN_MEDIA_CONNECT_STATUS
	OidData->Oid = OID_GEN_MEDIA_CONNECT_STATUS;
	OidData->Length = sizeof (ULONG);
	if (PacketRequest(AdapterObject, FALSE, OidData) == FALSE)
	{
		free(OidData);
		PacketCloseAdapter(AdapterObject);
		return FALSE;
	}
	MediaState=*((UINT*)OidData->Data);

	if (MediaState == NdisMediaStateConnected)
	{
		// Get OID_GEN_LINK_SPEED
		OidData->Oid = OID_GEN_LINK_SPEED;
		OidData->Length = sizeof (ULONG);

		if (PacketRequest(AdapterObject, FALSE, OidData) == FALSE)
		{
			free(OidData);
			PacketCloseAdapter(AdapterObject);
			return FALSE;
		}
		// The unit of measurement is 100 bps, so a value of 100,000 represents a hardware bit rate of 10 Mbps.
		LinkSpeed=*((UINT*)OidData->Data)/10000;
	}

	*xo_MediaConnectStatus = (MediaState == NdisMediaStateConnected);
	*xo_LinkSpeed = LinkSpeed;

	free(OidData);
	PacketCloseAdapter(AdapterObject);
	return TRUE;
}

cg_stat_t STDCALL ETH_Query_adapter(ip_address_t xi_ip,
									bool*		xo_MediaConnectStatus,
									int*		xo_LinkSpeed)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t   *pnics,*iter;
    cg_stat_t   status = CG_STAT_BIND_ADAPTER_FAILED;
	bool found = FALSE;
	UINT32 family;
	pcap_addr_t *a;

	if (pcap_findalldevs(&pnics, errbuf) == -1)
	{
       return CG_STAT_ADAPTERS_QUERY_FAILED;
	}

    /* locate nic  */
    iter = pnics;

	/* in order to support Dual stack Ipv4/ Ipv6 we are looping over address */ 
    while (iter) 
	{
		found = FALSE;
		if (iter->addresses)
			{
			for(a=iter->addresses;a;a=a->next) 
				{
				family = (UINT32)(((pcap_if_t *)a->addr->sa_family));
				switch (family ) {
					case  AF_INET:
						/* Search only for IPv4 address */
						if	(((struct sockaddr_in *)a->addr)->sin_addr.S_un.S_addr == xi_ip)
							found = TRUE;
						break;
					case AF_INET6:
						/* TBD - Add support for IPv6 addresses */
						break;
				}
			}
		}
		if  (found) 
		{   
			if (GetAdapterInformation(iter->name, xo_MediaConnectStatus, xo_LinkSpeed) == TRUE)
			{
				status = CG_STAT_SUCCESS;
			}
			else
			{
				status = CG_STAT_FAILURE;
			}
            break;
        }
        iter = iter->next;
	}

	pcap_freealldevs(pnics);

    return status;   
}