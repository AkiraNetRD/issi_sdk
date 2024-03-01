#ifndef console_PacketHandling_h__
#define console_PacketHandling_h__

#include "console_typedefs.h"

#include "cgi.h"

typedef enum
{
	ePacketType_Ghn			= 0,
	ePacketType_SLOG		= 2,
} ePacketType;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


unsigned long TxPkt(Layer2Connection* layer2Connection, ePacketType PacketType, void* Packet);
unsigned long RxPkt(Layer2Connection* layer2Connection, ePacketType PacketType, void* Packet,int xi_timeout);

unsigned long Pkt_Send_Receive(	Layer2Connection*	layer2Connection,
								ePacketType			PacketType,
								void*				SendPacket,
								void*				ReceivePacket,
								int					xi_total_timeout,
								int					xi_timeout,
								bool				xi_bMustUseRetransmission);

unsigned long Pkt_Query_Devices(Layer2Connection* layer2Connection, SGhnPacket* SendPacket, SGhnQueryDevice* QueryDevices, int* Size);
unsigned long Pkt_Receive(Layer2Connection* layer2Connection, ePacketType PacketType, void* ReceivePacket, int xi_timeout);
unsigned long Pkt_Receive_SLOG(Layer2Connection* layer2Connection, SGhnSlogPacket* ReceivePacket, int xi_timeout);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
