#ifndef CMD_Helpers_SecurityParams_h__
#define CMD_Helpers_SecurityParams_h__

bool CMDHelpers_Get_Network_Encryption_Mode(	bool									xi_bHasDeviceIP,
												char*									xi_strDeviceIP,
												bool									xi_bHasAdapterIP,
												char*									xi_strAdapterIP,
												bool									xi_bHasDeviceMac,
												char*									xi_strDeviceMAC,
												sNetwork_Encryption_Mode_Information*	xio_NetworkEncryptionMode);

bool CMDHelpers_Set_Network_Encryption_Mode(	bool									xi_bHasDeviceIP,
												char*									xi_strDeviceIP,
												bool									xi_bHasAdapterIP,
												char*									xi_strAdapterIP,
												bool									xi_bHasDeviceMac,
												char*									xi_strDeviceMAC,
												sNetwork_Encryption_Mode_Information*	xio_NetworkEncryptionMode);

bool CMDHelpers_Get_Network_Device_Password(	bool									xi_bHasDeviceIP,
												char*									xi_strDeviceIP,
												bool									xi_bHasAdapterIP,
												char*									xi_strAdapterIP,
												bool									xi_bHasDeviceMac,
												char*									xi_strDeviceMAC,
												sNetwork_Device_Password_Information*	xio_NetworkDevicePassword);

bool CMDHelpers_Set_Network_Device_Password(	bool									xi_bHasDeviceIP,
												char*									xi_strDeviceIP,
												bool									xi_bHasAdapterIP,
												char*									xi_strAdapterIP,
												bool									xi_bHasDeviceMac,
												char*									xi_strDeviceMAC,
												sNetwork_Device_Password_Information*	xio_NetworkDevicePassword);

bool CMDHelpers_Get_Network_Domain_Name(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sNetwork_Domain_Name_Information*		xio_NetworkDomainName);

bool CMDHelpers_Set_Network_Domain_Name(	bool								xi_bHasDeviceIP,
											char*								xi_strDeviceIP,
											bool								xi_bHasAdapterIP,
											char*								xi_strAdapterIP,
											bool								xi_bHasDeviceMac,
											char*								xi_strDeviceMAC,
											sNetwork_Domain_Name_Information*	xio_NetworkDomainName);

#endif // CMD_Helpers_SecurityParams_h__
