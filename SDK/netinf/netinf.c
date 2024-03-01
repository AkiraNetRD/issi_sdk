
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <time.h>

#ifdef _WIN32
#include "windows.h"
#include "optarg.h"
#include "direct.h"
#include "conio.h"
#endif

#ifdef __linux__
#include <pthread.h> 
#include <signal.h>
#include <sys/time.h>
#endif

#ifdef _WIN32
#define OS_Sleep(t) Sleep(t) 
#elif __linux__
#define OS_Sleep(t) usleep(t*1000) 
#endif

#include "common.h"

#include "GHN_LIB_ext.h"
#include "CMD_Helpers.h"

// Global Variables
bool						g_bGenerateTraffic = FALSE;

void usage(bool bAdvanced, char *cmd)
{
	fprintf(stdout,"The Channel Information tool monitors several parameters upon real traffic in a specific channel.\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"%s [-i <ip> -m <mac>] [-I <ip>] -T <mac> -R <mac> [-s <num>] [-p <num>] [-t <folder>]\n",cmd);
	fprintf(stdout,"-i <ip>     - network card ip address\n");
	fprintf(stdout,"-I <ip>     - IP address of the device\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-m <mac>    - Use specific Local mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"-T <mac>    - Use specific Transmitter mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"-R <mac>    - Use specific Receiver mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-s <num>    - Test Duration - Number of Seconds (default = 60)\n");
	fprintf(stdout,"-p <num>    - Sampling Periods (default = 1 second)\n");
	
	if (bAdvanced)
	{
		fprintf(stdout,"-B <num>    - Refresh the BitLoading Table in Sampling Periods units (default = 0 (none))\n");
	}

	fprintf(stdout,"-t <name>   - Test name (for example: -t \"House 1\")\n");
	fprintf(stdout,"-o <folder> - Output folder (for example: -o \"C:\\My_Results\")\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-g          - Generate internal traffic\n");
	fprintf(stdout,"-G          - Generate internal traffic (Override mode)\n");
	fprintf(stdout,"-b <num>    - Set Burst Size, valid values 1..50 (default = 50)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Way of use:\n");
	fprintf(stdout,"   1. You can provide the ip address of the network card which the device is attached to (-i) and the device mac (-m)\n");
	fprintf(stdout,"   2. (or) You can provide the ip address of the device itself (-I). In this case parameters (-i) and (-m) are forbidden\n");
}

bool ThreadProc_Is_Running;

#ifdef _WIN32
DWORD WINAPI ThreadProc(LPVOID lpParameter)
#elif __linux__
void* ThreadProc(void* lpParameter)
#endif
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	sNetinf_Information*		netinf = (sNetinf_Information*)lpParameter;

	ThreadProc_Is_Running = TRUE;

	GHN_LIB_STAT = Ghn_Netinf(netinf);

	if (GHN_LIB_STAT == eGHN_LIB_STAT_NETINF_ALREADY_RUNNING)
	{
		if (g_bGenerateTraffic)
		{
			char StatusDescription[1024];
			sprintf(StatusDescription, "There is another process running netinf from the selected TX device");
			Print_Error_Description("Failed to run netinf", GHN_LIB_STAT, StatusDescription, netinf->ErrorDescription);
			printf("\n");
			printf("Please consider running the tool with parameter -G\n");
			free(netinf);
			ThreadProc_Is_Running = FALSE;
#ifdef _WIN32
			return 1;
#elif __linux__
			return NULL;
#endif
		}
		else
		{
			printf("Warning! There is another process running netinf from the selected TX device.\n");
		}

		netinf->bOverideTraffic = TRUE;
		GHN_LIB_STAT = Ghn_Netinf(netinf);
	}

	if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to run netinf", GHN_LIB_STAT, StatusDescription, netinf->ErrorDescription);
		free(netinf);
		ThreadProc_Is_Running = FALSE;
#ifdef _WIN32
		return 1;
#elif __linux__
		return NULL;
#endif
	}


	free(netinf);
	ThreadProc_Is_Running = FALSE;
#ifdef _WIN32
	return 0;
#elif __linux__
	return NULL	;
#endif
}

