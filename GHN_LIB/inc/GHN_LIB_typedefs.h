
#ifndef _GHN_LIB_TYPEDEF_H
#define _GHN_LIB_TYPEDEF_H

#include "GHN_LIB_common.h"
#include "GHN_LIB_consts.h"


typedef struct
{
	char			OUT		Name[GHN_LIB_STRING_256];
	char			OUT		Value[GHN_LIB_STRING_256];
} sAttribute_Info;

typedef struct
{
	char			OUT		DeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	UINT8			OUT		GhnDeviceID;
	char			OUT		DeviceIP[GHN_LIB_IP_ADDRESS_LEN];
	char			OUT		DeviceState[GHN_LIB_DEVICE_STATE];			// "Firmware" / "B1" / "BootCode" / "Unknown"
	bool			OUT		bLocalDevice;								// Is it a local connected device
} sStation;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Connection Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	bool					bHasAdapterIP;								// Use specific network card & MAC
	char					AdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char					DeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];

	bool					bHasDeviceIP;								// Use specific device IP address
	char					DeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	char					SelectedNetworkCardIP[GHN_LIB_IP_ADDRESS_LEN];

	bool					bIsBootCode;								// Device is in Boot-Code mode
	bool					bSnifferMode;								// When enable, snif all messages between host and G.hn device
	bool					bAllow_Incoming_Broadcast_Packets;			// Allow Incoming Broadcast Packets from device
} sConnection_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Adapter Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	char			OUT		IP[GHN_LIB_IP_ADDRESS_LEN];
	char			OUT		Description[GHN_LIB_ADAPTER_DESC_LEN];
} sAdapter_Info;

typedef struct
{
	UINT32			OUT		Size;
	sAdapter_Info	OUT		Array[GHN_LIB_MAX_ADAPTER_INFORMATION];
} sAdapter_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Device Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	bool					IN		bAdvanced;

	UINT32					OUT		Size;
	sAttribute_Info			OUT		AttributeArray[GHN_LIB_MAX_LOCALDEVICE_INFORMATION];

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sDevice_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Get Device State Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	char			IN		AdapterIP[GHN_LIB_IP_ADDRESS_LEN];
	char			IN		DeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];

	char			OUT		DeviceState[GHN_LIB_DEVICE_STATE];			// "Firmware" / "B1" / "BootCode" / "Unknown"

	char			OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sGet_Device_State_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Get Local Devices Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	char			IN		AdapterIP[GHN_LIB_IP_ADDRESS_LEN];

	UINT32			OUT		Size;
	sStation		OUT		sStationArray[GHN_LIB_MAX_GETSTATIONS];

	char			OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sGet_Local_Devices_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Get station Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	bool					IN		bQueryOnlyLocalDevice;

	UINT32					OUT		Size;
	sStation				OUT		sStationArray[GHN_LIB_MAX_GETSTATIONS];

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sGet_stations_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Reset Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef enum
{
	eReset_Mode_Firmware = 0,
	eReset_Mode_EtherBoot = 1
} eReset_Mode;

typedef struct
{
	sConnection_Information	IN		Connection;

	eReset_Mode				IN		ResetMode;
	bool					IN		HardwareReset;

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sReset_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Phy Diag Config Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	char			IN		DeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	// This sets the interface name to use as outgoing network interface
	bool			IN		bHasNetworkCardIP;
	char			IN		NetworkCardIP[GHN_LIB_IP_ADDRESS_LEN]; 

	char			OUT		PhyDiagReceiverMAC[GHN_LIB_STRING_256];
	UINT32			OUT		PhyDiagTrafficBurstSize;	// 1..25
	UINT32			OUT		PhyDiagTrafficTimeOut;		// In Seconds
	bool			OUT		TrafficIsAcked;
	bool			OUT		StartPhyDiagTest;
	UINT32			OUT		OperationResult;			// ErrorCode (0=OK)

	char			OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sPhy_Diag_Config_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Netinf Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					IN		TransmitterDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char					IN		ReceiverDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];

	char					IN		BaseFolder[GHN_LIB_MAX_PATH];
	char					IN		TestFolder[GHN_LIB_MAX_PATH];

	UINT32					IN		TestDuration;						// 0 = indefinitely 
	float					IN		SamplingPeriods;					// [Seconds]
	UINT32					IN		BitLoadingTableSamplingPeriods;		// 0 = Never read the BitLoading Table

	bool					IN		bGenerateTraffic;
	bool					IN		bOverideTraffic;

	UINT32					IN		TrafficBurstSize;	// 1..25 (25 more packets over unit time)

	bool					IN		bStopOnReadingError;

	bool					IN		bSaveAdvancedGraphs;

	bool					IN		bSavePERGraph;

	bool					IN		bQueryOnlyLocalDevice;

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sNetinf_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

