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
	fprintf(stdout,"The security parameters tool get/set security information on local/remote device\n");
	fprintf(stdout,"\n");

	fprintf(stdout,"%s [-i <ip>] -m <mac> [-a <on/off>] [-b <Device Password>] [-c <Domain Name> ] [-d <on/off>] [-e <Timer>] [-f <Timer>] [-g <val>] [-j <on/off>] [-k <threshold>]\n",cmd);
	fprintf(stdout,"-i <ip>              - network card ip address\n");
	fprintf(stdout,"-I <ip>              - IP address of the device\n");
	fprintf(stdout,"-m <mac>             - Use specific mac address (MAC-format xx:xx:xx:xx:xx:xx)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-a <on/off>          - Set the network encryption mode \"on\" or \"off\"\n");
	fprintf(stdout,"-b <Device Password> - Set the network Device Password (12 characters)\n");
	fprintf(stdout,"-c <Domain Name>     - Set the network Domain Name (Up to 32 characters)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-d <on/off>          - Set the Power Save Mode Status \"on\" or \"off\"\n");
	fprintf(stdout,"-e <Timer>           - Set the Power Save Mode Link-Down Timer (In Seconds)\n");
	fprintf(stdout,"-f <Timer>           - Set the Power Save Mode No-Traffic Timer (In Seconds)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-g <val>             - Set the Master-Selection Mode (1=Force DM, 2=Force RN, 3=Auto Selection, 4=Auto Selection(DHCP)\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"-j <on/off>          - Set the Coexistence mode \"on\" or \"off\"\n");
	fprintf(stdout,"-k <threshold>       - Set the Coexistence Threshold (-128...+127 dB)\"\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Way of use:\n");
	fprintf(stdout,"   1. You can provide the ip address of the network card which the device is attached to (-i) and the device mac (-m)\n");
	fprintf(stdout,"   2. (or) You can provide the ip address of the device itself (-I). In this case parameters (-i) and (-m) are forbidden\n");
}

