
#include <stdio.h>
#include <time.h>

#include "console_PacketHandling.h"
#include "console_Logger.h"
#include "console_Helpers.h"
#include "console_CommandFunc.h"


unsigned long TxPkt(Layer2Connection* layer2Connection, ePacketType PacketType, void* Packet)
{	
	cg_stat_t           status;
	int					length;

	SGhnPacket*			Packet_Ghn;
	UINT16				MMHeader_Length;

	switch(PacketType)
	{
		case ePacketType_Ghn:
		{
			Packet_Ghn = (SGhnPacket*)Packet;

			LOG_INFO("MAC("MAC_ADDR_FMT") MsgId(0x%08x) TransId(0x%04x)", MAC_ADDR(Packet_Ghn->Layer2Header.au8DestMACAddr), Packet_Ghn->MsgId, Packet_Ghn->TransId);

			MMHeader_Length = MMHEADER_GET_LENGTH(Packet_Ghn->MMHeader);
			length = sizeof(SLayer2Header) + MMHEADER_SIZE + MMHeader_Length;

			// Support Big/Little endian machines 
			Packet_Ghn->Layer2Header.u16EthType = htobes(Packet_Ghn->Layer2Header.u16EthType);

			Packet_Ghn->MsgId = htolel(Packet_Ghn->MsgId);
			Packet_Ghn->TransId = htoles(Packet_Ghn->TransId);
			Packet_Ghn->ErrorCode = htoles(Packet_Ghn->ErrorCode);

		} break;

		default:
		{
			return CG_STAT_FAILURE;
		}
	}

	// Send the packet
	status = ETH_tx(layer2Connection->m_eth_handle_t, Packet, length);

	switch(PacketType)
	{
		case ePacketType_Ghn:
		{
			// Support Big/Little endian machines 
			Packet_Ghn->Layer2Header.u16EthType = betohs(Packet_Ghn->Layer2Header.u16EthType);

			Packet_Ghn->MsgId = letohl(Packet_Ghn->MsgId);
			Packet_Ghn->TransId = letohs(Packet_Ghn->TransId);
			Packet_Ghn->ErrorCode = letohs(Packet_Ghn->ErrorCode);
		} break;

		default:
		{
			return CG_STAT_FAILURE;
		}
	}
	
	if (status != CG_STAT_SUCCESS)
	{
		printf("TxPkt() ETH_tx failed\n");
	}

	return status;
}

// Will return failure in case we get the wrong packet
unsigned long RxPkt(Layer2Connection* layer2Connection, ePacketType PacketType, void* Packet, int xi_timeout)
{
	cg_stat_t			status = CG_STAT_TIMEOUT;

	UINT8				Buffer[MAX_ETH_PACKET_SIZE];
	UINT16				size;

	SLayer2Header*		Layer2Header;						// Ethernet Layer 2 Header
	UINT8*				MMHeader;
	UINT16				Opcode;

	SGhnPacket*			Packet_Ghn;
	SGhnSlogPacket*		Packet_SLOG;

	size = sizeof(Buffer);

	if (ETH_rx(layer2Connection->m_eth_handle_t, (unsigned char*)Buffer, &size,xi_timeout) != CG_STAT_SUCCESS)
	{
		return status;
	}

	Layer2Header = (SLayer2Header*)&Buffer[0];
	
	// Support Big/Little endian machines
	Layer2Header->u16EthType = betohs(Layer2Header->u16EthType);
		
	if (Layer2Header->u16EthType == ETHER_TYPE_GHN) 
	{
		MMHeader = (UINT8*)&Buffer[sizeof(SLayer2Header)];
		Opcode = MMHEADER_GET_OPCODE(MMHeader);

		if ((Opcode == MMHEADER_CLI_VSM_OPCODE) && 
			(PacketType == ePacketType_Ghn))
		{
			Packet_Ghn = (SGhnPacket*)&Buffer[0];

			// Support Big/Little endian machines
			Packet_Ghn->MsgId = letohl(Packet_Ghn->MsgId);
			Packet_Ghn->TransId = letohs(Packet_Ghn->TransId);
			Packet_Ghn->ErrorCode = letohs(Packet_Ghn->ErrorCode);

			LOG_INFO("MAC("MAC_ADDR_FMT") MsgId(0x%08x) TransId(0x%04x) ErrorCode(0x%04x)", MAC_ADDR(Packet_Ghn->Layer2Header.au8SrcMACAddr), Packet_Ghn->MsgId, Packet_Ghn->TransId, Packet_Ghn->ErrorCode);
					
			memcpy(Packet, Packet_Ghn, sizeof(SGhnPacket));

			return CG_STAT_SUCCESS;
		}
	}

	if (Layer2Header->u16EthType == ETHER_TYPE_SLOG) 
	{
		Packet_SLOG = (SGhnSlogPacket*)&Buffer[0];

		// Support Big/Little endian machines
		Packet_SLOG->SequenceNumber = letohl(Packet_SLOG->SequenceNumber);
		Packet_SLOG->PayloadSize = letohl(Packet_SLOG->PayloadSize);

		LOG_INFO("MAC("MAC_ADDR_FMT") SequenceNumber(0x%08x) PayloadSize(0x%04x)\n",
				MAC_ADDR(Packet_SLOG->Layer2Header.au8SrcMACAddr), Packet_SLOG->SequenceNumber, Packet_SLOG->PayloadSize);

		memcpy(Packet, Packet_SLOG, sizeof(SGhnSlogPacket));

		return CG_STAT_SUCCESS;

	}

	return CG_STAT_FAILURE;
}

