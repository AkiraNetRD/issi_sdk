#ifndef CMD_Helpers_PowerSaveMode_h__
#define CMD_Helpers_PowerSaveMode_h__

bool CMDHelpers_Get_Power_Save_Mode_Status(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sPower_Save_Mode_Status_Information*	xio_PowerSaveModeStatus);

bool CMDHelpers_Set_Power_Save_Mode_Status(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sPower_Save_Mode_Status_Information*	xio_PowerSaveModeStatus);

bool CMDHelpers_Get_Power_Save_Mode_Link_Down_Timer(bool									xi_bHasDeviceIP,
													char*									xi_strDeviceIP,
													bool									xi_bHasAdapterIP,
													char*									xi_strAdapterIP,
													bool									xi_bHasDeviceMac,
													char*									xi_strDeviceMAC,
													sPower_Save_Mode_Link_Down_Timer_Information*	xio_PowerSaveModeLinkDownTimer);

bool CMDHelpers_Set_Power_Save_Mode_Link_Down_Timer(bool											xi_bHasDeviceIP,
													char*											xi_strDeviceIP,
													bool											xi_bHasAdapterIP,
													char*											xi_strAdapterIP,
													bool											xi_bHasDeviceMac,
													char*											xi_strDeviceMAC,
													sPower_Save_Mode_Link_Down_Timer_Information*	xio_PowerSaveModeLinkDownTimer);

bool CMDHelpers_Get_Power_Save_Mode_No_Traffic_Timer(	bool											xi_bHasDeviceIP,
														char*											xi_strDeviceIP,
														bool											xi_bHasAdapterIP,
														char*											xi_strAdapterIP,
														bool											xi_bHasDeviceMac,
														char*											xi_strDeviceMAC,
														sPower_Save_Mode_No_Traffic_Timer_Information*	xio_PowerSaveModeNoTrafficTimer);

bool CMDHelpers_Set_Power_Save_Mode_No_Traffic_Timer(	bool											xi_bHasDeviceIP,
														char*											xi_strDeviceIP,
														bool											xi_bHasAdapterIP,
														char*											xi_strAdapterIP,
														bool											xi_bHasDeviceMac,
														char*											xi_strDeviceMAC,
														sPower_Save_Mode_No_Traffic_Timer_Information*	xio_PowerSaveModeNoTrafficTimer);

#endif // CMD_Helpers_PowerSaveMode_h__
