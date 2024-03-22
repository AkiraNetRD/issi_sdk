#include <stdio.h>
#include <string.h>

#include "common.h"

#include "GHN_LIB_ext.h"
#include "CMD_Helpers.h"

#include "ixxi.h"

int isDM(void)
{
	INIT_SDK_VARIABLE;

	if (CMDHelpers_GetStation(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
							  bHasDeviceMac,strDeviceMAC,FALSE,TRUE,&getstation) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	if (getstation.Size < 1 || !getstation.sStationArray[0].bLocalDevice) {
		return STATUS_CODE_ISSI_API_RESULT_UNEXPECTED;
	}

	sDevice_Information			devinf;
	memset(&devinf, 0x00, sizeof(sDevice_Information));

	devinf.Connection.bHasDeviceIP = TRUE;
	strcpy(devinf.Connection.DeviceIP,getstation.sStationArray[0].DeviceIP);
	strcpy(devinf.Connection.SelectedNetworkCardIP, getstation.Connection.SelectedNetworkCardIP);

	if (Ghn_Get_Device_Information(&devinf) == eGHN_LIB_STAT_SUCCESS) {
		for (int j = 1; j <= devinf.Size; j++) {
			if (strcmp(devinf.AttributeArray[j].Name, "Node Type") == 0) {
				return (strcmp(devinf.AttributeArray[j].Value, "DM")==0)?1:0;
			}
		}
	}

    return STATUS_CODE_UNKNOWN_ERROR;
}

int grabNeighborCount(void)
{
	INIT_SDK_VARIABLE;

	if (CMDHelpers_GetStation(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                          bHasDeviceMac,strDeviceMAC,FALSE,FALSE,&getstation) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	} else {
		return getstation.Size-1;
	}

	return STATUS_CODE_UNKNOWN_ERROR;
}

int grabLocalDeviceInfo(struct _GhnDeviceInfo *deviceInfo)
{
	INIT_SDK_VARIABLE;
	printf("start grabLocalDeviceInfon");
	memset(deviceInfo, 0x00, sizeof(struct _GhnDeviceInfo));

	if (CMDHelpers_GetStation(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                          bHasDeviceMac,strDeviceMAC,FALSE,TRUE,&getstation) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}
	sDevice_Information			devinf;
	memset(&devinf, 0x00, sizeof(sDevice_Information));

	devinf.Connection.bHasDeviceIP = TRUE;
	strcpy(devinf.Connection.DeviceIP,getstation.sStationArray[0].DeviceIP);
	strcpy(devinf.Connection.SelectedNetworkCardIP, getstation.Connection.SelectedNetworkCardIP);
	if (Ghn_Get_Device_Information(&devinf) == eGHN_LIB_STAT_SUCCESS) {
		for (int j = 1; j <= devinf.Size; j++) {
			if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_GHN_MAC) == 0) {
				strcpy(deviceInfo->ghnMac, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_DEVICE_NAME) == 0) {
				strcpy(deviceInfo->devName, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_NODE_TYPE) == 0) {
				deviceInfo->nodeType = (strcmp(devinf.AttributeArray[j].Value, "DM")==0)?1:0;
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_DOMAIN_NAME) == 0) {
				strcpy(deviceInfo->domainName, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_ENC_STATE) == 0) {
				deviceInfo->encStatus = (strcmp(devinf.AttributeArray[j].Value, "On")==0)?1:0;
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_ENC_PWD) == 0) {
				strcpy(deviceInfo->encPwd, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_UPTIME) == 0) {
				strcpy(deviceInfo->upTime, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_NODE_TYPE_CONFIGURATION) == 0) {
				deviceInfo->isMIMO = (strcmp(devinf.AttributeArray[j].Value, "MIMO")==0)?1:0;
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_BANDWIDTH) == 0) {
				strcpy(deviceInfo->plcBandwidth, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_FIRMWARE_VERSION) == 0) {
				strcpy(deviceInfo->plcFwVersion, devinf.AttributeArray[j].Value);
			}
		}
	} else {
		return STATUS_CODE_ISSI_API_FAILE;
	}

    return STATUS_CODE_OK;
}

