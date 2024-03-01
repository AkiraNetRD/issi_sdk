#ifndef CMD_Helpers_MasterSelection_h__
#define CMD_Helpers_MasterSelection_h__

bool CMDHelpers_Get_Master_Selection_Mode(	bool								xi_bHasDeviceIP,
											char*								xi_strDeviceIP,
											bool								xi_bHasAdapterIP,
											char*								xi_strAdapterIP,
											bool								xi_bHasDeviceMac,
											char*								xi_strDeviceMAC,
											sMaster_Selection_Mode_Information*	xio_MasterSelectionMode);

bool CMDHelpers_Set_Master_Selection_Mode(	bool								xi_bHasDeviceIP,
											char*								xi_strDeviceIP,
											bool								xi_bHasAdapterIP,
											char*								xi_strAdapterIP,
											bool								xi_bHasDeviceMac,
											char*								xi_strDeviceMAC,
											sMaster_Selection_Mode_Information*	xio_MasterSelectionMode);

#endif // CMD_Helpers_MasterSelection_h__
