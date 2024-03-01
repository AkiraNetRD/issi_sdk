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
	fprintf(stdout,"The Utilization tool get/set Utilization information on local/remote device\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"%s [-i <ip>] -m <mac> [-a <value>]\n",cmd);
	fprintf(stdout,"-i <ip>              - network card ip address\n");
	fprintf(stdout,"-I <ip>              - IP address of the device\n");
	fprintf(stdout,"-m <mac>             - Use specific mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-a <value>           - Set the Utilization Alpha Value\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Way of use:\n");
	fprintf(stdout,"   1. You can provide the ip address of the network card which the device is attached to (-i) and the device mac (-m)\n");
	fprintf(stdout,"   2. (or) You can provide the ip address of the device itself (-I). In this case parameters (-i) and (-m) are forbidden\n");
}

int main(int argc, char* argv[])
{
	int							c;

	sUtilization_Field_Information					utilization_Field;
	sUtilization_Field_Alpha_Information			utilization_Alpha_Field;

	sApply_Parameters_Setting_Information			ApplyParametersSetting;

	bool											bHasAdapterIP = FALSE;				// Use specific Network-Adapter
	bool											bHasDeviceMac = FALSE;				// Use specific device MAC address
	bool											bHasDeviceIP = FALSE;				// Use specific device IP address

	char											strAdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char											strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char											strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	bool											bHasUtilizationAlpha = FALSE;
	bool											bNeedApplyParametersSetting = FALSE;


	memset(&utilization_Field, 0x00, sizeof(sUtilization_Field_Information));
	memset(&utilization_Alpha_Field, 0x00, sizeof(sUtilization_Field_Alpha_Information));

	while ((c = getopt(argc,argv,"m:i:I:?hVa:")) >= 0) 
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
			bHasUtilizationAlpha = TRUE;

			utilization_Alpha_Field.Value = atoi(optarg);
			
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

	// Display all the Parameters
	if (bHasUtilizationAlpha == FALSE)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Utilization Field
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Utilization_Field(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&utilization_Field) == FALSE)
		{
			// Failed to get the Utilization Field
			return 1;
		}

		CMDHelpers_Print_Connection_Information(utilization_Field.Connection);

		printf("Utilization Field = %d%%\n", utilization_Field.Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the utilization_Field.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(utilization_Field.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		/*
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Utilization Alpha Field
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Utilization_Alpha_Field(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&utilization_Alpha_Field) == FALSE)
		{
			// Failed to get the Utilization Alpha Field
			return 1;
		}

		printf("Utilization Alpha Field = %d\n", utilization_Alpha_Field.Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-S
		*/
		return 0;
	}

	// Set the Utilization Alpha
	if (bHasUtilizationAlpha)
	{
		if (CMDHelpers_Set_Utilization_Alpha_Field(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&utilization_Alpha_Field) == FALSE)
		{
			return 1;
		}

		printf("Setting Utilization Alpha Field = %d\n", utilization_Alpha_Field.Value);

		if (utilization_Alpha_Field.bNeedApplyParameterSetting == TRUE)
		{
			bNeedApplyParametersSetting = TRUE;
		}
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
