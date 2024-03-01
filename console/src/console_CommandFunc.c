
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "console_Helpers.h"
#include "console_CommandFunc.h"

#include "console_PacketHandling.h"
#include "console_Logger.h"


unsigned long Update_MMHeader(UINT8* MMHeader, UINT16 OpCode, UINT16 Length)
{
	UINT8		Reserved1			= 0x00;
	UINT8		Reserved2			= 0x00;
	UINT8		NumberOfSegments	= 0x00;
	UINT8		SegmentNumber		= 0x00;
	UINT16		SegmentSeqNumber	= 0x00;

	memset(MMHeader, 0x00, MMHEADER_SIZE);

	MMHeader[0]  = (Length & 0x00FF);
	MMHeader[1]  = (Length & 0x0F00) >> 8;

	MMHeader[1] |= (OpCode & 0x000F) << 4;
	MMHeader[2]  = (OpCode & 0x0FF0) >> 4;

	MMHeader[3]  = Reserved1;

	MMHeader[4]  = (NumberOfSegments & 0x0F);
	MMHeader[4] |= (SegmentNumber & 0x0F) << 4;
	
	MMHeader[5]  = (SegmentSeqNumber & 0x0FF);
	MMHeader[6]  = (SegmentSeqNumber & 0xFF00) >> 8;

	MMHeader[7]  = Reserved2;

	return 0;
}

unsigned long Init_Ghn_Packet(Layer2Connection* layer2Connection, SGhnPacket* Packet,UINT32 MsgId,int Payload_Length,UINT8 Special_Flags)
{
	memset(Packet,0x00,sizeof(SGhnPacket));

	memcpy(Packet->Layer2Header.au8SrcMACAddr,HandleToConn(layer2Connection->m_eth_handle_t)->host, sizeof(mac_address_t));    
	memcpy(Packet->Layer2Header.au8DestMACAddr, &layer2Connection->m_MAC_Address, sizeof(mac_address_t));    
	Packet->Layer2Header.u16EthType = ETHER_TYPE_GHN;

	Update_MMHeader(&Packet->MMHeader[0], MMHEADER_CLI_VSM_OPCODE, VSM_SPECIAL_FLAGS_1BYTE+CLI_PADDING_1BYTE+GHN_HEADER_BEFORE_PAYLOAD_SIZE+Payload_Length);

	Packet->Special_Flags = Special_Flags;
	Packet->MsgId = MsgId;
	Packet->TransId = layer2Connection->m_transIdCounter++;
	Packet->ErrorCode = CP_e_NO_ERROR;

	return 0;
}

unsigned long Init_Packet_HLD_RSP(	Layer2Connection*	layer2Connection,
									SGhnPacket*			sGhnPacket,
									UINT32				MsgId,
									int					Payload_Length,
									UINT16				TransId,
									UINT16				ErrorCode)
{
	memset(sGhnPacket,0x00,sizeof(SGhnPacket));

	memcpy(sGhnPacket->Layer2Header.au8SrcMACAddr,HandleToConn(layer2Connection->m_eth_handle_t)->host, sizeof(mac_address_t));    
	memcpy(sGhnPacket->Layer2Header.au8DestMACAddr, &layer2Connection->m_MAC_Address, sizeof(mac_address_t));    
	sGhnPacket->Layer2Header.u16EthType = ETHER_TYPE_GHN;

	Update_MMHeader(&sGhnPacket->MMHeader[0], MMHEADER_CLI_VSM_OPCODE, VSM_SPECIAL_FLAGS_1BYTE+CLI_PADDING_1BYTE+GHN_HEADER_BEFORE_PAYLOAD_SIZE+Payload_Length);

	sGhnPacket->MsgId = MsgId;
	sGhnPacket->TransId = TransId;
	sGhnPacket->ErrorCode = ErrorCode;

	return 0;
}

unsigned long SetMemoryCommandFunctionUINT8(Layer2Connection* layer2Connection, UINT32 Address,UINT8 Value)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnSetMemory*		sGhnSetMemory = (SGhnSetMemory*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SET_MEM,2*sizeof(UINT32)+4,0x00);

	sGhnSetMemory->Address = Address;
	sGhnSetMemory->Size = 1;
	memcpy(sGhnSetMemory->Buffer,&Value,1);

	// Support Big/Little endian machines
	sGhnSetMemory->Address = htolel(sGhnSetMemory->Address);
	sGhnSetMemory->Size = htolel(sGhnSetMemory->Size);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

unsigned long SetMemoryCommandFunctionUINT32(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Value)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnSetMemory*		sGhnSetMemory = (SGhnSetMemory*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SET_MEM,2*sizeof(UINT32)+4,0x00);

	sGhnSetMemory->Address = Address;
	sGhnSetMemory->Size = 4;
	memcpy(sGhnSetMemory->Buffer,&Value,4);

	// Support Big/Little endian machines
	sGhnSetMemory->Address = htolel(sGhnSetMemory->Address);
	sGhnSetMemory->Size = htolel(sGhnSetMemory->Size);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

unsigned long SetMemoryCommandFunctionUINT32NoReply(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Value)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnSetMemory*		sGhnSetMemory = (SGhnSetMemory*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SET_MEM,2*sizeof(UINT32)+4,0x00);

	sGhnSetMemory->Address = Address;
	sGhnSetMemory->Size = 4;
	memcpy(sGhnSetMemory->Buffer,&Value,4);

	// Support Big/Little endian machines
	sGhnSetMemory->Address = htolel(sGhnSetMemory->Address);
	sGhnSetMemory->Size = htolel(sGhnSetMemory->Size);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, NULL, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

unsigned long SetMemoryCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size, UINT8* Buffer)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnSetMemory*		sGhnSetMemory;

	UINT32				OffsetIndex = 0;
	UINT32				BlockSize;

	do 
	{
		BlockSize = min(Size,BUFFER_PAYLOAD_LIMIT);

		Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SET_MEM,2*sizeof(UINT32)+BlockSize,0x00);

		sGhnSetMemory = (SGhnSetMemory*)SendPacket.Payload;
		sGhnSetMemory->Address = Address + OffsetIndex;
		sGhnSetMemory->Size = BlockSize;
		memcpy(sGhnSetMemory->Buffer,&Buffer[OffsetIndex],BlockSize);

		// Support Big/Little endian machines
		sGhnSetMemory->Address = htolel(sGhnSetMemory->Address);
		sGhnSetMemory->Size = htolel(sGhnSetMemory->Size);

		status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

		if (status != CG_STAT_SUCCESS)
		{
			return status;
		}

		OffsetIndex = OffsetIndex + BlockSize;
		Size = Size - BlockSize;

	} while (Size>0);

	return status;
}

unsigned long GetMemoryCommandFunctionUINT32(Layer2Connection* layer2Connection, UINT32 Address, UINT32* Value)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnGetMemory*		sGhnGetMemory;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_GET_MEM,2*sizeof(UINT32)+4,0x00);

	sGhnGetMemory = (SGhnGetMemory*)SendPacket.Payload;
	sGhnGetMemory->Address = Address;
	sGhnGetMemory->Size = 4;

	// Support Big/Little endian machines
	sGhnGetMemory->Address = htolel(sGhnGetMemory->Address);
	sGhnGetMemory->Size = htolel(sGhnGetMemory->Size);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	if (status != CG_STAT_SUCCESS)
	{
		return status;
	}

	sGhnGetMemory = (SGhnGetMemory*)ReceivePacket.Payload;

	memcpy(Value,sGhnGetMemory->Buffer,4);	
	
	// Support Big/Little endian machines
	*Value = letohl(*Value);

	return status;
}

