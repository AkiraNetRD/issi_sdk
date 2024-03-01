
#include "stdio.h"
#include "string.h"

#include "common.h"

#ifdef _WIN32
#include "optarg.h"
#endif

#include "GHN_LIB_ext.h"
#include "CMD_Helpers.h"

void usage(char *cmd)
{
	fprintf(stdout,"The Reset tool performs a soft-reset.\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"%s [-i <ip>] -m <mac> [-E] [-H]\n",cmd);
	fprintf(stdout,"-i <ip>  - network card ip address\n");
	fprintf(stdout,"-I <ip>  - IP address of the device\n");
	fprintf(stdout,"-m <mac> - Use specific mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"-a       - Reset Entire network\n");
	fprintf(stdout,"-E       - Reset to EtherBoot mode\n");
	fprintf(stdout,"-H       - Hardware reset\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Way of use:\n");
	fprintf(stdout,"   1. You can provide the ip address of the network card which the device is attached to (-i) and the device mac (-m)\n");
	fprintf(stdout,"   2. (or) You can provide the ip address of the device itself (-I). In this case parameters (-i) and (-m) are forbidden\n");
}

int main(int argc, char* argv[])
{
	sGet_stations_Information	getstation;

	int							c;
	UINT32						i;

	bool						bHasAdapterIP = FALSE;				// Use specific Network-Adapter
	bool						bHasDeviceMac = FALSE;				// Use specific device MAC address
	bool						bHasDeviceIP = FALSE;				// Use specific device IP address

	char						strAdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char						strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char						strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	eReset_Mode					Resetmode = eReset_Mode_Firmware;
	bool						HardwareReset = FALSE;
	bool						ResetAllDevices = FALSE;

	while ((c = getopt(argc,argv,"m:i:I:aEH?hV")) >= 0) 
	{
		switch(c)
		{
		case '?':
		case 'h':
			usage(argv[0]);
			return 0;
			break;
		case 'V':
			version_info();
			return 0;
			break;
		case 'i':
			if (is_valid_str_ip(optarg) == FALSE)
			{
				fprintf(stderr,"Invalid IP address (%s)\n",optarg);
				return 1;
			}
			bHasAdapterIP = TRUE;
			strcpy(strAdapterIP,optarg);
			break;
		case 'I':
			bHasDeviceIP = TRUE;
			strcpy(strDeviceIP,optarg);
			break;
		case 'm':
			if (is_valid_str_mac(optarg) == FALSE)
			{
				fprintf(stderr,"Invalid MAC address (%s)\n",optarg);
				return 1;
			}
			bHasDeviceMac = TRUE;
			strcpy(strDeviceMAC,optarg);
			break;
		case 'a':
			ResetAllDevices = TRUE;
			break;
		case 'E':
			Resetmode = eReset_Mode_EtherBoot;
			break;
		case 'H':
			HardwareReset = TRUE;
			break;
		default:
			fprintf(stderr,"unknown switch %c\n",c);
			return 1;
		}
	}

	if (bHasAdapterIP && bHasDeviceIP)
	{
		fprintf(stderr,"Specify both parameters (-i) and (-I) are forbidden.\n");
		return 1;
	}

	if (bHasDeviceMac && bHasDeviceIP)
	{
		fprintf(stderr,"Specify both parameters (-m) and (-I) are forbidden.\n");
		return 1;
	}


	if (ResetAllDevices == TRUE)
	{
		// Get the Network card IP according to Device IP
		if ((bHasAdapterIP == FALSE) && (bHasDeviceIP == TRUE))
		{
			ip_address_t		DeviceIP=0;
			ip_address_t		NetworkCardIP=0;

			// The user specify the IP address of the device 
			DeviceIP = str_to_ip(strDeviceIP);

			if (Ghn_Get_Network_Interface_By_Device_IP(DeviceIP,&NetworkCardIP) != eGHN_LIB_STAT_SUCCESS)
			{
			}

			bHasAdapterIP = TRUE;
			ip_to_str(NetworkCardIP,strAdapterIP);
		}

		// GetStation from the local connected device
		if (CMDHelpers_GetStation(FALSE,strDeviceIP,bHasAdapterIP,strAdapterIP,FALSE,strDeviceMAC,FALSE,FALSE,&getstation) == FALSE)
		{
			return 1;
		}

		// The user didn't specify -A or -m or -I
		if ((bHasDeviceIP == FALSE) && (bHasDeviceMac == FALSE))
		{
			// Mark the local device as the device we want prog
			bHasDeviceMac = TRUE;
			strcpy(strDeviceMAC,getstation.sStationArray[0].DeviceMAC);
		}

		// Get the Network card IP of the local device
		if (bHasAdapterIP == FALSE)
		{
			bHasAdapterIP = TRUE;
			strcpy(strAdapterIP,getstation.Connection.SelectedNetworkCardIP);
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Reset all the remote devices
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		for (i=2 ; i <= getstation.Size ; i++)
		{
			char						strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];

			printf("Send reset command to remote device MAC(%s) IP(%s)...",
				getstation.sStationArray[i-1].DeviceMAC,
				getstation.sStationArray[i-1].DeviceIP);

			strcpy(strDeviceMAC,getstation.sStationArray[i-1].DeviceMAC);

			if (CMDHelpers_Reset(	FALSE,
									NULL,
									bHasAdapterIP,
									strAdapterIP,
									TRUE,
									getstation.sStationArray[i-1].DeviceMAC,
									Resetmode,
									HardwareReset) == FALSE)
			{
				Printf_Highlight("[FAILED]\n");
				continue;;
			}

			Printf_Highlight("[OK]\n");
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Reset the local device
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		i = 1;

		printf("Send reset command to local  device MAC(%s) IP(%s)...",
				getstation.sStationArray[i-1].DeviceMAC,
				getstation.sStationArray[i-1].DeviceIP);

			if (CMDHelpers_Reset(	FALSE,
									NULL,
									bHasAdapterIP,
									strAdapterIP,
									TRUE,
									getstation.sStationArray[i-1].DeviceMAC,
									Resetmode,
									HardwareReset) == FALSE)
			{
				Printf_Highlight("[FAILED]\n");
			}

			Printf_Highlight("[OK]\n");
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}
	else
	{
		// Reset the local device
		if (CMDHelpers_Reset(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,Resetmode,HardwareReset) == FALSE)
		{
			return 1;
		}
	}

	return 0;
}

