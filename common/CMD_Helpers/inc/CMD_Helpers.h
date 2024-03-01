#ifndef CMD_Helpers_h__
#define CMD_Helpers_h__

#include "GHN_LIB_common.h"
#include "GHN_LIB_typedefs.h"

#include "CMD_Helpers_Coexistence.h"
#include "CMD_Helpers_SecurityParams.h"
#include "CMD_Helpers_PowerSaveMode.h"
#include "CMD_Helpers_MasterSelection.h"
#include "CMD_Helpers_DeviceName.h"
#include "CMD_Helpers_Utilization.h"

bool Select_Local_Adapter(sAdapter_Information* adapter, char* strAdapterIP);
bool Select_Local_Device(sGet_Local_Devices_Information* localdevices, char* strDeviceMAC, char* strDeviceIP, char* strDeviceState);

void Print_Station_List_Table(sGet_stations_Information* getstation);
void Print_Device_Information(sGet_stations_Information* getstation, bool bAdvanced);
void Print_Local_Hosts_MAC_Addresses_Table(sLocal_Hosts_MAC_Addresses_Information* LocalHosts);

void Print_Image_Header_Information(sImage_Header_Information* ImageHeaderInformation);
void Print_Error_Description(char* strError, int iStatus, char* strStatus, char* strDescription);

bool Prepare_Connection(bool							xi_bHasDeviceIP,
						char*							xi_strDeviceIP,
						bool							xi_bHasAdapterIP,
						char*							xi_strAdapterIP,
						bool							xi_bHasDeviceMac,
						char*							xi_strDeviceMAC,
						bool							xi_AllowBootCode,
						sConnection_Information*		xo_Connection);

void CMDHelpers_Print_Connection_Information(sConnection_Information Connection);

bool CMDHelpers_Update_Connection_Parameters(	sConnection_Information		xi_Connection,
												bool*						xo_bHasDeviceIP,
												char*						xo_strDeviceIP,
												bool*						xo_bHasAdapterIP,
												char*						xo_strAdapterIP,
												bool*						xo_bHasDeviceMac,
												char*						xo_strDeviceMAC);

bool CMDHelpers_GetStation(	bool							xi_bHasDeviceIP,
							char*							xi_strDeviceIP,
							bool							xi_bHasAdapterIP,
							char*							xi_strAdapterIP,
							bool							xi_bHasDeviceMac,
							char*							xi_strDeviceMAC,
							bool							xi_AllowBootCode,
							bool							xi_bQueryOnlyLocalDevice,
							sGet_stations_Information*		xo_getstation);

bool CMDHelpers_Reset(	bool						xi_bHasDeviceIP,
						char*						xi_strDeviceIP,
						bool						xi_bHasAdapterIP,
						char*						xi_strAdapterIP,
						bool						xi_bHasDeviceMac,
						char*						xi_strDeviceMAC,
						eReset_Mode					xi_Resetmode,
						bool						xi_HardwareReset);

bool CMDHelpers_Restore_Factory_Default(bool									xi_bHasDeviceIP,
										char*									xi_strDeviceIP,
										bool									xi_bHasAdapterIP,
										char*									xi_strAdapterIP,
										bool									xi_bHasDeviceMac,
										char*									xi_strDeviceMAC,
										sRestore_Factory_Default_Information*	xio_sRestoreFactoryDefault);

bool CMDHelpers_Pair_Device(	bool							xi_bHasDeviceIP,
								char*							xi_strDeviceIP,
								bool							xi_bHasAdapterIP,
								char*							xi_strAdapterIP,
								bool							xi_bHasDeviceMac,
								char*							xi_strDeviceMAC,
								sPair_Device_Information*		xio_sPairDevice);

bool CMDHelpers_Unpair_Device(	bool							xi_bHasDeviceIP,
								char*							xi_strDeviceIP,
								bool							xi_bHasAdapterIP,
								char*							xi_strAdapterIP,
								bool							xi_bHasDeviceMac,
								char*							xi_strDeviceMAC,
								sUnpair_Device_Information*		xio_sUnpairDevice);

bool CMDHelpers_Apply_Parameters_Setting(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sApply_Parameters_Setting_Information*	xio_sApplyParametersSettings);

bool CMDHelpers_Local_Hosts_MAC_Addresses(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sLocal_Hosts_MAC_Addresses_Information*	LocalHosts);

#endif // CMD_Helpers_h__