unsigned long GetMemoryCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size, UINT8* Buffer)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnGetMemory*		sGhnGetMemory;

	UINT32				OffsetIndex = 0;
	UINT32				BlockSize;

	do 
	{
		BlockSize = min(Size,BUFFER_PAYLOAD_LIMIT);

		Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_GET_MEM,2*sizeof(UINT32)+BlockSize,0x00);

		sGhnGetMemory = (SGhnGetMemory*)SendPacket.Payload;
		sGhnGetMemory->Address = Address + OffsetIndex;
		sGhnGetMemory->Size = BlockSize;

		// Support Big/Little endian machines
		sGhnGetMemory->Address = htolel(sGhnGetMemory->Address);
		sGhnGetMemory->Size = htolel(sGhnGetMemory->Size);

		status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

		if (status != CG_STAT_SUCCESS)
		{
			return status;
		}

		sGhnGetMemory = (SGhnGetMemory*)ReceivePacket.Payload;

		memcpy(&Buffer[OffsetIndex],sGhnGetMemory->Buffer,BlockSize);

		OffsetIndex = OffsetIndex + BlockSize;
		Size = Size - BlockSize;

	} while (Size>0);

	return status;
}

unsigned long SetMemoryCommandFunctionString(Layer2Connection* layer2Connection, UINT32 Address,char* Message)
{
	cg_stat_t				status;
	SGhnPacket				SendPacket;
	SGhnPacket				ReceivePacket;
	SGhnSetMemoryString*	sGhnSetMemoryString = (SGhnSetMemoryString*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SET_MEM,2*sizeof(UINT32)+strlen(Message)+1,0x00);

	sGhnSetMemoryString->Address = Address;
	sGhnSetMemoryString->Size = strlen(Message)+1;
	memset(sGhnSetMemoryString->Value,0x00,sizeof(sGhnSetMemoryString->Value));
	strcpy(sGhnSetMemoryString->Value,Message);

	// Support Big/Little endian machines
	sGhnSetMemoryString->Address = htolel(sGhnSetMemoryString->Address);
	sGhnSetMemoryString->Size = htolel(sGhnSetMemoryString->Size);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

unsigned long GetMemoryCommandFunctionString(Layer2Connection* layer2Connection, UINT32 Address, char* Response, UINT32 MaxResponseTimeOut)
{
	cg_stat_t				status;
	SGhnPacket				SendPacket;
	SGhnPacket				ReceivePacket;
	SGhnGetMemoryString*	sGhnGetMemoryString			= (SGhnGetMemoryString*)SendPacket.Payload;
	SGhnGetMemoryString*	sGhnGetMemoryStringResponse	= (SGhnGetMemoryString*)ReceivePacket.Payload;
	int						Index = 0;
	int						ResponseLength;
	int						bGotTheResponse = FALSE;

	clock_t					StartTime;
	
	StartTime = console_get_msectime();

	while (((UINT32)(console_get_msectime() - StartTime) < MaxResponseTimeOut) && (bGotTheResponse == FALSE))
	{
		Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_GET_MEM,sizeof(SGhnGetMemoryString),0x00);

		sGhnGetMemoryString->Address = Address;
		sGhnGetMemoryString->Size = DEBUGINFO_PAYLOAD_LIMIT;

		// Support Big/Little endian machines
		sGhnGetMemoryString->Address = htolel(sGhnGetMemoryString->Address);
		sGhnGetMemoryString->Size = htolel(sGhnGetMemoryString->Size);

		status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

		if (status != CG_STAT_SUCCESS)
		{
			return status;
		}

		if (isStrTerminal(sGhnGetMemoryStringResponse->Value,DEBUGINFO_PAYLOAD_LIMIT))
		{
			ResponseLength = strlen(sGhnGetMemoryStringResponse->Value);
			strcpy(&Response[Index],sGhnGetMemoryStringResponse->Value);
		}
		else
		{
			ResponseLength = DEBUGINFO_PAYLOAD_LIMIT;
			strncpy(&Response[Index],sGhnGetMemoryStringResponse->Value,ResponseLength);
		}

		LOG_INFO("ResponseLength(%d)",ResponseLength);

		Index += ResponseLength;

		if ((Index > 0) && (ResponseLength < DEBUGINFO_PAYLOAD_LIMIT))
		{
			bGotTheResponse = TRUE;
		}
		else
		{
			// Wait some time for the FW to prepare the response
			OS_Sleep(100);
		}
	}

	return status;
}

void ParseQueryDevice(SGhnQueryDevice* QueryDevice)
{
	char	Buffer[1024];

	sprintf(Buffer,"MAC("MAC_ADDR_FMT") State(%d) Wire(%d)",
		MAC_ADDR(QueryDevice->au8MACAddr),
		QueryDevice->DeviceState,
		QueryDevice->OptionFlags&0x00000001);

	LOG_INFO(Buffer);
}

void ParseQueryDeviceIP(SGhnQueryDeviceIP* QueryDeviceIP)
{
	char	Buffer[1024];
	char	BufferIP[1024];

	sprintf(Buffer,"MAC("MAC_ADDR_FMT") State(%d) Wire(%d) ",
		MAC_ADDR(QueryDeviceIP->au8MACAddr),
		QueryDeviceIP->DeviceState,
		QueryDeviceIP->OptionFlags&0x00000001);

	if (((QueryDeviceIP->OptionFlags & 0x00000006) >> 1) == 0x00)
	{
		sprintf(BufferIP,"IP1(N/A) ");
	}

	if (((QueryDeviceIP->OptionFlags & 0x00000006) >> 1) == 0x01)
	{
		sprintf(BufferIP,"IP1(%d.%d.%d.%d) ",
			QueryDeviceIP->IP1[0],
			QueryDeviceIP->IP1[1],
			QueryDeviceIP->IP1[2],
			QueryDeviceIP->IP1[3]);
	}

	if (((QueryDeviceIP->OptionFlags & 0x00000006) >> 1) == 0x02)
	{
		sprintf(BufferIP,"IP1(%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d) ",
			QueryDeviceIP->IP1[0],
			QueryDeviceIP->IP1[1],
			QueryDeviceIP->IP1[2],
			QueryDeviceIP->IP1[3],
			QueryDeviceIP->IP1[4],
			QueryDeviceIP->IP1[5],
			QueryDeviceIP->IP1[6],
			QueryDeviceIP->IP1[7],
			QueryDeviceIP->IP1[8],
			QueryDeviceIP->IP1[9],
			QueryDeviceIP->IP1[10],
			QueryDeviceIP->IP1[11],
			QueryDeviceIP->IP1[12],
			QueryDeviceIP->IP1[13],
			QueryDeviceIP->IP1[14],
			QueryDeviceIP->IP1[15]);
	}

	strcat(Buffer,BufferIP);

	if (((QueryDeviceIP->OptionFlags & 0x00000018) >> 3) == 0x00)
	{
		sprintf(BufferIP,"IP2(N/A)");
	}

	if (((QueryDeviceIP->OptionFlags & 0x00000018) >> 3) == 0x01)
	{
		sprintf(BufferIP,"IP2(%d.%d.%d.%d)",
			QueryDeviceIP->IP2[0],
			QueryDeviceIP->IP2[1],
			QueryDeviceIP->IP2[2],
			QueryDeviceIP->IP2[3]);
	}

	if (((QueryDeviceIP->OptionFlags & 0x00000018) >> 3) == 0x02)
	{
		sprintf(BufferIP,"IP2(%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d)",
			QueryDeviceIP->IP2[0],
			QueryDeviceIP->IP2[1],
			QueryDeviceIP->IP2[2],
			QueryDeviceIP->IP2[3],
			QueryDeviceIP->IP2[4],
			QueryDeviceIP->IP2[5],
			QueryDeviceIP->IP2[6],
			QueryDeviceIP->IP2[7],
			QueryDeviceIP->IP2[8],
			QueryDeviceIP->IP2[9],
			QueryDeviceIP->IP2[10],
			QueryDeviceIP->IP2[11],
			QueryDeviceIP->IP2[12],
			QueryDeviceIP->IP2[13],
			QueryDeviceIP->IP2[14],
			QueryDeviceIP->IP2[15]);
	}

	strcat(Buffer,BufferIP);

	LOG_INFO(Buffer);
}

