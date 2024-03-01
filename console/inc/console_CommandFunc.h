#ifndef console_CommandFunc_h__
#define console_CommandFunc_h__


#include "console_typedefs.h"

#include "cdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	unsigned long SetMemoryCommandFunctionUINT8(Layer2Connection* layer2Connection, UINT32 Address,UINT8 Value);
	unsigned long SetMemoryCommandFunctionUINT32(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Value);
	unsigned long SetMemoryCommandFunctionUINT32NoReply(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Value);
	unsigned long SetMemoryCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size, UINT8* Buffer);

	unsigned long GetMemoryCommandFunctionUINT32(Layer2Connection* layer2Connection, UINT32 Address, UINT32* Value);
	unsigned long GetMemoryCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size, UINT8* Buffer);

	// DebugInfo
	unsigned long SetMemoryCommandFunctionString(Layer2Connection* layer2Connection, UINT32 Address,char* Message);
	unsigned long GetMemoryCommandFunctionString(Layer2Connection* layer2Connection, UINT32 Address, char* Response, UINT32 MaxResponseTimeOut);
	
	unsigned long ResetCommandFunction(Layer2Connection* layer2Connection, int NewState);

	unsigned long QueryDeviceCommandFunction(Layer2Connection* layer2Connection, SGhnQueryDevice* QueryDevice, bool bQueryFast);

	unsigned long Query_Local_DevicesCommandFunction(Layer2Connection* layer2Connection, int* Size, sDevice* devices);
	unsigned long Query_Network_DevicesCommandFunction(Layer2Connection* layer2Connection, int* Size, sDevice* devices);

	unsigned long QueryDeviceIPCommandFunction(Layer2Connection* layer2Connection, SGhnQueryDeviceIP* QueryDeviceIP);
	unsigned long SetImagerHeaderCommandFunction(Layer2Connection* layer2Connection, BIN_image_header_t* ptr);
	unsigned long SetImagerDataCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size,UINT8 * ptr);
	unsigned long ExecuteCommandCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Value);
	
	unsigned long EraseFlashCommandFunction(Layer2Connection* layer2Connection, UINT32 Offset, UINT32 Size);
	unsigned long EraseFlashCommandFunction_HLD_RSP(Layer2Connection* layer2Connection, UINT32 Offset, UINT32 Size, UINT16 TransId, UINT16 ErrorCode);
	
	unsigned long SetFlashCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size,UINT8* ptr, bool bContainChecksumLength);
	unsigned long SetFlashCommandFunction_HLD_RSP(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size, UINT16 TransId, UINT16 ErrorCode);
	
	unsigned long GetFlashCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size,UINT8* ptr);
	unsigned long GetFlashCommandFunction_HLD_RSP(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size,UINT8* ptr, UINT16 TransId, UINT16 ErrorCode);


	unsigned long DownloadImageFromFileCommandFunction(Layer2Connection* layer2Connection, char* FileName);

	unsigned long DownloadImageFromBufferCommandFunction(	Layer2Connection*	layer2Connection,
															UINT8*				SectionBuffer,
															UINT32				SectionSize);
	
	unsigned long SetFlashFromBufferCommandFunction(Layer2Connection*	layer2Connection,
													UINT8*				SectionBuffer,
													UINT32				SectionSize,
													UINT32				Address,
													bool				bContainChecksumLength,
													bool				bUpdateProgressIndication);

	unsigned long SetFlashFromFileCommandFunction(	Layer2Connection*	layer2Connection,
													char*				FileName,
													UINT32				Address,
													bool				bContainChecksumLength,
													bool				bUpdateProgressIndication);

	unsigned long GetFlashIntoFileCommandFunction(	Layer2Connection*	layer2Connection,
													UINT32				Address,
													UINT32				Size,
													char*				FileName,
													bool				bTrim_Suffix_Of_0xFF);

	unsigned long QueryDeviceIP(Layer2Connection* layer2Connection, macStruct* DeviceMac, eDeviceState* DeviceState, bool* LocalDevice, ip_address_t* DeviceIP1, ip_address_t* DeviceIP2);

	unsigned long Enter_PHY_ModeCommandFunction(Layer2Connection* layer2Connection, UINT32 Data,UINT8 disable_slog,UINT8 sniffing_mode);

	unsigned long Enter_Remote_FW_UpgradeCommandFunction(Layer2Connection* layer2Connection, UINT8* Status);

	unsigned long PhyInitConfigCommandFunction(	Layer2Connection*	layer2Connection,
												UINT8				PRDThreshold2,
												UINT8				PRDThreshold3,
												UINT16				MinPowerThreshold,
												UINT16				CSTE_backoff,
												UINT16				BackoffSAT,
												UINT16				gainGuard,
												INT8				TRD_Threshold0,
												INT8				TRD_Threshold1,
												INT8				TRD_Threshold2,
												UINT8				TRD_FaultNum,
												UINT32				GI_val);

	unsigned long PhyStartTXCommandFunction(Layer2Connection*	layer2Connection,
											UINT32				TX_Mode,
											UINT32				FrameNum,
											UINT16				SymbolsNum,
											UINT16				IFG_Time,
											UINT8				HeaderSymbolNum,
											UINT8				PermanentMask);

	unsigned long PhyStartRXCommandFunction(Layer2Connection*	layer2Connection,
											UINT32				RX_Mode,
											UINT32				FrameNum,
											UINT16				SymbolsNum,
											UINT8				HeaderSymbolNum,
											UINT8				PermanentMask);

	unsigned long PhyStopTXCommandFunction(Layer2Connection*	layer2Connection);
	unsigned long PhyStopRXCommandFunction(Layer2Connection*	layer2Connection);

	unsigned long Phy_Start_Line_SnifferCommandFunction(Layer2Connection* layer2Connection,
														UINT32 offset,
														UINT32 gap,
														UINT8 gain_pn,
														UINT8 gain_ng,
														UINT16 Nbuff,
														UINT8 Nrep);

	unsigned long GetFwStateCommandFunction(Layer2Connection* layer2Connection, SGhnGetActiveFWState* sGhnGetActiveFWState);

	unsigned long RestoreFactoryDefaultCommandFunction(Layer2Connection* layer2Connection);
	unsigned long PairDeviceCommandFunction(Layer2Connection* layer2Connection);
	unsigned long UnpairDeviceCommandFunction(Layer2Connection* layer2Connection);

	unsigned long SetDeviceModeCommandFunction(Layer2Connection* layer2Connection, UINT8 DeviceMode);

	// Slog Support
	unsigned long SlogStartCaptureCommandFunction(Layer2Connection*	layer2Connection);
	unsigned long SlogStopCaptureCommandFunction(Layer2Connection*	layer2Connection);

	unsigned long Get_DC_Calibration_VectorCommandFunction(Layer2Connection* layer2Connection, VSM_msg_get_dc_calibration_t* DC_Calibration);

	unsigned long Set_Drop_packets_CommandFunction(Layer2Connection* layer2Connection, bool bShouldDrop);

	// BPL-Support
	unsigned long Get_NTCLK_CommandFunction(Layer2Connection* layer2Connection, UINT32* NTCLK);
	unsigned long Set_Devices_DID_CommandFunction(	Layer2Connection*	layer2Connection,
													UINT8				GhnDMDeviceID,
													UINT8				NumbersOfDevices,
													SGhnStaticEntry		Table[BPL_MAX_SUPPORTED_DEVICES]);
	unsigned long Set_Static_Routing_Topology_CommandFunction(	Layer2Connection*	layer2Connection,
																UINT8				Reset_On_Calibration_End,
																UINT32				Calibration_NTCLK,
																UINT16				TTL,
																UINT8				NumbersOfDevices,
																SGhnStaticEntry		Table[BPL_MAX_SUPPORTED_DEVICES]);
	unsigned long Set_Static_Network_Topology_CommandFunction(	Layer2Connection*		layer2Connection,
																UINT8					NumbersOfDevices,
																SGhnStaticNetworkEntry	Table[BPL_STATIC_MAX_SUPPORTED_DEVICES]);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
