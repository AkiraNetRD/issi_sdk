#ifndef GHN_LIB_netinf_h__
#define GHN_LIB_netinf_h__

#include "GHN_LIB_consts.h"
#include "GHN_LIB_int_consts.h"
#include "XMLParser.h"

#ifdef __cplusplus
extern "C" {
#endif


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Netinf_IntervalCEParameters
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	// Device.Ghn.Interface.AssociatedDevice.IntervalCEParameters.TotalBitLoading
	UINT32							TotalBitLoading;

	// Device.Ghn.Interface.AssociatedDevice.IntervalCEParameters.CodeRate
	// (SDOUBLE)1/2, (SDOUBLE)16/21
	UINT32							CodeRate;

	// Device.Ghn.Interface.AssociatedDevice.IntervalCEParameters.RxTotalSegments
	UINT32							RxTotalSegments;

	// Device.Ghn.Interface.AssociatedDevice.IntervalCEParameters.RxCRCSegments
	UINT32							RxCRCSegments;			// Number of Bad Blocks

	// Device.Ghn.Interface.AssociatedDevice.IntervalCEParameters.AverageSNR
	SINT32							AverageSNR;

	// Device.Ghn.Interface.AssociatedDevice.IntervalCEParameters.GroupID
	UINT32							GroupID;

	// Device.Ghn.Interface.AssociatedDevice.IntervalCEParameters.BitAllocationTable
	UINT8							BitAllocationTable[MAX_BIT_ALLOCATION_TABLE_SIZE];

} sNetinf_IntervalCEParameters;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Netinf
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	// "Device.Time.CurrentLocalTime"
	char							TimeStamp[80];
	time_t							Clock;

	// "Device.DeviceInfo.X_00C5D9_UpTimeMs"
	UINT32							X_00C5D9_UpTimeMs;

	// "Device.Ghn.Interface.LastChange"
	UINT32							LastChange;

	// "Device.Ghn.Interface.GhnDeviceID"
	UINT32							GhnDeviceID_Interface;

	// "Device.Ghn.Interface.Stats.PacketsReceived"
	UINT32							PacketsReceived;		// Received from Host  (Using in PER calculation)

	// "Device.Ghn.Interface.Stats.DiscardPacketsSent"
	UINT32							DiscardPacketsSent;		// PacketsDropped      (Using in PER calculation)

	// "Device.Ghn.Interface.X_00C5D9_Stats.ReceivedFrames"
	UINT32							ReceivedFrames;

	// "Device.Ghn.Interface.Stats.ReceivedHCSErrors"
	UINT32							ReceivedHCSErrors;

	// "Device.Ghn.Interface.X_00C5D9_Stats.FalseAlarms"
	UINT32							FalseAlarms;

	// "Device.Ghn.Interface.BitAllocationTableSize"
	UINT32							BitAllocationTableSize;

	// Device.Ghn.Interface.AssociatedDevice.X_0023C7_Index
	SINT32							X_0023C7_Index;				// Correlate for "BitLoadTable" 

	// Device.Ghn.Interface.AssociatedDevice.GhnDeviceID
	SINT32							GhnDeviceID; 

	// Device.Ghn.Interface.AssociatedDevice.RxPowerChannel_0
	// Device.Ghn.Interface.AssociatedDevice.RxPowerChannel_1
	SINT32							RxPowerChannel_0;			// RX-Power 0
	SINT32							RxPowerChannel_1;			// RX-Power 1

	// Device.Ghn.Interface.AssociatedDevice.RxEnergyChannel_0
	// Device.Ghn.Interface.AssociatedDevice.RxEnergyChannel_1
	INT16							RxEnergyChannel_0;			// RX Energy Power 0
	INT16							RxEnergyChannel_1;			// RX Energy Power 1

	// Device.Ghn.Interface.AssociatedDevice.X_00C5D9_BytesReceived
	ULONG64							BytesReceived;

	// "Device.Ghn.Interface.AssociatedDevice.IntervalCEParametersNumberOfEntries" - Number of used Intervals
	UINT32							IntervalCEParametersNumberOfEntries;

	sNetinf_IntervalCEParameters	IntervalCEParameters[PHY_MAX_INTERVALS];

} sNetinf;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


typedef enum
{
	eNetinf_ActiveMedium_Coax = 0,
	eNetinf_ActiveMedium_PowerLine = 1,
} eNetinf_ActiveMedium;