/***************************************************************************************************
* QueryDeviceCommandFunction()                                                                     *
*                                                                                                  *
* Query the first local device that reply                                                          *
*                                                                                                  *
***************************************************************************************************/
unsigned long QueryDeviceCommandFunction(Layer2Connection* layer2Connection, SGhnQueryDevice* QueryDevice, bool bQueryFast)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_QUERY_DEVICE,sizeof(SGhnQueryDevice),0x00);

	if (bQueryFast == FALSE)
	{
		status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_QUERY_TIMEOUT, DEV_SHORT_TIMEOUT, TRUE);
	}
	else
	{
		status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_QUERY_FAST_TIMEOUT, DEV_QUERY_FAST_TIMEOUT, FALSE);
	}


	if (status == CG_STAT_SUCCESS)
	{
		memcpy(QueryDevice,ReceivePacket.Payload,sizeof(SGhnQueryDevice));

		ParseQueryDevice(QueryDevice);
	}

	return status;
}

/***************************************************************************************************
* Query_Local_DevicesCommandFunction()                                                             *
*                                                                                                  *
* Query all the local devices information                                                          *
* Skip reply messages that come from remote devices                                                *
*                                                                                                  *
***************************************************************************************************/
unsigned long Query_Local_DevicesCommandFunction(Layer2Connection* layer2Connection, int* Size, sDevice* devices)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnQueryDevice		QueryDevicesTable[64];
	SGhnQueryDevice*	QueryDevice;
	int					QuerySize = 64;
	int					i;
	eDeviceState		DeviceState;
	bool				bLocalDevice;
	ip_address_t		DeviceIP1;
	ip_address_t		DeviceIP2;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_QUERY_DEVICE,sizeof(SGhnQueryDevice),0x00);

	status = Pkt_Query_Devices(layer2Connection, &SendPacket,&QueryDevicesTable[0],&QuerySize);

	if (status != CG_STAT_SUCCESS)
	{
		return status;
	}


	// Build the reply

	*Size = 0x00;;

	for (i=1 ; i <= QuerySize ; i++)
	{
		QueryDevice = &QueryDevicesTable[i-1];

		QueryDevice->OptionFlags = letohl(QueryDevice->OptionFlags);

		ParseQueryDevice(QueryDevice);

		// Skip reply messages that come from remote devices
		if ((QueryDevice->OptionFlags & 0x00000001) == 1)
		{
			continue;
		}

		memcpy(&(devices[*Size].DeviceMac),QueryDevice->au8MACAddr,HMAC_LEN);

		devices[*Size].DeviceState = (eDeviceState)QueryDevice->DeviceState;

		if ((devices[*Size].DeviceState == eDeviceState_BootCode) || 
			(devices[*Size].DeviceState == eDeviceState_B1))
		{
			devices[*Size].DeviceIP1 = 0x00; // N/A
			devices[*Size].DeviceIP2 = 0x00; // N/A
		}
		else // (devices[*Size].DeviceState == eDeviceState_FW)
		{
			// Query the IPs
			if ((status=QueryDeviceIP(layer2Connection, &(devices[*Size].DeviceMac),&DeviceState, &bLocalDevice, &DeviceIP1, &DeviceIP2)) == CG_STAT_SUCCESS)
			{
				devices[*Size].DeviceIP1 = DeviceIP1;
				devices[*Size].DeviceIP2 = DeviceIP2;
			}
			else
			{
				devices[*Size].DeviceIP1 = 0x00; // N/A
				devices[*Size].DeviceIP2 = 0x00; // N/A
			}
		}

		*Size = *Size + 1;
	}

	return CG_STAT_SUCCESS;
}


/***************************************************************************************************
* Query_Network_DevicesCommandFunction()                                                             *
*                                                                                                  *
* Query all the local and remote devices information                                               *
*                                                                                                  *
***************************************************************************************************/
unsigned long Query_Network_DevicesCommandFunction(Layer2Connection* layer2Connection, int* Size, sDevice* devices)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnQueryDevice		QueryDevicesTable[64];
	SGhnQueryDevice*	QueryDevice;
	int					QuerySize = 64;
	int					i;
	eDeviceState		DeviceState;
	bool				bLocalDevice;
	ip_address_t		DeviceIP1;
	ip_address_t		DeviceIP2;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_QUERY_DEVICE,sizeof(SGhnQueryDevice),0x00);

	status = Pkt_Query_Devices(layer2Connection, &SendPacket,&QueryDevicesTable[0],&QuerySize);

	if (status != CG_STAT_SUCCESS)
	{
		return status;
	}


	// Build the reply

	*Size = 0x00;;

	for (i=1 ; i <= QuerySize ; i++)
	{
		QueryDevice = &QueryDevicesTable[i-1];

		QueryDevice->OptionFlags = letohl(QueryDevice->OptionFlags);

		ParseQueryDevice(QueryDevice);

		memcpy(&(devices[*Size].DeviceMac),QueryDevice->au8MACAddr,HMAC_LEN);

		devices[*Size].DeviceState = (eDeviceState)QueryDevice->DeviceState;

		if ((devices[*Size].DeviceState == eDeviceState_BootCode) ||
			(devices[*Size].DeviceState == eDeviceState_B1))
		{
			devices[*Size].DeviceIP1 = 0x00; // N/A
			devices[*Size].DeviceIP2 = 0x00; // N/A
		}
		else // (devices[*Size].DeviceState == eDeviceState_FW)
		{
			// Query the IPs
			if ((status=QueryDeviceIP(layer2Connection, &devices[*Size].DeviceMac,&DeviceState, &bLocalDevice, &DeviceIP1, &DeviceIP2)) == CG_STAT_SUCCESS)
			{
				devices[*Size].DeviceIP1 = DeviceIP1;
				devices[*Size].DeviceIP2 = DeviceIP2;
			}
			else
			{
				devices[*Size].DeviceIP1 = 0x00; // N/A
				devices[*Size].DeviceIP2 = 0x00; // N/A
			}
		}

		*Size = *Size + 1;
	}

	return CG_STAT_SUCCESS;
}

/***************************************************************************************************
* ResetCommandFunction()                                                                           *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long ResetCommandFunction(Layer2Connection* layer2Connection, int NewState)
{
	cg_stat_t				status;
	SGhnPacket				SendPacket;
	SGhnResetCommand*		sGhnResetCommand = (SGhnResetCommand*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_RESET,sizeof(SGhnResetCommand),0x00);

	sGhnResetCommand->State = NewState;

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, NULL, DEV_DEFAULT_TIMEOUT, 0, FALSE);

	return status;
}

/***************************************************************************************************
* QueryDeviceCommandFunction()                                                                     *
*                                                                                                  *
* Query the IP device information of a specific device (according to MAC)                          *
*                                                                                                  *
***************************************************************************************************/
unsigned long QueryDeviceIPCommandFunction(Layer2Connection* layer2Connection, SGhnQueryDeviceIP* QueryDeviceIP)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_QUERY_DEVICE_IP,sizeof(SGhnQueryDeviceIP),0x00);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	if (status == CG_STAT_SUCCESS)
	{
		memcpy(QueryDeviceIP,ReceivePacket.Payload,sizeof(SGhnQueryDeviceIP));

		QueryDeviceIP->OptionFlags = letohl(QueryDeviceIP->OptionFlags);

		ParseQueryDeviceIP(QueryDeviceIP);
	}

	return status;
}

