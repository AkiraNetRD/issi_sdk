
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
	fprintf(stdout,"The Pairing tool enable/disable the pairing feature on local/remote device\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"%s [-i <ip>] -m <mac> [-a or -b]\n",cmd);
	fprintf(stdout,"-i <ip>              - network card ip address\n");
	fprintf(stdout,"-I <ip>              - IP address of the device\n");
	fprintf(stdout,"-m <mac>             - Use specific mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-a                   - Pair Device\n");
	fprintf(stdout,"-b                   - Unpair Device\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Way of use:\n");
	fprintf(stdout,"   1. You can provide the ip address of the network card which the device is attached to (-i) and the device mac (-m)\n");
	fprintf(stdout,"   2. (or) You can provide the ip address of the device itself (-I). In this case parameters (-i) and (-m) are forbidden\n");
}

int main(int argc, char* argv[])
{
	int							c;

	sPair_Device_Information	PairDevice;
	sUnpair_Device_Information	UnpairDevice;

	bool						bHasAdapterIP = FALSE;				// Use specific Network-Adapter
	bool						bHasDeviceMac = FALSE;				// Use specific device MAC address
	bool						bHasDeviceIP = FALSE;				// Use specific device IP address

	char						strAdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char						strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char						strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	bool						bHasPairDevice = FALSE;
	bool						bHasUnpairDevice = FALSE;

	memset(&PairDevice, 0x00, sizeof(sPair_Device_Information));
	memset(&UnpairDevice, 0x00, sizeof(sUnpair_Device_Information));

	while ((c = getopt(argc,argv,"m:i:I:?hVab")) >= 0) 
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
			bHasPairDevice = TRUE;
			break;

		case 'b':
			bHasUnpairDevice = TRUE;
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

	if (bHasPairDevice + bHasUnpairDevice == 0)
	{
		usage(argv[0]);
		return 1;
	}
	
	if (bHasPairDevice + bHasUnpairDevice > 1)
	{
		fprintf(stderr,"Please specify only one command at a time (only -a or only -b)\n");
		return 1;
	}

	if (bHasPairDevice == TRUE)
	{
		// Pair Device
		if (CMDHelpers_Pair_Device(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&PairDevice) == FALSE)
		{
			return 1;
		}

		CMDHelpers_Print_Connection_Information(PairDevice.Connection);

		printf("Pair Device\t\t\t");
		Printf_Highlight("[OK]\n");

		return 0;
	}

	if (bHasUnpairDevice == TRUE)
	{
		// Unpair Device
		if (CMDHelpers_Unpair_Device(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&UnpairDevice) == FALSE)
		{
			return 1;
		}

		CMDHelpers_Print_Connection_Information(UnpairDevice.Connection);

		printf("Unpair Device\t\t\t");
		Printf_Highlight("[OK]\n");


		return 0;
	}

	return 0;
}
