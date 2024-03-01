
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <time.h>

#ifdef _WIN32
#include "windows.h"
#include "optarg.h"
#endif

#include "common.h"

#include "GHN_LIB_ext.h"
#include "CMD_Helpers.h"

void usage(char *cmd)
{
	fprintf(stdout,"The Link Statistics tool monitors several PHY parameters in a specific channel.\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"%s [-i <ip> -m <mac>] [-I <ip>] -T <mac> -R <mac>\n",cmd);
	fprintf(stdout,"-i <ip>     - network card ip address\n");
	fprintf(stdout,"-I <ip>     - IP address of the device\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-m <mac>    - Use specific Local mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"-T <mac>    - Use specific Transmitter mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"-R <mac>    - Use specific Receiver mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Way of use:\n");
	fprintf(stdout,"   1. You can provide the ip address of the network card which the device is attached to (-i) and the device mac (-m)\n");
	fprintf(stdout,"   2. (or) You can provide the ip address of the device itself (-I). In this case parameters (-i) and (-m) are forbidden\n");
}

void Capacity_Format(ULONG64 Value, char* strBuffer)
{
	if (Value > 1073741824) // > 1GB
	{
		sprintf(strBuffer, "%.2f GB", (double)Value / 1073741824);
	}
	else if (Value > 1048576) // > 1MB
	{
		sprintf(strBuffer, "%.2f MB", (double)Value / 1048576);
	}
	else if (Value > 1024) // > 1KB
	{
		sprintf(strBuffer, "%.2f KB", (double)Value / 1024);
	}
	else
	{
		sprintf(strBuffer, "%llu bytes", Value);
	}
}

int main(int argc, char* argv[])
{
	int								c;

	eGHN_LIB_STAT					GHN_LIB_STAT;
	
	sGet_stations_Information		getstation;
	sLink_Statistics_Information*	linkStatistics;

	bool						bHasAdapterIP = FALSE;				// Use specific Network-Adapter
	bool						bHasDeviceMac = FALSE;				// Use specific device MAC address
	bool						bHasTransmitterDeviceMac = FALSE;	// Use specific Transmitter device MAC address
	bool						bHasReceiverDeviceMac = FALSE;		// Use specific Receiver device MAC address
	bool						bHasDeviceIP = FALSE;				// Use specific device IP address

	char						strAdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char						strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char						strTransmitterDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char						strReceiverDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char						strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	char						strBytesReceived[256];

	while ((c = getopt(argc,argv,"m:i:I:T:R:?hV")) >= 0) 
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
		case 'T':
			if (is_valid_str_mac(optarg) == FALSE)
			{
				fprintf(stderr,"Invalid MAC address (%s)\n",optarg);
				return 1;
			}
			bHasTransmitterDeviceMac = TRUE;
			strcpy(strTransmitterDeviceMAC,optarg);
			break;
		case 'R':
			if (is_valid_str_mac(optarg) == FALSE)
			{
				fprintf(stderr,"Invalid MAC address (%s)\n",optarg);
				return 1;
			}
			bHasReceiverDeviceMac= TRUE;
			strcpy(strReceiverDeviceMAC,optarg);
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

	if (((bHasTransmitterDeviceMac == TRUE) && (bHasReceiverDeviceMac == FALSE)) ||
		((bHasTransmitterDeviceMac == FALSE) && (bHasReceiverDeviceMac == TRUE)))
	{
		fprintf(stdout,"You must specific both parameters: -T <mac> and -R <mac>\n");
		return 1;
	}

	// TBD2011
	if ((bHasTransmitterDeviceMac == FALSE) || (bHasReceiverDeviceMac == FALSE))
	{
		fprintf(stdout,"You must specific both parameters: -T <mac> and -R <mac>\n");
		return 1;
	}

	if (CMDHelpers_GetStation(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,FALSE,FALSE,&getstation) == FALSE)
	{
		return 1;
	}

	strcpy(strDeviceIP,getstation.sStationArray[0].DeviceIP);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Run Link Statistics on specific channel
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	linkStatistics = (sLink_Statistics_Information*)malloc(sizeof(sLink_Statistics_Information));

	if (linkStatistics == NULL)
	{
		fprintf(stdout,"Failed to malloc memory\n");
		return 1;
	}

	memset(linkStatistics, 0x00, sizeof(sLink_Statistics_Information));

	linkStatistics->Connection.bHasDeviceIP = TRUE;
	strcpy(linkStatistics->Connection.DeviceIP,strDeviceIP);
	strcpy(linkStatistics->Connection.SelectedNetworkCardIP, getstation.Connection.SelectedNetworkCardIP);

	//linkStatistics->Connection.bHasAdapterIP = TRUE;
	//strcpy(linkStatistics->Connection.AdapterIP, "169.254.10.10");
	//strcpy(linkStatistics->Connection.DeviceMAC, "00:C5:D9:51:12:34");

	strcpy(linkStatistics->TransmitterDeviceMAC,strTransmitterDeviceMAC);
	strcpy(linkStatistics->ReceiverDeviceMAC,strReceiverDeviceMAC);
	
	GHN_LIB_STAT = Ghn_Link_Statistics(linkStatistics);

	if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to run Link_Statistics", GHN_LIB_STAT, StatusDescription, linkStatistics->ErrorDescription);
		goto exit;
	}

	fprintf(stdout,"\n");

	fprintf(stdout,"MAC(%s) --> MAC(%s)  PHY      = %6.2f\n", strTransmitterDeviceMAC, strReceiverDeviceMAC, linkStatistics->PHY);
	fprintf(stdout,"MAC(%s) --> MAC(%s)  SNR      = %6.2f\n", strTransmitterDeviceMAC, strReceiverDeviceMAC, linkStatistics->SNR);
	fprintf(stdout,"MAC(%s) --> MAC(%s)  RXPower0 = %6.2f\n", strTransmitterDeviceMAC, strReceiverDeviceMAC, linkStatistics->RXPower0);
	fprintf(stdout,"MAC(%s) --> MAC(%s)  RXPower1 = %6.2f\n", strTransmitterDeviceMAC, strReceiverDeviceMAC, linkStatistics->RXPower1);

	Capacity_Format(linkStatistics->BytesReceived, strBytesReceived);

	fprintf(stdout,"MAC(%s) --> MAC(%s)  RX Bytes = %llu (%s)\n", strTransmitterDeviceMAC, strReceiverDeviceMAC, linkStatistics->BytesReceived, strBytesReceived);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Print the TX Bytes (BytesReceived on the Transmitter-Device)
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	strcpy(linkStatistics->TransmitterDeviceMAC,strReceiverDeviceMAC);
	strcpy(linkStatistics->ReceiverDeviceMAC,strTransmitterDeviceMAC);

	GHN_LIB_STAT = Ghn_Link_Statistics(linkStatistics);

	if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to run Link_Statistics", GHN_LIB_STAT, StatusDescription, linkStatistics->ErrorDescription);
		goto exit;
	}

	Capacity_Format(linkStatistics->BytesReceived, strBytesReceived);

	fprintf(stdout,"MAC(%s) --> MAC(%s)  TX Bytes = %llu (%s)\n", strTransmitterDeviceMAC, strReceiverDeviceMAC, linkStatistics->BytesReceived, strBytesReceived);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

exit:
	free(linkStatistics);
	return 0;
}
