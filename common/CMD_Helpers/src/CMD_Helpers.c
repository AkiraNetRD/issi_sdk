
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "common.h"

#ifdef _WIN32
#include "conio.h"
#include "windows.h"
#endif

#ifdef __linux__
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifdef _WIN32
#define OS_Sleep(t) Sleep(t) 
#define OS_MAX_PATH MAX_PATH
#define OS_STRICMP(str1,str2) _stricmp(str1,str2)
#elif __linux__
#include <linux/limits.h>
#define OS_Sleep(t) usleep(t*1000) 
#define OS_MAX_PATH PATH_MAX
#define OS_STRICMP(str1,str2) strcasecmp(str1,str2)
#endif /* __LINUX__ */


#include "GHN_LIB_ext.h"

#include "CMD_Helpers.h"

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool Select_Local_Adapter(sAdapter_Information* adapter, char* strAdapterIP)
{
	if (adapter->Size >= 1) {
		// always use first one
		strcpy(strAdapterIP,adapter->Array[0].IP);
		
		return TRUE;
	}

	return FALSE;

	

	// int			i;
	// int			idx = 0x00;

	// printf("\n");

	// if (adapter->Size >= 2)
	// {
	// 	for (i=1 ; i <= (int)adapter->Size ; i++)
	// 	{
	// 		printf("%d) %-15s\t(%s)\n",
	// 			i-1,
	// 			adapter->Array[i-1].IP,
	// 			adapter->Array[i-1].Description);
	// 	}

	// 	do 
	// 	{
	// 		printf("\nPlease select a network adapter: ");
	// 		idx = read_int();
	// 	}
	// 	while (idx<0 || idx>=(int)adapter->Size);

	// 	fprintf(stdout,"using network adapter %s\n", adapter->Array[idx].IP);
	// }

	// strcpy(strAdapterIP,adapter->Array[idx].IP);

	// return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool Select_Local_Device(sGet_Local_Devices_Information* localdevices, char* strDeviceMAC, char* strDeviceIP, char* strDeviceState)
{
	int			i;
	int			idx = 0x00;

	if (localdevices->Size >= 2)
	{
		for (i=1 ; i <= (int)localdevices->Size ; i++)
		{
			printf("%d) MAC=%s IP=%s STATE=%s\n",
				i-1,
				localdevices->sStationArray[i-1].DeviceMAC,
				localdevices->sStationArray[i-1].DeviceIP,
				localdevices->sStationArray[i-1].DeviceState);
		}

		do 
		{
			printf("\nPlease select local device: ");
			idx = read_int();
		}
		while (idx<0 || idx>=(int)localdevices->Size);

		fprintf(stdout,"using local-device %s with IP=%s\n", 
			localdevices->sStationArray[idx].DeviceMAC,
			localdevices->sStationArray[idx].DeviceIP);
	}

	strcpy(strDeviceMAC,localdevices->sStationArray[idx].DeviceMAC);
	strcpy(strDeviceIP,localdevices->sStationArray[idx].DeviceIP);
	strcpy(strDeviceState,localdevices->sStationArray[idx].DeviceState);

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
void Print_Station_List_Table(sGet_stations_Information* getstation)
{
	UINT32 i;

	printf("\n");

	if (getstation->Size == 0)
	{
		printf("No stations found.\n");
		return;
	}

	printf(" Station | MAC-Address       | IP-Address      | Device-State   \n");
	printf("---------+-------------------+-----------------+----------------\n");

	for (i=1 ; i <= getstation->Size ; i++)
	{
		printf("   %2d    | %s | %-15s | %s\n",
			i,
			getstation->sStationArray[i-1].DeviceMAC,
			getstation->sStationArray[i-1].DeviceIP,
			getstation->sStationArray[i-1].DeviceState);
	}

	printf("\n");
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
void Print_Device_Information(sGet_stations_Information* getstation, bool bAdvanced)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	sDevice_Information			devinf;
	UINT32						i,j;

	printf("\n");

	if (getstation->Size == 0)
	{
		printf("No stations found.\n");
		return;
	}

	for (i=1 ; i <= getstation->Size ; i++)
	{
		if (getstation->sStationArray[i-1].bLocalDevice == TRUE)
		{
			printf("Local device = %s\n",getstation->sStationArray[i-1].DeviceMAC);
		}
	}
	printf("\n");

	for (i=1 ; i <= getstation->Size ; i++)
	{
		memset(&devinf, 0x00, sizeof(sDevice_Information));

		devinf.Connection.bHasDeviceIP = TRUE;
		strcpy(devinf.Connection.DeviceIP,getstation->sStationArray[i-1].DeviceIP);
		strcpy(devinf.Connection.SelectedNetworkCardIP, getstation->Connection.SelectedNetworkCardIP);

		//devinf.Connection.bHasAdapterIP = TRUE;
		//strcpy(devinf.Connection.AdapterIP, "169.254.10.10");
		//strcpy(devinf.Connection.DeviceMAC, getstation->sStationArray[i-1].DeviceMAC);

		devinf.bAdvanced = bAdvanced;

		if ((GHN_LIB_STAT = Ghn_Get_Device_Information(&devinf)) != eGHN_LIB_STAT_SUCCESS)
		{
			printf("Failed to get device information from MAC %s\n", getstation->sStationArray[i-1].DeviceMAC);
			continue;
		}

		printf("%d)",i);

		if (strcmp(getstation->sStationArray[i-1].DeviceIP, "0.0.0.0") == 0)
		{
			printf("\t%-30s = %s\n", "GhnMACAddress", getstation->sStationArray[i-1].DeviceMAC);
		}

		for (j=1; j <=devinf.Size; j++)
		{
			printf("\t%-30s = %s\n",devinf.AttributeArray[j-1].Name,devinf.AttributeArray[j-1].Value);
		}
	}
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
void Print_Local_Hosts_MAC_Addresses_Table(sLocal_Hosts_MAC_Addresses_Information* LocalHosts)
{
	UINT32 i;

	printf("\n");

	if (LocalHosts->Size == 0)
	{
		printf("No stations found.\n");
		return;
	}

	printf(" Station | MAC-Address       \n");
	printf("---------+-------------------\n");

	for (i=1 ; i <= LocalHosts->Size ; i++)
	{
		printf("   %2d    | %s\n",
			i,
			LocalHosts->sMACAddressArray[i-1].DeviceMAC);
	}

	printf("\n");
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
void Print_Image_Header_Information(sImage_Header_Information* ImageHeaderInformation)
{
	printf("ModelName                = %s\n", ImageHeaderInformation->ModelName);
	printf("FW_Version               = %s\n", ImageHeaderInformation->FW_Version);
	printf("ConfigurationName        = %s\n", ImageHeaderInformation->ConfigurationName);
	printf("BuildImageCreationTime   = %s\n", ImageHeaderInformation->BuildImageCreationTime);
	printf("TotalImageSize           = %d bytes\n", ImageHeaderInformation->TotalImageSize);
	printf("FW_Signature             = %s\n", ImageHeaderInformation->FW_Signature);
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
void Print_Error_Description(char* strError, int iStatus, char* strStatus, char* strErrorDescription)
{
	printf("\n");

	printf("********************************************************************************\n");
	printf("** Error       = %s\n", strError);
	printf("** Status      = %s (%d)\n", strStatus, iStatus);

	if (strlen(strErrorDescription) > 0)
	{
		printf("** Description = %s\n", strErrorDescription);
	}
	printf("********************************************************************************\n");

	printf("\n");
}

/***************************************************************************************************
* Prepare_Connection()                                                                             *
*                                                                                                  *
* Create the structure "sConnection_Information"                                                   *
*                                                                                                  *
* + Query and select the network card IP                                                           *
* + Query and select the local device MAC                                                          *
***************************************************************************************************/
bool Prepare_Connection(bool						xi_bHasDeviceIP,
						char*						xi_strDeviceIP,
						bool						xi_bHasAdapterIP,
						char*						xi_strAdapterIP,
						bool						xi_bHasDeviceMac,
						char*						xi_strDeviceMAC,
						bool						xi_AllowBootCode,
						sConnection_Information*	xo_Connection)
{
	eGHN_LIB_STAT					GHN_LIB_STAT;
	sAdapter_Information			adapter;
	sGet_Local_Devices_Information	localdevices;
	sGet_Device_State_Information	deviceState;

	char						strAdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char						strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char						strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	char						strDeviceState[GHN_LIB_DEVICE_STATE];

	memset(xo_Connection,0x00,sizeof(sConnection_Information));

	if (xi_bHasDeviceIP == TRUE)
	{
		xo_Connection->bHasDeviceIP = TRUE;

		strcpy(xo_Connection->DeviceIP,xi_strDeviceIP);
	}
	else
	{
		xo_Connection->bHasAdapterIP = TRUE;

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Query and select the network card IP
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (xi_bHasAdapterIP == TRUE)
		{
			strcpy(xo_Connection->AdapterIP,xi_strAdapterIP);
		}
		else
		{
			GHN_LIB_STAT = Ghn_Get_Adapter_List(&adapter);

			if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
			{
				fprintf(stderr,"failed to get network interface\n");
				return FALSE;
			}

			Select_Local_Adapter(&adapter,strAdapterIP);
			strcpy(xo_Connection->AdapterIP,strAdapterIP);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Query and select the local device MAC
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (xi_bHasDeviceMac == TRUE)
		{
			// Get the local device state
			strcpy(xo_Connection->DeviceMAC,xi_strDeviceMAC);

			memset(&deviceState, 0x00,sizeof(sGet_Device_State_Information));

			strcpy(deviceState.AdapterIP,xo_Connection->AdapterIP);
			strcpy(deviceState.DeviceMAC,xo_Connection->DeviceMAC);

			GHN_LIB_STAT = Ghn_Query_Device_State(&deviceState);

			if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
			{
				char StatusDescription[1024];
				Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
				Print_Error_Description("Failed to get the device state", GHN_LIB_STAT, StatusDescription, deviceState.ErrorDescription);
				return FALSE;
			}

			strcpy(strDeviceState, deviceState.DeviceState);
		}
		else
		{
			memset(&localdevices,0x00,sizeof(sGet_Local_Devices_Information));

			strcpy(localdevices.AdapterIP,xo_Connection->AdapterIP);

			GHN_LIB_STAT = Ghn_Query_Local_Devices(&localdevices);

			if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
			{
				char StatusDescription[1024];
				Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
				Print_Error_Description("Failed to get the local devices list", GHN_LIB_STAT, StatusDescription, localdevices.ErrorDescription);
				return FALSE;
			}

			Select_Local_Device(&localdevices, strDeviceMAC, strDeviceIP, strDeviceState);
			strcpy(xo_Connection->DeviceMAC,strDeviceMAC);
		}

		if (strcmp(strDeviceState,"BootCode")==0)
		{
			xo_Connection->bIsBootCode = TRUE;

			if (xi_AllowBootCode == FALSE)
			{
				char StatusDescription[1024];
				GHN_LIB_STAT = eGHN_LIB_STAT_FW_BOOT_CODE;
				Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
				Print_Error_Description("Failed to connect to local device", GHN_LIB_STAT, StatusDescription, "");
				return FALSE;
			}
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
void CMDHelpers_Print_Connection_Information(sConnection_Information Connection)
{
	if (Connection.bHasAdapterIP)
	{
		printf("\n");
		printf("using device MAC-Address(%s)\n", Connection.DeviceMAC);
	}
	else if (Connection.bHasDeviceIP)
	{
		printf("\n");
		printf("using device IP-Address(%s)\n", Connection.DeviceIP);
	}
}


/***************************************************************************************************
* CMDHelpers_Update_Connection_Parameters()                                                        *
*                                                                                                  *
* If the user didn't specify some of the connection parameters, update the information from the    *
* xi_Connection structure                                                                          *
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Update_Connection_Parameters(	sConnection_Information		xi_Connection,
												bool*						xo_bHasDeviceIP,
												char*						xo_strDeviceIP,
												bool*						xo_bHasAdapterIP,
												char*						xo_strAdapterIP,
												bool*						xo_bHasDeviceMac,
												char*						xo_strDeviceMAC)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;
	char					StatusDescription[1024];

	if (xi_Connection.bHasAdapterIP == TRUE)
	{
		*xo_bHasDeviceIP = FALSE;
		strcpy(xo_strDeviceIP, "");
		*xo_bHasAdapterIP = TRUE;
		strcpy(xo_strAdapterIP, xi_Connection.AdapterIP);
		*xo_bHasDeviceMac = TRUE;
		strcpy(xo_strDeviceMAC, xi_Connection.DeviceMAC);

		return TRUE;
	}
	else if (xi_Connection.bHasDeviceIP == TRUE)
	{
		*xo_bHasDeviceIP = TRUE;
		strcpy(xo_strDeviceIP, xi_Connection.DeviceIP);
		*xo_bHasAdapterIP = FALSE;
		strcpy(xo_strAdapterIP, "");
		*xo_bHasDeviceMac = FALSE;
		strcpy(xo_strDeviceMAC, "");

		return TRUE;
	}

	GHN_LIB_STAT = eGHN_LIB_STAT_INVALID_PARAMETER;
	Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
	Print_Error_Description("Failed to Update Connection Parameters", GHN_LIB_STAT, StatusDescription, "");

	return FALSE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_GetStation(	bool						xi_bHasDeviceIP,
							char*						xi_strDeviceIP,
							bool						xi_bHasAdapterIP,
							char*						xi_strAdapterIP,
							bool						xi_bHasDeviceMac,
							char*						xi_strDeviceMAC,
							bool						xi_AllowBootCode,
							bool						xi_bQueryOnlyLocalDevice,
							sGet_stations_Information*	xo_getstation)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;

	memset(xo_getstation, 0x00, sizeof(sGet_stations_Information));

	if (Prepare_Connection(	xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							xi_AllowBootCode,
							&xo_getstation->Connection) == FALSE)
	{
		return FALSE;
	}

	if (xo_getstation->Connection.bIsBootCode)
	{
		xo_getstation->Size = 0x01;
		strcpy(xo_getstation->sStationArray[0].DeviceMAC, xo_getstation->Connection.DeviceMAC);
		strcpy(xo_getstation->sStationArray[0].DeviceIP, "0.0.0.0");
		strcpy(xo_getstation->sStationArray[0].DeviceState, "BootCode");
		xo_getstation->sStationArray[0].bLocalDevice = TRUE;

		return TRUE;
	}

	xo_getstation->bQueryOnlyLocalDevice = xi_bQueryOnlyLocalDevice;

	if ((GHN_LIB_STAT = Ghn_Get_Stations(xo_getstation)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to get the stations list", GHN_LIB_STAT, StatusDescription, xo_getstation->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Reset(	bool						xi_bHasDeviceIP,
						char*						xi_strDeviceIP,
						bool						xi_bHasAdapterIP,
						char*						xi_strAdapterIP,
						bool						xi_bHasDeviceMac,
						char*						xi_strDeviceMAC,
						eReset_Mode					xi_Resetmode,
						bool						xi_HardwareReset)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	
	sReset_Information			reset;


	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							TRUE, /* xi_AllowBootCode */
							&reset.Connection) == FALSE)
	{
		return FALSE;
	}

	reset.ResetMode = xi_Resetmode;

	if ((reset.Connection.bIsBootCode) || (xi_HardwareReset == TRUE))
	{
		reset.HardwareReset = TRUE;
	}
	else
	{
		reset.HardwareReset = FALSE;

	}

	if ((GHN_LIB_STAT = Ghn_Reset(&reset)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to reset", GHN_LIB_STAT, StatusDescription, reset.ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Restore_Factory_Default(bool									xi_bHasDeviceIP,
										char*									xi_strDeviceIP,
										bool									xi_bHasAdapterIP,
										char*									xi_strAdapterIP,
										bool									xi_bHasDeviceMac,
										char*									xi_strDeviceMAC,
										sRestore_Factory_Default_Information*	xio_sRestoreFactoryDefault)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	
	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_sRestoreFactoryDefault->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Restore_Factory_Default(xio_sRestoreFactoryDefault)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to Restore Factory Default", GHN_LIB_STAT, StatusDescription, xio_sRestoreFactoryDefault->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Pair_Device(	bool							xi_bHasDeviceIP,
								char*							xi_strDeviceIP,
								bool							xi_bHasAdapterIP,
								char*							xi_strAdapterIP,
								bool							xi_bHasDeviceMac,
								char*							xi_strDeviceMAC,
								sPair_Device_Information*		xio_sPairDevice)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_sPairDevice->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Pair_Device(xio_sPairDevice)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to Pair Device", GHN_LIB_STAT, StatusDescription, xio_sPairDevice->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Unpair_Device(	bool							xi_bHasDeviceIP,
								char*							xi_strDeviceIP,
								bool							xi_bHasAdapterIP,
								char*							xi_strAdapterIP,
								bool							xi_bHasDeviceMac,
								char*							xi_strDeviceMAC,
								sUnpair_Device_Information*		xio_sUnpairDevice)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	
	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_sUnpairDevice->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Unpair_Device(xio_sUnpairDevice)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to Unpair Device", GHN_LIB_STAT, StatusDescription, xio_sUnpairDevice->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}


/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Apply_Parameters_Setting(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sApply_Parameters_Setting_Information*	xio_sApplyParametersSetting)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	
	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_sApplyParametersSetting->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Apply_Parameters_Setting(xio_sApplyParametersSetting)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to apply parameters setting", GHN_LIB_STAT, StatusDescription, xio_sApplyParametersSetting->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Local_Hosts_MAC_Addresses(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sLocal_Hosts_MAC_Addresses_Information*	LocalHosts)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	
	memset(LocalHosts, 0x00, sizeof(sLocal_Hosts_MAC_Addresses_Information));

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&LocalHosts->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Local_Hosts_MAC_Addresses(LocalHosts)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to get the Local Hosts MAC-Addresses", GHN_LIB_STAT, StatusDescription, LocalHosts->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

