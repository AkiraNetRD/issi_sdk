#include "GHN_LIB_int.h"
#include "console.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

eGHN_LIB_STAT ConvertReturnStatus(cg_stat_t Status)
{
	if (Status == CG_STAT_TIMEOUT)					return eGHN_LIB_STAT_TIMEOUT;
	if (Status == CG_STAT_NO_DEVICES_FOUND)			return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
	if (Status == CG_STAT_ADAPTERS_QUERY_FAILED)	return eGHN_LIB_STAT_ADAPTERS_QUERY_FAILED;
	if (Status == CG_STAT_BIND_ADAPTER_FAILED)		return eGHN_LIB_STAT_BIND_ADAPTER_FAILED;
	if (Status == CG_STAT_NOT_SUPPORTED)			return eGHN_LIB_STAT_NOT_SUPPORTED;
	if (Status == CG_STAT_TEST_WAS_INTERRUPT)		return eGHN_LIB_STAT_TEST_WAS_INTERRUPT;
	if (Status == CG_STAT_FILE_NOT_FOUND)			return eGHN_LIB_STAT_MISSING_FILE;

	return eGHN_LIB_STAT_FAILURE;
}

// Function returns 1 if the IP falls in the subnet. Otherwise returns 0.
// Example:
// IP Address   Subnet      Mask          Return Value
// 192.168.1.10 192.168.1.0 255.255.255.0 1
// 192.168.2.10 192.168.1.0 255.255.255.0 0
// ip = DeviceIP
// subnet = NIC-IP
// mask = NIC-MASK
int Is_Ip_In_The_Subnet(ip_address_t ip, ip_address_t subnet, ip_address_t mask)
{
	if((ip & mask) == (subnet & mask)) return 1;
	return 0;
}

bool Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(ip_address_t xi_DeviceIP,ip_address_t xi_NetworkCard)
{
	#define DEF_ARR_SIZE 64

	int size,idx; 
	AdapterInfo inf[DEF_ARR_SIZE];
	cg_stat_t status;

	size = DEF_ARR_SIZE;
	status = NIC_get_adapters(inf,&size);
	if (status != CG_STAT_SUCCESS)
	{
		LOG_INFO("failed to query network interface list (stat=%lu)",status);
		return FALSE;
	}

	for (idx=0;idx < size;idx++)
	{
		if (inf[idx].ip != 0x0100007F)
		{
			if (Is_Ip_In_The_Subnet(xi_DeviceIP,inf[idx].ip,inf[idx].SubnetMask))
			{
				if (inf[idx].ip == xi_NetworkCard)
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
eGHN_LIB_STAT Get_IP_Address_From_Station_List(sGet_stations_Information* getstation, char* strMAC, char* strIP)
{
	UINT32 i;

	if (getstation->Size == 0)
	{
		return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
	}

	for (i=1 ; i <= getstation->Size ; i++)
	{
		if (OS_STRICMP(getstation->sStationArray[i-1].DeviceMAC,strMAC)==0)
		{
			// Entry Found
			if (strcmp(getstation->sStationArray[i-1].DeviceIP,"0.0.0.0")==0)
			{
				// Invalid IP
				return eGHN_LIB_STAT_IP_ADDRESS_INVALID;
			}

			strcpy(strIP,getstation->sStationArray[i-1].DeviceIP);
			return eGHN_LIB_STAT_SUCCESS;
		}
	}

	// Not-Found
	return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
}

void Prepare_Netinf_Output_Folder(char* BaseFolder, char* TestFolder, char* ReceiverDeviceMAC, char* TransmitterDeviceMAC, char* OutputFolder)
{
	mac_address_t	ReceiverMAC;
	mac_address_t	TransmitterMAC;
	char			strNewFolder[OS_MAX_PATH];

	str_to_mac(ReceiverDeviceMAC,&ReceiverMAC);
	str_to_mac(TransmitterDeviceMAC,&TransmitterMAC);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Create the Testing folder
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	{
		OS_MKDIR(BaseFolder);

		if (BaseFolder[strlen(BaseFolder)-1] == '\\')
		{
			BaseFolder[strlen(BaseFolder)-1]= '\0';
		}

		sprintf(strNewFolder,"%s/"MAC_ADDR_NO_COLON,
							BaseFolder,
							MAC_ADDR(ReceiverMAC));

		OS_MKDIR(strNewFolder);

		sprintf(strNewFolder,"%s/"MAC_ADDR_NO_COLON"/"MAC_ADDR_NO_COLON,
							BaseFolder,
							MAC_ADDR(ReceiverMAC),
							MAC_ADDR(TransmitterMAC));

		OS_MKDIR(strNewFolder);

		sprintf(strNewFolder,"%s/"MAC_ADDR_NO_COLON"/"MAC_ADDR_NO_COLON"/%s",
							BaseFolder,
							MAC_ADDR(ReceiverMAC),
							MAC_ADDR(TransmitterMAC),
							TestFolder);
		OS_MKDIR(strNewFolder);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	strcpy(OutputFolder, strNewFolder);
}

void SaveDataModelBufferIntoFile(char* OutputFolder, char* DataModelBuffer)
{
	int		length;
	FILE*	fp;
	char	FileName[OS_MAX_PATH];

#ifdef _WIN32
	SYSTEMTIME sTime;
#elif __linux__
	time_t						time_value;
	struct tm*					now = NULL;
#endif


#ifdef _WIN32
	GetLocalTime(&sTime);
#elif __linux__
	time_value = time(NULL);
	now = localtime(&time_value);
#endif


#ifdef _WIN32
	sprintf(FileName,"%s/DataModel_%04d%02d%02d_%02d%02d%02d.%03d.xml",
		OutputFolder,
		sTime.wYear,
		sTime.wMonth,
		sTime.wDay,
		sTime.wHour,
		sTime.wMinute,
		sTime.wSecond,
		sTime.wMilliseconds);
#elif __linux__
	sprintf(FileName,"%s/DataModel_%04d%02d%02d_%02d%02d%02d.xml",
		OutputFolder,
		now->tm_year+1900,
		now->tm_mon+1,
		now->tm_mday,
		now->tm_hour,
		now->tm_min,
		now->tm_sec);
#endif


	fp = fopen(FileName, "w");

	length = strlen(DataModelBuffer);

	fwrite(DataModelBuffer,1,length,fp);

	fclose(fp);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