/***************************************************************************************************
* Pkt_Send_Receive()                                                                               *
*                                                                                                  *
* SendPacket - Packet to send to the device                                                        *
* ReceivePacket - Reply form the device                                                            *
*                                                                                                  *
* In case ReceivePacket is NULL: Send the packet and don't wait for reply                          *
*                                                                                                  *
***************************************************************************************************/
unsigned long Pkt_Send_Receive(	Layer2Connection*	layer2Connection,
								ePacketType			PacketType,
								void*				SendPacket,
								void*				ReceivePacket,
								int					xi_total_timeout,
								int					xi_timeout,
								bool				xi_bMustUseRetransmission)
{
	cg_stat_t			status		= CG_STAT_TIMEOUT;

	int					LeftRetries;				// Left retries
	bool				FirstTime	= TRUE;
	clock_t				StartTime;
	macStruct			BroadcastDeviceMac = { {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF} };
	
	int					TimeoutPerLoop;				// In milliseconds
	int					LeftTimeoutPerLoop;			// In milliseconds

#ifdef CONSOLE_NO_LAYER2_RETRANSMISSION
	if ((xi_timeout > 0) && (xi_bMustUseRetransmission == FALSE))
	{
		// Disable the retransmission and use the toal timeout as our timeout for the loop
		xi_total_timeout = xi_timeout;
		xi_timeout = 0;
	}
#endif

	if (xi_timeout > 0)
	{
		// Use retransmissions
		LeftRetries = xi_total_timeout / xi_timeout;

		TimeoutPerLoop = xi_timeout; // in milliseconds
	}
	else
	{
		// Without retransmissions
		LeftRetries = 1;

		TimeoutPerLoop = xi_total_timeout; // in milliseconds
	}

	while (LeftRetries > 0)
	{
		LeftRetries--;

		if ((FirstTime) || (xi_timeout>0))
		{
			TxPkt(layer2Connection, PacketType, SendPacket);
		}

		if (ReceivePacket == NULL)
		{
			// Send the packet and don't wait for reply
			status = CG_STAT_SUCCESS;
			return status;
		}
		
		StartTime = console_get_msectime();

		// Try to get the reply for "DEV_SHORT_TIMEOUT" milliseconds before sending retransmission
		while ((LeftTimeoutPerLoop = (console_get_msectime() - StartTime)) < (TimeoutPerLoop))
		{
			// try to read packet
			if (RxPkt(layer2Connection, PacketType, (void*)ReceivePacket, TimeoutPerLoop - LeftTimeoutPerLoop) == CG_STAT_SUCCESS)
			{
				if (PacketType == ePacketType_Ghn)
				{
					SGhnPacket* Packet_Send = (SGhnPacket*)SendPacket;
					SGhnPacket* Packet_Receive = (SGhnPacket*)ReceivePacket;

					// If we send a Broadcast msg, allow incoming packets from any MAC-Address
					// If we send a Unicast msg, allow incoming packets only from the relevant MAC-Address
					if ((memcmp(&(Packet_Send->Layer2Header.au8DestMACAddr[0]),&(BroadcastDeviceMac.macAddress[0]),HMAC_LEN) == 0) ||
						(memcmp(&(Packet_Send->Layer2Header.au8DestMACAddr[0]),&(Packet_Receive->Layer2Header.au8SrcMACAddr[0]),HMAC_LEN)==0))
					{
						// Try to get the corresponding result (same TransId)
						if (Packet_Send->TransId == Packet_Receive->TransId)
						{
							if (Packet_Receive->ErrorCode == CP_e_NO_ERROR)
							{
								return CG_STAT_SUCCESS;
							}

							return CG_STAT_FAILURE;
						}
					}
				}
			}
		}

		FirstTime = FALSE;
	}

	return status;
}

