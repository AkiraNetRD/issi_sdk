#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "common.h"

#ifdef _WIN32
#include "optarg.h"
#endif

#include "GHN_LIB_ext.h"
#include "CMD_Helpers.h"

void usage(char *cmd)
{
	fprintf(stdout,"The Change Device Name tool get/set the Device-Name on local/remote device\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"%s [-i <ip>] -m <mac>\n",cmd);
	fprintf(stdout,"-i <ip>              - network card ip address\n");
	fprintf(stdout,"-I <ip>              - IP address of the device\n");
	fprintf(stdout,"-m <mac>             - Use specific mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-d <Device Name>     - Set the Device-Name (Up to %d characters)\n", (GHN_LIB_MAX_DEVICE_NAME_LEN-1));
	fprintf(stdout,"\n");
	fprintf(stdout,"Way of use:\n");
	fprintf(stdout,"   1. You can provide the ip address of the network card which the device is attached to (-i) and the device mac (-m)\n");
	fprintf(stdout,"   2. (or) You can provide the ip address of the device itself (-I). In this case parameters (-i) and (-m) are forbidden\n");
}

int main(int argc, char* argv[])
{
	int												c;

	sDevice_Name_Information						DeviceName;

	sApply_Parameters_Setting_Information			ApplyParametersSetting;

	bool											bHasAdapterIP = FALSE;				// Use specific Network-Adapter
	bool											bHasDeviceMac = FALSE;				// Use specific device MAC address
	bool											bHasDeviceIP = FALSE;				// Use specific device IP address

	char											strAdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char											strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char											strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	// Device Name
	bool											bHasDeviceName = FALSE;

	bool											bNeedApplyParametersSetting = FALSE;

	memset(&DeviceName, 0x00, sizeof(sDevice_Name_Information));

	while ((c = getopt(argc,argv,"m:i:I:?hVd:")) >= 0) 
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

		// Device Name
		case 'd':
			if (strlen(optarg) > (GHN_LIB_MAX_DEVICE_NAME_LEN-1))
			{
				fprintf(stderr,"Device Name maximum length is %d characters\n", (GHN_LIB_MAX_DEVICE_NAME_LEN-1));
				return 1;
			}

			bHasDeviceName= TRUE;

			strcpy(DeviceName.DeviceName, optarg);
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

	// Display the Device Name
	if (bHasDeviceName == FALSE)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Device Name
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Device_Name(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&DeviceName) == FALSE)
		{
			// Failed to get the DeviceName
			return 1;
		}

		CMDHelpers_Print_Connection_Information(DeviceName.Connection);

		printf("Current Device Name = %s\n", DeviceName.DeviceName);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the DeviceName.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(DeviceName.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		return 0;
	}

	// Set the Device Name
	if (bHasDeviceName)
	{
		if (CMDHelpers_Set_Device_Name(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&DeviceName) == FALSE)
		{
			return 1;
		}

		printf("Setting Device Name = %s\n", DeviceName.DeviceName);

		if (DeviceName.bNeedApplyParameterSetting == TRUE)
		{
			bNeedApplyParametersSetting = TRUE;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the DeviceName.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(DeviceName.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bNeedApplyParametersSetting == TRUE)
	{
		printf("Apply parameter(s) settings to the device...");

		if (CMDHelpers_Apply_Parameters_Setting(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&ApplyParametersSetting) == FALSE)
		{
			return 1;
		}

		Printf_Highlight("\t[OK]\n");
	}
	else
	{
		printf("Device parameter(s) were set without resetting the device\n");
	}

	return 0;
}