int grabAllDeviceInfo(struct _GhnDeviceInfo devInfoAry[], int arraySize)
{
	if (arraySize < 1) {
		return STATUS_CODE_PARAMETER_ERROR;
	}

	INIT_SDK_VARIABLE;

	int count = 0;

	if (CMDHelpers_GetStation(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                          bHasDeviceMac,strDeviceMAC,FALSE,FALSE,&getstation) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	memset(devInfoAry, 0x00, sizeof(struct _GhnDeviceInfo)*arraySize);

	GhnDeviceInfo* pInfo = &(devInfoAry)[0];

	sDevice_Information			devinf;
	for (int i = 1; i <= getstation.Size; i++) {
		memset(&devinf, 0x00, sizeof(sDevice_Information));

		devinf.Connection.bHasDeviceIP = TRUE;
		strcpy(devinf.Connection.DeviceIP,getstation.sStationArray[i-1].DeviceIP);
		strcpy(devinf.Connection.SelectedNetworkCardIP, getstation.Connection.SelectedNetworkCardIP);
		
		devinf.bAdvanced = FALSE;

		if (Ghn_Get_Device_Information(&devinf) != eGHN_LIB_STAT_SUCCESS) {
			printf("Failed to get device information from MAC %s\n", getstation.sStationArray[i-1].DeviceMAC);
			continue;
		}

		for (int j=1; j <= devinf.Size; j++) {
			if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_GHN_MAC) == 0) {
				strcpy(pInfo->ghnMac, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_DEVICE_NAME) == 0) {
				strcpy(pInfo->devName, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_NODE_TYPE) == 0) {
				pInfo->nodeType = (strcmp(devinf.AttributeArray[j].Value, "DM")==0)?1:0;
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_DOMAIN_NAME) == 0) {
				strcpy(pInfo->domainName, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_ENC_STATE) == 0) {
				pInfo->encStatus = (strcmp(devinf.AttributeArray[j].Value, "On")==0)?1:0;
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_ENC_PWD) == 0) {
				strcpy(pInfo->encPwd, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_UPTIME) == 0) {
				strcpy(pInfo->upTime, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_NODE_TYPE_CONFIGURATION) == 0) {
				pInfo->isMIMO = (strcmp(devinf.AttributeArray[j].Value, "MIMO")==0)?1:0;
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_BANDWIDTH) == 0) {
				strcpy(pInfo->plcBandwidth, devinf.AttributeArray[j].Value);
			}
			else if (strcmp(devinf.AttributeArray[j].Name, GHN_ATTR_FIRMWARE_VERSION) == 0) {
				strcpy(pInfo->plcFwVersion, devinf.AttributeArray[j].Value);
			}
		}

		pInfo++;
		count++;
		if (count == arraySize) {
			break;
		}
	}
	
	return count;
}

