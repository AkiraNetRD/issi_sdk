#ifndef CMD_Helpers_Utilization_h__
#define CMD_Helpers_Utilization_h__

bool CMDHelpers_Get_Utilization_Field(	bool									xi_bHasDeviceIP,
										char*									xi_strDeviceIP,
										bool									xi_bHasAdapterIP,
										char*									xi_strAdapterIP,
										bool									xi_bHasDeviceMac,
										char*									xi_strDeviceMAC,
										sUtilization_Field_Information*			xio_Utilization_Field);

bool CMDHelpers_Get_Utilization_Alpha_Field(bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sUtilization_Field_Alpha_Information*	xio_Utilization_Alpha_Field);


bool CMDHelpers_Set_Utilization_Alpha_Field(bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sUtilization_Field_Alpha_Information*	xio_Utilization_Alpha_Field);

#endif // CMD_Helpers_Utilization_h__
