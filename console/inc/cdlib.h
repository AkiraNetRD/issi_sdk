#ifndef _CDL_H
#define _CDL_H
/********************************************************************************
                                INCLUDE
********************************************************************************/

#include <stdlib.h>

#include "common.h"

#ifdef _WIN32
#ifndef _WINSOCKAPI_ /* Prevent inclusion of winsock.h from windows.h */
#define _WINSOCKAPI_ /* since wpcap uses winsock2   */
#endif /* _WINSOCKAPI_ */
#include <windows.h>
#include <winsock2.h>


#define STDCALL     __stdcall
#define _STD_H      /* disable cgtypes basic type definitio */
#else  /*  __LINUX__ */
#define STDCALL
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))


#endif /* __LINUX__ */

// mkdir
#ifdef _WIN32
#include <direct.h>
#elif __linux__
#include <sys/stat.h>
#endif


#ifdef _WIN32
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* _WIN32 */


/********************************************************************************
						OS DEPENDANT MACROS
			NOTE 1: MACRO prefix should be "OS_"
			NOTE 2: Each MACRO must appear in WIN32 and Linux section
********************************************************************************/
#ifdef _WIN32
#define OS_Sleep(t) Sleep(t) 
#define OS_FlushInput() fflush(stdin)
#define OS_ClearStdout() system("cls")
#define OS_MAX_PATH MAX_PATH
#define OS_STRICMP(str1,str2) _stricmp(str1,str2)
#define OS_MKDIR(x) _mkdir(x)
#define OS_CHDIR(path) _chdir(path)
#elif defined __linux__
#include <linux/limits.h>
#define OS_Sleep(t) usleep(t*1000) 
#define OS_FlushInput() _clear_kb()
#define OS_ClearStdout() system("clear")
#define OS_MAX_PATH PATH_MAX
#define OS_STRICMP(str1,str2) strcasecmp(str1,str2)
#define OS_MKDIR(x) mkdir(x,0777)
#define OS_CHDIR(path) chdir(path)
#endif /* __LINUX__ */

/********************************************************************************
                          MACRO DEFINITIONS
********************************************************************************/ 

#define MAX_ETH_SIZE            1514
#define MIN_ETH_SIZE            64
#define WAIT_INFINITE           0xFFFFFFFF

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


/*
 *	Device reset options
 */
#define RESET_OPT_NO_WAIT       0x0080
	
 /********************************************************************************
                           TYPE DEFINITIONS
********************************************************************************/

/*
 *	Basic types
 */



/*
 *	Connection handle
 */
typedef void*       eth_handle_t;

/*
 *	Status code 
 */
typedef unsigned long cg_stat_t;

/*
 *	Network Adapter information entry
 */
typedef struct tagAdapterInfo
{
    mac_address_t   mac;
    ip_address_t    ip;
    ip_address_t    SubnetMask;
    char            name[256];
    int             idx;
    
} AdapterInfo;



/*
 *	Basic Ethernet header packet
 */
typedef struct tagETH_header_t
{
    mac_address_t   da;
    mac_address_t   sa;
    unsigned short  etype;
} ETH_header_t;




/********************************************************************************
                             STATUS CODES
********************************************************************************/
#define     CG_SUCCESS(s)   ((s) <= CG_STAT_MAX_SUCCESS)
/*
 *	Success codes
 */
#define     CG_STAT_SUCCESS                 (0L)    /* general success */
#define     CG_STAT_FAILURE                 (-1)    /* general failure */
#define     CG_STAT_USB_DEVICE              (1L)    /* success query of a CG USB device */
#define     CG_STAT_SECTION_NOT_FOUND       (2L)    /* requested section was not found */
#define     CG_STAT_REACH_MAX_RETRIES       (3L)    /* reached max retries */
#define     CG_STAT_TEST_WAS_INTERRUPT      (4L)    /* test was stopped / interrupt */

#define     CG_STAT_MAX_SUCCESS             (49L)
/*
 *	Error/Failure codes
 */
#define     CG_STAT_GENERAL_ERROR           (50L)
#define     CG_STAT_INSUFFICIENT_MEMORY     (51L)   /* insufficient memory (allocation failed) */
#define     CG_STAT_INVALID_HANDLE          (52L)   /* invalid handle */
#define     CG_STAT_BUFFER_TOO_SMALL        (53L)   /* buffer too small */
#define     CG_STAT_TIMEOUT                 (54L)   /* timeout expired */
#define     CG_STAT_FILE_NOT_FOUND          (55L)   /* file not found */
#define     CG_STAT_READ_FILE_ERROR         (56L)   /* file read op error */
#define     CG_STAT_NOT_SUPPORTED           (57L)   /* operation not supported */
#define     CG_STAT_OVERFLOW                (58L)   /* buffer overflow */
#define     CG_STAT_INVALID_PARAMETER       (59L)   /* invalid parameter */
#define     CG_STAT_NO_DEVICES_FOUND        (60L)   /* no hpna devices found */

/* NICInfo routines related codes */
#define     CG_STAT_ADAPTERS_QUERY_FAILED   (200L)  /* adapter query/processing failed */
#define     CG_STAT_BIND_ADAPTER_FAILED     (201L)  /* failed to bind adapter */

/* IO related errors */
#define     CG_STAT_SEND_FAILED             (250L)  /* failed to post message to device */
#define     CG_STAT_RECV_FAILED             (251L)  /* RX failed  */
#define     CG_STAT_READ_MEM_FAILED         (252L)  /* failed to read device's memory */
#define     CG_STAT_WRITE_MEM_FAILED        (233L)  /* failed to write device's memory */
#define     CG_STAT_BAD_IO                  (234L)  /* bad IO reply from device */