/***************************************************************************************************
* Pkt_Query_Devices()                                                                              *
*                                                                                                  *
* SendPacket - Packet to send to the device                                                        *
*                                                                                                  *
***************************************************************************************************/
unsigned long Pkt_Query_Devices(Layer2Connection* layer2Connection, SGhnPacket* SendPacket, SGhnQueryDevice* QueryDevices, int* Size)
{
	cg_stat_t			status		= CG_STAT_TIMEOUT;
	SGhnPacket			ReceivePacket;
	int					MaxSize = *Size;
	const int			MaxRetries = 5;				// Max retries
	int					NumberOfRetries;			// send ping-request again if no ping-reply answer from device
	clock_t				StartTime, WaitTime;

	*Size = 0x00;

	NumberOfRetries = MaxRetries;

	while (NumberOfRetries >0)
	{
		NumberOfRetries--;

		TxPkt(layer2Connection, ePacketType_Ghn, SendPacket);

		WaitTime = DEV_SHORT_TIMEOUT;
		StartTime = console_get_msectime();

		// Try to get the corresponding result (same TransId)
		while (RxPkt(layer2Connection, ePacketType_Ghn, (void*)&ReceivePacket,WaitTime) == CG_STAT_SUCCESS)
		{
			if (SendPacket->TransId == ReceivePacket.TransId)
			{
				// Got Ping-reply

				if (((*Size)+1) > MaxSize)
				{
					return CG_STAT_BUFFER_TOO_SMALL;
				}

				// Copy the information from the reply
				memcpy(&QueryDevices[*Size],ReceivePacket.Payload,sizeof(SGhnQueryDevice));
				(*Size)++;

				NumberOfRetries = 0;
				WaitTime = DEV_SHORT_TIMEOUT;
			}
			else
			{
				WaitTime -= (console_get_msectime() - StartTime);

				if (WaitTime <= 0)
				{
					break;
				}
			}
			
			StartTime = console_get_msectime();
		}
	}

	if (*Size > 0)
	{
		status = CG_STAT_SUCCESS;
	}

	return status;
}

/***************************************************************************************************
* Pkt_Receive()                                                                                    *
*                                                                                                  *
* ReceivePacket - Reply form the device                                                            *
*                                                                                                  *
* xi_timeout (In milliseconds)                                                                     *
***************************************************************************************************/
unsigned long Pkt_Receive(Layer2Connection* layer2Connection, ePacketType PacketType, void* ReceivePacket, int xi_timeout)
{
	cg_stat_t			status		= CG_STAT_TIMEOUT;

	// try to read packet
	if (RxPkt(layer2Connection, PacketType, (void*)ReceivePacket, xi_timeout) == CG_STAT_SUCCESS)
	{
		return CG_STAT_SUCCESS;
	}

	return status;
}
