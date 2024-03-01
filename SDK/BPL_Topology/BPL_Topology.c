
#include "stdio.h"
#include "string.h"

#include "common.h"

#ifdef _WIN32
#include "optarg.h"
#endif

#include "GHN_LIB_ext.h"
#include "CMD_Helpers.h"

#ifdef _WIN32
#define OS_STRICMP(str1,str2) _stricmp(str1,str2)
#elif defined __linux__
#define OS_STRICMP(str1,str2) strcasecmp(str1,str2)
#endif /* __LINUX__ */

#define BPL_TOPOLOGY_DEVICE_NAME_LENGTH_LIMIT 11
#define BPL_TOPOLOGY_COLUMN_SIZE_LENGTH "%-12s"

void usage(char *cmd)
{
	fprintf(stdout,"The BPL Topology tool show the connectivity path in a partial-mesh network.\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"%s [-i <ip>] -m <mac>\n",cmd);
	fprintf(stdout,"-i <ip>   - network card ip address\n");
	fprintf(stdout,"-I <ip>   - IP address of the device\n");
	fprintf(stdout,"-m <mac>  - Use specific mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-f <file> - Use XML file (read with the GetDataModel tool on the DM device)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-B        - Print Broadcast Information");
	fprintf(stdout,"\n");
	fprintf(stdout,"Way of use:\n");
	fprintf(stdout,"   1. You can provide the ip address of the network card which the device is attached to (-i) and the device mac (-m)\n");
	fprintf(stdout,"   2. (or) You can provide the ip address of the device itself (-I). In this case parameters (-i) and (-m) are forbidden\n");
}

void Print_The_URT_Table(sURT_Info URT)
{
	int				y,x;

	printf("\n");
	printf("URT-Table:\n");
	printf("----------\n");

	// Header
	printf("\t");
	for (x=1 ; x<=URT.Size ; x++)
	{
		printf("%d\t", URT.Device_did_Mapping[x-1]);
	}
	printf("\n");

	for (y=1 ; y<=URT.Size ; y++)
	{
		printf("%d\t", URT.Device_did_Mapping[y-1]);

		for (x=1 ; x<=URT.Size ; x++)
		{
			printf("%d\t", URT.Array[y-1][x-1]);
		}
		printf("\n");
	}
}

void Print_The_BRT_Root_Path_Table(sBRT_Info BRT)
{
	int				y,x;

	printf("\n");
	printf("BRT Root-Path Table:\n");
	printf("--------------------\n");

	// Header
	printf("\t");
	for (x=1 ; x<=BRT.Size ; x++)
	{
		printf("%d\t", BRT.Device_did_Mapping[x-1]);
	}
	printf("\n");

	for (y=1 ; y<=BRT.Size ; y++)
	{
		printf("%d\t", BRT.Device_did_Mapping[y-1]);

		for (x=1 ; x<=BRT.Size ; x++)
		{
			printf("%d\t", BRT.Root_Array[y-1][x-1]);
		}
		printf("\n");
	}
}

void Print_The_BRT_Branch_Path_Table(sBRT_Info BRT)
{
	int				y,x;

	printf("\n");
	printf("BRT Branch-Path Table:\n");
	printf("----------------------\n");

	// Header
	printf("\t");
	for (x=1 ; x<=BRT.Size ; x++)
	{
		printf("%d\t", BRT.Device_did_Mapping[x-1]);
	}
	printf("\n");

	for (y=1 ; y<=BRT.Size ; y++)
	{
		printf("%d\t", BRT.Device_did_Mapping[y-1]);

		for (x=1 ; x<=BRT.Size ; x++)
		{
			printf("%d\t", BRT.Branch_Array[y-1][x-1]);
		}
		printf("\n");
	}
}

void Print_The_Mapping_Table(sBPL_Mapping_Info Mapping)
{
	int				i;

	printf("\n");
	printf("Mapping-Table:\n");
	printf("--------------\n");
	printf("\n");
	printf("  GhnDeviceMAC    | GhnDeviceID | DeviceName\n");
	printf("------------------+-------------+-----------\n");

	for (i=1; i<= Mapping.Size; i++)
	{
		printf("%17s | %-11d | %s\n", Mapping.Array[i-1].GhnDeviceMAC, Mapping.Array[i-1].GhnDeviceID, Mapping.Array[i-1].DeviceName);
	}

}

char* Get_Device_Description_According_To_GhnDeviceID(sBPL_Mapping_Info* Mapping, INT32 GhnDeviceID, bool LimitLength)
{
	int i;
	int DeviceNameLength;

	for (i=1; i<= Mapping->Size; i++)
	{
		if (Mapping->Array[i-1].GhnDeviceID == GhnDeviceID)
		{
			DeviceNameLength = strlen(Mapping->Array[i-1].DeviceName);

			if ((OS_STRICMP(Mapping->Array[i-1].DeviceName, "SIGMA") != 0) && (DeviceNameLength > 0))
			{
				if ((LimitLength == TRUE) && (DeviceNameLength > BPL_TOPOLOGY_DEVICE_NAME_LENGTH_LIMIT))
				{
					return &Mapping->Array[i-1].DeviceName[DeviceNameLength-BPL_TOPOLOGY_DEVICE_NAME_LENGTH_LIMIT];
				}
				else
				{
					return &Mapping->Array[i-1].DeviceName[0];
				}
			}
			else
			{
				return &Mapping->Array[i-1].GhnDeviceMAC[9]; // Return only the last 6 digits
			}
		}
	}

	return "N/A";
}

int Get_DID_Mapping_Index_According_To_GhnDeviceID(UINT8* Device_did_Mapping, INT32 GhnDeviceID)
{
	int i;

	for (i=1; i<= BPL_MAX_SUPPORTED_DEVICES; i++)
	{
		if (Device_did_Mapping[i-1] == GhnDeviceID)
		{
			return i;
		}
	}

	return 0;
}

void Print_The_URT_Table_With_DeviceName(sBPL_Mapping_Info Mapping, sURT_Info URT)
{
	int				y,x;

	printf("\n");
	printf("URT-Table:\n");
	printf("----------\n");

	// Header
	printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, "");
	for (x=1 ; x<=URT.Size ; x++)
	{
		printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, Get_Device_Description_According_To_GhnDeviceID(&Mapping, URT.Device_did_Mapping[x-1], TRUE));
	}
	printf("\n");

	for (y=1 ; y<=URT.Size ; y++)
	{
		printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, Get_Device_Description_According_To_GhnDeviceID(&Mapping, URT.Device_did_Mapping[y-1], TRUE));

		for (x=1 ; x<=URT.Size ; x++)
		{
			if (y==x)
			{
				printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, "X");
				continue;
			}

			if (URT.Array[y-1][x-1] == 0)
			{
				printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, "+");
				continue;
			}

			printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, Get_Device_Description_According_To_GhnDeviceID(&Mapping, URT.Array[y-1][x-1], TRUE));
		}
		printf("\n");
	}
}