int resetDevice(char *target)
{
	INIT_SDK_VARIABLE;

	if (target) {
		if (is_valid_str_mac(target) == FALSE) {
			return STATUS_CODE_INVALID_MAC_ADDRESS;
		}
		bHasDeviceMac = TRUE;
		strcpy(strDeviceMAC, target);
	}

	if (CMDHelpers_Reset(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                     bHasDeviceMac,strDeviceMAC,eReset_Mode_Firmware,TRUE) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	return STATUS_CODE_OK;
}

int restoreFactoryDefault(char *target)
{
	INIT_SDK_VARIABLE;

	sRestore_Factory_Default_Information	restoreFactoryDefault;
	memset(&restoreFactoryDefault, 0x00, sizeof(sRestore_Factory_Default_Information));

	if (target) {
		if (is_valid_str_mac(target) == FALSE) {
			return STATUS_CODE_INVALID_MAC_ADDRESS;
		}
		bHasDeviceMac = TRUE;
		strcpy(strDeviceMAC, target);
	}

	if (CMDHelpers_Restore_Factory_Default(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                                       bHasDeviceMac,strDeviceMAC,&restoreFactoryDefault) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	return STATUS_CODE_OK;
}

int getPhyRateTable(struct _PhyRateData *prData)
{
	INIT_SDK_VARIABLE;

	eGHN_LIB_STAT					GHN_LIB_STAT;
	sGet_PHY_Rate_Table_Information	get_PHY_Rate;

	if (CMDHelpers_GetStation(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                          bHasDeviceMac,strDeviceMAC,FALSE,FALSE,&getstation) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	memset(&get_PHY_Rate,0x00,sizeof(sGet_PHY_Rate_Table_Information));
	get_PHY_Rate.getstation = getstation;

	if ((GHN_LIB_STAT = Ghn_Get_PHY_Rate_Table_Information(&get_PHY_Rate)) != eGHN_LIB_STAT_SUCCESS) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	int y, x;
	memset(prData, 0x00, sizeof(PhyRateData));

	for (y = 1; y <= get_PHY_Rate.getstation.Size; y++) {
		if (get_PHY_Rate.getstation.sStationArray[y-1].bLocalDevice == TRUE) {
			strcpy(prData->saMac, get_PHY_Rate.getstation.sStationArray[y-1].DeviceMAC);

			for (x = 1; x <= get_PHY_Rate.getstation.Size; x++) {
				if (y == x) {
					continue;//for x
				}

				strcpy(prData->daMac[prData->daCount], get_PHY_Rate.getstation.sStationArray[x-1].DeviceMAC);
				if (get_PHY_Rate.RX_PhyRateTable[y-1][x-1] == 0) {
					prData->rate[prData->daCount] = -1;
				} else {
					prData->rate[prData->daCount] = get_PHY_Rate.RX_PhyRateTable[y-1][x-1];
				}

				prData->daCount++;
				if (prData->daCount == 15) {
					break;//for x
				}
			}
			break;//for y
		} else {
			continue;//for y
		}
	}

    return STATUS_CODE_OK;
}

//todo
int firmwareUpdate(void)
{
    INIT_SDK_VARIABLE;
    return STATUS_CODE_OK;
}

int getNetworkEncryptionMode(char *target)
{
	INIT_SDK_VARIABLE;

	sNetwork_Encryption_Mode_Information			NetworkEncryptionMode;
	memset(&NetworkEncryptionMode, 0x00, sizeof(sNetwork_Encryption_Mode_Information));

	if (target) {
		if (is_valid_str_mac(target) == FALSE) {
			return STATUS_CODE_INVALID_MAC_ADDRESS;
		}
		bHasDeviceMac = TRUE;
		strcpy(strDeviceMAC, target);
	}

	if (CMDHelpers_Get_Network_Encryption_Mode(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                                           bHasDeviceMac,strDeviceMAC,&NetworkEncryptionMode) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	return (NetworkEncryptionMode.Mode==TRUE)?1:0;
}

int setNetworkEncryptionMode(char *target, int mode)
{
	INIT_SDK_VARIABLE;

	if (target) {
		if (is_valid_str_mac(target) == FALSE) {
			return STATUS_CODE_INVALID_MAC_ADDRESS;
		}
		bHasDeviceMac = TRUE;
		strcpy(strDeviceMAC, target);
	}

	sNetwork_Encryption_Mode_Information			NetworkEncryptionMode;
	sApply_Parameters_Setting_Information			ApplyParametersSetting;
	bool											bNeedApplyParametersSetting = FALSE;

	memset(&NetworkEncryptionMode, 0x00, sizeof(sNetwork_Encryption_Mode_Information));

	if (mode) {
		NetworkEncryptionMode.Mode = TRUE;
	} else {
        NetworkEncryptionMode.Mode = FALSE;
	}

	if (CMDHelpers_Set_Network_Encryption_Mode(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                                           bHasDeviceMac,strDeviceMAC,&NetworkEncryptionMode) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	if (NetworkEncryptionMode.bNeedApplyParameterSetting == TRUE) {
		bNeedApplyParametersSetting = TRUE;
	}

	if (CMDHelpers_Update_Connection_Parameters(NetworkEncryptionMode.Connection, &bHasDeviceIP, strDeviceIP, 
	                                            &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	if (bNeedApplyParametersSetting == TRUE) {
		if (CMDHelpers_Apply_Parameters_Setting(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
		                                        bHasDeviceMac,strDeviceMAC,&ApplyParametersSetting) == FALSE) {
			return STATUS_CODE_ISSI_API_FAILE;
		}
	}

	return STATUS_CODE_OK;
}

int getNetworkDevicePassword(char *target, char *password)
{
	if (!password || sizeof(password) < GHN_LIB_MAX_PASSWORD_LEN) {
		return STATUS_CODE_RESULT_IS_NULL_OR_SIZE_ERROR;
	}
	memset(password, 0x00, sizeof(password));

	INIT_SDK_VARIABLE;

	sNetwork_Device_Password_Information			NetworkDevicePassword;
	memset(&NetworkDevicePassword, 0x00, sizeof(sNetwork_Device_Password_Information));

	if (target) {
		if (is_valid_str_mac(target) == FALSE) {
			return STATUS_CODE_INVALID_MAC_ADDRESS;
		}
		bHasDeviceMac = TRUE;
		strcpy(strDeviceMAC, target);
	}

	if (CMDHelpers_Get_Network_Device_Password(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                                           bHasDeviceMac,strDeviceMAC,&NetworkDevicePassword) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	strcpy(password, NetworkDevicePassword.DevicePassword);
	return STATUS_CODE_OK;
}

int setNetworkDevicePassword(char *target, char *password)
{
	INIT_SDK_VARIABLE;
	if (target) {
		if (is_valid_str_mac(target) == FALSE) {
			return STATUS_CODE_INVALID_MAC_ADDRESS;
		}
		bHasDeviceMac = TRUE;
		strcpy(strDeviceMAC, target);
	}

	if (!password || strlen(password) != GHN_LIB_MAX_PASSWORD_LEN-1) {
		return STATUS_CODE_PARAMETER_ERROR;
	}

	sNetwork_Device_Password_Information			NetworkDevicePassword;
	sApply_Parameters_Setting_Information			ApplyParametersSetting;
	bool											bNeedApplyParametersSetting = FALSE;

	memset(&NetworkDevicePassword, 0x00, sizeof(sNetwork_Device_Password_Information));
    strcpy(NetworkDevicePassword.DevicePassword, password);

	if (CMDHelpers_Set_Network_Device_Password(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                                           bHasDeviceMac,strDeviceMAC,&NetworkDevicePassword) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	if (NetworkDevicePassword.bNeedApplyParameterSetting == TRUE) {
		bNeedApplyParametersSetting = TRUE;
	}

	if (CMDHelpers_Update_Connection_Parameters(NetworkDevicePassword.Connection, &bHasDeviceIP, strDeviceIP, 
	                                            &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	if (bNeedApplyParametersSetting == TRUE) {
		if (CMDHelpers_Apply_Parameters_Setting(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
		                                        bHasDeviceMac,strDeviceMAC,&ApplyParametersSetting) == FALSE) {
			return STATUS_CODE_ISSI_API_FAILE;
		}
	}

	return STATUS_CODE_OK;
}

int getNetworkDomainName(char *target, char *domainName)
{
	if (!domainName || sizeof(domainName) < GHN_LIB_MAX_DOMAIN_NAME_LEN) {
		return STATUS_CODE_RESULT_IS_NULL_OR_SIZE_ERROR;
	}
	memset(domainName, 0x00, sizeof(domainName));

	INIT_SDK_VARIABLE;	

	sNetwork_Domain_Name_Information				NetworkDomainName;
	memset(&NetworkDomainName, 0x00, sizeof(sNetwork_Domain_Name_Information));

	if (target) {
		if (is_valid_str_mac(target) == FALSE) {
			return STATUS_CODE_INVALID_MAC_ADDRESS;
		}
		bHasDeviceMac = TRUE;
		strcpy(strDeviceMAC, target);
	}

	if (CMDHelpers_Get_Network_Domain_Name(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                                       bHasDeviceMac,strDeviceMAC,&NetworkDomainName) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	strcpy(domainName, NetworkDomainName.DomainName);
	return STATUS_CODE_OK;
}

int setNetworkDomainName(char *target, char *domainName)
{
	INIT_SDK_VARIABLE;
	if (target) {
		if (is_valid_str_mac(target) == FALSE) {
			return STATUS_CODE_INVALID_MAC_ADDRESS;
		}
		bHasDeviceMac = TRUE;
		strcpy(strDeviceMAC, target);
	}

	if (!domainName || strlen(domainName) > GHN_LIB_MAX_DOMAIN_NAME_LEN-1) {
		return STATUS_CODE_PARAMETER_ERROR;
	}

	sNetwork_Domain_Name_Information			    NetworkDomainName;
	sApply_Parameters_Setting_Information			ApplyParametersSetting;
	bool											bNeedApplyParametersSetting = FALSE;

	memset(&NetworkDomainName, 0x00, sizeof(sNetwork_Domain_Name_Information));
    strcpy(NetworkDomainName.DomainName, domainName);

	if (CMDHelpers_Set_Network_Domain_Name(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                                           bHasDeviceMac,strDeviceMAC,&NetworkDomainName) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	if (NetworkDomainName.bNeedApplyParameterSetting == TRUE) {
		bNeedApplyParametersSetting = TRUE;
	}

	if (CMDHelpers_Update_Connection_Parameters(NetworkDomainName.Connection, &bHasDeviceIP, strDeviceIP, 
	                                            &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	if (bNeedApplyParametersSetting == TRUE) {
		if (CMDHelpers_Apply_Parameters_Setting(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
		                                        bHasDeviceMac,strDeviceMAC,&ApplyParametersSetting) == FALSE) {
			return STATUS_CODE_ISSI_API_FAILE;
		}
	}

	return STATUS_CODE_OK;
}

int getDeviceName(char *target, char *name)
{
	if (!name || sizeof(name) < GHN_LIB_MAX_DEVICE_NAME_LEN) {
		return STATUS_CODE_RESULT_IS_NULL_OR_SIZE_ERROR;
	}
	memset(name, 0x00, sizeof(name));

	INIT_SDK_VARIABLE;	

	sDevice_Name_Information DeviceName;
	memset(&DeviceName, 0x00, sizeof(sDevice_Name_Information));

	if (target) {
		if (is_valid_str_mac(target) == FALSE) {
			return STATUS_CODE_INVALID_MAC_ADDRESS;
		}
		bHasDeviceMac = TRUE;
		strcpy(strDeviceMAC, target);
	}

	if (CMDHelpers_Get_Device_Name(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                               bHasDeviceMac,strDeviceMAC,&DeviceName) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	strcpy(name, DeviceName.DeviceName);
	return STATUS_CODE_OK;
}

int setDeviceName(char *target, char *name)
{
	INIT_SDK_VARIABLE;
	if (target) {
		if (is_valid_str_mac(target) == FALSE) {
			return STATUS_CODE_INVALID_MAC_ADDRESS;
		}
		bHasDeviceMac = TRUE;
		strcpy(strDeviceMAC, target);
	}

	if (!name || strlen(name) > GHN_LIB_MAX_DEVICE_NAME_LEN-1) {
		return STATUS_CODE_PARAMETER_ERROR;
	}

	sDevice_Name_Information			            DeviceName;
	sApply_Parameters_Setting_Information			ApplyParametersSetting;
	bool											bNeedApplyParametersSetting = FALSE;

	memset(&DeviceName, 0x00, sizeof(sDevice_Name_Information));
    strcpy(DeviceName.DeviceName, name);

	if (CMDHelpers_Set_Device_Name(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
	                               bHasDeviceMac,strDeviceMAC,&DeviceName) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	if (DeviceName.bNeedApplyParameterSetting == TRUE) {
		bNeedApplyParametersSetting = TRUE;
	}

	if (CMDHelpers_Update_Connection_Parameters(DeviceName.Connection, &bHasDeviceIP, strDeviceIP, 
	                                            &bHasAdapterIP, strAdapterIP, &bHasDeviceMac, strDeviceMAC) == FALSE) {
		return STATUS_CODE_ISSI_API_FAILE;
	}

	if (bNeedApplyParametersSetting == TRUE) {
		if (CMDHelpers_Apply_Parameters_Setting(bHasDeviceIP,strDeviceIP,bHasAdapterIP,strAdapterIP,
		                                        bHasDeviceMac,strDeviceMAC,&ApplyParametersSetting) == FALSE) {
			return STATUS_CODE_ISSI_API_FAILE;
		}
	}

	return STATUS_CODE_OK;
}
