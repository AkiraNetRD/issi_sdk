#ifndef CMD_Helpers_Coexistence_h__
#define CMD_Helpers_Coexistence_h__

bool CMDHelpers_Get_Coexistence_Mode(	bool								xi_bHasDeviceIP,
										char*								xi_strDeviceIP,
										bool								xi_bHasAdapterIP,
										char*								xi_strAdapterIP,
										bool								xi_bHasDeviceMac,
										char*								xi_strDeviceMAC,
										sCoexistence_Mode_Information*		xio_Coexistence_Mode);

bool CMDHelpers_Set_Coexistence_Mode(	bool								xi_bHasDeviceIP,
										char*								xi_strDeviceIP,
										bool								xi_bHasAdapterIP,
										char*								xi_strAdapterIP,
										bool								xi_bHasDeviceMac,
										char*								xi_strDeviceMAC,
										sCoexistence_Mode_Information*		xio_Coexistence_Mode);

bool CMDHelpers_Get_Coexistence_Threshold(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sCoexistence_Threshold_Information*		xio_Coexistence_Threshold);

bool CMDHelpers_Set_Coexistence_Threshold(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sCoexistence_Threshold_Information*		xio_Coexistence_Threshold);


#endif // CMD_Helpers_Coexistence_h__
