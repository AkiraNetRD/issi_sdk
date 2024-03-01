#ifndef GHN_LIB_INT_CONSTS_H
#define GHN_LIB_INT_CONSTS_H


#define TOTAL_SYMBOL_DURATION_IN_MICRO_SECONDS (40.96+6.4)
#define PHY_MAX_INTERVALS			6
#define MAX_BIT_ALLOCATION_TABLE_SIZE	4096


#define Node_DeviceInfo							"Device.DeviceInfo"
#define Node_IP									"Device.IP"
#define Node_IP_Interface						"Device.IP.Interface"
#define Node_Time								"Device.Time"
#define Node_LANConfigSecurity					"Device.LANConfigSecurity"
#define Node_Interface							"Device.Ghn.Interface"
#define Node_Interface_DeviceName				"Device.Ghn.Interface.Name"
#define Node_Interface_Stats					"Device.Ghn.Interface.Stats"
#define Node_Interface_X_00C5D9_Stats			"Device.Ghn.Interface.X_00C5D9_Stats"
#define Node_Interface_PeriodicStatistics		"Device.Ghn.Interface.X_00C5D9_PeriodicStatistics"
#define Node_Interface_X_00C5D9_PhyDiagConfig	"Device.Ghn.Interface.X_00C5D9_PhyDiagConfig"
#define Node_Interface_Debug_Coex				"Device.Ghn.Interface.X_00C5D9_Debug.Coex"
#define Node_Interface_Debug_Coex_Selected		"Device.Ghn.Interface.X_00C5D9_Debug.Coex.Selected"
#define Node_Interface_Debug_Coex_Transmitter	"Device.Ghn.Interface.X_00C5D9_Debug.Coex.Transmitter"
#define Node_Interface_Debug_GHN_NN				"Device.Ghn.Interface.X_00C5D9_Debug.GHN_NN"
#define Node_Interface_X_00C5D9_BPL_URT			"Device.Ghn.Interface.X_00C5D9_BPL.URT"
#define Node_Interface_X_00C5D9_BPL_BRT			"Device.Ghn.Interface.X_00C5D9_BPL.BRT"
#define Node_Interface_1_AssociatedDevice		"Device.Ghn.Interface.1.AssociatedDevice"
#define Node_VLAN			                    "Device.X_00C5D9_VLAN"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Netinf
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define Node_PhyDiagReceiverMAC					"Device.Ghn.Interface.1.X_00C5D9_PhyDiagConfig.PhyDiagReceiverMAC"
#define Node_PhyDiagTrafficBurstSize			"Device.Ghn.Interface.1.X_00C5D9_PhyDiagConfig.PhyDiagTrafficBurstSize"
#define Node_PhyDiagTrafficTimeOut				"Device.Ghn.Interface.1.X_00C5D9_PhyDiagConfig.PhyDiagTrafficTimeOut"
#define Node_TrafficIsAcked						"Device.Ghn.Interface.1.X_00C5D9_PhyDiagConfig.TrafficIsAcked"
#define Node_StartPhyDiagTest					"Device.Ghn.Interface.1.X_00C5D9_PhyDiagConfig.StartPhyDiagTest"
#define Node_OperationResult					"Device.Ghn.Interface.1.X_00C5D9_PhyDiagConfig.OperationResult"
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define Nodes_Interface_GhnMACAddress			"Device.Ghn.Interface.*.GhnMACAddress"
#define Nodes_Interface_GhnDeviceID				"Device.Ghn.Interface.*.GhnDeviceID"
#define Nodes_AssociatedDevice_GhnMACAddress	"Device.Ghn.Interface.*.AssociatedDevice.*.GhnMACAddress"
#define Nodes_AssociatedDevice_GhnDeviceID		"Device.Ghn.Interface.*.AssociatedDevice.*.GhnDeviceID"

#define Nodes_Interface_Stats					"Device.Ghn.Interface.*.Stats"
#define Nodes_Interface_X_00C5D9_Stats			"Device.Ghn.Interface.*.X_00C5D9_Stats"
#define Nodes_Interface_Debug					"Device.Ghn.Interface.*.X_00C5D9_Debug"
#define Nodes_Interface_Pblock					"Device.Ghn.Interface.*.X_00C5D9_Debug.Pblock"
#define Nodes_Interface_Debug_Coex				"Device.Ghn.Interface.*.X_00C5D9_Debug.Coex"
#define Nodes_Interface_Debug_GHN_NN			"Device.Ghn.Interface.*.X_00C5D9_Debug.GHN_NN"
#define Nodes_Interface_X_00C5D9_BPL			"Device.Ghn.Interface.*.X_00C5D9_BPL"
#define Nodes_Interface_PeriodicStatistics		"Device.Ghn.Interface.*.X_00C5D9_PeriodicStatistics"
#define Nodes_Interface_PhyDiagConfig			"Device.Ghn.Interface.*.X_00C5D9_PhyDiagConfig"
#define Nodes_Interface_Alarms					"Device.Ghn.Interface.*.X_00C5D9_Alarms"
#define Nodes_Interface_NN						"Device.Ghn.Interface.*.X_00C5D9_NN"

#define Nodes_Interface_AssociatedDevice		"Device.Ghn.Interface.*.AssociatedDevice"
#define Nodes_IntervalCEParameters				"Device.Ghn.Interface.*.AssociatedDevice.*.IntervalCEParameters"
#define Nodes_BitLoadingTable					"Device.Ghn.Interface.*.AssociatedDevice.*.IntervalCEParameters.*.BitLoadTable"

#define Node_ap_stat_ResponseStatus				"ap-stat.ResponseStatus"

// Security Parameters
#define Node_X_00C5D9_EncryptionStatus			"X_00C5D9_EncryptionStatus"
#define Node_GhnNetworkPassword					"GhnNetworkPassword"
#define Node_DomainName							"DomainName"

// Power Save Mode Parameters
#define X_00C5D9_PowerSaveModeStatus			"X_00C5D9_PowerSaveModeStatus"
#define X_00C5D9_PowerSaveModeLinkDownTimer		"X_00C5D9_PowerSaveModeLinkDownTimer"
#define X_00C5D9_PowerSaveModeNoTrafficTimer	"X_00C5D9_PowerSaveModeNoTrafficTimer"

// Master Selection Mode
#define X_00C5D9_AMSMode						"X_00C5D9_AMSMode"

// Change Device Name							
#define X_00C5D9_DeviceName						"X_00C5D9_DeviceName"

// Coexistence Support
#define Node_Coex_Enable						"CoexEnable"
#define Node_Coex_Active_Threshold				"CoexActiveThreshold"

// Utilization Support
#define X_00C5D9_Utilization_Field				"X_00C5D9_Utilization_Field"
#define X_00C5D9_Utilization_Alpha_Field		"X_00C5D9_Utilization_Alpha_Field"

#define X_00C5D9_T0_Time_sec					"X_00C5D9_T0_Time_sec"

#endif // GHN_LIB_CONSTS_H
