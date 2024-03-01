#ifndef CMD_Helpers_DeviceName_h__
#define CMD_Helpers_DeviceName_h__

bool CMDHelpers_Get_Device_Name(bool								xi_bHasDeviceIP,
								char*								xi_strDeviceIP,
								bool								xi_bHasAdapterIP,
								char*								xi_strAdapterIP,
								bool								xi_bHasDeviceMac,
								char*								xi_strDeviceMAC,
								sDevice_Name_Information*			xio_DeviceName);

bool CMDHelpers_Set_Device_Name(bool								xi_bHasDeviceIP,
								char*								xi_strDeviceIP,
								bool								xi_bHasAdapterIP,
								char*								xi_strAdapterIP,
								bool								xi_bHasDeviceMac,
								char*								xi_strDeviceMAC,
								sDevice_Name_Information*			xio_DeviceName);


#endif // CMD_Helpers_DeviceName_h__