void Print_The_BRT_Root_Path_Table_With_DeviceName(sBPL_Mapping_Info Mapping, sBRT_Info BRT)
{
	int				y,x;

	printf("\n");
	printf("BRT Root-Path Table:\n");
	printf("--------------------\n");

	// Header
	printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, "");
	for (x=1 ; x<=BRT.Size ; x++)
	{
		printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, Get_Device_Description_According_To_GhnDeviceID(&Mapping, BRT.Device_did_Mapping[x-1], TRUE));
	}
	printf("\n");

	for (y=1 ; y<=BRT.Size ; y++)
	{
		printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, Get_Device_Description_According_To_GhnDeviceID(&Mapping, BRT.Device_did_Mapping[y-1], TRUE));

		for (x=1 ; x<=BRT.Size ; x++)
		{
			if (y==x)
			{
				printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, "X");
				continue;
			}

			if (BRT.Root_Array[y-1][x-1] == 0)
			{
				printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, "+");
				continue;
			}

			printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, Get_Device_Description_According_To_GhnDeviceID(&Mapping, BRT.Root_Array[y-1][x-1], TRUE));
		}
		printf("\n");
	}
}

void Print_The_BRT_Branch_Path_Table_With_DeviceName(sBPL_Mapping_Info Mapping, sBRT_Info BRT)
{
	int				y,x;

	printf("\n");
	printf("BRT Branch-Path Table:\n");
	printf("----------------------\n");

	// Header
	printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, "");
	for (x=1 ; x<=BRT.Size ; x++)
	{
		printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, Get_Device_Description_According_To_GhnDeviceID(&Mapping, BRT.Device_did_Mapping[x-1], TRUE));
	}
	printf("\n");

	for (y=1 ; y<=BRT.Size ; y++)
	{
		printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, Get_Device_Description_According_To_GhnDeviceID(&Mapping, BRT.Device_did_Mapping[y-1], TRUE));

		for (x=1 ; x<=BRT.Size ; x++)
		{
			if (y==x)
			{
				printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, "X");
				continue;
			}

			if (BRT.Branch_Array[y-1][x-1] == 0)
			{
				printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, "+");
				continue;
			}

			printf(BPL_TOPOLOGY_COLUMN_SIZE_LENGTH, Get_Device_Description_According_To_GhnDeviceID(&Mapping, BRT.Branch_Array[y-1][x-1], TRUE));
		}
		printf("\n");
	}
}