typedef struct
{
	char					IN		Name[GHN_LIB_MAX_BRANCH_NAME_SIZE];
} sBranch_Info;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Get BBT Data Model Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	UINT32					IN		IncludeBranch_Size;
	sBranch_Info			IN		IncludeBranch_Array[GHN_LIB_MAX_BRANCHS];

	UINT32					IN		ExcludeBranch_Size;
	sBranch_Info			IN		ExcludeBranch_Array[GHN_LIB_MAX_BRANCHS];

	UINT32					OUT		DataModel_Size;
	char					OUT		DataModel_Buffer[GHN_LIB_MAX_DATA_MODEL_BUFFER_SIZE];

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sGet_BBT_Data_Model_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

typedef struct
{
	char					IN		Name[GHN_LIB_MAX_BRANCH_NAME_SIZE];
	char					IN		Value[GHN_LIB_MAX_BRANCH_NAME_SIZE];
} sParameter_Info;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Set BBT Data Model Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	UINT32					IN		SetParameter_Size;
	sParameter_Info			IN		SetParameter_Array[GHN_LIB_MAX_BRANCHS];

	UINT32					OUT		DataModel_Size;
	char					OUT		DataModel_Buffer[GHN_LIB_MAX_DATA_MODEL_BUFFER_SIZE];

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sSet_BBT_Data_Model_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// BBT Data-Model One Parameter Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					IN		AttributePath[GHN_LIB_STRING_256];
	char					IN		AttributeName[GHN_LIB_STRING_256];

	char					IN OUT	AttributeValue[GHN_LIB_STRING_256];

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sBBT_Data_Model_One_Parameter_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Upgrade Firmware Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	bool					IN		bUpgradeFirmware;
	bool					IN		bUpdateActiveBit;				// Update the Active-Bit
	char					IN		FW_BIN_FileName[GHN_LIB_MAX_PATH];

	bool					IN		bKeepDeviceParameterSettings;	// Save Parameters from section "eBIN_Section_Parameters_Block"
	
	bool					IN		bUpdate_Boot_Loader_Version;	// Update the Extended Boot-sLoader version

	bool					IN		bHas_RemoteMonitoring;			// Override the Remote Monitoring default setting
	UINT8					IN		RemoteMonitoring;

	bool					IN		bUpdateMacAddress;				// Override the MAC-Address
	char					IN		UpdateDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];

	bool					IN		bResetDeviceOnError;
	bool					IN		bQueryDeviceAfterReset;
	bool					IN		bPrint_Detail_Information;
	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sUpgrade_Firmware_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Validate Chip Type Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					IN		FW_BIN_FileName[GHN_LIB_MAX_PATH];

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sValidate_Chip_Type_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Image Header Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	UINT16					OUT		HeaderVersion;
	char					OUT		ModelName[GHN_LIB_MODEL_NAME_LEN*GHN_LIB_MODEL_NAME_REPETITION];
	char					OUT		FW_Version[GHN_LIB_FW_VERSION_LEN];
	char					OUT		ConfigurationName[GHN_LIB_CONFIGURATION_NAME_LEN];
	char					OUT		BuildImageCreationTime[GHN_LIB_BUILD_IMAGE_CREATION_TIME_LEN];
	UINT32					OUT		TotalImageSize;
	char					OUT		FW_Signature[GHN_LIB_MD5_SIGNATURE_LEN];
} sImage_Header_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Get Image Header From File Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	char						IN		FW_BIN_FileName[GHN_LIB_MAX_PATH];

	sImage_Header_Information	OUT		ImageHeaderInformation;

	char						OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sGet_Image_Header_From_File_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Network Encryption Mode Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	bool					IN OUT	Mode;			// True  = Enable
													// False = Disable

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sNetwork_Encryption_Mode_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Network Device Password Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					IN OUT	DevicePassword[GHN_LIB_MAX_PASSWORD_LEN];

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sNetwork_Device_Password_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Network Domain Name Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					IN OUT	DomainName[GHN_LIB_MAX_DOMAIN_NAME_LEN];

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sNetwork_Domain_Name_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Power Save Mode Status Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	bool					IN OUT	Status;			// True  = Enable
													// False = Disable

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sPower_Save_Mode_Status_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Power Save Mode Link Down Timer Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	UINT32					IN OUT	Timer;			// In Seconds

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sPower_Save_Mode_Link_Down_Timer_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Power Save Mode No Traffic Timer Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	UINT32					IN OUT	Timer;			// In Seconds

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sPower_Save_Mode_No_Traffic_Timer_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Master-Selection Mode Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	UINT8					IN OUT	Mode;			// (1=Force DM, 2=Force RN, 3=Auto Selection)

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sMaster_Selection_Mode_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Device Name Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					IN OUT	DeviceName[GHN_LIB_MAX_DEVICE_NAME_LEN];

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sDevice_Name_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Restore Factory Default Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sRestore_Factory_Default_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Pair Device Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sPair_Device_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Unpair Device Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sUnpair_Device_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Link Statistics Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					IN		TransmitterDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char					IN		ReceiverDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];

	double					OUT		PHY;
	double					OUT		SNR;

	double					OUT		RXPower0;
	double					OUT		RXPower1;

	ULONG64					OUT		BytesReceived;

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sLink_Statistics_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Apply Parameters setting Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sApply_Parameters_Setting_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

