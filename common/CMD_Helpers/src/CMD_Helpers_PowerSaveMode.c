
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
bool CMDHelpers_Get_Power_Save_Mode_Status(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sPower_Save_Mode_Status_Information*	xio_PowerSaveModeStatus)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_PowerSaveModeStatus->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Get_Power_Save_Mode_Status(xio_PowerSaveModeStatus)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to Get Power Save Mode Status", GHN_LIB_STAT, StatusDescription, xio_PowerSaveModeStatus->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Set_Power_Save_Mode_Status(	bool									xi_bHasDeviceIP,
											char*									xi_strDeviceIP,
											bool									xi_bHasAdapterIP,
											char*									xi_strAdapterIP,
											bool									xi_bHasDeviceMac,
											char*									xi_strDeviceMAC,
											sPower_Save_Mode_Status_Information*	xio_PowerSaveModeStatus)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_PowerSaveModeStatus->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Set_Power_Save_Mode_Status(xio_PowerSaveModeStatus)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to Set Power Save Mode Status", GHN_LIB_STAT, StatusDescription, xio_PowerSaveModeStatus->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Get_Power_Save_Mode_Link_Down_Timer(bool											xi_bHasDeviceIP,
													char*											xi_strDeviceIP,
													bool											xi_bHasAdapterIP,
													char*											xi_strAdapterIP,
													bool											xi_bHasDeviceMac,
													char*											xi_strDeviceMAC,
													sPower_Save_Mode_Link_Down_Timer_Information*	xio_PowerSaveModeLinkDownTimer)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_PowerSaveModeLinkDownTimer->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Get_Power_Save_Mode_Link_Down_Timer(xio_PowerSaveModeLinkDownTimer)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to Get Power Save Mode Link-Down Timer", GHN_LIB_STAT, StatusDescription, xio_PowerSaveModeLinkDownTimer->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Set_Power_Save_Mode_Link_Down_Timer(bool											xi_bHasDeviceIP,
													char*											xi_strDeviceIP,
													bool											xi_bHasAdapterIP,
													char*											xi_strAdapterIP,
													bool											xi_bHasDeviceMac,
													char*											xi_strDeviceMAC,
													sPower_Save_Mode_Link_Down_Timer_Information*	xio_PowerSaveModeLinkDownTimer)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_PowerSaveModeLinkDownTimer->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Set_Power_Save_Mode_Link_Down_Timer(xio_PowerSaveModeLinkDownTimer)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to Set Power Save Mode Link-Down Timer", GHN_LIB_STAT, StatusDescription, xio_PowerSaveModeLinkDownTimer->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Get_Power_Save_Mode_No_Traffic_Timer(	bool											xi_bHasDeviceIP,
														char*											xi_strDeviceIP,
														bool											xi_bHasAdapterIP,
														char*											xi_strAdapterIP,
														bool											xi_bHasDeviceMac,
														char*											xi_strDeviceMAC,
														sPower_Save_Mode_No_Traffic_Timer_Information*	xio_PowerSaveModeNoTrafficTimer)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_PowerSaveModeNoTrafficTimer->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Get_Power_Save_Mode_No_Traffic_Timer(xio_PowerSaveModeNoTrafficTimer)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to Get Power Save Mode No-Traffic Timer", GHN_LIB_STAT, StatusDescription, xio_PowerSaveModeNoTrafficTimer->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
bool CMDHelpers_Set_Power_Save_Mode_No_Traffic_Timer(	bool											xi_bHasDeviceIP,
														char*											xi_strDeviceIP,
														bool											xi_bHasAdapterIP,
														char*											xi_strAdapterIP,
														bool											xi_bHasDeviceMac,
														char*											xi_strDeviceMAC,
														sPower_Save_Mode_No_Traffic_Timer_Information*	xio_PowerSaveModeNoTrafficTimer)
{
	eGHN_LIB_STAT			GHN_LIB_STAT;

	if (Prepare_Connection (xi_bHasDeviceIP,
							xi_strDeviceIP,
							xi_bHasAdapterIP,
							xi_strAdapterIP,
							xi_bHasDeviceMac,
							xi_strDeviceMAC,
							FALSE, /* xi_AllowBootCode */
							&xio_PowerSaveModeNoTrafficTimer->Connection) == FALSE)
	{
		return FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Set_Power_Save_Mode_No_Traffic_Timer(xio_PowerSaveModeNoTrafficTimer)) != eGHN_LIB_STAT_SUCCESS)
	{
		char StatusDescription[1024];
		Ghn_Get_Status_Description(GHN_LIB_STAT,StatusDescription);
		Print_Error_Description("Failed to Set Power Save Mode No-Traffic Timer", GHN_LIB_STAT, StatusDescription, xio_PowerSaveModeNoTrafficTimer->ErrorDescription);
		return FALSE;
	}

	return TRUE;
}
