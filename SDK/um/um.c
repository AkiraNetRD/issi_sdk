
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <time.h>

#ifdef _WIN32
#include "windows.h"
#include "optarg.h"
#include "direct.h"
#endif

#ifdef __linux__
#include <pthread.h> 
#include <signal.h>
#include <sys/time.h>
#endif

#ifdef _WIN32
#define OS_Sleep(t) Sleep(t)
#define OS_STRICMP(str1,str2) _stricmp(str1,str2)
#elif __linux__
#define OS_Sleep(t) usleep(t*1000) 
#define OS_STRICMP(str1,str2) strcasecmp(str1,str2)
#endif

#include "common.h"

#include "GHN_LIB_ext.h"
#include "CMD_Helpers.h"


// _getcwd
#ifdef _WIN32
#include <direct.h>
#define OS_Sleep(t) Sleep(t) 
#define OS_GETCWD(buf,size) _getcwd(buf,size)
#elif __linux__
#include <unistd.h>
#define OS_Sleep(t) usleep(t*1000) 
#define OS_GETCWD(buf,size) getcwd(buf,size)
#endif

#define MAX_THREAD GHN_LIB_MAX_SUPPORTED_DEVICES

void usage(char *cmd)
{
	fprintf(stdout,"The Update Manager tool can be used to upgrade the firmware on local/remote devices.\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"%s [-i <ip>] [-I <ip>] [-m <mac>] [-E] [-f <filename>]\n",cmd);

	fprintf(stdout,"-i <ip>       - network card ip address\n");

	fprintf(stdout,"-I <ip>       - IP-Address of the device to be upgraded\n");
	fprintf(stdout,"-m <mac>      - MAC-Address of the device to be upgraded (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"-M <mac>      - New MAC-Address to program on device (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"-E            - Update/Query Entire network\n");

	fprintf(stdout,"\n");
	fprintf(stdout,"-f <filename> - FW BIN file\n");
	fprintf(stdout,"-r            - Remote-Monitoring (0=Disable, 1=Enable)\n");
	fprintf(stdout,"-K            - Keep Device Parameter Settings\n");
	fprintf(stdout,"-B            - Update the Boot-Loader (Non-secured mode)\n");
	fprintf(stdout,"\n");
}

void TrimCurrentWorkingDirectory(char* CurrentWorkingDirectory, char* FileName)
{
	unsigned int CurrentWorkingDirectory_Length = strlen(CurrentWorkingDirectory);
	char Buffer[GHN_LIB_MAX_PATH];

	if (strlen(FileName) > CurrentWorkingDirectory_Length)
	{
		if (strncmp(FileName,CurrentWorkingDirectory,CurrentWorkingDirectory_Length) == 0)
		{
			strcpy(Buffer,&FileName[CurrentWorkingDirectory_Length+1]);
			strcpy(FileName,Buffer);
		}
	}
}

typedef struct
{
	sUpgrade_Firmware_Information*	upgradeFirmware;
	bool							Thread_Is_Running;
	int								Thread_Exit_Code;
} sThreadInformation;

#ifdef _WIN32
DWORD WINAPI ThreadProc(LPVOID lpParameter)
#elif __linux__
void* ThreadProc(void* lpParameter)
#endif
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	sThreadInformation*			ThreadInformation = (sThreadInformation*)lpParameter;

	sUpgrade_Firmware_Information*	upgradeFirmware = ThreadInformation->upgradeFirmware;

	ThreadInformation->Thread_Is_Running = TRUE;

	// UpgradeFirmware one device
	if ((GHN_LIB_STAT = Ghn_Upgrade_Firmware(upgradeFirmware)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		char ErrorDescription[1024];

		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);

		if (GHN_LIB_STAT == eGHN_LIB_STAT_OLD_EBL_VERSION)
		{
			sprintf(upgradeFirmware->ErrorDescription,"Your device has an older Boot-Loading version. Please program your device with the local programming tool or try again the remote upgrade tool with the -B flag");
		}

		if (upgradeFirmware->Connection.bHasAdapterIP)
		{
			sprintf(ErrorDescription,"%s for device MAC(%s)",
							upgradeFirmware->ErrorDescription,
							upgradeFirmware->Connection.DeviceMAC);
		}
		else // upgradeFirmware->Connection.bHasDeviceIP
		{
			sprintf(ErrorDescription,"%s for device IP(%s)",
							upgradeFirmware->ErrorDescription,
							upgradeFirmware->Connection.DeviceIP);
		}

		Print_Error_Description("Failed to Upgrade Firmware",
						GHN_LIB_STAT,
						StatusDescription,
						ErrorDescription);

		ThreadInformation->Thread_Exit_Code = FALSE;

	}
	else
	{
		ThreadInformation->Thread_Exit_Code = TRUE;
	}

	ThreadInformation->Thread_Is_Running = FALSE;

#ifdef _WIN32
	return 0;
#elif __linux__
	return NULL;
#endif
}

bool IsDMStatus(char* strDeviceIP, char* SelectedNetworkCardIP)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	sDevice_Information			devinf;
	UINT32						i;

	memset(&devinf, 0x00, sizeof(sDevice_Information));

	devinf.Connection.bHasDeviceIP = TRUE;
	strcpy(devinf.Connection.DeviceIP,strDeviceIP);
	strcpy(devinf.Connection.SelectedNetworkCardIP, SelectedNetworkCardIP);

	if ((GHN_LIB_STAT = Ghn_Get_Device_Information(&devinf)) != eGHN_LIB_STAT_SUCCESS)
	{
		printf("Failed to get device information from IP %s\n", strDeviceIP);
		return FALSE;
	}

	for (i=1; i <=devinf.Size; i++)
	{
		if (strcmp(devinf.AttributeArray[i-1].Name,"Node Type") == 0)
		{
			if (strcmp(devinf.AttributeArray[i-1].Value, "DM") == 0)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}

	return FALSE;
}

int main(int argc, char* argv[])
{
	int										status = 0;
	eGHN_LIB_STAT							GHN_LIB_STAT;

	UINT32									i;
	int										c;
	
	sUpgrade_Firmware_Information*			Thread_upgradeFirmware;
	
	sGet_stations_Information				getstation;
	UINT32									StationList_Size;
	sStation								StationList_Array[GHN_LIB_MAX_GETSTATIONS];

	// 4 Calculate Delay Time before query devices
	sT0_Timer_Information					t0_Timer;
	sGHN_NN_Number_Of_Interference			GHN_NN_NumInterference;
	UINT16									T0_Timer = 40;						// Seconds
	UINT16									InterferenceNumberOfEntries = 0;
	int										Delay_Time_Before_Query_Devices;

	sGet_Local_Devices_Information			localdevices;

	sGet_Image_Header_From_File_Information	getImageHeader;

	bool									bHasAdapterIP = FALSE;				// Use specific Network-Adapter
	bool									bHasDeviceMac = FALSE;				// Use specific device MAC address
	bool									bHasDeviceIP = FALSE;				// Use specific device IP address

	bool									bUpdateEntireNetwork = FALSE;		// Update/Query Entire network

	char									strAdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char									strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char									strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];
	
	bool									bHasProgramDeviceMac = FALSE;					// Use specific MAC address to program
	char									strProgramDeviceMac[GHN_LIB_MAC_ADDRESS_LEN];	// Mac address to program on device 

	// FW section
	bool									bProgramFW = FALSE;
	char									FW_BIN_FileName[GHN_LIB_MAX_PATH];

	bool									bKeepDeviceParameterSettings = FALSE;

	bool									bUpdate_Boot_Loader_Version = FALSE;
	
	bool									bHasUser_RemoteMonitoring = FALSE;
	UINT8									User_RemoteMonitoring;

	char									CurrentWorkingDirectory[GHN_LIB_MAX_PATH];
	
	bool									bFlagExit;


	sThreadInformation						ThreadsInformation[MAX_THREAD];	

// Thread
#ifdef _WIN32
	DWORD									dwThreadID;
	LPVOID									lpParameter;
	HANDLE									hThread[MAX_THREAD];
#elif __linux__
	pthread_t								pThread[MAX_THREAD];
	int										pThread_iret;
	void*									lpParameter;
#endif

	bool									DMStatus_Table[MAX_THREAD];

	// Progress Indication
	UINT32									ProgressIndication=0;
	UINT32									ProgressIndication_Sum=0;
	UINT32									ProgressIndication_Num=0;
    int										NumberOfDevicesToUpdate = 0;


	memset(ThreadsInformation, 0x00, sizeof(ThreadsInformation));

	while ((c = getopt(argc,argv,"m:M:i:I:E?hVf:r:KB")) >= 0) 
	{
		switch(c)
		{
		case '?':
		case 'h':
			usage(argv[0]);
			exit(0);
			break;
		case 'V':
			version_info();
			exit(0);
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
		case 'E':
			bUpdateEntireNetwork = TRUE;
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

		case 'M':
			if (is_valid_str_mac(optarg) == FALSE)
			{
				fprintf(stderr,"Invalid MAC address (%s)\n",optarg);
				return 1;
			}

			bHasProgramDeviceMac = TRUE;
			strcpy(strProgramDeviceMac,optarg);
			break;

		// FW section
		case 'f':
			strcpy(FW_BIN_FileName,optarg);
			bProgramFW = TRUE;
			break;

		case 'r':
			bHasUser_RemoteMonitoring = TRUE;
			User_RemoteMonitoring = atoi(optarg);
			break;

		case 'K':
			bKeepDeviceParameterSettings = TRUE;
			break;

		case 'B':
			bUpdate_Boot_Loader_Version = TRUE;
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

	if ((bHasUser_RemoteMonitoring == TRUE) && (bProgramFW == FALSE))
	{
		fprintf(stderr,"You must provide the FW BIN file (parameter -f).\n");
		return 1;
	}

	if ((bKeepDeviceParameterSettings == TRUE) && (bProgramFW == FALSE))
	{
		fprintf(stderr,"You must provide the FW BIN file (parameter -f).\n");
		return 1;
	}

	if ((bUpdate_Boot_Loader_Version == TRUE) && (bProgramFW == FALSE))
	{
		fprintf(stderr,"You must provide the FW BIN file (parameter -f).\n");
		return 1;
	}

	if ((bHasProgramDeviceMac== TRUE) && (bUpdateEntireNetwork  == TRUE))
	{
		fprintf(stderr,"Specify both parameters (-M) and (-E) are forbidden.\n");
		return 1;
	}

	if ((bHasProgramDeviceMac== TRUE) && (bProgramFW == FALSE))
	{
		fprintf(stderr,"You must provide the FW BIN file (parameter -f).\n");
		return 1;
	}

	printf("\n");

	if (bProgramFW == TRUE)
	{
		OS_GETCWD(CurrentWorkingDirectory,GHN_LIB_MAX_PATH);
		printf("Current Working Directory:\n\"%s\"\n",CurrentWorkingDirectory);
		printf("\n");

		TrimCurrentWorkingDirectory(CurrentWorkingDirectory, FW_BIN_FileName);
		printf("Programming FW BIN file: %s\n",FW_BIN_FileName);

		strcpy(getImageHeader.FW_BIN_FileName, FW_BIN_FileName);
		if ((GHN_LIB_STAT = Ghn_Get_Image_Header_From_File(&getImageHeader)) != eGHN_LIB_STAT_SUCCESS)
		{
			char StatusDescription[1024];
			Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
			Print_Error_Description("Failed to run Get_Image_Header_From_File", GHN_LIB_STAT, StatusDescription, getImageHeader.ErrorDescription);
			return 1;
		}

		printf("\n");
		printf("Checking FW BIN file\t\t\t\t");
		Printf_Highlight("[OK]\n");

		Print_Image_Header_Information(&(getImageHeader.ImageHeaderInformation));
		printf("\n");

	}

// 	if (CMDHelpers_GetStation(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&getstation) == FALSE)
// 	{
// 		return 1;
// 	}

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

	// The user didn't specify -E or -m or -I
	if ((bUpdateEntireNetwork == FALSE) && (bHasDeviceIP == FALSE) && (bHasDeviceMac == FALSE))
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

	printf("\n");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the T0 Timer
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	memset(&t0_Timer, 0x00, sizeof(sT0_Timer_Information));

	t0_Timer.Connection.bHasAdapterIP = TRUE;
	strcpy(t0_Timer.Connection.AdapterIP,strAdapterIP);
	strcpy(t0_Timer.Connection.DeviceMAC,getstation.sStationArray[0].DeviceMAC);

	if ((GHN_LIB_STAT = Ghn_Get_T0_Timer(&t0_Timer)) == eGHN_LIB_STAT_SUCCESS)
	{
		T0_Timer = t0_Timer.T0_Timer;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the GHN NN Number Of Interference
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	memset(&GHN_NN_NumInterference, 0x00, sizeof(sGHN_NN_Number_Of_Interference));

	GHN_NN_NumInterference.Connection.bHasAdapterIP = TRUE;
	strcpy(GHN_NN_NumInterference.Connection.AdapterIP,strAdapterIP);
	strcpy(GHN_NN_NumInterference.Connection.DeviceMAC,getstation.sStationArray[0].DeviceMAC);

	if ((GHN_LIB_STAT = Ghn_Get_GHN_NN_Number_Of_Interference(&GHN_NN_NumInterference)) == eGHN_LIB_STAT_SUCCESS)
	{
		InterferenceNumberOfEntries = GHN_NN_NumInterference.Number_Of_Interference;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Filter the getstation list
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	StationList_Size = 0;
	for (i=1 ; i <= getstation.Size ; i++)
	{
		if ((bUpdateEntireNetwork == TRUE)||
			(((bHasDeviceIP == TRUE) && (strcmp(strDeviceIP,getstation.sStationArray[i-1].DeviceIP)==0)) ||
			((bHasDeviceMac == TRUE) && (OS_STRICMP(strDeviceMAC,getstation.sStationArray[i-1].DeviceMAC)==0))))
		{
			// Check Chip-Type
			if (bProgramFW == TRUE)
			{
				sValidate_Chip_Type_Information chipType;

				memset(&chipType, 0x00, sizeof(sValidate_Chip_Type_Information));
				
				chipType.Connection.bHasAdapterIP = TRUE;
				strcpy(chipType.Connection.AdapterIP,strAdapterIP);
				strcpy(chipType.Connection.DeviceMAC,getstation.sStationArray[i-1].DeviceMAC);

				strcpy(chipType.FW_BIN_FileName, FW_BIN_FileName);

				GHN_LIB_STAT = Ghn_Validate_Chip_Type(&chipType);

				if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
				{
					printf("Skipping device MAC(%s) IP(%s) - wrong ChipType.\n",
									getstation.sStationArray[i-1].DeviceMAC,
									getstation.sStationArray[i-1].DeviceIP);

					continue;
				}
				// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			}

			memcpy(&StationList_Array[StationList_Size], &getstation.sStationArray[i-1], sizeof(sStation));
			StationList_Size++;
		}
	}

	printf("\n");

	if (StationList_Size == 0)
	{
		printf("Selected device(s) not found.\n");
		printf("Quiting without upgrade!\n");
		return 1;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Run several UpgradeFirmware in threads
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	for (i=1 ; i <= StationList_Size ; i++)
	{
		if (bProgramFW == TRUE)
		{
            NumberOfDevicesToUpdate++;
			DMStatus_Table[i-1] = IsDMStatus(StationList_Array[i-1].DeviceIP, strAdapterIP);

			if (DMStatus_Table[i-1])
			{
				printf("%d. Start updating DM device MAC(%s) IP(%s)...\n",
                    NumberOfDevicesToUpdate,
					StationList_Array[i-1].DeviceMAC,
					StationList_Array[i-1].DeviceIP);
			}
			else
			{
				printf("%d. Start updating RN device MAC(%s) IP(%s)...\n",
                    NumberOfDevicesToUpdate,
					StationList_Array[i-1].DeviceMAC,
					StationList_Array[i-1].DeviceIP);
			}
		}
		else
		{
			printf("Query device MAC(%s) IP(%s)...\n",
				StationList_Array[i-1].DeviceMAC,
				StationList_Array[i-1].DeviceIP);
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Create one thread
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		Thread_upgradeFirmware = (sUpgrade_Firmware_Information*)malloc(sizeof(sUpgrade_Firmware_Information));

		memset(Thread_upgradeFirmware,0x00,sizeof(sUpgrade_Firmware_Information));

		//Thread_upgradeFirmware->Connection.bHasDeviceIP = TRUE;
		//strcpy(Thread_upgradeFirmware->Connection.DeviceIP,StationList_Array[i-1].DeviceIP);

		Thread_upgradeFirmware->Connection.bHasAdapterIP = TRUE;
		strcpy(Thread_upgradeFirmware->Connection.AdapterIP,strAdapterIP);
		strcpy(Thread_upgradeFirmware->Connection.DeviceMAC,StationList_Array[i-1].DeviceMAC);

		if (bProgramFW == TRUE)
		{
			Thread_upgradeFirmware->bPrint_Detail_Information = FALSE;
		}
		else
		{
			Thread_upgradeFirmware->bPrint_Detail_Information = TRUE;
		}
		Thread_upgradeFirmware->bUpgradeFirmware = (bProgramFW == TRUE);
		Thread_upgradeFirmware->bUpdateActiveBit = ((bProgramFW == TRUE) && (bUpdateEntireNetwork == FALSE));
		Thread_upgradeFirmware->bQueryDeviceAfterReset = FALSE;

		if (Thread_upgradeFirmware->bUpgradeFirmware)
		{
			strcpy(Thread_upgradeFirmware->FW_BIN_FileName, FW_BIN_FileName);
		}

		Thread_upgradeFirmware->bKeepDeviceParameterSettings = bKeepDeviceParameterSettings;
		Thread_upgradeFirmware->bUpdate_Boot_Loader_Version = bUpdate_Boot_Loader_Version;
		Thread_upgradeFirmware->bHas_RemoteMonitoring = bHasUser_RemoteMonitoring;
		Thread_upgradeFirmware->RemoteMonitoring = User_RemoteMonitoring;

		Thread_upgradeFirmware->bUpdateMacAddress = bHasProgramDeviceMac;
		if (bHasProgramDeviceMac == TRUE)
		{
			strcpy(Thread_upgradeFirmware->UpdateDeviceMAC, strProgramDeviceMac);
		}

		// Run UpgradeFirmware in a thread

		ThreadsInformation[i-1].upgradeFirmware = Thread_upgradeFirmware;
		lpParameter = &ThreadsInformation[i-1];

#ifdef _WIN32
		hThread[i-1] = CreateThread(0,0,ThreadProc,lpParameter,0,&dwThreadID);
#elif __linux__

        int ret ,stacksize;
        pthread_attr_t attr;

        ret = pthread_attr_init(&attr);
        
        ret = pthread_attr_getstacksize(&attr, &stacksize);
        
        stacksize <<= 2;
        ret = pthread_attr_setstacksize(&attr, stacksize);

		pThread_iret = pthread_create( &pThread[i-1], &attr, ThreadProc, (void*) lpParameter);
		if (pThread_iret != 0)
		{
			printf("pthread_create() failed!\n");
		}

#endif

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		if (bProgramFW == FALSE)
		{
#ifdef _WIN32
			// Wait for the UpgradeFirmware thread to stop (only one thread)
			WaitForSingleObject(hThread[i-1], INFINITE);
#elif __linux__
			// Wait for the UpgradeFirmware thread to stop (only one thread)
			(void) pthread_join(pThread[i-1], NULL);
#endif

			free(ThreadsInformation[i-1].upgradeFirmware);
			ThreadsInformation[i-1].upgradeFirmware = NULL;

			// Print this line between two devices
			if ((bUpdateEntireNetwork == TRUE) && (StationList_Size > 1) && (i != StationList_Size))
			{
				printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
			}
		}
	}

	for (i=1 ; i <= StationList_Size ; i++)
	{
#ifdef _WIN32
		CloseHandle(hThread[i-1]);
#endif
	}

	fflush(stdout);

	if (bProgramFW == FALSE)
	{
		// Quit in case we only query devices information
		return 0;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	printf("\n");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Wait until all threads finished (Upgrade Firmware process)
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	bFlagExit = FALSE;
	while (bFlagExit == FALSE)
	{
		OS_Sleep(2000);

		// Set as all devices finish upgrade
		bFlagExit = TRUE;

		for (i=1 ; i <= StationList_Size ; i++)
		{
			OS_Sleep(10);

			// Check if UpgradeFirmware has stopped
			if (ThreadsInformation[i-1].Thread_Is_Running == TRUE)
			{
				bFlagExit = FALSE;
			}
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Update Total Progress Indication
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		ProgressIndication_Sum = 0;
		ProgressIndication_Num = 0;

		for (i=1 ; i <= StationList_Size ; i++)
		{
			mac_address_t mac;
			macStruct mac2;

			str_to_mac(StationList_Array[i-1].DeviceMAC,&mac);
			memcpy(&mac2,mac,6);

			ProgressIndication = GetProgressIndication(mac2);

			if (ProgressIndication > 0)
			{
				ProgressIndication_Sum = ProgressIndication_Sum + ProgressIndication;
				ProgressIndication_Num++;
			}
			else
			{
				// treat as thread finish to upgrade
				ProgressIndication_Sum = ProgressIndication_Sum + 100;
				ProgressIndication_Num++;
			}
		}

		if (ProgressIndication_Num > 0)
		{
			ProgressIndication_Sum = ProgressIndication_Sum / ProgressIndication_Num;
		}

		printf("\rUpdating Progress = %d%%...", ProgressIndication_Sum);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	printf("\rUpdating Progress = %d%%...", 100);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Free memory for "sUpgrade_Firmware_Information*"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	for (i=1 ; i <= StationList_Size ; i++)
	{
		free(ThreadsInformation[i-1].upgradeFirmware);
		ThreadsInformation[i-1].upgradeFirmware = NULL;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	printf("\n");
	printf("\n");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Set the Active-Bit
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (bUpdateEntireNetwork == TRUE)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Check that we have no failures
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		for (i=1 ; i <= StationList_Size ; i++)
		{
			// Check if Upgrade Firmware was failed
			if (ThreadsInformation[i-1].Thread_Exit_Code == FALSE)
			{
				char Buffer[100];

				sprintf(Buffer,"Upgrade Firmware for MAC(%s) IP(%s) failed!\n",
					StationList_Array[i-1].DeviceMAC,
					StationList_Array[i-1].DeviceIP);

				Printf_Highlight(Buffer);

				status = -1;
			}
		}

		if (status != 0)
		{
			printf("\n");
			printf("There are one or more devices which failed to upgrade the flash.\n");
			printf("Quiting without upgrade!\n");

			return status;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Run several UpgradeFirmware (Only bUpdateActiveBit) in threads
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		for (i=1 ; i <= StationList_Size ; i++)
		{
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// Create one thread
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			Thread_upgradeFirmware = (sUpgrade_Firmware_Information*)malloc(sizeof(sUpgrade_Firmware_Information));

			memset(Thread_upgradeFirmware,0x00,sizeof(sUpgrade_Firmware_Information));

			Thread_upgradeFirmware->Connection.bHasAdapterIP = TRUE;
			strcpy(Thread_upgradeFirmware->Connection.AdapterIP,strAdapterIP);
			strcpy(Thread_upgradeFirmware->Connection.DeviceMAC,StationList_Array[i-1].DeviceMAC);

			Thread_upgradeFirmware->bPrint_Detail_Information = FALSE;
			Thread_upgradeFirmware->bUpgradeFirmware = FALSE;
			Thread_upgradeFirmware->bUpdateActiveBit = TRUE;
			Thread_upgradeFirmware->bQueryDeviceAfterReset = FALSE;

			strcpy(Thread_upgradeFirmware->FW_BIN_FileName, FW_BIN_FileName);

			Thread_upgradeFirmware->bKeepDeviceParameterSettings = FALSE;
			Thread_upgradeFirmware->bUpdate_Boot_Loader_Version = FALSE;
			Thread_upgradeFirmware->bHas_RemoteMonitoring = FALSE;
			Thread_upgradeFirmware->RemoteMonitoring = 0x00;

			// Run UpgradeFirmware in a thread

			ThreadsInformation[i-1].upgradeFirmware = Thread_upgradeFirmware;
			lpParameter = &ThreadsInformation[i-1];

#ifdef _WIN32
			hThread[i-1] = CreateThread(0,0,ThreadProc,lpParameter,0,&dwThreadID);
#elif __linux__

        int ret ,stacksize;
        pthread_attr_t attr;

        ret = pthread_attr_init(&attr);
        
        ret = pthread_attr_getstacksize(&attr, &stacksize);
        
        stacksize <<= 2;
        ret = pthread_attr_setstacksize(&attr, stacksize);

			pThread_iret = pthread_create( &pThread[i-1], &attr, ThreadProc, (void*) lpParameter);
			if (pThread_iret != 0)
			{
				printf("pthread_create() failed!\n");
			}

#endif

			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		}

		fflush(stdout);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		printf("\n");

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Wait until all threads finished (Upgrade Firmware (Only bUpdateActiveBit) process)
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		bFlagExit = FALSE;
		while (bFlagExit == FALSE)
		{
			OS_Sleep(2000);

			// Set as all devices finish upgrade
			bFlagExit = TRUE;

			for (i=1 ; i <= StationList_Size ; i++)
			{
				OS_Sleep(10);

				// Check if UpgradeFirmware has stopped
				if (ThreadsInformation[i-1].Thread_Is_Running == TRUE)
				{
					bFlagExit = FALSE;
				}
			}
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Free memory for "sUpgrade_Firmware_Information*"
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		for (i=1 ; i <= StationList_Size ; i++)
		{
			free(ThreadsInformation[i-1].upgradeFirmware);
			ThreadsInformation[i-1].upgradeFirmware = NULL;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Check that we have no failures
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		for (i=1 ; i <= StationList_Size ; i++)
		{
			// Check if Upgrade Firmware was failed
			if (ThreadsInformation[i-1].Thread_Exit_Code == FALSE)
			{
				char Buffer[100];

				sprintf(Buffer,"Upgrade Firmware for MAC(%s) IP(%s) failed!\n",
					StationList_Array[i-1].DeviceMAC,
					StationList_Array[i-1].DeviceIP);

				Printf_Highlight(Buffer);

				status = -1;
			}
		}

		if (status != 0)
		{
			printf("\n");
			printf("There are one or more devices which failed to upgrade the flash.\n");
			printf("Device(s) state is unstable!!!\n");

			return status;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		for (i=1 ; i <= StationList_Size ; i++)
		{
#ifdef _WIN32
			CloseHandle(hThread[i-1]);
#endif
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	printf("\n");
	printf("\n");

    NumberOfDevicesToUpdate = 0;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Reset all the remote RN devices
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	for (i=1 ; i <= StationList_Size ; i++)
	{
		// Check if Upgrade Firmware was failed
		if (ThreadsInformation[i-1].Thread_Exit_Code == FALSE)
		{
			continue;
		} 

		if ((StationList_Array[i-1].bLocalDevice == FALSE) &&
			(DMStatus_Table[i-1] == FALSE))
		{
            NumberOfDevicesToUpdate++;

			printf("%d. Send reset command to remote RN device MAC(%s) IP(%s)...\n",
                    NumberOfDevicesToUpdate,
					StationList_Array[i-1].DeviceMAC,
					StationList_Array[i-1].DeviceIP);


			// Reset device
			if (CMDHelpers_Reset(	FALSE,NULL,
									TRUE,strAdapterIP,
									TRUE,StationList_Array[i-1].DeviceMAC,
									eReset_Mode_Firmware,
									FALSE) == FALSE)
			{
				//return 1;
			}
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	fflush(stdout);

    
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Reset the DM device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	for (i=1 ; i <= StationList_Size ; i++)
	{
		// Check if Upgrade Firmware was failed
		if (ThreadsInformation[i-1].Thread_Exit_Code == FALSE)
		{
			continue;
		} 

		if (DMStatus_Table[i-1] == TRUE)
		{
            NumberOfDevicesToUpdate++;

			if (StationList_Array[i-1].bLocalDevice == FALSE)
			{
				printf("%d. Send reset command to remote DM device MAC(%s) IP(%s)...\n",
                    NumberOfDevicesToUpdate,
					StationList_Array[i-1].DeviceMAC,
					StationList_Array[i-1].DeviceIP);
			}
			else
			{
				printf("%d. Send reset command to local  DM device MAC(%s) IP(%s)...\n",
                    NumberOfDevicesToUpdate,
					StationList_Array[i-1].DeviceMAC,
					StationList_Array[i-1].DeviceIP);
			}

			// Reset device
			if (CMDHelpers_Reset(	FALSE,NULL,
									TRUE,strAdapterIP,
									TRUE,StationList_Array[i-1].DeviceMAC,
									eReset_Mode_Firmware,
									FALSE) == FALSE)
			{
				//return 1;
			}
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	fflush(stdout);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Reset the local device (only if it's RN)
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	for (i=1 ; i <= StationList_Size ; i++)
	{
		// Check if Upgrade Firmware was failed
		if (ThreadsInformation[i-1].Thread_Exit_Code == FALSE)
		{
			continue;
		} 

		if ((StationList_Array[i-1].bLocalDevice == TRUE) &&
			(DMStatus_Table[i-1] == FALSE))
		{
            NumberOfDevicesToUpdate++;

			printf("%d. Send reset command to local  RN device MAC(%s) IP(%s)...\n",
                NumberOfDevicesToUpdate,
				StationList_Array[i-1].DeviceMAC,
				StationList_Array[i-1].DeviceIP);


			// Reset device
			if (CMDHelpers_Reset(	FALSE,NULL,
									TRUE,strAdapterIP,
									TRUE,StationList_Array[i-1].DeviceMAC,
									eReset_Mode_Firmware,
									FALSE) == FALSE)
			{
				//return 1;
			}
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	fflush(stdout);
	
	if (NumberOfDevicesToUpdate > 0)
	{
		Delay_Time_Before_Query_Devices = T0_Timer + (2 + 5*InterferenceNumberOfEntries) * NumberOfDevicesToUpdate;

		printf("\n");
		printf("Wait %d seconds before trying to query device(s)...\n", Delay_Time_Before_Query_Devices);
		fflush(stdout);
		OS_Sleep(Delay_Time_Before_Query_Devices * 1000);

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Send a dummy broadcast query command, so the DM device will publish it's LAAT (MAC-Address the Host)
		// Before we try to reach remove devices
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		memset(&localdevices,0x00,sizeof(sGet_Local_Devices_Information));
		strcpy(localdevices.AdapterIP,strAdapterIP);
		GHN_LIB_STAT = Ghn_Query_Local_Devices(&localdevices);
		OS_Sleep(3000); // Sleep 3 seconds
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Query all devices after reset
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		for (i=1 ; i <= StationList_Size ; i++)
		{
			// Check if Upgrade Firmware was failed
			if (ThreadsInformation[i-1].Thread_Exit_Code == FALSE)
			{
				continue;
			} 

			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// Run UpgradeFirmware (only query)
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			Thread_upgradeFirmware = (sUpgrade_Firmware_Information*)malloc(sizeof(sUpgrade_Firmware_Information));
			memset(Thread_upgradeFirmware,0x00,sizeof(sUpgrade_Firmware_Information));

			// 				Thread_upgradeFirmware->Connection.bHasDeviceIP = TRUE;
			// 				strcpy(Thread_upgradeFirmware->Connection.DeviceIP,StationList_Array[i-1].DeviceIP);

			Thread_upgradeFirmware->Connection.bHasAdapterIP = TRUE;
			strcpy(Thread_upgradeFirmware->Connection.AdapterIP,strAdapterIP);

			if (bHasProgramDeviceMac == TRUE)
			{
				strcpy(Thread_upgradeFirmware->Connection.DeviceMAC, strProgramDeviceMac);
			}
			else
			{
				strcpy(Thread_upgradeFirmware->Connection.DeviceMAC,StationList_Array[i-1].DeviceMAC);
			}

			Thread_upgradeFirmware->bPrint_Detail_Information = FALSE;
			Thread_upgradeFirmware->bUpgradeFirmware = FALSE;
			Thread_upgradeFirmware->bUpdateActiveBit = FALSE;
			Thread_upgradeFirmware->bQueryDeviceAfterReset = TRUE;

			// Run UpgradeFirmware in a thread
			ThreadsInformation[i-1].upgradeFirmware = Thread_upgradeFirmware;
			lpParameter = &ThreadsInformation[i-1];

#ifdef _WIN32
			hThread[i-1] = CreateThread(0,0,ThreadProc,lpParameter,0,&dwThreadID);
#elif __linux__

        int ret ,stacksize;
        pthread_attr_t attr;

        ret = pthread_attr_init(&attr);
        
        ret = pthread_attr_getstacksize(&attr, &stacksize);
        
        stacksize <<= 2;
        ret = pthread_attr_setstacksize(&attr, stacksize);

			pThread_iret = pthread_create( &pThread[i-1], &attr, ThreadProc, (void*) lpParameter);
			if (pThread_iret != 0)
			{
				printf("pthread_create() failed!\n");
			}

#endif

			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		fflush(stdout);

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Wait until all threads finished (Query all devices after reset)
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		bFlagExit = FALSE;
		while (bFlagExit == FALSE)
		{
			OS_Sleep(1000);

			// Set as all devices finish upgrade
			bFlagExit = TRUE;
		
			for (i=1 ; i <= StationList_Size ; i++)
			{
				// Check if UpgradeFirmware has stopped
				if (ThreadsInformation[i-1].Thread_Is_Running == TRUE)
				{
					bFlagExit = FALSE;
				}
			}
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		for (i=1 ; i <= StationList_Size ; i++)
		{
#ifdef _WIN32
			CloseHandle(hThread[i-1]);
#endif
		}
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Free memory for "sUpgrade_Firmware_Information*"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	for (i=1 ; i <= StationList_Size ; i++)
	{
		free(ThreadsInformation[i-1].upgradeFirmware);
		ThreadsInformation[i-1].upgradeFirmware = NULL;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	printf("\n");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Print the UpgradeFirmware success list
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	printf("Upgrade-Firmware Summary:\n");
	printf("-------------------------\n");
    NumberOfDevicesToUpdate = 0;
	for (i=1 ; i <= StationList_Size ; i++)
	{
        NumberOfDevicesToUpdate++;

		// Check if Upgrade Firmware was failed
		if (ThreadsInformation[i-1].Thread_Exit_Code == FALSE)
		{
			char Buffer[100];

			sprintf(Buffer,"%d. Upgrade Firmware for MAC(%s) IP(%s) failed!\n",
                            NumberOfDevicesToUpdate,
							StationList_Array[i-1].DeviceMAC,
							StationList_Array[i-1].DeviceIP);

			Printf_Highlight(Buffer);

			status = -1;

			continue;
		}

		printf("%d. Upgrade Firmware for MAC(%s) IP(%s) finish successfully\n",
            NumberOfDevicesToUpdate,
			StationList_Array[i-1].DeviceMAC,
			StationList_Array[i-1].DeviceIP);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	return status;
}