int main(int argc, char* argv[])
{
	int							c;

	sNetwork_Encryption_Mode_Information			NetworkEncryptionMode;
	sNetwork_Device_Password_Information			NetworkDevicePassword;
	sNetwork_Domain_Name_Information				NetworkDomainName;

	sPower_Save_Mode_Status_Information				PowerSaveModeStatus;
	sPower_Save_Mode_Link_Down_Timer_Information	PowerSaveModeLinkDownTimer;
	sPower_Save_Mode_No_Traffic_Timer_Information	PowerSaveModeNoTrafficTimer;

	sMaster_Selection_Mode_Information				MasterSelectionMode;

	sCoexistence_Mode_Information					CoexistenceMode;
	sCoexistence_Threshold_Information				CoexistenceThreshold;

	sApply_Parameters_Setting_Information			ApplyParametersSetting;

	bool											bHasAdapterIP = FALSE;				// Use specific Network-Adapter
	bool											bHasDeviceMac = FALSE;				// Use specific device MAC address
	bool											bHasDeviceIP = FALSE;				// Use specific device IP address

	char											strAdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char											strDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char											strDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	// Security Parameters
	bool											bHasNetworkEncryptionMode = FALSE;
	bool											bHasNetworkDevicePassword = FALSE;
	bool											bHasNetworkDomainName = FALSE;

	// Power Save Mode
	bool											bHasPowerSaveModeStatus = FALSE;
	bool											bHasPowerSaveModeLinkDownTimer = FALSE;
	bool											bHasPowerSaveModeNoTrafficTimer = FALSE;
	int												Timer;
	
	// MasterSelection
	bool											bHasMasterSelectionMode = FALSE;
	UINT8											User_Master_Selection_Config;
	char											strMasterSelectionConfig[5][30] = {	"Jumper Selection",
																						"Domain Master",
																						"Regular Device",
																						"Auto Master Selection",
																						"Auto Master Selection (DHCP)"};

	// Coexistence Support
	bool											bHasCoexistenceMode = FALSE;
	
	bool											bHasCoexistenceThreshold = FALSE;
	int												User_Coexistence_Threshold;

	bool											bNeedApplyParametersSetting = FALSE;


	memset(&NetworkEncryptionMode, 0x00, sizeof(sNetwork_Encryption_Mode_Information));
	memset(&NetworkDevicePassword, 0x00, sizeof(sNetwork_Device_Password_Information));
	memset(&NetworkDomainName, 0x00, sizeof(sNetwork_Domain_Name_Information));

	memset(&PowerSaveModeStatus, 0x00, sizeof(sPower_Save_Mode_Status_Information));
	memset(&PowerSaveModeLinkDownTimer, 0x00, sizeof(sPower_Save_Mode_Link_Down_Timer_Information));
	memset(&PowerSaveModeNoTrafficTimer, 0x00, sizeof(sPower_Save_Mode_No_Traffic_Timer_Information));

	memset(&MasterSelectionMode, 0x00, sizeof(sMaster_Selection_Mode_Information));

	memset(&CoexistenceMode, 0x00, sizeof(sCoexistence_Mode_Information));
	memset(&CoexistenceThreshold, 0x00, sizeof(sCoexistence_Threshold_Information));

	while ((c = getopt(argc,argv,"m:i:I:?hVa:b:c:d:e:f:g:j:k:")) >= 0) 
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

		// Security Parameters
		case 'a':
			bHasNetworkEncryptionMode = TRUE;

			if ((strcmp(optarg,"on") == 0) || (strcmp(optarg,"ON") == 0))
			{
				NetworkEncryptionMode.Mode = TRUE;
			}
			else  if ((strcmp(optarg,"off") == 0) || (strcmp(optarg,"OFF") == 0))
			{
				NetworkEncryptionMode.Mode = FALSE;
			}
			else
			{
				fprintf(stderr,"Valid options for the 'a' parameter are: \"on\" and \"off\"\n");
				return 1;
			}
			break;

		// Security Parameters
		case 'b':
			if (strlen(optarg) != 12)
			{
				fprintf(stderr,"Network Device Password length must be 12 characters\n");
				return 1;
			}

			bHasNetworkDevicePassword = TRUE;

			strcpy(NetworkDevicePassword.DevicePassword, optarg);
			break;

		// Security Parameters
		case 'c':
			if (strlen(optarg)>32)
			{
				fprintf(stderr,"Network Domain Name maximum length is 32 characters\n");
				return 1;
			}

			bHasNetworkDomainName= TRUE;

			strcpy(NetworkDomainName.DomainName, optarg);
			break;

		// Power Save Mode
		case 'd':
			bHasPowerSaveModeStatus = TRUE;

			if ((strcmp(optarg,"on") == 0) || (strcmp(optarg,"ON") == 0))
			{
				PowerSaveModeStatus.Status = TRUE;
			}
			else  if ((strcmp(optarg,"off") == 0) || (strcmp(optarg,"OFF") == 0))
			{
				PowerSaveModeStatus.Status = FALSE;
			}
			else
			{
				fprintf(stderr,"Valid options for the 'd' parameter are: \"on\" and \"off\"\n");
				return 1;
			}
			break;

		// Power Save Mode
		case 'e':
			Timer = atoi(optarg);

			if (Timer < 0)
			{
				fprintf(stderr,"Power Save Mode Link-Down Timer must be a natural number (i.e >0)\n");
				return 1;
			}

			bHasPowerSaveModeLinkDownTimer = TRUE;

			PowerSaveModeLinkDownTimer.Timer = Timer;
			break;

		// Power Save Mode
		case 'f':
			Timer = atoi(optarg);

			if (Timer < 0)
			{
				fprintf(stderr,"Power Save Mode No-Traffic Timer must be a natural number (i.e >0)\n");
				return 1;
			}

			bHasPowerSaveModeNoTrafficTimer = TRUE;

			PowerSaveModeNoTrafficTimer.Timer = Timer;
			break;

		// MasterSelection
		case 'g':

			User_Master_Selection_Config = atoi(optarg);
			if ((User_Master_Selection_Config < 1) || (User_Master_Selection_Config > 4))
			{
				printf("Master Selection Config should be a DEC value [1..4]\n");
				return 1;
			}

			bHasMasterSelectionMode = TRUE;

			MasterSelectionMode.Mode = User_Master_Selection_Config;
			
			break;

		// Coexistence Support
		case 'j':

			bHasCoexistenceMode = TRUE;

			if ((strcmp(optarg,"on") == 0) || (strcmp(optarg,"ON") == 0))
			{
				CoexistenceMode.Mode = 1;
			}
			else if ((strcmp(optarg,"off") == 0) || (strcmp(optarg,"OFF") == 0))
			{
				CoexistenceMode.Mode = 0;
			}
			else
			{
				fprintf(stderr,"Valid options for the 'j' parameter are: \"on\" and \"off\"\n");
				return 1;
			}
			break;

		case 'k':

			User_Coexistence_Threshold = atoi(optarg);
			if ((User_Coexistence_Threshold < -128) || (User_Coexistence_Threshold > 127))
			{
				printf("Coexistence Threshold should be a between -128..+127\n");
				return 1;
			}

			bHasCoexistenceThreshold= TRUE;

			CoexistenceThreshold.Threshold = User_Coexistence_Threshold;

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
	if ((bHasNetworkEncryptionMode == FALSE) &&
		(bHasNetworkDevicePassword == FALSE) &&
		(bHasNetworkDomainName == FALSE) &&
		(bHasPowerSaveModeStatus == FALSE) &&
		(bHasPowerSaveModeLinkDownTimer == FALSE) &&
		(bHasPowerSaveModeNoTrafficTimer == FALSE) &&
		(bHasMasterSelectionMode == FALSE) &&
		(bHasCoexistenceMode == FALSE) &&
		(bHasCoexistenceThreshold== FALSE))
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Network Encryption Mode
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Network_Encryption_Mode(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&NetworkEncryptionMode) == FALSE)
		{
			// Failed to get the Network Encryption Mode
			return 1;
		}

		CMDHelpers_Print_Connection_Information(NetworkEncryptionMode.Connection);

		if (NetworkEncryptionMode.Mode == TRUE)
		{
			printf("Current Network Encryption Mode = %s\n", "On");
		}
		else
		{
			printf("Current Network Encryption Mode = %s\n", "Off");
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the NetworkEncryptionMode.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(NetworkEncryptionMode.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Network Device Password
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Network_Device_Password(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&NetworkDevicePassword) == FALSE)
		{
			// Failed to get the Network Device Password
			return 1;
		}

		printf("Current Network Device Password = %s\n", NetworkDevicePassword.DevicePassword);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Network Domain Name
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Network_Domain_Name(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&NetworkDomainName) == FALSE)
		{
			// Failed to get the Network Domain Name
			return 1;
		}

		printf("Current Network Domain Name = %s\n", NetworkDomainName.DomainName);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Power Save Mode Status
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Power_Save_Mode_Status(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&PowerSaveModeStatus) == FALSE)
		{
			// Failed to get the Power Save Mode Status
			return 1;
		}

		if (PowerSaveModeStatus.Status == TRUE)
		{
			printf("Current Power Save Mode Status = %s\n", "On");
		}
		else
		{
			printf("Current Power Save Mode Status = %s\n", "Off");
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Power Save Mode Link-Down Timer
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Power_Save_Mode_Link_Down_Timer(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&PowerSaveModeLinkDownTimer) == FALSE)
		{
			// Failed to get the Power Save Mode Link-Down Timer
			return 1;
		}

		printf("Current Power Save Mode Link-Down Timer = %d\n", PowerSaveModeLinkDownTimer.Timer);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Power Save Mode No-Traffic Timer
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Power_Save_Mode_No_Traffic_Timer(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&PowerSaveModeNoTrafficTimer) == FALSE)
		{
			// Failed to get the Power Save Mode No-Traffic Timer
			return 1;
		}

		printf("Current Power Save Mode No-Traffic Timer = %d\n", PowerSaveModeNoTrafficTimer.Timer);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Master Selection Mode
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Master_Selection_Mode(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&MasterSelectionMode) == FALSE)
		{
			// Failed to get the Master Selection Mode
			return 1;
		}

		printf("Master-Selection Config  = %s\n", strMasterSelectionConfig[MasterSelectionMode.Mode]);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Coexistence Mode
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Coexistence_Mode(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&CoexistenceMode) == FALSE)
		{
			// Failed to get the Coexistence Mode
			printf("Current Coexistence Mode = %s\n", "N/A");
		}
		else
		{
			if (CoexistenceMode.Mode == TRUE)
			{
				printf("Current Coexistence Mode = %s\n", "On");
			}
			else
			{
				printf("Current Coexistence Mode = %s\n", "Off");
			}
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Display The Coexistence Threshold
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Get_Coexistence_Threshold(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&CoexistenceThreshold) == FALSE)
		{
			// Failed to get the Coexistence Threshold
			printf("Current Coexistence Threshold = %s\n", "N/A");
		}
		else
		{
			printf("Current Coexistence Threshold = %d\n", CoexistenceThreshold.Threshold);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		return 0;
	}

	// Set the Network Encryption Mode
	if (bHasNetworkEncryptionMode)
	{
		if (CMDHelpers_Set_Network_Encryption_Mode(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&NetworkEncryptionMode) == FALSE)
		{
			return 1;
		}

		if (NetworkEncryptionMode.Mode == TRUE)
		{
			printf("Setting Network Encryption Mode = %s\n", "On");
		}
		else
		{
			printf("Setting Network Encryption Mode = %s\n", "Off");
		}

		if (NetworkEncryptionMode.bNeedApplyParameterSetting == TRUE)
		{
			bNeedApplyParametersSetting = TRUE;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the NetworkEncryptionMode.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(NetworkEncryptionMode.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	// Set the Network Device Password
	if (bHasNetworkDevicePassword)
	{
		if (CMDHelpers_Set_Network_Device_Password(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&NetworkDevicePassword) == FALSE)
		{
			return 1;
		}

		printf("Setting Network Device Password = %s\n", NetworkDevicePassword.DevicePassword);

		if (NetworkDevicePassword.bNeedApplyParameterSetting == TRUE)
		{
			bNeedApplyParametersSetting = TRUE;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the NetworkDevicePassword.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(NetworkDevicePassword.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	// Set the Network Domain Name
	if (bHasNetworkDomainName)
	{
		if (CMDHelpers_Set_Network_Domain_Name(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&NetworkDomainName) == FALSE)
		{
			return 1;
		}

		printf("Setting Network Domain Name = %s\n", NetworkDomainName.DomainName);

		if (NetworkDomainName.bNeedApplyParameterSetting == TRUE)
		{
			bNeedApplyParametersSetting = TRUE;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the NetworkDomainName.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(NetworkDomainName.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}


	// Set the Power Save Mode
	if (bHasPowerSaveModeStatus)
	{
		if (CMDHelpers_Set_Power_Save_Mode_Status(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&PowerSaveModeStatus) == FALSE)
		{
			return 1;
		}

		if (PowerSaveModeStatus.Status == TRUE)
		{
			printf("Setting Power Save Mode Status = %s\n", "On");
		}
		else
		{
			printf("Setting Power Save Mode Status = %s\n", "Off");
		}

		if (PowerSaveModeStatus.bNeedApplyParameterSetting == TRUE)
		{
			bNeedApplyParametersSetting = TRUE;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the PowerSaveModeStatus.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(PowerSaveModeStatus.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	// Set the Power Save Link-Down Timer
	if (bHasPowerSaveModeLinkDownTimer)
	{
		if (CMDHelpers_Set_Power_Save_Mode_Link_Down_Timer(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&PowerSaveModeLinkDownTimer) == FALSE)
		{
			return 1;
		}

		printf("Setting Power Save Mode Link-Down Timer = %d\n", PowerSaveModeLinkDownTimer.Timer);

		if (PowerSaveModeLinkDownTimer.bNeedApplyParameterSetting == TRUE)
		{
			bNeedApplyParametersSetting = TRUE;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the PowerSaveModeLinkDownTimer.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(PowerSaveModeLinkDownTimer.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	// Set the Power Save No-Traffic Timer
	if (bHasPowerSaveModeNoTrafficTimer)
	{
		if (CMDHelpers_Set_Power_Save_Mode_No_Traffic_Timer(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&PowerSaveModeNoTrafficTimer) == FALSE)
		{
			return 1;
		}

		printf("Setting Power Save Mode No-Traffic Timer = %d\n", PowerSaveModeNoTrafficTimer.Timer);

		if (PowerSaveModeNoTrafficTimer.bNeedApplyParameterSetting == TRUE)
		{
			bNeedApplyParametersSetting = TRUE;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the PowerSaveModeNoTrafficTimer.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(PowerSaveModeNoTrafficTimer.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}


	// Set the Master Selection Mode
	if (bHasMasterSelectionMode)
	{
		if (CMDHelpers_Set_Master_Selection_Mode(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&MasterSelectionMode) == FALSE)
		{
			return 1;
		}

		printf("Setting Master-Selection Config  = %s\n", strMasterSelectionConfig[MasterSelectionMode.Mode]);

		if (MasterSelectionMode.bNeedApplyParameterSetting == TRUE)
		{
			bNeedApplyParametersSetting = TRUE;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the MasterSelectionMode.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(MasterSelectionMode.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	// Set the Coexistence Mode
	if (bHasCoexistenceMode)
	{
		if (CMDHelpers_Set_Coexistence_Mode(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&CoexistenceMode) == FALSE)
		{
			return 1;
		}

		if (CoexistenceMode.Mode == TRUE)
		{
			printf("Setting Coexistence Mode = %s\n", "On");
		}
		else
		{
			printf("Setting Coexistence Mode = %s\n", "Off");
		}

		if (CoexistenceMode.bNeedApplyParameterSetting == TRUE)
		{
			bNeedApplyParametersSetting = TRUE;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the PowerSaveModeStatus.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(CoexistenceMode.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
		{
			return 1;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	// Set the Coexistence Threshold
	if (bHasCoexistenceThreshold)
	{
		if (CMDHelpers_Set_Coexistence_Threshold(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,bHasDeviceMac,strDeviceMAC,&CoexistenceThreshold) == FALSE)
		{
			return 1;
		}

		printf("Setting the Coexistence Threshold = %d\n", CoexistenceThreshold.Threshold);
		
		if (CoexistenceThreshold.bNeedApplyParameterSetting == TRUE)
		{
			bNeedApplyParametersSetting = TRUE;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// If the user didn't specify some of the connection parameters, update the information from the PowerSaveModeStatus.Connection
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (CMDHelpers_Update_Connection_Parameters(CoexistenceThreshold.Connection, &bHasDeviceIP, strDeviceIP, &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE)
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
