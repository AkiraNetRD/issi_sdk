
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <features.h>    /* for the glibc version number */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#endif
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include <cdlib.h>
#include "cgi.h"
#include <stdio.h>

#include "console_typedefs.h"

/********************************************************************************
                          MACRO DEFINITIONS
********************************************************************************/

#define MSEC_IN_SEC    1000
#define NSEC_IN_MSEC   1000000
#define NSEC_IN_SEC    (MSEC_IN_SEC * NSEC_IN_MSEC)

#define USEC_IN_SEC 1000000
#define USEC_IN_MSEC 1000

/********************************************************************************
                            INTERNAL TYPES
********************************************************************************/
typedef struct {
    int                 devfd;  
    struct sockaddr_ll  sock;
                 
} eth_desc_t;


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
	
	static struct timeval start = {0,0};
	struct timeval end;
	long mtime, seconds, useconds;     


	fprintf(stdout,"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	
	gettimeofday(&end, NULL); 
	seconds  = end.tv_sec  - start.tv_sec; 
	useconds = end.tv_usec - start.tv_usec; 
	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5; 
	start = end;
	printf("Elapsed time: %ld milliseconds\n", mtime); 
	
	fprintf(stdout,"Operation %d: %s\n",Counter++,Operation);
	fprintf(stdout,"EthernetII Source: " MAC_ADDR_FMT "\n",MAC_ADDR(xi_pkt->sa));
	fprintf(stdout,"EthernetII Destination: " MAC_ADDR_FMT "\n",MAC_ADDR(xi_pkt->da));
	fprintf(stdout,"EthernetII Type: 0x%04X\n",betohs(xi_pkt->etype));
	fprintf(stdout,"Length: %d\n\n",xi_len);

	for(i =0; i< xi_len; i++)
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
    short          len;
    eth_desc_t   *netif;
    
    netif = (eth_desc_t*)((connection_t*)xi_con)->adapter;

	/* For debugging purpose Only !!! */ 
#ifdef DUMP_PACKET_TX
	DumpPacket("TX",xi_pkt,xi_len-14);
#endif

	// If the packet is smaller than "MIN_ETH_SIZE", add padding at the end
	if (xi_len < MIN_ETH_SIZE)
	{
		char Buffer[MIN_ETH_SIZE];

		memset(Buffer,0x00,MIN_ETH_SIZE);
		memcpy(Buffer,xi_pkt,xi_len);

		len = sendto(netif->devfd, (unsigned char*)Buffer, MIN_ETH_SIZE, MSG_DONTWAIT | MSG_NOSIGNAL , NULL, 0);
		if (len < 0)
		{
			printf("(!) error in sendto, errno=%d\n",errno);
			return CG_STAT_SEND_FAILED;
		}
		return CG_STAT_SUCCESS;
	}
    
    /* Truncate if packet is too long */
    len = min(xi_len,MAX_ETH_SIZE);
    
    len = sendto(netif->devfd, xi_pkt, len, MSG_DONTWAIT | MSG_NOSIGNAL , NULL, 0);
    if (len < 0)
	{
        printf("(!) error in sendto, errno=%d\n",errno);
        return CG_STAT_SEND_FAILED;
	}
    return CG_STAT_SUCCESS;
}

