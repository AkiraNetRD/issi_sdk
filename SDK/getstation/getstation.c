
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
	fprintf(stdout,"Getstation Displays the MAC Addresses and IP Addresses of all the devices found in the same domain.\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"%s [-i <ip>] -m <mac>\n",cmd);
	fprintf(stdout,"-i <ip>  - network card ip address\n");
	fprintf(stdout,"-I <ip>  - IP address of the device\n");
	fprintf(stdout,"-m <mac> - Use specific mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Way of use:\n");
	fprintf(stdout,"   1. You can provide the ip address of the network card which the device is attached to (-i) and the device mac (-m)\n");
	fprintf(stdout,"   2. (or) You can provide the ip address of the device itself (-I). In this case parameters (-i) and (-m) are forbidden\n");
}

int main(int argc, char* argv[])
{
	int							c;

	sGet_stations_Information	getstation;

	bool						bHasAdapterIP = FALSE;				// Use specific Network-Adapter
	bool						bHasDeviceMac = FALSE;				// Use specific device MAC address
	bool						bHasDeviceIP = FALSE;				// Use specific device IP address

	char						strAdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char						strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char						strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	while ((c = getopt(argc,argv,"m:i:I:?hV")) >= 0) 
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

	if (CMDHelpers_GetStation(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,TRUE,FALSE,&getstation) == FALSE)
	{
		return 1;
	}

	Print_Station_List_Table(&getstation);

	return 0;
}
