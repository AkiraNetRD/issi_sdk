
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

#ifdef _WIN32
#define OS_STRICMP(str1,str2) _stricmp(str1,str2)
#elif defined __linux__
#define OS_STRICMP(str1,str2) strcasecmp(str1,str2)
#endif /* __LINUX__ */

#define PHY_RATE_TABLE_DEVICE_NAME_LENGTH_LIMIT 11
#define PHY_RATE_TABLE_COLUMN_SIZE_LENGTH_s "%-12s"
#define PHY_RATE_TABLE_COLUMN_SIZE_LENGTH_d "%-12d"

void usage(char *cmd)
{
	fprintf(stdout,"The PHY-Rate table tool monitors the PHY-Rates in all channels.\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"%s [-i <ip> -m <mac>] [-I <ip>] [-g] [-s <num>]\n",cmd);
	fprintf(stdout,"-i <ip>     - network card ip address\n");
	fprintf(stdout,"-I <ip>     - IP address of the device\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-m <mac>    - Use specific Local mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"-g          - Generate internal traffic\n");
	fprintf(stdout,"-t <num>    - Test Duration - Number of Seconds (default = 5)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Way of use:\n");
	fprintf(stdout,"   1. You can provide the ip address of the network card which the device is attached to (-i) and the device mac (-m)\n");
	fprintf(stdout,"   2. (or) You can provide the ip address of the device itself (-I). In this case parameters (-i) and (-m) are forbidden\n");
}

char* Get_Device_Description_According_To_GhnDeviceID(sGet_PHY_Rate_Table_Information* get_PHY_Rate, int Index, bool LimitLength)
{
	int DeviceNameLength;

	DeviceNameLength = strlen(get_PHY_Rate->Array[Index-1].DeviceName);

	if ((OS_STRICMP(get_PHY_Rate->Array[Index-1].DeviceName, "SIGMA") != 0) && (DeviceNameLength > 0))
	{
		if ((LimitLength == TRUE) && (DeviceNameLength > PHY_RATE_TABLE_DEVICE_NAME_LENGTH_LIMIT))
		{
			return &get_PHY_Rate->Array[Index-1].DeviceName[DeviceNameLength-PHY_RATE_TABLE_DEVICE_NAME_LENGTH_LIMIT];
		}
		else
		{
			return &get_PHY_Rate->Array[Index-1].DeviceName[0];
		}
	}
	else
	{
		return &get_PHY_Rate->getstation.sStationArray[Index-1].DeviceMAC[9]; // Return only the last 6 digits
	}

	return "N/A";
}

void Print_The_Phy_Rate_Table(sGet_PHY_Rate_Table_Information* get_PHY_Rate)
{
	int				y,x;
	char			TxDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char			RxDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];

	printf("\n");
	printf("Phy-Rate Table:\n");
	printf("---------------\n");

	// Header
	printf(PHY_RATE_TABLE_COLUMN_SIZE_LENGTH_s, "");
	for (x=1 ; x<=get_PHY_Rate->getstation.Size ; x++)
	{
		strcpy(RxDeviceMAC, Get_Device_Description_According_To_GhnDeviceID(get_PHY_Rate, x, TRUE));
		
		printf(PHY_RATE_TABLE_COLUMN_SIZE_LENGTH_s, RxDeviceMAC);
	}
	printf("\n");

	for (y=1 ; y<=get_PHY_Rate->getstation.Size ; y++)
	{
		strcpy(TxDeviceMAC, Get_Device_Description_According_To_GhnDeviceID(get_PHY_Rate, y, TRUE));
		
		printf(PHY_RATE_TABLE_COLUMN_SIZE_LENGTH_s, TxDeviceMAC);

		for (x=1 ; x<=get_PHY_Rate->getstation.Size ; x++)
		{
			if (y==x)
			{
				printf(PHY_RATE_TABLE_COLUMN_SIZE_LENGTH_s, "X");
				continue;
			}

			if (get_PHY_Rate->RX_PhyRateTable[y-1][x-1] == 0)
			{
				printf(PHY_RATE_TABLE_COLUMN_SIZE_LENGTH_s, "No data");
			}
			else
			{
				printf(PHY_RATE_TABLE_COLUMN_SIZE_LENGTH_d, get_PHY_Rate->RX_PhyRateTable[y-1][x-1]);
			}
		}
		printf("\n");
	}
}

int main(int argc, char* argv[])
{
	int								c;

	eGHN_LIB_STAT					GHN_LIB_STAT;
	
	sGet_stations_Information		getstation;
	sGet_PHY_Rate_Table_Information	get_PHY_Rate;
	sNetinf_Entire_Network_Information netinf_Entire;

	bool						bHasAdapterIP = FALSE;				// Use specific Network-Adapter
	bool						bHasDeviceMac = FALSE;				// Use specific device MAC address
	bool						bHasDeviceIP = FALSE;				// Use specific device IP address

	char						strAdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char						strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char						strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	bool						bGenerateTraffic = FALSE;
	UINT32						TestDuration = 5;

	while ((c = getopt(argc,argv,"m:i:I:gt:?hV")) >= 0) 
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
		case 'g':
			bGenerateTraffic = TRUE;
			break;
		case 't':
			TestDuration = atoi(optarg);
			if ((TestDuration < 1) || (TestDuration > 2147483647))
			{
				fprintf(stderr,"Test-Duration must be a natural number\n");
				return 1;
			}
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

	if (CMDHelpers_GetStation(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,FALSE,FALSE,&getstation) == FALSE)
	{
		return 1;
	}

	if (bGenerateTraffic == TRUE)
	{
		memset(&netinf_Entire,0x00,sizeof(sNetinf_Entire_Network_Information));

		netinf_Entire.getstation = getstation;

		netinf_Entire.TestDuration = TestDuration;

		if ((GHN_LIB_STAT = Ghn_Run_Netinf_Entire_Network(&netinf_Entire)) != eGHN_LIB_STAT_SUCCESS)
		{
			char StatusDescription[1024];

			Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);

			Print_Error_Description("Failed to run the Netinf Entire-Network",
										GHN_LIB_STAT,
										StatusDescription,
										netinf_Entire.ErrorDescription);

			return 1;
		}
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get PHY-Rate Table
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	memset(&get_PHY_Rate,0x00,sizeof(sGet_PHY_Rate_Table_Information));

	get_PHY_Rate.getstation = getstation;

	if ((GHN_LIB_STAT = Ghn_Get_PHY_Rate_Table_Information(&get_PHY_Rate)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];

		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);

		Print_Error_Description("Failed to get the PHY-Rate Table",
									GHN_LIB_STAT,
									StatusDescription,
									get_PHY_Rate.ErrorDescription);

		return 1;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	Print_The_Phy_Rate_Table(&get_PHY_Rate);

	return 0;
}