/* FW related errors */
#define     CG_STAT_NVM_PROGRAM_FAIL        (260L)  /* failed to program nvm */
#define     CG_STAT_SECTION_NOT_LOADED      (261L)  /* fw file section load failed */
#define     CG_STAT_ACTIVATION_FAILURE      (262L)  /* device activation failed */
#define     CG_STAT_INVALID_SECTION         (264L)  /* section was invalid/corrupted */





/********************************************************************************
                         INTERFACE DEFINITION
********************************************************************************/


/********************************************************************************
*: Function Name: CM_connect()
*: Abstract:
*		Establish a connection with a device through the specified network 
*       interface (identified by its IP address)
*: Parameters:
* 		xi_ip = network interface IP
* 		*xi_dev_mac = target device's MAC address
*		xi_etype = network Ethernet protocol
*       *xo_handle = returned handle to connection, NULL if failed
*       bSnifferMode = When enable, snif all messages between host and G.hn device
*: Returns:
*		cg_stat_t - 
*
*: Notes:
*	
********************************************************************************/
cg_stat_t STDCALL CM_connect(ip_address_t xi_ip, mac_address_t *xi_dev_mac,UINT16 xi_etype, eth_handle_t *xo_handle, bool bSnifferMode, bool bAllow_Incoming_Broadcast_Packets);


/********************************************************************************
*: Function Name: CM_disconnect()
*: Abstract:
*		Disconnect from a device
*: Parameters:
* 		xi_con = handle to a previously opened connection
*
*: Notes:
*	
********************************************************************************/
void STDCALL CM_disconnect(eth_handle_t xi_con);		

#ifdef _WIN32
/********************************************************************************
*: Function Name: CM_Query_adapter()
*: Abstract:
*		Query Adapter Information (MediaConnectStatus and LinkSpeed)
*: Parameters:
* 		xi_ip = network interface IP
*       *xo_MediaConnectStatus = returned Media Connect Status
*       *xo_LinkSpeed = returned Link-Speed (0/10/100/1000)
*: Returns:
*		cg_stat_t - 
*
*: Notes:
*	
********************************************************************************/
cg_stat_t STDCALL CM_Query_adapter(	ip_address_t xi_ip,
									bool*		xo_MediaConnectStatus,
									int*		xo_LinkSpeed);
#endif

/********************************************************************************
                              Basic Ethernet I/O
********************************************************************************/

#if CONSOLE_DUMP_DEBUG_PACKETS
void DumpPacket(char *Operation,ETH_header_t* xi_pkt, UINT16 xi_len);
#endif

/********************************************************************************
*: Function Name: cg_stat_t	ETH_tx()
*: Abstract:
*		Send raw ethernet packet over a previously opened connection
*: Parameters:
* 		xi_con = connection handle
* 		*xi_pkt = a packet to transmit, the routine will format the DA and SA 
*                 fields in the packet based on the xi_con 
* 		xi_len = packet total length, lengths shorter or longer than the
*                min/max ethernet frame length will be truncated
*: Returns:
*       cg_stat_t
********************************************************************************/
cg_stat_t STDCALL ETH_tx(eth_handle_t xi_con, void* xi_pkt, UINT16 xi_len);

/********************************************************************************
*: Function Name: ETH_rx()
*: Abstract:
*		Attempt to recieve one packet from a connection.
*: Parameters:
* 		xi_con = connection handle
* 		*xi_pkt = receive buffer
* 		len = receive buffer length
* 		xi_timeout [msec] xi_timeout to wait for a packet before returning
*: Returns:
*       cg_stat_t
*: Notes:
*	This function will wait for xi_timeout milliseconds for a packet to arrive
*   from the line blocking the thread. If no packet arrives after xi_timeout msec 
*   it will return with a CG_STAT_TIMEOUT status.
*   NOTE:
*   If xi_pkt is NULL, the function will pull a packet from the queue and return 
*   a CG_STAT_BUFFER_TOO_SMALL status code, and the packet will be lost.
********************************************************************************/
cg_stat_t STDCALL ETH_rx(eth_handle_t xi_con, unsigned char *xi_pkt, UINT16 *xio_len, int xi_timeout); 
cg_stat_t STDCALL ETH_rx2(eth_handle_t xi_con, struct timeval *ts,int *length,int *Protocol);

void STDCALL ETH_flush_rx(eth_handle_t xi_con);

/********************************************************************************
                    Network Adapters Helper Routine
********************************************************************************/

/********************************************************************************
*: Function Name: nic_get_adapters()
*: Abstract:
*		
*: Parameters:
* 		*xio_adapters = AdapterInfo array
* 		*xio_size = (IN) size of array
*                   (OUT) number of items written to array
*: Returns:
*		cg_stat_t
*
*: Notes:
*	The behavior of this routine is OS dependent.
*   Under some OS the retrieved adapters list includes only devices
********************************************************************************/
cg_stat_t STDCALL NIC_get_adapters(AdapterInfo *xio_adapters, int *xio_size);

/********************************************************************************
*: Function Name: long       nic_get_mac_by_ip()
*: Abstract:
*		
*: Parameters:
* 		xi_ip = ip_address_t
* 		*xo_mac = mac_address_t
*: Returns:
*		unsigned
*
*: Notes:
*	
********************************************************************************/
cg_stat_t STDCALL NIC_get_mac_by_ip(ip_address_t xi_ip, mac_address_t *xo_mac);



#ifdef _WIN32
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _WIN32 */

#endif /* _CDL_H */