void Print_The_URT_Paths_With_DeviceName(sBPL_Mapping_Info Mapping, sURT_Info URT)
{
	int				y,x;
	int				Source;
	int				Next_Hop;
	char			Line[256];
	char			Buffer[256];

	printf("\n");
	printf("URT-Path:\n");
	printf("---------\n");

	for (y=1 ; y<=URT.Size ; y++)
	{
		for (x=1 ; x<=URT.Size ; x++)
		{
			if (y==x)
			{
				continue;
			}

			sprintf(Line, "\"%s\" -> ", Get_Device_Description_According_To_GhnDeviceID(&Mapping, URT.Device_did_Mapping[y-1], FALSE));

			Source = y;

			do 
			{
				Next_Hop = URT.Array[Source-1][x-1];

				if (Next_Hop == 0)
				{
					// Got to Destination
					sprintf(Buffer, "\"%s\"", Get_Device_Description_According_To_GhnDeviceID(&Mapping, URT.Device_did_Mapping[x-1], FALSE));
					strcat(Line, Buffer);
				}
				else
				{
					sprintf(Buffer, "\"%s\" -> ", Get_Device_Description_According_To_GhnDeviceID(&Mapping, URT.Array[Source-1][x-1], FALSE));
					strcat(Line, Buffer);

					//Source = Next_Hop;
					Source = Get_DID_Mapping_Index_According_To_GhnDeviceID(URT.Device_did_Mapping, Next_Hop);
				}

			} while (Next_Hop != 0);

			printf("%s\n", Line);
		}
	}
}

void Print_The_BRT_Root_Paths_With_DeviceName(sBPL_Mapping_Info Mapping, sBRT_Info BRT)
{
	int				y,x;
	int				Source;
	int				Next_Hop;
	char			Line[256];
	char			Buffer[256];

	printf("\n");
	printf("BRT Root-Path:\n");
	printf("--------------\n");

	for (y=1 ; y<=BRT.Size ; y++)
	{
		for (x=1 ; x<=BRT.Size ; x++)
		{
			if (y==x)
			{
				continue;
			}

			sprintf(Line, "\"%s\" -> ", Get_Device_Description_According_To_GhnDeviceID(&Mapping, BRT.Device_did_Mapping[y-1], FALSE));

			Source = y;

			do 
			{
				Next_Hop = BRT.Root_Array[Source-1][x-1];

				if (Next_Hop == BRT.Device_did_Mapping[x-1])
				{
					// Got to Destination
					sprintf(Buffer, "\"%s\"", Get_Device_Description_According_To_GhnDeviceID(&Mapping, BRT.Device_did_Mapping[x-1], FALSE));
					strcat(Line, Buffer);
				}
				else
				{
					sprintf(Buffer, "\"%s\" -> ", Get_Device_Description_According_To_GhnDeviceID(&Mapping, BRT.Root_Array[Source-1][x-1], FALSE));
					strcat(Line, Buffer);

					//Source = Next_Hop;
					Source = Get_DID_Mapping_Index_According_To_GhnDeviceID(BRT.Device_did_Mapping, Next_Hop);
				}

			} while (Next_Hop != BRT.Device_did_Mapping[x-1]);

			printf("%s\n", Line);
		}
	}
}

