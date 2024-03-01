
#ifndef _GHN_LIB_EXT_H
#define _GHN_LIB_EXT_H

#include "GHN_LIB_typedefs.h"

#ifdef _WIN32
#define DllExport   __declspec( dllexport ) 
#else
#define DllExport
#endif

#ifdef __cplusplus
extern "C" {
#endif

// SDK High-Level API Functions
DllExport eGHN_LIB_STAT Ghn_Get_Stations(sGet_stations_Information* getstation);
DllExport eGHN_LIB_STAT Ghn_Get_Device_Information(sDevice_Information* devinf);
DllExport eGHN_LIB_STAT Ghn_Reset(sReset_Information* reset);
DllExport eGHN_LIB_STAT Ghn_Netinf(sNetinf_Information* netinf);
DllExport eGHN_LIB_STAT Ghn_Stop_Netinf();
DllExport eGHN_LIB_STAT Ghn_Upgrade_Firmware(sUpgrade_Firmware_Information* upgradeFirmware);
DllExport eGHN_LIB_STAT Ghn_Validate_Chip_Type(sValidate_Chip_Type_Information* chipType);
DllExport eGHN_LIB_STAT Ghn_Get_PHY_Rate_Table_Information(sGet_PHY_Rate_Table_Information* get_PHY_Rate);
DllExport int Ghn_Get_Netinf_Entire_Network_Progress_Inication();
DllExport eGHN_LIB_STAT Ghn_Run_Netinf_Entire_Network(sNetinf_Entire_Network_Information* netinf_Entire);

// SDK Helpers API Functions
DllExport eGHN_LIB_STAT Ghn_Get_Status_Description(eGHN_LIB_STAT status, char* StatusDescription);
DllExport eGHN_LIB_STAT Ghn_Get_Adapter_List(sAdapter_Information* adapterinformation);
DllExport eGHN_LIB_STAT Ghn_Query_Device_State(sGet_Device_State_Information* deviceState);
DllExport eGHN_LIB_STAT Ghn_Query_Local_Devices(sGet_Local_Devices_Information* localdevices);
DllExport eGHN_LIB_STAT Ghn_Query_Network_Devices(sGet_Local_Devices_Information* localdevices);
DllExport eGHN_LIB_STAT Ghn_Get_Network_Interface_By_Device_IP(ip_address_t xi_DeviceIP,ip_address_t* xo_NetworkCard);
DllExport eGHN_LIB_STAT Ghn_Get_Image_Header_From_File(sGet_Image_Header_From_File_Information* getImageHeader);

// Security Parameters
DllExport eGHN_LIB_STAT Ghn_Get_Network_Encryption_Mode(sNetwork_Encryption_Mode_Information* NetworkEncryptionMode);
DllExport eGHN_LIB_STAT Ghn_Set_Network_Encryption_Mode(sNetwork_Encryption_Mode_Information* NetworkEncryptionMode);
DllExport eGHN_LIB_STAT Ghn_Get_Network_Device_Password(sNetwork_Device_Password_Information* NetworkDevicePassword);
DllExport eGHN_LIB_STAT Ghn_Set_Network_Device_Password(sNetwork_Device_Password_Information* NetworkDevicePassword);
DllExport eGHN_LIB_STAT Ghn_Get_Network_Domain_Name(sNetwork_Domain_Name_Information* NetworkDomainName);
DllExport eGHN_LIB_STAT Ghn_Set_Network_Domain_Name(sNetwork_Domain_Name_Information* NetworkDomainName);

// Power Save Mode Parameters
DllExport eGHN_LIB_STAT Ghn_Get_Power_Save_Mode_Status(sPower_Save_Mode_Status_Information* PowerSaveModeStatus);
DllExport eGHN_LIB_STAT Ghn_Set_Power_Save_Mode_Status(sPower_Save_Mode_Status_Information* PowerSaveModeStatus);
DllExport eGHN_LIB_STAT Ghn_Get_Power_Save_Mode_Link_Down_Timer(sPower_Save_Mode_Link_Down_Timer_Information* PowerSaveModeLinkDownTimer);
DllExport eGHN_LIB_STAT Ghn_Set_Power_Save_Mode_Link_Down_Timer(sPower_Save_Mode_Link_Down_Timer_Information* PowerSaveModeLinkDownTimer);
DllExport eGHN_LIB_STAT Ghn_Get_Power_Save_Mode_No_Traffic_Timer(sPower_Save_Mode_No_Traffic_Timer_Information* PowerSaveModeNoTrafficTimer);
DllExport eGHN_LIB_STAT Ghn_Set_Power_Save_Mode_No_Traffic_Timer(sPower_Save_Mode_No_Traffic_Timer_Information* PowerSaveModeNoTrafficTimer);

// Master Selection Mode
DllExport eGHN_LIB_STAT Ghn_Get_Master_Selection_Mode(sMaster_Selection_Mode_Information* MasterSelectionMode);
DllExport eGHN_LIB_STAT Ghn_Set_Master_Selection_Mode(sMaster_Selection_Mode_Information* MasterSelectionMode);

// Change Device Name
DllExport eGHN_LIB_STAT Ghn_Get_Device_Name(sDevice_Name_Information* DeviceName);
DllExport eGHN_LIB_STAT Ghn_Set_Device_Name(sDevice_Name_Information* DeviceName);

// Coexistence Support
DllExport eGHN_LIB_STAT Ghn_Get_Coexistence_Mode(sCoexistence_Mode_Information* CoexistenceMode);
DllExport eGHN_LIB_STAT Ghn_Set_Coexistence_Mode(sCoexistence_Mode_Information* CoexistenceMode);
DllExport eGHN_LIB_STAT Ghn_Get_Coexistence_Threshold(sCoexistence_Threshold_Information* CoexistenceThreshold);
DllExport eGHN_LIB_STAT Ghn_Set_Coexistence_Threshold(sCoexistence_Threshold_Information* CoexistenceThreshold);

// Utilization Support
DllExport eGHN_LIB_STAT Ghn_Get_Utilization_Field(sUtilization_Field_Information* utilization_Field);
DllExport eGHN_LIB_STAT Ghn_Get_Utilization_Alpha_Field(sUtilization_Field_Alpha_Information* utilization_Alpha_Field);
DllExport eGHN_LIB_STAT Ghn_Set_Utilization_Alpha_Field(sUtilization_Field_Alpha_Information* utilization_Alpha_Field);

DllExport eGHN_LIB_STAT Ghn_Get_T0_Timer(sT0_Timer_Information* t0_Timer);
DllExport eGHN_LIB_STAT Ghn_Get_GHN_NN_Number_Of_Interference(sGHN_NN_Number_Of_Interference* GHN_NN_NumInterference);

// Restore Factory Default
DllExport eGHN_LIB_STAT Ghn_Restore_Factory_Default(sRestore_Factory_Default_Information* RestoreFactoryDefault);

// Pairing
DllExport eGHN_LIB_STAT Ghn_Pair_Device(sPair_Device_Information* PairDevice);
DllExport eGHN_LIB_STAT Ghn_Unpair_Device(sUnpair_Device_Information* UnpairDevice);

// Link Statistics
DllExport eGHN_LIB_STAT Ghn_Link_Statistics(sLink_Statistics_Information* linkStatistics);

// Apply Parameters Setting
DllExport eGHN_LIB_STAT Ghn_Apply_Parameters_Setting(sApply_Parameters_Setting_Information* ApplyParametersSetting);

// Local Hosts MAC-Addresses
DllExport eGHN_LIB_STAT Ghn_Local_Hosts_MAC_Addresses(sLocal_Hosts_MAC_Addresses_Information* LocalHosts);

// Get Data Model
DllExport eGHN_LIB_STAT Ghn_Get_BBT_Data_Model(sGet_BBT_Data_Model_Information* getBBTDataModel);

// Get Chip-Type
DllExport eGHN_LIB_STAT Ghn_Get_Chip_Type_Information(sGet_Chip_Type_Information* getChipTypeInformation);

// G.hn BPL Topology Support
DllExport eGHN_LIB_STAT Ghn_Get_BPL_Topology_Information(sBPL_Topology_Information* BPL_Info);
DllExport eGHN_LIB_STAT Ghn_Get_BPL_Topology_Information_From_XML_File(sBPL_Topology_XML_File_Information* BPL_Info);

#ifdef __cplusplus
}
#endif

#endif /* _GHN_LIB_EXT_H */