unsigned long SetImagerHeaderCommandFunction(Layer2Connection* layer2Connection, BIN_image_header_t* ptr)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	BIN_image_header_t* BIN_image_header = (BIN_image_header_t*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SET_IMAGE_HEADER,sizeof(BIN_image_header_t),0x00);

	memcpy(BIN_image_header,ptr,sizeof(BIN_image_header_t));

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

unsigned long SetImagerDataCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size,UINT8* ptr)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnSetImageData*	sGhnSetImageData = (SGhnSetImageData*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SET_IMAGE_DATA,2*sizeof(UINT32)+Size,0x00);

	sGhnSetImageData->Address = Address;
	sGhnSetImageData->Size = Size;
	memcpy(sGhnSetImageData->Payload,ptr,Size);

	// Support Big/Little endian machines
	sGhnSetImageData->Address = htolel(sGhnSetImageData->Address);
	sGhnSetImageData->Size = htolel(sGhnSetImageData->Size);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

unsigned long ExecuteCommandCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Value)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnExecuteCommand*	sGhnExecuteCommand = (SGhnExecuteCommand*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_EXECUTE_CMD,sizeof(SGhnExecuteCommand),0x00);

	sGhnExecuteCommand->Address = Address;
	sGhnExecuteCommand->Value = Value;

	// Support Big/Little endian machines
	sGhnExecuteCommand->Address = htolel(sGhnExecuteCommand->Address);
	sGhnExecuteCommand->Value = htolel(sGhnExecuteCommand->Value);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

unsigned long EraseFlashCommandFunction(Layer2Connection* layer2Connection, UINT32 Offset, UINT32 Size)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;	
	SGhnPacket			ReceivePacket;
	SGhnEraseFlash*		sGhnEraseFlash = (SGhnEraseFlash*)SendPacket.Payload;

	LOG_INFO("Offset(0x%08x) Size(0x%08x)", Offset, Size);

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SPI_ERASE,sizeof(SGhnEraseFlash),0x00);

	sGhnEraseFlash->Offset = Offset;
	sGhnEraseFlash->Size = Size;

	// Support Big/Little endian machines
	sGhnEraseFlash->Offset = htolel(sGhnEraseFlash->Offset);
	sGhnEraseFlash->Size = htolel(sGhnEraseFlash->Size);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_ERASE_TOTAL_TIMEOUT, DEV_ERASE_TIMEOUT, FALSE);

	return status;
}

unsigned long EraseFlashCommandFunction_HLD_RSP(Layer2Connection* layer2Connection, UINT32 Offset, UINT32 Size, UINT16 TransId, UINT16 ErrorCode)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;	
	SGhnEraseFlash*		sGhnEraseFlash = (SGhnEraseFlash*)SendPacket.Payload;

	LOG_INFO("Offset(0x%08x) Size(0x%08x)", Offset, Size);

	Init_Packet_HLD_RSP(layer2Connection, &SendPacket,VSM_MSG_HLD_ERASE_BLOCK_RSP,sizeof(SGhnEraseFlash),TransId,ErrorCode);

	sGhnEraseFlash->Offset = Offset;
	sGhnEraseFlash->Size = Size;

	// Support Big/Little endian machines
	sGhnEraseFlash->Offset = htolel(sGhnEraseFlash->Offset);
	sGhnEraseFlash->Size = htolel(sGhnEraseFlash->Size);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, NULL, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

unsigned long SetFlashCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size,UINT8* ptr, bool bContainChecksumLength)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnSetFlash*		sGhnSetFlash = (SGhnSetFlash*)SendPacket.Payload;
	UINT8				Special_Flags = 0x00;

	if (bContainChecksumLength == TRUE)
	{
		Special_Flags |= VSM_SPECIAL_FLAGS_PAYLOAD_CONTAINS_CHECKSUM_AND_LENGTH;
	}

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_WRITE_FLASH,2*sizeof(UINT32)+Size,Special_Flags);

	sGhnSetFlash->Address = Address;
	sGhnSetFlash->Size = Size;

	memcpy(sGhnSetFlash->Payload,ptr,Size);

	// Support Big/Little endian machines
	sGhnSetFlash->Address = htolel(sGhnSetFlash->Address);
	sGhnSetFlash->Size = htolel(sGhnSetFlash->Size);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, 3*DEV_DEFAULT_TIMEOUT, DEV_DEFAULT_TIMEOUT, FALSE);

	return status;
}

unsigned long SetFlashCommandFunction_HLD_RSP(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size, UINT16 TransId, UINT16 ErrorCode)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnSetFlash*		sGhnSetFlash = (SGhnSetFlash*)SendPacket.Payload;

	Init_Packet_HLD_RSP(layer2Connection, &SendPacket,VSM_MSG_HLD_WRITE_BLOCK_RSP,2*sizeof(UINT32)+Size,TransId,ErrorCode);

	sGhnSetFlash->Address = Address;
	sGhnSetFlash->Size = Size;
	
	// Support Big/Little endian machines
	sGhnSetFlash->Address = htolel(sGhnSetFlash->Address);
	sGhnSetFlash->Size = htolel(sGhnSetFlash->Size);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, NULL, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

unsigned long GetFlashCommandFunction(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size,UINT8* ptr)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnGetFlash*		sGhnGetFlash = (SGhnGetFlash*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SPI_READ_MEM,2*sizeof(UINT32)+Size,0x00);

	sGhnGetFlash->Address = Address;
	sGhnGetFlash->Size = Size;

	// Support Big/Little endian machines
	sGhnGetFlash->Address = htolel(sGhnGetFlash->Address);
	sGhnGetFlash->Size = htolel(sGhnGetFlash->Size);

	if ((status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, 3*DEV_DEFAULT_TIMEOUT, DEV_DEFAULT_TIMEOUT, FALSE)) != CG_STAT_SUCCESS)
	{
		return status;
	}

	sGhnGetFlash = (SGhnGetFlash*)ReceivePacket.Payload;

	memcpy(ptr,sGhnGetFlash->Payload,Size);

	return status;
}

unsigned long GetFlashCommandFunction_HLD_RSP(Layer2Connection* layer2Connection, UINT32 Address,UINT32 Size,UINT8* ptr, UINT16 TransId, UINT16 ErrorCode)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnGetFlash*		sGhnGetFlash = (SGhnGetFlash*)SendPacket.Payload;

	Init_Packet_HLD_RSP(layer2Connection, &SendPacket,VSM_MSG_HLD_READ_BLOCK_RSP,2*sizeof(UINT32)+Size,TransId,ErrorCode);

	sGhnGetFlash->Address = Address;
	sGhnGetFlash->Size = Size;

	// Support Big/Little endian machines
	sGhnGetFlash->Address = htolel(sGhnGetFlash->Address);
	sGhnGetFlash->Size = htolel(sGhnGetFlash->Size);

	memcpy(sGhnGetFlash->Payload, ptr, Size);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, NULL, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

