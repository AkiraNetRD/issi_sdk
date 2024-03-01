#ifndef GHN_LIB_LinkStatistics_h__
#define GHN_LIB_LinkStatistics_h__

#include "GHN_LIB_consts.h"
#include "GHN_LIB_int_consts.h"
#include "XMLParser.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	eLink_Statistics_NodeTypeConfiguration_MIMO = 0,
	eLink_Statistics_NodeTypeConfiguration_SISO = 1,
} eLink_Statistics_NodeTypeConfiguration;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Link_Statistics_IntervalCEParameters
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	// Device.Ghn.Interface.AssociatedDevice.IntervalCEParameters.TotalBitLoading
	UINT32							TotalBitLoading;

	// Device.Ghn.Interface.AssociatedDevice.IntervalCEParameters.CodeRate
	// (SDOUBLE)1/2, (SDOUBLE)16/21
	UINT32							CodeRate;

	// Device.Ghn.Interface.AssociatedDevice.IntervalCEParameters.AverageSNR
	SINT32							AverageSNR;

} sLink_Statistics_IntervalCEParameters;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Link_Statistics
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	// Device.Ghn.Interface.AssociatedDevice.RxEnergyChannel_0
	// Device.Ghn.Interface.AssociatedDevice.RxEnergyChannel_1
	INT16									RxEnergyChannel_0;			// RX Energy Power 0
	INT16									RxEnergyChannel_1;			// RX Energy Power 1

	// Device.Ghn.Interface.AssociatedDevice.X_00C5D9_BytesReceived
	ULONG64									BytesReceived;

	// "Device.Ghn.Interface.AssociatedDevice.IntervalCEParametersNumberOfEntries" - Number of used Intervals
	UINT32									IntervalCEParametersNumberOfEntries;

	sLink_Statistics_IntervalCEParameters	IntervalCEParameters[PHY_MAX_INTERVALS];

} sLink_Statistics;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Link_Statistics Test
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	// Device.Ghn.Interface.AssociatedDevice.GhnMACAddress
	mac_address_t							StaTxMAC;							// Source Device

	// Device.Ghn.Interface.GhnMACAddress
	mac_address_t							StaRxMAC;							// Target Device

	// Device.Ghn.Interface.NodeTypeConfiguration								// Can be "MIMO" or "SISO"
	eLink_Statistics_NodeTypeConfiguration	NodeTypeConfiguration;

	sLink_Statistics						counters;							// Counters of the current read

} sLink_Statistics_Test;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Link_Statistics Result
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	// TimeStamp of the sample
	char							TimeStamp[80];

	// Number of used Intervals
	UINT32							NumberOfInterval;

	char							strCodeRateMultipleFactor[PHY_MAX_INTERVALS][6];

	double							PHY;

	SINT32							SNR[PHY_MAX_INTERVALS];

	double							RXPower0;
	double							RXPower1;

	ULONG64							BytesReceived;

} sLink_Statistics_Result;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

eGHN_LIB_STAT Link_Statistics_ReadCEParameters(XMLParserHandler XMLHandler,sLink_Statistics_Test* linkStatistics_Test, char* NodePrefix);
eGHN_LIB_STAT Link_Statistics_CalcCEParameters(sLink_Statistics_Test* Link_Statistics_Test,sLink_Statistics_Result* linkStatistics_Result);

#ifdef __cplusplus
}
#endif

#endif // GHN_LIB_LinkStatistics_h__