typedef enum
{
	eNetinf_NodeTypeConfiguration_MIMO = 0,
	eNetinf_NodeTypeConfiguration_SISO = 1,
} eNetinf_NodeTypeConfiguration;

typedef enum
{
	eNetinf_ParsingMethod_Online = 0,
	eNetinf_ParsingMethod_XML_Files,
	eNetinf_ParsingMethod_XML_Files_HD_RemoteMonitoring,
	eNetinf_ParsingMethod_XML_Files_BPL,
} eNetinf_ParsingMethod;

#ifdef GHN_LIB_SUPPORT_VFS 
#include "VFS_LIB_ext.h"
#else
#define FOPEN(x,y)       fopen(x,y)
#define FCLOSE(x)        fclose(x)
#define FEOF(x)          feof(x)
#define FSEEK(x,y,z)     fseek(x,y,z)
#define FPRINTF(x,y,...) fprintf(x,y,##__VA_ARGS__)
#define FGETS(x,y,z)     fgets(x,y,z)
#define FSCANF(x,y, ...) fscanf(x,y, ##__VA_ARGS__)
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Netinf Test
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	// Device.Ghn.Interface.AssociatedDevice.GhnMACAddress
	mac_address_t					StaTxMAC;							// Source Device

	// Device.Ghn.Interface.GhnMACAddress
	mac_address_t					StaRxMAC;							// Target Device

	// Device.Ghn.Interface.NodeTypeActiveMedium						// Can be "Coax" or "PowerLine"
	eNetinf_ActiveMedium			ActiveMedium;

	// Device.Ghn.Interface.NodeTypeConfiguration						// Can be "MIMO" or "SISO"
	eNetinf_NodeTypeConfiguration	NodeTypeConfiguration;

	sNetinf							counters_curr;						// Counters of the current read
	sNetinf							counters_prev;						// Counters of the previous read

	int								ThresholdNumberOfSegments;

} sNetinf_Test;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Netinf Result
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	// TimeStamp of the sample (Written in the text result files)
	char							TimeStamp[80];

	// Number of used Intervals
	UINT32							NumberOfInterval;

	char							strCodeRateMultipleFactor[PHY_MAX_INTERVALS][6];

	double							PHY;
	double							EffectivePHY;

	double							BLER[PHY_MAX_INTERVALS];
	double							TotalBLER;

	double							ReceivedFrames;
	double							ReceivedHCSErrors;								// HeaderCheckSum
	double							FalseAlarms;

	double							PER;											// PacketsDropped

	UINT32							BitLoading[PHY_MAX_INTERVALS];

	SINT32							SNR[PHY_MAX_INTERVALS];

	double							RXPower0;
	double							RXPower1;

	ULONG64							EstimatedTXBytes[PHY_MAX_INTERVALS];
	ULONG64							TotalEstimatedTXBytes;

	UINT8							BitAllocationTable[PHY_MAX_INTERVALS][MAX_BIT_ALLOCATION_TABLE_SIZE];
	UINT32							BitAllocationTableSize;
} sNetinf_Result;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

eGHN_LIB_STAT Netinf_ReadCEParameters(	XMLParserHandler		XMLHandler,
										sNetinf_Test*			Netinf_Test,
										eNetinf_ParsingMethod	Netinf_ParsingMethod,
										char*					NodePrefix,
										int						SampleSet_Index,
										char*					XMLFileName,
										bool					bOutputDateTimeWithMilliseconds);

eGHN_LIB_STAT Netinf_ReadCEParameters_BitLoading(XMLParserHandler XMLHandler,sNetinf_Test* Netinf_Test);
eGHN_LIB_STAT Netinf_CalcCEParameters(sNetinf_Test* Netinf_Test,sNetinf_Result* Netinf_Result, bool FirstIteration);

eGHN_LIB_STAT Netinf_PrintCEParameters(sNetinf_Test* Netinf_Test);
eGHN_LIB_STAT Netinf_PrintCalcCEParameters(sNetinf_Result* Netinf_Result);

eGHN_LIB_STAT Netinf_SaveCalcCEParameters(sNetinf_Result* Netinf_Result, char* ReceiverTransmitterOutputFolder, bool bGenerateTraffic, bool bSavePERGraph, bool bSaveAdvancedGraphs, bool FirstIteration, bool LastIteration);
eGHN_LIB_STAT Netinf_Update_Status_File(char* ReceiverTransmitterOutputFolder, char* SampleStatus);

#ifdef __cplusplus
}
#endif

#endif // GHN_LIB_netinf_h__