// Type is 0=memory or 1=flash
/***************************************************************************************************
* DownloadImageFromFileCommandFunction()                                                           *
*                                                                                                  *
* Used for Updating the FUUF                                                                       *
* After updating, we execute the FUUF                                                              *
*                                                                                                  *
***************************************************************************************************/
unsigned long DownloadImageFromFileCommandFunction(Layer2Connection* layer2Connection, char* FileName)
{
	cg_stat_t				status = CG_STAT_SUCCESS;
	FILE*					file;
	UINT8*					Buffer = NULL;

	UINT32					Size;
	UINT32					Offset;
	UINT32					Address;

	UINT32					SizePacket;
	UINT32					i;

	BIN_image_loader_t*		image;
	long					FileSize;

	LOG_INFO("FileName(%s)",FileName);

	FileSize = file_get_size(FileName);

	Buffer = (UINT8*)malloc(FileSize);
	if (Buffer == NULL)
	{
		printf("Failed to malloc");
		return CG_STAT_INSUFFICIENT_MEMORY;
	}

	image = (BIN_image_loader_t*)Buffer;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Open the file
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	file = fopen(FileName,"rb");
	if (file == NULL)
	{
		printf("Failed to open the file \"%s\"\n", FileName);
		free(Buffer);
		return CG_STAT_FILE_NOT_FOUND;
	}
	fread(Buffer,1,FileSize,file);
	fclose(file);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// SetImagerHeaderCommandFunction
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	LOG_INFO("Call SetImagerHeaderCommandFunction()");
	if ((status = SetImagerHeaderCommandFunction(layer2Connection, (BIN_image_header_t*)&Buffer[0])) != CG_STAT_SUCCESS)
	{
		LOG_INFO("Call SetImagerHeaderCommandFunction() Failed");
		free(Buffer);
		return status;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	for (i=0; i<image->image_header.numOfSections; i++)
	{

		Size = image->image_sections[i].size;		// size of section (should be used by the SET_MEM command)
		Offset = image->image_sections[i].offset;	// offset from start of the image file
		Address = image->image_sections[i].address;	// address to load to (should be used by the SET_MEM command)

		while (Size > 0)
		{
			SizePacket = min(Size,BUFFER_PAYLOAD_LIMIT);

			// Send Packet
			//LOG_INFO(".");
			if ((status = SetImagerDataCommandFunction(layer2Connection, Address,SizePacket,&Buffer[Offset])) != CG_STAT_SUCCESS)
			{
				LOG_INFO("Call SetImagerDataCommandFunction() Failed");
				free(Buffer);
				return status;
			}

			Size = Size - SizePacket;
			Offset = Offset + SizePacket;
			Address = Address + SizePacket;

		}
	}

	free(Buffer);

	return CG_STAT_SUCCESS;
}

// Type is 0=memory or 1=flash
/***************************************************************************************************
* DownloadImageFromBufferCommandFunction()                                                         *
*                                                                                                  *
* Used for Updating the FUUF                                                                       *
* After updating, we execute the FUUF                                                              *
*                                                                                                  *
***************************************************************************************************/
unsigned long DownloadImageFromBufferCommandFunction(	Layer2Connection*	layer2Connection,
														UINT8*				SectionBuffer,
														UINT32				SectionSize)
{
	cg_stat_t				status = CG_STAT_SUCCESS;

	UINT32					Size;
	UINT32					Offset;
	UINT32					Address;

	UINT32					SizePacket;
	UINT32					i;

	BIN_image_loader_t*		image;

	LOG_INFO("SectionSize(0x%04x))", SectionSize);

	image = (BIN_image_loader_t*)SectionBuffer;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// SetImagerHeaderCommandFunction
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	LOG_INFO("Call SetImagerHeaderCommandFunction()");
	if ((status = SetImagerHeaderCommandFunction(layer2Connection, (BIN_image_header_t*)&SectionBuffer[0])) != CG_STAT_SUCCESS)
	{
		LOG_INFO("Call SetImagerHeaderCommandFunction() Failed");
		return status;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	for (i=0; i<image->image_header.numOfSections; i++)
	{
		Size = image->image_sections[i].size;		// size of section (should be used by the SET_MEM command)
		Offset = image->image_sections[i].offset;	// offset from start of the image file
		Address = image->image_sections[i].address;	// address to load to (should be used by the SET_MEM command)

		while (Size > 0)
		{
			SizePacket = min(Size,BUFFER_PAYLOAD_LIMIT);

			// Send Packet
			//LOG_INFO(".");
			if ((status = SetImagerDataCommandFunction(layer2Connection, Address,SizePacket,&SectionBuffer[Offset])) != CG_STAT_SUCCESS)
			{
				LOG_INFO("Call SetImagerDataCommandFunction() Failed");
				return status;
			}

			Size = Size - SizePacket;
			Offset = Offset + SizePacket;
			Address = Address + SizePacket;

		}
	}

	return CG_STAT_SUCCESS;
}

unsigned long SetFlashFromBufferCommandFunction(Layer2Connection*	layer2Connection,
												UINT8*				SectionBuffer,
												UINT32				SectionSize,
												UINT32				Address,
												bool				bContainChecksumLength,
												bool				bUpdateProgressIndication)
{
	cg_stat_t		status = CG_STAT_SUCCESS;
	
	UINT32			Size;
	UINT32			Offset;
	UINT32			SizePacket;
	UINT32			ProgressIndication=0;

	LOG_INFO("SectionSize(0x%04x) Address(0x%08x)", SectionSize, Address);

	Size = SectionSize;
	Offset = 0x00;

	while (Size > 0)
	{
		SizePacket = min(Size,BUFFER_PAYLOAD_LIMIT);

		// Send Packet
		if ((status = SetFlashCommandFunction(layer2Connection, Address,SizePacket,&SectionBuffer[Offset], bContainChecksumLength)) != CG_STAT_SUCCESS)
		{
			if (bUpdateProgressIndication)
			{
				DeleteProgressIndication(layer2Connection->m_MAC_Address);
			}
			//PrintProgressIndication(0,10,0,1);
			LOG_INFO("Call SetFlashCommandFunction() Failed");
			return status;
		}

		Size = Size - SizePacket;
		Offset = Offset + SizePacket;
		Address = Address + SizePacket;

		bContainChecksumLength = FALSE;

		if (bUpdateProgressIndication)
		{
			if (ProgressIndication != (SectionSize-Size) * 100 / SectionSize)
			{
				ProgressIndication = (SectionSize-Size) * 100 / SectionSize;
				//PrintProgressIndication(ProgressIndication,10,bFirstTime, Size==0);
				UpdateProgressIndication(layer2Connection->m_MAC_Address,ProgressIndication);
			}
		}

	}

	if (bUpdateProgressIndication)
	{
		DeleteProgressIndication(layer2Connection->m_MAC_Address);
	}
	
	return CG_STAT_SUCCESS;
}

unsigned long SetFlashFromFileCommandFunction(	Layer2Connection*	layer2Connection,
												char*				FileName,
												UINT32				Address,
												bool				bContainChecksumLength,
												bool				bUpdateProgressIndication)
{
	cg_stat_t		status = CG_STAT_SUCCESS;
	FILE*			file;
	UINT8*			Buffer = NULL;
	UINT32			ByteRead;
	
	UINT32			Size;
	UINT32			Offset;
	UINT32			SizePacket;
	long			FileSize;
	UINT32			ProgressIndication=0;

	LOG_INFO("FileName(%s) Address(0x%08x)", FileName, Address);

	FileSize = file_get_size(FileName);

	Buffer = (UINT8*)malloc(FileSize);
	if (Buffer == NULL)
	{
		printf("Failed to malloc");
		return CG_STAT_INSUFFICIENT_MEMORY;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Open the file
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	file = fopen(FileName,"rb");
	if (file == NULL)
	{
		printf("Failed to open the file \"%s\"\n", FileName);
		free(Buffer);
		return CG_STAT_FILE_NOT_FOUND;
	}
	ByteRead = fread(Buffer,1,FileSize,file);
	fclose(file);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	Size = ByteRead;
	Offset = 0x00;

	while (Size > 0)
	{
		SizePacket = min(Size,BUFFER_PAYLOAD_LIMIT);

		// Send Packet
		if ((status = SetFlashCommandFunction(layer2Connection, Address,SizePacket,&Buffer[Offset],bContainChecksumLength)) != CG_STAT_SUCCESS)
		{
			if (bUpdateProgressIndication)
			{
				DeleteProgressIndication(layer2Connection->m_MAC_Address);
			}
			//PrintProgressIndication(0,10,0,1);
			LOG_INFO("Call SetFlashCommandFunction() Failed");
			return status;
		}

		Size = Size - SizePacket;
		Offset = Offset + SizePacket;
		Address = Address + SizePacket;

		bContainChecksumLength = FALSE;

		if (bUpdateProgressIndication)
		{
			if (ProgressIndication != (ByteRead-Size) * 100 / ByteRead)
			{
				ProgressIndication = (ByteRead-Size) * 100 / ByteRead;
				//PrintProgressIndication(ProgressIndication,10,bFirstTime, Size==0);
				UpdateProgressIndication(layer2Connection->m_MAC_Address,ProgressIndication);
			}
		}

	}

	if (bUpdateProgressIndication)
	{
		DeleteProgressIndication(layer2Connection->m_MAC_Address);
	}

	free(Buffer);
	
	return CG_STAT_SUCCESS;
}

unsigned long GetFlashIntoFileCommandFunction(	Layer2Connection*	layer2Connection,
												UINT32				Address,
												UINT32				Size,
												char*				FileName,
												bool				bTrim_Suffix_Of_0xFF)
{
	cg_stat_t		status = CG_STAT_SUCCESS;
	FILE*			file;
	UINT8*			Buffer = NULL;

	UINT32			Offset;
	UINT32			LeftSize;
	UINT32			SizePacket;
	int				loop;

	LOG_INFO("Address(0x%08x) Size(%d) FileName(%s)", Address, Size, FileName);

	Buffer = (UINT8*)malloc(Size);
	if (Buffer == NULL)
	{
		printf("Failed to malloc");
		return CG_STAT_INSUFFICIENT_MEMORY;
	}

	Offset = 0;

	loop = 1;
	LeftSize = Size;
	while (LeftSize > 0)
	{
		SizePacket = min(LeftSize,BUFFER_PAYLOAD_LIMIT);

		// Send Packet
		//LOG_INFO(".");

		if ((loop++ % 80) == 0)
		{
			//LOG_INFO("\n");
		}

		if ((status = GetFlashCommandFunction(layer2Connection, Address,SizePacket,&Buffer[Offset])) != CG_STAT_SUCCESS)
		{
			LOG_INFO("Call GetFlashCommandFunction() Failed");
			return status;
		}

		LeftSize = LeftSize - SizePacket;
		Offset = Offset + SizePacket;
		Address = Address + SizePacket;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Trim Suffix Of 0xFF from the end of the section
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (bTrim_Suffix_Of_0xFF == TRUE)
	{
		while ((Size > 0) && (Buffer[Size-1] == 0xFF))
		{
			Size--;
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Save into the file
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	file = fopen(FileName,"wb");
	if (file == NULL)
	{
		printf("Failed to open the file \"%s\"\n", FileName);
		free(Buffer);
		return CG_STAT_FILE_NOT_FOUND;
	}
	fwrite(Buffer,1,Size,file);
	fclose(file);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	free(Buffer);

	return CG_STAT_SUCCESS;
}

/***************************************************************************************************
* QueryDeviceIP()                                                                                  *
*                                                                                                  *
* Send a Unicast message to remote device (destination MAC) in order to retrieve the information   *
*                                                                                                  *
***************************************************************************************************/
unsigned long QueryDeviceIP(Layer2Connection* layer2Connection, macStruct* DeviceMac, eDeviceState* DeviceState, bool* LocalDevice, ip_address_t* DeviceIP1, ip_address_t* DeviceIP2)
{
	cg_stat_t			status = CG_STAT_SUCCESS;
	SGhnQueryDeviceIP	QueryDeviceIP;


	memcpy(&layer2Connection->m_MAC_Address, DeviceMac, sizeof(macStruct));

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Query Devices
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if ((status = QueryDeviceIPCommandFunction(layer2Connection, &QueryDeviceIP)) != CG_STAT_SUCCESS)
	{
		LOG_INFO("Call QueryDeviceIPCommandFunction() Failed");
		return status;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	*DeviceState = (eDeviceState)QueryDeviceIP.DeviceState;


	// Check if reply messages came from a local device
	if ((QueryDeviceIP.OptionFlags & 0x00000001) == 0)
	{
		*LocalDevice = TRUE;
	}
	else
	{
		*LocalDevice = FALSE;
	}

	if (((QueryDeviceIP.OptionFlags & 0x00000006) >> 1) == 0x01)
	{
		*DeviceIP1 =	(QueryDeviceIP.IP1[0] <<  0) + (QueryDeviceIP.IP1[1] <<  8) +
						(QueryDeviceIP.IP1[2] << 16) + (QueryDeviceIP.IP1[3] << 24);


		*DeviceIP1 = letohl(*DeviceIP1);

		// If IP is 169.254.0.0
		//if (*DeviceIP1 == 0x0000FEA9)
		//{
		//	*DeviceIP1 = 0x00; // N/A
		//}
	}
	else
	{
		*DeviceIP1 = 0x00; // N/A
	}

	if (((QueryDeviceIP.OptionFlags & 0x00000018) >> 3) == 0x01)
	{
		*DeviceIP2 =	(QueryDeviceIP.IP2[0] <<  0) + (QueryDeviceIP.IP2[1] <<  8) +
						(QueryDeviceIP.IP2[2] << 16) + (QueryDeviceIP.IP2[3] << 24);

		*DeviceIP2 = letohl(*DeviceIP2);

		// If IP is 169.254.0.0
		//if (*DeviceIP2 == 0x0000FEA9)
		//{
		//	*DeviceIP2 = 0x00; // N/A
		//}
	}
	else
	{
		*DeviceIP2 = 0x00; // N/A
	}

	return status;

}


/***************************************************************************************************
* Enter_PHY_ModeCommandFunction()                                                                  *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long Enter_PHY_ModeCommandFunction(Layer2Connection* layer2Connection, UINT32 Data,UINT8 disable_slog,UINT8 sniffing_mode)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnEnterPhyMode*	sGhnEnterPhyMode = (SGhnEnterPhyMode*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_ENTER_PHY_MODE,sizeof(SGhnEnterPhyMode),0x00);

	sGhnEnterPhyMode->Data = Data;
	sGhnEnterPhyMode->disable_slog = disable_slog;
	sGhnEnterPhyMode->sniffing_mode = sniffing_mode;

	// Support Big/Little endian machines
	sGhnEnterPhyMode->Data = htolel(sGhnEnterPhyMode->Data);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

/***************************************************************************************************
* Enter_Remote_FW_UpgradeCommandFunction()                                                         *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long Enter_Remote_FW_UpgradeCommandFunction(Layer2Connection* layer2Connection, UINT8* Status)
{
	cg_stat_t					status;
	SGhnPacket					SendPacket;
	SGhnPacket					ReceivePacket;
	sGhnEnterRemoteFWUpgrade*	sGhnRemoteUpgradeFW = (sGhnEnterRemoteFWUpgrade*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_ENTER_REMOTE_FW_UPGRADE,sizeof(sGhnEnterRemoteFWUpgrade),0x00);

	if ((status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, 3*DEV_DEFAULT_TIMEOUT, DEV_DEFAULT_TIMEOUT, FALSE)) != CG_STAT_SUCCESS)
	{
		return status;
	}

	sGhnRemoteUpgradeFW = (sGhnEnterRemoteFWUpgrade*)ReceivePacket.Payload;

	*Status = sGhnRemoteUpgradeFW->Status;

	return status;
}

/***************************************************************************************************
* PhyInitConfigCommandFunction()                                                                   *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
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
											UINT32				GI_val)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnPhyInitConfig*	PhyInitConfig = (SGhnPhyInitConfig*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_PHY_INIT_CONFIGURATION,sizeof(SGhnPhyInitConfig),0x00);

	PhyInitConfig->PRDThreshold2 = PRDThreshold2;
	PhyInitConfig->PRDThreshold3 = PRDThreshold3;
	PhyInitConfig->MinPowerThreshold = MinPowerThreshold;
	PhyInitConfig->CSTE_backoff = CSTE_backoff;
	PhyInitConfig->BackoffSAT = BackoffSAT;
	PhyInitConfig->gainGuard = gainGuard;
	PhyInitConfig->TRD_Threshold0 = TRD_Threshold0;
	PhyInitConfig->TRD_Threshold1 = TRD_Threshold1;
	PhyInitConfig->TRD_Threshold2 = TRD_Threshold2;
	PhyInitConfig->TRD_FaultNum = TRD_FaultNum;
	PhyInitConfig->GI_val = GI_val;

	// Support Big/Little endian machines
	PhyInitConfig->MinPowerThreshold = htoles(PhyInitConfig->MinPowerThreshold);
	PhyInitConfig->CSTE_backoff = htoles(PhyInitConfig->CSTE_backoff);
	PhyInitConfig->BackoffSAT = htoles(PhyInitConfig->BackoffSAT);
	PhyInitConfig->gainGuard = htoles(PhyInitConfig->gainGuard);
	PhyInitConfig->GI_val = htolel(PhyInitConfig->GI_val);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

/***************************************************************************************************
* PhyStartTXCommandFunction()                                                                      *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long PhyStartTXCommandFunction(Layer2Connection*	layer2Connection,
										UINT32				TX_Mode,
										UINT32				FrameNum,
										UINT16				SymbolsNum,
										UINT16				IFG_Time,
										UINT8				HeaderSymbolNum,
										UINT8				PermanentMask)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnPhyStartTX*		PhyStartTX = (SGhnPhyStartTX*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_PHY_START_TX,sizeof(SGhnPhyStartTX),0x00);

	PhyStartTX->TX_Mode = TX_Mode;
	PhyStartTX->FrameNum = FrameNum;
	PhyStartTX->SymbolsNum = SymbolsNum;
	PhyStartTX->IFG_Time = IFG_Time;
	PhyStartTX->HeaderSymbolNum = HeaderSymbolNum;
	PhyStartTX->PermanentMask = PermanentMask;

	// Support Big/Little endian machines
	PhyStartTX->TX_Mode = htolel(PhyStartTX->TX_Mode);
	PhyStartTX->FrameNum = htolel(PhyStartTX->FrameNum);
	PhyStartTX->SymbolsNum = htoles(PhyStartTX->SymbolsNum);
	PhyStartTX->IFG_Time = htoles(PhyStartTX->IFG_Time);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

/***************************************************************************************************
* PhyStartRXCommandFunction()                                                                      *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long PhyStartRXCommandFunction(Layer2Connection*	layer2Connection,
										UINT32				RX_Mode,
										UINT32				FrameNum,
										UINT16				SymbolsNum,
										UINT8				HeaderSymbolNum,
										UINT8				PermanentMask)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnPhyStartRX*		PhyStartRX = (SGhnPhyStartRX*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_PHY_START_RX,sizeof(SGhnPhyStartRX),0x00);

	PhyStartRX->RX_Mode = RX_Mode;
	PhyStartRX->FrameNum = FrameNum;
	PhyStartRX->SymbolsNum = SymbolsNum;
	PhyStartRX->HeaderSymbolNum = HeaderSymbolNum;
	PhyStartRX->PermanentMask = PermanentMask;

	// Support Big/Little endian machines
	PhyStartRX->RX_Mode = htolel(PhyStartRX->RX_Mode);
	PhyStartRX->FrameNum = htolel(PhyStartRX->FrameNum);
	PhyStartRX->SymbolsNum = htoles(PhyStartRX->SymbolsNum);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

/***************************************************************************************************
* PhyStopTXCommandFunction()                                                                       *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long PhyStopTXCommandFunction(Layer2Connection*	layer2Connection)
{
	cg_stat_t				status;
	SGhnPacket				SendPacket;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_PHY_STOP_TX, 0, 0x00);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, NULL, DEV_DEFAULT_TIMEOUT, 0, FALSE);

	return status;
}

/***************************************************************************************************
* PhyStopRXCommandFunction()                                                                       *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long PhyStopRXCommandFunction(Layer2Connection*	layer2Connection)
{
	cg_stat_t				status;
	SGhnPacket				SendPacket;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_PHY_STOP_RX, 0, 0x00);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, NULL, DEV_DEFAULT_TIMEOUT, 0, FALSE);

	return status;
}

/***************************************************************************************************
* Phy_Start_Line_SnifferCommandFunction()                                                          *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long Phy_Start_Line_SnifferCommandFunction(Layer2Connection* layer2Connection,
													UINT32 offset,
													UINT32 gap,
													UINT8 gain_pn,
													UINT8 gain_ng,
													UINT16 Nbuff,
													UINT8 Nrep)
{
	cg_stat_t					status;
	SGhnPacket					SendPacket;
	SGhnPhyStartLineSniffer*	sGhnPhyStartLineSniffer = (SGhnPhyStartLineSniffer*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_PHY_START_LINE_SNIFFING,sizeof(SGhnPhyStartLineSniffer),0x00);

	sGhnPhyStartLineSniffer->offset = offset;
	sGhnPhyStartLineSniffer->gap = gap;
	sGhnPhyStartLineSniffer->gain_pn = gain_pn;
	sGhnPhyStartLineSniffer->gain_ng = gain_ng;
	sGhnPhyStartLineSniffer->Nbuff = Nbuff;
	sGhnPhyStartLineSniffer->Nrep = Nrep;

	// Support Big/Little endian machines
	sGhnPhyStartLineSniffer->offset = htolel(sGhnPhyStartLineSniffer->offset);
	sGhnPhyStartLineSniffer->gap = htolel(sGhnPhyStartLineSniffer->gap);
	sGhnPhyStartLineSniffer->Nbuff = htoles(sGhnPhyStartLineSniffer->Nbuff);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, NULL, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}


/***************************************************************************************************
* Enter_PHY_ModeCommandFunction()                                                                  *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long GetFwStateCommandFunction(Layer2Connection* layer2Connection, SGhnGetActiveFWState* sGhnGetActiveFWState)
{
	cg_stat_t				status;
	SGhnPacket				SendPacket;
	SGhnPacket				ReceivePacket;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_UM_GET_FW_STATE,sizeof(SGhnGetActiveFWState),0x00);

	if ((status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE)) != CG_STAT_SUCCESS)
	{
		return status;
	}

	memcpy(sGhnGetActiveFWState,ReceivePacket.Payload,sizeof(SGhnGetActiveFWState));

	return status;
}

/***************************************************************************************************
* RestoreFactoryDefaultCommandFunction()                                                           *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long RestoreFactoryDefaultCommandFunction(Layer2Connection* layer2Connection)
{
	cg_stat_t				status;
	SGhnPacket				SendPacket;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_RESTORE_DEFAULT_FACTORY, 0,0x00);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, NULL, DEV_DEFAULT_TIMEOUT, 0, FALSE);

	return status;
}

/***************************************************************************************************
* PairDeviceCommandFunction()                                                                      *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long PairDeviceCommandFunction(Layer2Connection* layer2Connection)
{
	cg_stat_t				status;
	SGhnPacket				SendPacket;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_Pair_Device, 0,0x00);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, NULL, DEV_DEFAULT_TIMEOUT, 0, FALSE);

	return status;
}

/***************************************************************************************************
* UnpairDeviceCommandFunction()                                                                *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long UnpairDeviceCommandFunction(Layer2Connection* layer2Connection)
{
	cg_stat_t				status;
	SGhnPacket				SendPacket;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_UnPair_Device, 0,0x00);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, NULL, DEV_DEFAULT_TIMEOUT, 0, FALSE);

	return status;
}

/***************************************************************************************************
* Set_Device_ModeCommandFunction()                                                                 *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long SetDeviceModeCommandFunction(Layer2Connection* layer2Connection, UINT8 DeviceMode)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnSetDeviceMode*	SSetDeviceMode = (SGhnSetDeviceMode*)SendPacket.Payload;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_PHY_SET_MODE,sizeof(SGhnSetDeviceMode),0x00);

	SSetDeviceMode->dev_mode = DeviceMode;

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

/***************************************************************************************************
* SlogStartCaptureCommandFunction()                                                                *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long SlogStartCaptureCommandFunction(Layer2Connection*	layer2Connection)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SLOG_INIT,sizeof(SGhnPhyStartTX),0x00);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

/***************************************************************************************************
* SlogStopCaptureCommandFunction()                                                                 *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
unsigned long SlogStopCaptureCommandFunction(Layer2Connection*	layer2Connection)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SLOG_HALT,sizeof(SGhnPhyStartTX),0x00);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

/***************************************************************************************************
* Get_DC_Calibration_VectorCommandFunction()                                                       *
*                                                                                                  *
* Get the DC Calibration Vector from the FW                                                        *
* The DC Calibration is done when FW launched                                                      *
*                                                                                                  *
***************************************************************************************************/
unsigned long Get_DC_Calibration_VectorCommandFunction(Layer2Connection* layer2Connection, VSM_msg_get_dc_calibration_t* DC_Calibration)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_GET_DC_CALIBRATION_VECTOR,sizeof(VSM_msg_get_dc_calibration_t),0x00);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	if (status == CG_STAT_SUCCESS)
	{
		memcpy(DC_Calibration,ReceivePacket.Payload,sizeof(VSM_msg_get_dc_calibration_t));
	}

	return status;
}

/***************************************************************************************************
* Set_Drop_packet_CommandFunction()                                                                *
*                                                                                                  *
* Override the FW mode if it should drop packets or not                                            *
*                                                                                                  *
***************************************************************************************************/
unsigned long Set_Drop_packets_CommandFunction(Layer2Connection* layer2Connection, bool bShouldDrop)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnDropPacketsReq*	DropPacketsReq;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_DROP_PACKETS_REQ,sizeof(SGhnDropPacketsReq),0x00);

	DropPacketsReq = (SGhnDropPacketsReq*)SendPacket.Payload;
	
	if (bShouldDrop ==  TRUE)
	{
		// Drop Packets
		DropPacketsReq->Value = 1;
	}
	else
	{
		// Forward Packets
		DropPacketsReq->Value = 0;
	}

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket,&ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

/***************************************************************************************************
* Get_NTCLK_CommandFunction()                                                                      *
*                                                                                                  *
* Read the NTCLK Value from the device                                                             *
*                                                                                                  *
***************************************************************************************************/
unsigned long Get_NTCLK_CommandFunction(Layer2Connection* layer2Connection, UINT32* NTCLK)
{
	cg_stat_t			status;
	SGhnPacket			SendPacket;
	SGhnPacket			ReceivePacket;
	SGhnGetNTCLKDrop*	GetNTCLK;

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_GET_NTCLK, 0,0x00);

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	if (status != CG_STAT_SUCCESS)
	{
		return status;
	}

	GetNTCLK = (SGhnGetNTCLKDrop*)ReceivePacket.Payload;

	*NTCLK = GetNTCLK->NTCLK;

	return status;
}

/***************************************************************************************************
* Set_Devices_DID_CommandFunction()                                                                *
*                                                                                                  *
***************************************************************************************************/
unsigned long Set_Devices_DID_CommandFunction(	Layer2Connection*	layer2Connection,
												UINT8				GhnDMDeviceID,
												UINT8				NumbersOfDevices,
												SGhnStaticEntry		Table[BPL_MAX_SUPPORTED_DEVICES])
{
	cg_stat_t						status;
	SGhnPacket						SendPacket;
	SGhnPacket						ReceivePacket;
	SGhnSetDevicesDID*				SetDevicesDID;
	int								Payload_Length;

	if (NumbersOfDevices == 0)
	{
		// Clear Devices DID
		Payload_Length = 0;
	}
	else
	{
		// Set Devices DID
		Payload_Length = sizeof(UINT8) + sizeof(UINT8) + NumbersOfDevices*(sizeof(SGhnStaticEntry));
	}

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SET_DEVICES_DID, Payload_Length,0x00);

	if (NumbersOfDevices > 0)
	{
		SetDevicesDID = (SGhnSetDevicesDID*)SendPacket.Payload;

		SetDevicesDID->GhnDMDeviceID = GhnDMDeviceID;
		SetDevicesDID->NumbersOfDevices = NumbersOfDevices;
		memcpy(&SetDevicesDID->Table[0], &Table[0], sizeof(SetDevicesDID->Table));
	}

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

/***************************************************************************************************
* Set_Static_Routing_Topology_CommandFunction()                                                    *
*                                                                                                  *
***************************************************************************************************/
unsigned long Set_Static_Routing_Topology_CommandFunction(	Layer2Connection*	layer2Connection,
															UINT8				Reset_On_Calibration_End,
															UINT32				Calibration_NTCLK,
															UINT16				TTL,
															UINT8				NumbersOfDevices,
															SGhnStaticEntry		Table[BPL_MAX_SUPPORTED_DEVICES])
{
	cg_stat_t						status;
	SGhnPacket						SendPacket;
	SGhnPacket						ReceivePacket;
	SGhnSetStaticRoutingTopology*	SetStatic;
	int								Payload_Length;

	Payload_Length = sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT32) + sizeof(UINT16) + sizeof(UINT8) + NumbersOfDevices*(sizeof(SGhnStaticEntry));

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SET_STATIC_ROUTING_TOPOLOGY, Payload_Length,0x00);

	SetStatic = (SGhnSetStaticRoutingTopology*)SendPacket.Payload;

	SetStatic->Version = 1;
	SetStatic->Reset_On_Calibration_End = Reset_On_Calibration_End;
	SetStatic->Calibration_NTCLK = Calibration_NTCLK;
	SetStatic->TTL = TTL;
	SetStatic->NumbersOfDevices = NumbersOfDevices;
	memcpy(&SetStatic->Table[0], &Table[0], sizeof(SetStatic->Table));

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}

/***************************************************************************************************
* Set_Static_Network_Topology_CommandFunction()                                                    *
*                                                                                                  *
***************************************************************************************************/
unsigned long Set_Static_Network_Topology_CommandFunction(	Layer2Connection*		layer2Connection,
															UINT8					NumbersOfDevices,
															SGhnStaticNetworkEntry	Table[BPL_STATIC_MAX_SUPPORTED_DEVICES])
{
	cg_stat_t						status;
	SGhnPacket						SendPacket;
	SGhnPacket						ReceivePacket;
	SGhnSetStaticNetworkTopology*	SetStaticNetwork;
	int								Payload_Length;

	Payload_Length = sizeof(UINT8) + NumbersOfDevices*(sizeof(SGhnStaticNetworkEntry));

	Init_Ghn_Packet(layer2Connection, &SendPacket,VSM_MSG_SET_STATIC_NETWORK_TOPOLOGY, Payload_Length,0x00);

	SetStaticNetwork = (SGhnSetStaticNetworkTopology*)SendPacket.Payload;

	SetStaticNetwork->NumbersOfDevices = NumbersOfDevices;
	memcpy(&SetStaticNetwork->Table[0], &Table[0], sizeof(SetStaticNetwork->Table));

	status = Pkt_Send_Receive(layer2Connection, ePacketType_Ghn, &SendPacket, &ReceivePacket, DEV_DEFAULT_TIMEOUT, DEV_SHORT_TIMEOUT, FALSE);

	return status;
}
