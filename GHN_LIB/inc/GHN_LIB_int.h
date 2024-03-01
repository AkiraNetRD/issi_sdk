#ifndef _GHN_LIB_INT_H
#define _GHN_LIB_INT_H

#include "GHN_LIB_consts.h"
#include "GHN_LIB_typedefs.h"
#include "common.h"
#include "cdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	eSamplingState_Recover = 0,
	eSamplingState_After_Recover = 1,
	eSamplingState_Normal = 2
} eSamplingState;

typedef enum
{
	eUM_Status_Success = 0,
	eUM_Status_Another_UM_Is_Currently_Running = 0,
	eUM_Status_FW_Is_Busy_With_Other_Operation = 0,
}eUM_Status;

eGHN_LIB_STAT ConvertReturnStatus(cg_stat_t Status);

int Is_Ip_In_The_Subnet(ip_address_t ip, ip_address_t subnet, ip_address_t mask);
bool Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(ip_address_t xi_DeviceIP,ip_address_t xi_NetworkCard);
eGHN_LIB_STAT Get_IP_Address_From_Station_List(sGet_stations_Information* getstation, char* strMAC,char* strIP);

void Prepare_Netinf_Output_Folder(char* BaseFolder, char* TestFolder, char* ReceiverDeviceMAC, char* TransmitterDeviceMAC, char* OutputFolder);
void SaveDataModelBufferIntoFile(char* OutputFolder, char* DataModelBuffer);

#ifdef __cplusplus
}
#endif

#endif