#ifdef _WIN32
bool WINAPI ConsoleHandler(DWORD CEvent)
{
	fprintf(stdout,"\n");
	
	switch(CEvent)
	{
		case CTRL_C_EVENT:
			fprintf(stdout,"User hit CTRL+C\n");
			break;
		case CTRL_BREAK_EVENT:
			fprintf(stdout,"User hit CTRL+Break\n");
			break;
		case CTRL_CLOSE_EVENT:
			fprintf(stdout,"Program being closed!\n");
			break;
		case CTRL_LOGOFF_EVENT:
			fprintf(stdout,"User is logging off!\n");
			break;
		case CTRL_SHUTDOWN_EVENT:
			fprintf(stdout,"System is shutting down!\n");
			break;
	}

	if (g_bGenerateTraffic)
	{
		fprintf(stdout,"Stopping the netinf traffic\n");
	}

	Ghn_Stop_Netinf();
	Sleep(500);

	return TRUE;
}
#endif

int main(int argc, char* argv[])
{
	int							c;

	time_t						time_value;
	struct tm*					now = NULL;

	sGet_stations_Information	getstation;
	sNetinf_Information*		netinf;

	UINT32						TestDuration = 60;
	float						SamplingPeriods = 1;
	UINT32						BitLoadingTableSamplingPeriods = 0;		// 0 = Never read the BitLoading Table

	// Generate Traffic
	bool						bOverideTraffic = FALSE;
	UINT32						TrafficBurstSize = 50; // 1..50 (50 more packets over unit time)

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
	char						strTestFolder[GHN_LIB_MAX_PATH] = "";
	char						strBaseFolder[GHN_LIB_MAX_PATH] = ".";

	bool						bFlagExit = FALSE;

	bool						bAdvanced = TRUE/*FALSE*/;

// Thread
#ifdef _WIN32
	DWORD						dwThreadID;
	LPVOID						lpParameter;
	HANDLE						hThread;
#elif __linux__
	pthread_t					pThread;
	int							pThread_iret;
	void*						lpParameter;
#endif

#ifdef __linux__
	struct termios term;
#endif /* ! __linux__ */

	while ((c = getopt(argc,argv,"m:i:I:T:R:t:o:s:p:B:gGb:?hVX")) >= 0) 
	{
		switch(c)
		{
		case '?':
		case 'h':
			usage(bAdvanced, argv[0]);
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
		case 't':
			strcpy(strTestFolder,optarg);
			break;
		case 'o':
			strcpy(strBaseFolder,optarg);
			break;
		case 's':
			TestDuration = atoi(optarg);
			if ((TestDuration < 1) || (TestDuration > 2147483647))
			{
				fprintf(stderr,"Test-Duration must be a natural number\n");
				return 1;
			}
			break;
		case 'p':
			SamplingPeriods = (float)atof(optarg);
			if ((SamplingPeriods <= 0) || (SamplingPeriods > 3600))
			{
				fprintf(stderr,"Sampling-Periods must be grater than 0 and smaller than 3600\n");
				return 1;
			}
			break;
		case 'B':
			BitLoadingTableSamplingPeriods = atoi(optarg);
			if (BitLoadingTableSamplingPeriods < 0)
			{
				fprintf(stderr,"BitLoading Table Sampling-Periods must be a positive number (>0)\n");
				return 1;
			}
			break;
		case 'g':
			g_bGenerateTraffic = TRUE;
			break;
		case 'G':
			g_bGenerateTraffic = TRUE;
			bOverideTraffic = TRUE;
			break;
		case 'b':
			TrafficBurstSize = atoi(optarg);
			if ((TrafficBurstSize < 1) || (TrafficBurstSize > 50))
			{
				fprintf(stderr," Burst size can be between 1 and 50\n");
				return 1;
			}
			break;
		case 'X':
			bAdvanced = TRUE;
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

	if (TestDuration < SamplingPeriods)
	{
		fprintf(stdout,"Test-Duration must be higher or equal to Sampling-Periods\n");
		return 1;
	}
	
	if ((BitLoadingTableSamplingPeriods*SamplingPeriods > TestDuration))
	{
		fprintf(stdout,"A combination of SampleingPeriod and BitLoading Table must be lower than Test Duration\n");
		return 1;
	}

	if (CMDHelpers_GetStation(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,FALSE,FALSE,&getstation) == FALSE)
	{
		return 1;
	}

	strcpy(strDeviceIP,getstation.sStationArray[0].DeviceIP);

	printf("Press the 'q' button to stop the test!\n\n");
	printf("\n");

#ifdef _WIN32
	if (SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		// unable to install handler... 
		// display message to the user
		printf("Unable to install handler!\n");
	}
#endif

	// Support Windows 8
	Check_Write_Permission_And_Update_Output_Folder(strBaseFolder);

	printf("\n");
	printf("Result files will be saved in \"%s\"\n", strBaseFolder);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Run netinf on specific channel
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	netinf = (sNetinf_Information*)malloc(sizeof(sNetinf_Information));
	memset(netinf, 0x00, sizeof(sNetinf_Information));

	netinf->Connection.bHasDeviceIP = TRUE;
	strcpy(netinf->Connection.DeviceIP,strDeviceIP);
	strcpy(netinf->Connection.SelectedNetworkCardIP, getstation.Connection.SelectedNetworkCardIP);

	//netinf->Connection.bHasAdapterIP = TRUE;
	//strcpy(netinf->Connection.AdapterIP, "169.254.10.10");
	//strcpy(netinf->Connection.DeviceMAC, "00:C5:D9:51:12:34");

	strcpy(netinf->TransmitterDeviceMAC,strTransmitterDeviceMAC);
	strcpy(netinf->ReceiverDeviceMAC,strReceiverDeviceMAC);
	strcpy(netinf->BaseFolder,strBaseFolder);

	time_value = time(NULL);
	now = localtime(&time_value);

	if (g_bGenerateTraffic == TRUE)
	{
		sprintf(netinf->TestFolder,"Netinf_%04d_%02d_%02d_%02d%02d%02d",
						now->tm_year+1900,
						now->tm_mon+1,
						now->tm_mday,
						now->tm_hour,
						now->tm_min,
						now->tm_sec);
	}
	else
	{
		sprintf(netinf->TestFolder,"Chaninf_%04d_%02d_%02d_%02d%02d%02d",
			now->tm_year+1900,
			now->tm_mon+1,
			now->tm_mday,
			now->tm_hour,
			now->tm_min,
			now->tm_sec);
	}

	if (strlen(strTestFolder)>0)
	{
		strcat(netinf->TestFolder,"_");
		strcat(netinf->TestFolder,strTestFolder);
	}
	
	netinf->TestDuration = TestDuration;
	netinf->SamplingPeriods = SamplingPeriods;
	netinf->BitLoadingTableSamplingPeriods = BitLoadingTableSamplingPeriods;
	netinf->bGenerateTraffic = g_bGenerateTraffic;
	netinf->bOverideTraffic = bOverideTraffic;
	netinf->TrafficBurstSize = TrafficBurstSize;
	netinf->bSaveAdvancedGraphs = bAdvanced;

	// Run netinf in a thread
	lpParameter = netinf;

#ifdef _WIN32
	hThread = CreateThread(0,0,ThreadProc,lpParameter,0,&dwThreadID);
#elif __linux__
	pThread_iret = pthread_create( &pThread, NULL, ThreadProc, (void*) lpParameter); 
	if (pThread_iret != 0)
	{
		printf("pthread_create() failed!\n");
	}
#endif
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef __linux__
	if (!set_term(&term))
	{
		return 0;
	}
#endif /*__linux__*/

	while (bFlagExit == FALSE)
	{

		// Check if user want to quit
		if (_kbhit())
		{
			c = _getch();
			if (c == 'q' || c == 'Q')
			{
				if (g_bGenerateTraffic)
				{
					fprintf(stdout,"Stopping the netinf traffic\n");
				}

				Ghn_Stop_Netinf();
#ifdef _WIN32
				// Wait for the netinf thread to stop
				WaitForSingleObject(hThread, INFINITE);
#elif __linux__
				// Wait for the thread to stop
				(void) pthread_join(pThread, NULL);
#endif

				fprintf(stdout,"\n");
				fprintf(stdout,"quit on user request\n");
			}
		}

		OS_Sleep(1000);

		if (ThreadProc_Is_Running == FALSE)
		{
			bFlagExit = TRUE;
		}
	}

	fprintf(stdout,"\n");

#ifdef __linux__
	unset_term(&term);
#endif /* __linux__ */

	return 0;
}