typedef struct
{
	char			OUT		DeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
} sMACAddress;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Local Hosts MAC Addresses Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	UINT32					OUT		Size;
	sMACAddress				OUT		sMACAddressArray[GHN_LIB_MAX_LOCAL_HOST_MAC_ADDRESSES];

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sLocal_Hosts_MAC_Addresses_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Get Chip Type Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					OUT		Device_ChipType[GHN_LIB_MODEL_NAME_LEN];

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sGet_Chip_Type_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Coexistence Mode Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	UINT8					IN OUT	Mode;				// (1=On, 0=Off)

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sCoexistence_Mode_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Coexistence Threshold Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	INT8					IN OUT	Threshold;			// (dB)

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sCoexistence_Threshold_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Utilization Field Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	UINT32					OUT		Value;

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sUtilization_Field_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Utilization Alpha Field Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	UINT32					IN OUT	Value;

	bool					OUT		bNeedApplyParameterSetting;					// Need to apply parameter setting before change take effect

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sUtilization_Field_Alpha_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// T0 Timer Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	UINT16					OUT		T0_Timer;			// Seconds

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sT0_Timer_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// GHN NN Number Of Interference Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	UINT16					OUT		Number_Of_Interference;

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sGHN_NN_Number_Of_Interference;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define	BPL_MAX_SUPPORTED_DEVICES				64
#define	BPL_STATIC_MAX_SUPPORTED_DEVICES		32

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// G.hn BPL Topology Support
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	UINT32						OUT		GhnDeviceID;
	char						OUT		GhnDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	char						OUT		DeviceName[GHN_LIB_MAX_DEVICE_NAME_LEN];
} sBPL_Mapping_Element;

typedef struct
{
	UINT32						OUT		Size;
	sBPL_Mapping_Element		OUT		Array[GHN_LIB_MAX_GETSTATIONS];
	UINT32						OUT		DM_Index;							// DM Index in "Array"
} sBPL_Mapping_Info;

typedef struct
{
	UINT32					OUT		Size;
	UINT8					OUT		Device_did_Mapping[BPL_MAX_SUPPORTED_DEVICES];
	UINT8					OUT		Array[BPL_MAX_SUPPORTED_DEVICES][BPL_MAX_SUPPORTED_DEVICES];  // Array[y][x]
} sURT_Info;

typedef struct
{
	UINT32					OUT		Size;
	UINT8					OUT		Device_did_Mapping[BPL_MAX_SUPPORTED_DEVICES];
	UINT8					OUT		Root_Array[BPL_MAX_SUPPORTED_DEVICES][BPL_MAX_SUPPORTED_DEVICES];  // Array[y][x]
	UINT8					OUT		Branch_Array[BPL_MAX_SUPPORTED_DEVICES][BPL_MAX_SUPPORTED_DEVICES];  // Array[y][x]
} sBRT_Info;

typedef struct
{
	sGet_stations_Information	IN		getstation;

	sBPL_Mapping_Info			OUT		Mapping;
	sURT_Info					OUT		URT;
	sBRT_Info					OUT		BRT;

	char						OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sBPL_Topology_Information;

typedef struct
{
	char						IN		FileName[GHN_LIB_MAX_PATH];

	bool						IN		bIsRM_XML_File;

	sBPL_Mapping_Info			OUT		Mapping;
	sURT_Info					OUT		URT;
	sBRT_Info					OUT		BRT;

	char						OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sBPL_Topology_XML_File_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// G.hn Get PHY-Rate Table Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	char						OUT		DeviceName[GHN_LIB_MAX_DEVICE_NAME_LEN];
} sGet_PHY_Rate_Table_Element;

typedef struct
{
	sGet_stations_Information	IN		getstation;

	sGet_PHY_Rate_Table_Element	OUT		Array[GHN_LIB_MAX_GETSTATIONS];

	UINT16						OUT		RX_PhyRateTable[GHN_LIB_MAX_GETSTATIONS][GHN_LIB_MAX_GETSTATIONS];  // Array[TX][RX]

	char						OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sGet_PHY_Rate_Table_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Netinf Entire-Network Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sGet_stations_Information	IN		getstation;

	UINT32						IN		TestDuration;						// seconds

	char						OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sNetinf_Entire_Network_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#endif
