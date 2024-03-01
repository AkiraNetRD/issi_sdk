
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "common.h"

#ifdef _WIN32
#include "conio.h"
#include "windows.h"
#endif

#ifdef __linux__
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifdef _WIN32
#define OS_Sleep(t) Sleep(t) 
#define OS_MAX_PATH MAX_PATH
#define OS_STRICMP(str1,str2) _stricmp(str1,str2)
#elif __linux__
#include <linux/limits.h>
#define OS_Sleep(t) usleep(t*1000) 
#define OS_MAX_PATH PATH_MAX
#define OS_STRICMP(str1,str2) strcasecmp(str1,str2)
#endif /* __LINUX__ */


#include "GHN_LIB_ext.h"

#include "CMD_Helpers.h"

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Get_Coexistence_Mode(	bool								xi_bHasDeviceIP,
										char*								xi_strDeviceIP,
										bool								xi_bHasAdapterIP,
										char*								xi_strAdapterIP,
										bool								xi_bHasDeviceMac,
										char*								xi_strDeviceMAC,
										sCoexistence_Mode_Information*		xio_Coexistence_Mode)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_Coexistence_Mode->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Get_Coexistence_Mode(xio_Coexistence_Mode)) != eGHN_LIB_STAT_SUCCESS)
	{
		//char StatusDescription[1024];
		//Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		//Print_Error_Description("Failed to Get the Coexistence-Mode", GHN_LIB_STAT, StatusDescription, xio_Coexistence_Mode->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Set_Coexistence_Mode(	bool								xi_bHasDeviceIP,
										char*								xi_strDeviceIP,
										bool								xi_bHasAdapterIP,
										char*								xi_strAdapterIP,
										bool								xi_bHasDeviceMac,
										char*								xi_strDeviceMAC,
										sCoexistence_Mode_Information*		xio_Coexistence_Mode)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_Coexistence_Mode->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Set_Coexistence_Mode(xio_Coexistence_Mode)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to Set the Coexistence-Mode", GHN_LIB_STAT, StatusDescription, xio_Coexistence_Mode->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Get_Coexistence_Threshold(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sCoexistence_Threshold_Information*		xio_Coexistence_Threshold)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_Coexistence_Threshold->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Get_Coexistence_Threshold(xio_Coexistence_Threshold)) != eGHN_LIB_STAT_SUCCESS)
	{
		//char StatusDescription[1024];
		//Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		//Print_Error_Description("Failed to Get the Coexistence-Threshold", GHN_LIB_STAT, StatusDescription, xio_Coexistence_Threshold->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Set_Coexistence_Threshold(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sCoexistence_Threshold_Information*		xio_Coexistence_Threshold)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_Coexistence_Threshold->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Set_Coexistence_Threshold(xio_Coexistence_Threshold)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to Set the Coexistence-Threshold", GHN_LIB_STAT, StatusDescription, xio_Coexistence_Threshold->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}
