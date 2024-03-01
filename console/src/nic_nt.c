
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <iprtrmib.h>
#include <iptypes.h>
#include "cdlib.h"


/********************************************************************************
nic_get_adapter_by_index()
	Description:
		Get network interface details by its index
	Parameters:
		*pAdapter = AdapterInfo
		idx = int
	Returns:
		True if successful
********************************************************************************/
bool STDCALL nic_get_adapter_by_index(AdapterInfo *pAdapter,int idx)
{
    DWORD dwBufLen;
    DWORD dwStatus ;
    IP_ADAPTER_INFO AdapterInfo2[16];
    PIP_ADAPTER_INFO pAdapterInfo;

    /* for up to 16 NICs */
    dwBufLen = sizeof(AdapterInfo2);
    
    dwStatus = GetAdaptersInfo(AdapterInfo2,&dwBufLen);   
    if (dwStatus != ERROR_SUCCESS)
    {
        printf("Failed to get Adapter list %d\n",dwStatus);
		return FALSE;
    }
    
    pAdapterInfo = AdapterInfo2;
    do 
    {
        if((DWORD)idx == pAdapterInfo->Index)
        {
            memmove(pAdapter->mac,pAdapterInfo->Address,6);
            strncpy(pAdapter->name,pAdapterInfo->Description,sizeof(pAdapter->name));
            return TRUE;
        }
        pAdapterInfo = pAdapterInfo->Next;
    }
    while(pAdapterInfo); /* Terminate if last adapter */
    return FALSE;
}

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
    DWORD dwBufLen=0;
    DWORD dwStatus ;
    PMIB_IPADDRTABLE    IpTable;
    PMIB_IPADDRROW      Ip;
    int                 idx;
    int					idx_to_use =0;
    dwStatus = GetIpAddrTable(NULL,&dwBufLen,FALSE);
    if (dwStatus != ERROR_INSUFFICIENT_BUFFER)
    {
        printf("Failed to get Adapter list %d\n",dwStatus);
        return CG_STAT_ADAPTERS_QUERY_FAILED;
    }
    
    dwBufLen += 50;
    IpTable = (PMIB_IPADDRTABLE) calloc(dwBufLen,sizeof(BYTE));
    if (IpTable == NULL)
    {
        printf("Failed to Allocate room for IpTable\n");
        return CG_STAT_INSUFFICIENT_MEMORY;
    }

    dwStatus = GetIpAddrTable(IpTable,&dwBufLen,FALSE);
    if (dwStatus != ERROR_SUCCESS)
    { 
        printf("Failed to get Adapter list %d\n",dwStatus);
        free(IpTable);
        return CG_STAT_ADAPTERS_QUERY_FAILED;
    }
    
    for (idx=0; idx < (int)IpTable->dwNumEntries;idx++)
    {
        if (idx == *xio_size)
        {
            free(IpTable);
            printf("IP table index out of range (%d out of %d) \n",idx+1, *xio_size);
            return CG_STAT_GENERAL_ERROR;
        }
        
        Ip = &IpTable->table[idx];
        memset(&xio_adapters[idx],0,sizeof(AdapterInfo));
        if (Ip->dwAddr !=0)
		{
			xio_adapters[idx_to_use].ip = Ip->dwAddr;
			xio_adapters[idx_to_use].SubnetMask = Ip->dwMask;

			if (Ip->dwAddr != 0x0100007F)
			{
				if (!nic_get_adapter_by_index(&xio_adapters[idx_to_use], Ip->dwIndex))
				{
					//DEBUGOUT(("No Info Was found for %u.%u.%u.%u\n",((unsigned char*)(&Ip->dwAddr))[0],((unsigned char*)(&Ip->dwAddr))[1],
						//((unsigned char*)(&Ip->dwAddr))[2],((unsigned char*)(&Ip->dwAddr))[3]));
					continue;
				}
			}
			idx_to_use++;
		}
    }
    
    *xio_size = idx_to_use;
    free(IpTable);

    return CG_STAT_SUCCESS;
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
    DWORD dwBufLen=0;
    DWORD dwStatus ;
    PMIB_IPADDRTABLE    IpTable;
    PMIB_IPADDRROW      Ip;
    DWORD               idx;
    AdapterInfo         adapter;
   
    dwStatus = GetIpAddrTable(NULL,&dwBufLen,FALSE);
    if (dwStatus != ERROR_INSUFFICIENT_BUFFER)
    {
        printf("Failed to get Adapter list %d\n",dwStatus);
        return CG_STAT_ADAPTERS_QUERY_FAILED;
    }
    
    dwBufLen += 50;
    IpTable = (PMIB_IPADDRTABLE)calloc(dwBufLen,sizeof(BYTE));
    if (IpTable == NULL)
    {
        printf("Failed to Allocate room for IpTable\n");
        return CG_STAT_INSUFFICIENT_MEMORY;
    }
    dwStatus = GetIpAddrTable(IpTable,&dwBufLen,FALSE);
    if (dwStatus != ERROR_SUCCESS)
    {
        printf("Failed to get Adapter list %d\n",dwStatus);
        free(IpTable);
        return CG_STAT_ADAPTERS_QUERY_FAILED;
    }
    
    for (idx=0; idx < IpTable->dwNumEntries;idx++)
    {
        Ip = &IpTable->table[idx];
        if (xi_ip == Ip->dwAddr)
        {
            bool bRes = nic_get_adapter_by_index(&adapter,Ip->dwIndex);
            free(IpTable);
            if (bRes)
            {
                memcpy(xo_mac,adapter.mac,sizeof(mac_address_t));
                return CG_STAT_SUCCESS;
            }
            else
                return CG_STAT_ADAPTERS_QUERY_FAILED;
        }
    }
    
    free(IpTable);
    return CG_STAT_ADAPTERS_QUERY_FAILED;
}