cg_stat_t STDCALL ETH_rx(eth_handle_t xi_con, unsigned char *xi_pkt, UINT16 *xio_len, int xi_timeout)
{

    eth_desc_t *desc;
    struct sockaddr_ll rll;
    socklen_t l = sizeof(rll);
    ETH_header_t *v = (ETH_header_t*)xi_pkt;

	struct timeval tv;
    
	static mac_address_t baddr = {0xff,0xff,0xff,0xff,0xff,0xff};
	macStruct MultiCast_B1_Mac = { {0x01,0x19,0xA7,0x52,0x76,0x96} }; // Support messages from B1 (MAC-Address 01:19:A7:52:76:96)

    int r = -1,ret;	
	fd_set rdfds;
	if (xi_timeout >=0) {
	    tv.tv_sec = xi_timeout / MSEC_IN_SEC ;
		tv.tv_usec = (xi_timeout * USEC_IN_MSEC ) % USEC_IN_SEC ;
	}
    desc  = (eth_desc_t*)(HandleToConn(xi_con)->adapter);
	FD_ZERO(&rdfds);
    do 
		{
	    FD_SET(desc->devfd, &rdfds);
		if (xi_timeout >=0)
			ret = select(desc->devfd + 1, &rdfds, NULL, NULL, &tv);
		else /* blocking */
			ret = select(desc->devfd + 1, &rdfds, NULL, NULL, NULL);

		if (ret < 0)
		{
			if (errno == 4)
			{
				// "interrupted system call" - Ignore that error message, may occure when nic is disconnect and connect
				// when the device is resetting
			}
			else
			{
				printf("(!) error in ETH_rx:Select failed (ret=%d,errno=%d)\n",ret,errno);
			}
			return CG_STAT_GENERAL_ERROR;
		}
		else if (ret == 0)
			return CG_STAT_TIMEOUT;
        else
		{
			r = recvfrom(desc->devfd, xi_pkt, *xio_len, MSG_DONTWAIT ,NULL, NULL);
		}
	
        if (r >= ETH_ALEN) {

			/* 
			 * Verify that the packet's MAC destination is for us (MAC of NIC)
			 */
			if (memcmp(v->da, HandleToConn(xi_con)->host, ETH_ALEN) != 0)
			{
				// Support messages from B1 (MAC-Address 01:19:A7:52:76:96)
				if (memcmp(v->da, MultiCast_B1_Mac.macAddress, ETH_ALEN) != 0)
				{
					// And not a broadcast packet
					if (memcmp(v->da, baddr, ETH_ALEN) != 0)
					{
						/* ignore packet */
						continue;
					}
				}
			}
            
            /*
             *	Filter out packets from other 
             *  devices on a unicast connection
             */
            if (memcmp(HandleToConn(xi_con)->device,baddr,sizeof(mac_address_t) != 0))
			{
                if (memcmp(v->sa, HandleToConn(xi_con)->device, ETH_ALEN) == 0)
				{
					// Received-Packet is OK
                    break;
                }
            }
			else // Broadcast message
			{
                /*
                 *	on broadcast connection we accept 
                 *  any packet with our ether type
                 */
				// Received-Packet is OK
                break;
            }

            /* ignore packet */
            continue;
        }

        printf("(!) error in recvfrom, errno=%d\n",errno);
        return CG_STAT_RECV_FAILED;
    } while (1);
	

    if (r < 0)
        return CG_STAT_TIMEOUT;

    if (l != sizeof(rll)) {
        printf("(!) warning: address is not of type LL, size=%d\n",l);
    }

	/* For debugging purpose Only !!! */ 
#ifdef DUMP_PACKET_RX
	DumpPacket("RX",(ETH_header_t*)xi_pkt,r-14);
#endif
    
	*xio_len = r;

    return CG_STAT_SUCCESS;    
}

/********************************************************************************
                          INTERNAL INTERFACE
********************************************************************************/
cg_stat_t STDCALL ETH_bind_adapter(ip_address_t xi_ip, mac_address_t *xi_src, 
                                   mac_address_t *xi_dst, UINT16 xi_etype, 
                                   void **xo_adapter, bool xi_bAllow_Incoming_Broadcast)
{  
    eth_desc_t *desc;
    AdapterInfo adapter;
    cg_stat_t status;

    desc = (eth_desc_t*)malloc(sizeof(eth_desc_t));
    if (desc == NULL)
        return CG_STAT_INSUFFICIENT_MEMORY;
    
    memset(desc,0,sizeof(eth_desc_t));
    
    /* open raw packet socket */
    desc->devfd = socket(PF_PACKET, SOCK_RAW, htons(xi_etype));
    if (desc->devfd < 0) {
        printf("(!) failed to open raw socket %d\n",desc->devfd);
        return CG_STAT_BIND_ADAPTER_FAILED;
    }
    
    desc->sock.sll_family   = AF_PACKET;
    desc->sock.sll_protocol = htons(xi_etype);
    desc->sock.sll_halen    = ETH_ALEN;

    /* get interface info */
    if ((status=NIC_get_adapter(xi_ip,&adapter)) != CG_STAT_SUCCESS) {
        printf("(!) failed to query nic device %lu\n",status);
        return status;
    }
    
    desc->sock.sll_ifindex = adapter.idx;
    
    /* bind to a specific interface */
    if (bind(desc->devfd, (struct sockaddr *) &desc->sock, sizeof(desc->sock)) < 0) {
        printf("(!) failed to bind!!!\n");
        return CG_STAT_BIND_ADAPTER_FAILED;
    } 
    
    desc->sock.sll_protocol = 0;
    *xo_adapter = desc;

    return CG_STAT_SUCCESS;
}

cg_stat_t STDCALL ETH_release_adapter(void *xi_adapter)
{
    eth_desc_t *desc = (eth_desc_t*)xi_adapter;
    close(desc->devfd);
    free(desc);
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