int main(int argc, char* argv[])
{
	int									c;

	sGet_stations_Information			getstation;
	sBPL_Topology_Information			BPL_Info;
	sBPL_Topology_XML_File_Information 	BPL_XML_File_Info;

	sBPL_Mapping_Info*					Mapping;
	sURT_Info*							URT;
	sBRT_Info*							BRT;

	eGHN_LIB_STAT						GHN_LIB_STAT;

	bool								bHasAdapterIP = FALSE;				// Use specific Network-Adapter
	bool								bHasDeviceMac = FALSE;				// Use specific device MAC address
	bool								bHasDeviceIP = FALSE;				// Use specific device IP address

	char								strAdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char								strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char								strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	bool								bPrint_BRT_Information = FALSE;
	
	bool								bUse_XML_File = FALSE;
	char								strFileName[GHN_LIB_MAX_PATH];

	while ((c = getopt(argc,argv,"m:i:I:Bf:?hV")) >= 0) 
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

		case 'B':
			bPrint_BRT_Information = TRUE;
			break;

		case 'f':
			bUse_XML_File = TRUE;
			strcpy(strFileName, optarg);
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

	if ((bUse_XML_File == TRUE) &&
		(bHasAdapterIP || bHasDeviceMac || bHasDeviceIP))
	{
		fprintf(stderr,"Specify parameter (-f) and (-i/-I/-m) are forbidden.\n");
		return 1;
	}


	if (bUse_XML_File == FALSE)
	{
		if (CMDHelpers_GetStation(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,FALSE,FALSE,&getstation) == FALSE)
		{
			return 1;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get BPL Topology
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		memset(&BPL_Info,0x00,sizeof(sBPL_Topology_Information));

		BPL_Info.getstation = getstation;

		if ((GHN_LIB_STAT = Ghn_Get_BPL_Topology_Information(&BPL_Info)) != eGHN_LIB_STAT_SUCCESS)
		{
			char StatusDescription[1024];

			Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);

			Print_Error_Description("Failed to get BPL Topology",
				GHN_LIB_STAT,
				StatusDescription,
				BPL_Info.ErrorDescription);

			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		Mapping = &BPL_Info.Mapping;
		URT = &BPL_Info.URT;
		BRT = &BPL_Info.BRT;
	}
	else // (bUse_XML_File == TRUE)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get BPL Topology From XML File
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		memset(&BPL_XML_File_Info,0x00,sizeof(sBPL_Topology_XML_File_Information));

		BPL_XML_File_Info.bIsRM_XML_File = FALSE;
		strcpy(BPL_XML_File_Info.FileName, strFileName);

		if ((GHN_LIB_STAT = Ghn_Get_BPL_Topology_Information_From_XML_File(&BPL_XML_File_Info)) != eGHN_LIB_STAT_SUCCESS)
		{
			char StatusDescription[1024];

			Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);

			Print_Error_Description("Failed to get BPL Topology",
				GHN_LIB_STAT,
				StatusDescription,
				BPL_XML_File_Info.ErrorDescription);

			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		Mapping = &BPL_XML_File_Info.Mapping;
		URT = &BPL_XML_File_Info.URT;
		BRT = &BPL_XML_File_Info.BRT;
	}

	Print_The_URT_Table(*URT);

	if (bPrint_BRT_Information == TRUE)
	{
		Print_The_BRT_Root_Path_Table(*BRT);
		Print_The_BRT_Branch_Path_Table(*BRT);
	}
	
	Print_The_Mapping_Table(*Mapping);
	
	Print_The_URT_Table_With_DeviceName(*Mapping, *URT);
	
	if (bPrint_BRT_Information == TRUE)
	{
		Print_The_BRT_Root_Path_Table_With_DeviceName(*Mapping, *BRT);
		Print_The_BRT_Branch_Path_Table_With_DeviceName(*Mapping, *BRT);
	}

	Print_The_URT_Paths_With_DeviceName(*Mapping, *URT);

	if (bPrint_BRT_Information == TRUE)
	{
		Print_The_BRT_Root_Paths_With_DeviceName(*Mapping, *BRT);
	}

	return 0;
}
