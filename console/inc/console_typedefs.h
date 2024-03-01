#ifndef console_typedefs_h__
#define console_typedefs_h__

#include "console_defs.h"
#include "cdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/********************************************************************************
								MACROS
********************************************************************************/

#define HMAC_LEN					6
#define MAC_ADDR_FMT				"%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_ADDR(p)					(p)[0],(p)[1],(p)[2],(p)[3],(p)[4],(p)[5]

#define NID_ADDR_FMT				"%02x:%02x:%02x:%02x:%02x:%02x:%02x"
#define NID_ADDR(p)					(p)[0],(p)[1],(p)[2],(p)[3],(p)[4],(p)[5],(p)[6]

#define SIG_FMT						"%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"
#define SIG_ADDR(p)					(p)[0],(p)[1],(p)[2],(p)[3],(p)[4],(p)[5],(p)[6],(p)[7],\
									(p)[8],(p)[9],(p)[10],(p)[11],(p)[12],(p)[13],(p)[14],(p)[15]

#define ETHER_TYPE_GHN					0x22E3
#define ETHER_TYPE_SLOG					0x22E4

#define	GHN_HEADER_BEFORE_PAYLOAD_SIZE	8

#define	VSM_SPECIAL_FLAGS_1BYTE			1
#define	CLI_PADDING_1BYTE				1
#define MAX_ETH_PACKET_SIZE				1514
#define MSG_PAYLOAD_LIMIT				1460
#define MSG_PAYLOAD_LIMIT_SLOG			1472
#define BUFFER_PAYLOAD_LIMIT			(1460 - 8)
#define DEBUGINFO_PAYLOAD_LIMIT			1000

// MMHeader
#define MMHEADER_CLI_VSM_OPCODE			0xFAC
#define	MMHEADER_SIZE					8
#define MMHEADER_GET_LENGTH(Buffer)		(((Buffer[0] & 0xFF) << 0) + (((Buffer[1] & 0x0F) << 8)))
#define MMHEADER_GET_OPCODE(Buffer)		(((Buffer[1] & 0xF0) >> 4) + (((Buffer[2] & 0xFF) << 4)))

#define IPV6_LEN						16

#define VSM_SPECIAL_FLAGS_PAYLOAD_CONTAINS_CHECKSUM_AND_LENGTH 0x01

#define MAX_ETHERNET_PKT_SIZE 1518
#define ETHERNET_PKT_HEADER_LEN 14

typedef enum
{
	VSM_MSG_SET_MEM =1,								// 1
	VSM_MSG_GET_MEM,								// 2
	VSM_MSG_QUERY_DEVICE,							// 3
	VSM_MSG_RESET,									// 4
	VSM_MSG_DELETE_DEFAULT_LOG_FILE,				// 5
	VSM_MSG_FILL_MEM,								// 6
	VSM_LOG_SET_LEVEL,								// 7
	VSM_LOG_GET_LEVEL,								// 8
	VSM_LOG_SET_ENABLE,								// 9
	VSM_LOG_GET_ENABLE,								// 10
	VSM_MSG_PHY_MODE,								// 11 place saver
	VSM_MSG_ARICENT_CMD_2,							// 12 place saver
	VSM_MSG_DOWNLOAD_IMAGE,							// 13
	VSM_MSG_SET_IMAGE_HEADER,						// 14 (0x0E)
	VSM_MSG_WRITE_FLASH,							// 15 (0x0F) VSM_MSG_SPI_WRITE_MEM
	VSM_MSG_EXECUTE_CMD,							// 16 (0x10)
	VSM_MSG_SPI_ERASE,								// 17 (0x11)
	VSM_MSG_SET_MAC_ADDR,							// 18
	VSM_MSG_SET_IMAGE_DATA,							// 19 (0x13)
	VSM_MSG_SLOG_FULL_CONFIG,						// 20
	VSM_MSG_SLOG_OTF_CONFIG,						// 21
	VSM_MSG_SLOG_DUMP_OCLA,							// 22
	VSM_MSG_SLOG_HALT,								// 23 (0x17) Slog Halt MSG
	VSM_MSG_START_MAP_TX,							// 24
	VSM_MSG_STOP_MAP_TX,    						// 25
	VSM_MSG_SPI_READ_MEM,							// 26 (0x1A)
	VSM_MSG_UM_GET_FW_STATE = 75,					// 75 (0x4B)

	VSM_MSG_QUERY_DEVICE_IP = 200,					// 200 (0xC8)
	VSM_MSG_SLOG_INIT = 201,						// 201 (0xC9) Slog Handshake MSG
	VSM_MSG_ENTER_PHY_MODE = 202,
	VSM_MSG_RESTORE_DEFAULT_FACTORY = 203,			// 203 (0xCB)
	VSM_MSG_Pair_Device = 204,						// 204 (VSM_MSG_ENTER_TO_SECURE_MODE)
	VSM_MSG_UnPair_Device = 205,					// 205 (VSM_MSG_ENTER_TO_UNSECURE_MODE)
	VSM_MSG_ENTER_REMOTE_FW_UPGRADE = 206,			// 206 (0xCE)

	// PHY VSM Messages
	VSM_MSG_PHY_START_LINE_SNIFFING = 0x110010,
	VSM_MSG_PHY_INIT_CONFIGURATION = 207,
	VSM_MSG_PHY_START_TX = 208,
	VSM_MSG_PHY_STOP_TX = 209,
	VSM_MSG_PHY_START_RX = 210,
	VSM_MSG_PHY_STOP_RX = 211,
	VSM_MSG_PHY_SET_MODE = 212,
	VSM_MSG_GET_DC_CALIBRATION_VECTOR = 214,
	VSM_MSG_DROP_PACKETS_REQ = 215,

	VSM_MSG_GET_NTCLK = 216,						// 216 (0xD8) Query from the local device
	VSM_MSG_SET_DEVICES_DID  = 217,					// 217 (0xD9) Set only the DM device
	VSM_MSG_SET_STATIC_ROUTING_TOPOLOGY = 218,		// 218 (0xDA) Set for every device in the network
	VSM_MSG_SET_STATIC_NETWORK_TOPOLOGY = 219,		// 219 (0xDB) Set only the DM device (Set Static Network Configuration)

	// Host-Loaded VSM Messages
	VSM_MSG_HLD_READ_BLOCK_IND = 300,				// (0x12C) Device -> HostLoaded (Read-Block Flash-File Request)
	VSM_MSG_HLD_READ_BLOCK_RSP = 301,				// (0x12D) HostLoaded -> Device (Read-Block Flash-File Reply)
	
	VSM_MSG_HLD_WRITE_BLOCK_IND = 302,				// (0x12E) Device -> HostLoaded (Write-Block Flash-File Request)
	VSM_MSG_HLD_WRITE_BLOCK_RSP = 303,				// (0x12F) HostLoaded -> Device (Write-Block Flash-File Reply)

	VSM_MSG_HLD_ERASE_BLOCK_IND = 304,				// (0x130) Device -> HostLoaded (Erase Block Flash-File Request)
	VSM_MSG_HLD_ERASE_BLOCK_RSP = 305,				// (0x131) HostLoaded -> Device (Erase Block Flash-File Reply)

	VSM_MSG_SLOG_HANDSHAKE		= 0x00FE0001,		// Slog Handshake Indication from the G.hn device

} VSM_MSG;


// STRAP_PINS
#define STRAP_PINS_L_REG                0x6000002C
#define STRAP_PINS_H_REG				0x6000007C
#define SYS_BIT(n)						(0x1<<(n))
#define STRAP_PINS_H_REG_EN_NV			4   // 1 = enable non-volatile memory


typedef struct
{
	eth_handle_t	m_eth_handle_t;
	macStruct		m_MAC_Address;
	UINT32			m_transIdCounter;
} Layer2Connection;


#if CONSOLE_HOST_LOADED_SUPPORT
typedef enum
{
	eDeviceState_BootCode	= 0,
	eDeviceState_B1			= 1,
	eDeviceState_FW			= 2
} eDeviceState;
#else
typedef enum
{
	eDeviceState_BootCode	= 0,
	eDeviceState_B1			= 0xFF,	/* Dummy */
	eDeviceState_FW			= 1
} eDeviceState;

#endif

typedef enum
{
	CP_e_NO_ERROR,
	CP_e_INVALID_VALUE,
	CP_e_INVALID_REQ,
	CP_e_MEM_ERROR,
	CP_e_INVALID_MODE,
	CP_e_INTERNAL_ERROR,
	CP_e_RSP_MAX_LEN_EXCEEDED,
	CP_e_MAX_ERROR
} CP_cli_error_codes_t;

typedef struct
{
	macStruct		DeviceMac;
	eDeviceState	DeviceState;
	ip_address_t	DeviceIP1;
	ip_address_t	DeviceIP2;
} sDevice;

// Ethernet Layer 2 Header Structure
PACKED(
struct SLayer2Header
{
	UINT8			au8DestMACAddr[HMAC_LEN];			// Destination MAC address
	UINT8			au8SrcMACAddr[HMAC_LEN];			// Source MAC address
	UINT16			u16EthType;							// Ethernet type
}); typedef struct SLayer2Header SLayer2Header;

/*
PACKED(
struct SMMHeader
{
	UINT32			Length				: 12;
	UINT32			Opcode				: 12;
	UINT32			Reserved1			: 8;
	UINT32			NumberOfSegment		: 4;
	UINT32			SegmentNumber		: 4;
	UINT32			SegmentSeqNumber	: 16;
	UINT32			rep_num				: 4;
	UINT32			fsb_val				: 1;
	UINT32			Reserved2			: 3;
}); typedef struct SMMHeader SMMHeader;
*/


// G.HN Packet Structure
PACKED(
struct SGhnPacket
{
	SLayer2Header	Layer2Header;						// Ethernet Layer 2 Header
	UINT8			MMHeader[MMHEADER_SIZE];			// MM-Header

	UINT8			Special_Flags;						// VSM_SPECIAL_FLAGS_PAYLOAD_CONTAINS_CHECKSUM_AND_LENGTH
	UINT8			Padding[CLI_PADDING_1BYTE];			// Padding

	UINT32			MsgId;								// Message Id
	UINT16			TransId;							// Transaction Id
	UINT16			ErrorCode;							// Error Code

	UINT8			Payload[MSG_PAYLOAD_LIMIT];			// Ethernet payload
}); typedef struct SGhnPacket SGhnPacket;

// G.HN SLOG Packet Structure
PACKED(
struct SGhnSlogPacket
{
	SLayer2Header	Layer2Header;						// Ethernet Layer 2 Header

	UINT16			Padding;							// Always Zero
	UINT32			SequenceNumber;
	UINT32			PayloadSize;						// Slog Payload Size (In Bytes)
	UINT8			Payload[MSG_PAYLOAD_LIMIT_SLOG];	// Slog Payload Buffer
}); typedef struct SGhnSlogPacket SGhnSlogPacket;

// G.HN QueryDevice Structure "VSM_MSG_QUERY_DEVICE"
PACKED(
struct SGhnQueryDevice
{
	UINT8		au8MACAddr[HMAC_LEN];			// Device MAC address
	UINT8		DeviceState;					// Device State
	UINT8		Reserved;						// Reserved
	UINT32		OptionFlags;					// Option Flags
												// Bit[0]    : Transfer from wire
												// Bit[3-31] : Reserved
}); typedef struct SGhnQueryDevice SGhnQueryDevice;

// G.HN QueryDeviceIP Structure "VSM_MSG_QUERY_DEVICE_IP"
PACKED(
struct SGhnQueryDeviceIP
{
	UINT8		au8MACAddr[HMAC_LEN];			// Device MAC address
	UINT8		DeviceState;					// Device State
	UINT8		Reserved;						// Reserved
	UINT32		OptionFlags;					// Option Flags
												// Bit[0]    : Transfer from wire
												// Bit[1-2]  : IP-Address 1 type
												// Bit[3-4]  : IP-Address 2 type
												//             IP-Address Types:
												//                0 - Device IP address Not-Available
												//                1 - Device IPv4 address
												//                2 - Device IPv6 address
												// Bit[5-31] : Reserved
      UINT8		IP1[IPV6_LEN];					// Device IP address 1 (IPv4/IPv6)
      UINT8		IP2[IPV6_LEN];					// Device IP address 2 (IPv4/IPv6)

}); typedef struct SGhnQueryDeviceIP SGhnQueryDeviceIP;

// G.HN Set-Memory Structure
PACKED(
struct SGhnSetMemory
{
	UINT32		Address;						// Address
	UINT32		Size;							// Size
	UINT32		Buffer[BUFFER_PAYLOAD_LIMIT];	// Buffer
}); typedef struct SGhnSetMemory SGhnSetMemory;

// G.HN Get-Memory Structure
PACKED(
struct SGhnGetMemory
{
	UINT32		Address;						// Address
	UINT32		Size;							// Size
	UINT32		Buffer[BUFFER_PAYLOAD_LIMIT];	// Buffer
}); typedef struct SGhnGetMemory SGhnGetMemory;

// G.HN Set-Memory String Structure
PACKED(
struct SGhnSetMemoryString
{
	UINT32		Address;						// Address
	UINT32		Size;							// Size
	char		Value[256];						// Value
}); typedef struct SGhnSetMemoryString SGhnSetMemoryString;

// G.HN Get-Memory String Structure
PACKED(
struct SGhnGetMemoryString
{
	UINT32		Address;						// Address
	UINT32		Size;							// Size
	char		Value[DEBUGINFO_PAYLOAD_LIMIT];	// Value
}); typedef struct SGhnGetMemoryString SGhnGetMemoryString;

// G.HN Set-Image Data Structure
PACKED(
struct SGhnSetImageData
{
	UINT32		Address;						// Address
	UINT32		Size;							// Size
	UINT8		Payload[BUFFER_PAYLOAD_LIMIT];	// Payload
}); typedef struct SGhnSetImageData SGhnSetImageData;

// G.HN Reset Structure
PACKED(
struct SGhnResetCommand
{
	UINT8		State;							// (State == 0) Reset to Firmware mode
												// (State == 1) Reset to EtherBoot mode
}); typedef struct SGhnResetCommand SGhnResetCommand;

// G.HN Execute-Command Structure
PACKED(
struct SGhnExecuteCommand
{
	UINT32		Address;						// Address
	UINT32		Value;							// Value
}); typedef struct SGhnExecuteCommand SGhnExecuteCommand;

// G.HN Erase-Flash Structure
PACKED(
struct SGhnEraseFlash
{
	UINT32		Offset;							// Offset to erase
	UINT32		Size;							// Size in bytes to erase
}); typedef struct SGhnEraseFlash SGhnEraseFlash;

// G.HN Set-Flash Data Structure
PACKED(
struct SGhnSetFlash
{
	UINT32		Address;						// Address
	UINT32		Size;							// Size
	UINT8		Payload[BUFFER_PAYLOAD_LIMIT];	// Payload
}); typedef struct SGhnSetFlash SGhnSetFlash;

// G.HN Get-Flash Data Structure
PACKED(
struct SGhnGetFlash
{
	UINT32		Address;						// Address
	UINT32		Size;							// Size
	UINT8		Payload[BUFFER_PAYLOAD_LIMIT];	// Payload
}); typedef struct SGhnGetFlash SGhnGetFlash;

// G.HN EnterPhyMode Data Structure
PACKED(
struct SGhnEnterPhyMode
{
	UINT32		Data;							// Data
	UINT8		disable_slog;					// disable slog
	UINT8		sniffing_mode;					// sniffing mode
}); typedef struct SGhnEnterPhyMode SGhnEnterPhyMode; /* IPCM_msg_cp_pcu_EnterPHYOnly_t */

PACKED(
struct sGhnEnterRemoteFWUpgrade
{
	UINT8      Status;
}); typedef struct sGhnEnterRemoteFWUpgrade sGhnEnterRemoteFWUpgrade;

PACKED(
struct SGhnPhyInitConfig
{
	UINT8       PRDThreshold2;     //Preamble detection threshold based on ave2
	UINT8       PRDThreshold3;     //Preamble detection threshold based on ave3
	UINT16      MinPowerThreshold; // Minimum power threshold for preamble detection
	UINT16      CSTE_backoff;      // CSTE calculation to account for pre-cursors
	UINT16      BackoffSAT;        // Backward correction threshold
	UINT16      gainGuard;         // Guard duration for AGC settling time
	INT8        TRD_Threshold0;    // Transition detection threshold
	INT8        TRD_Threshold1;    // Transition detection threshold
	INT8        TRD_Threshold2;    // Transition detection threshold
	UINT8       TRD_FaultNum;
	UINT32      GI_val;
}); typedef struct SGhnPhyInitConfig SGhnPhyInitConfig; /* IPCM_msg_cp_pcu_initConfig_t */

PACKED(
struct SGhnPhyStartTX
{
	UINT32		TX_Mode;
	UINT32		FrameNum;
	UINT16		SymbolsNum;
	UINT16		IFG_Time;
	UINT8		HeaderSymbolNum;
	UINT8		PermanentMask;
}); typedef struct SGhnPhyStartTX SGhnPhyStartTX; /* IPCM_msg_cp_pcu_StartTX_t */

PACKED(
struct SGhnPhyStartRX
{
	UINT32		RX_Mode; 
	UINT32		FrameNum;
	UINT16		SymbolsNum;
	UINT16		spare;
	UINT8		HeaderSymbolNum;
	UINT8		PermanentMask;
}); typedef struct SGhnPhyStartRX SGhnPhyStartRX; /* IPCM_msg_cp_pcu_StartRX_t */


// Set Device Mode
#define DEV_MODE_GHN    0
#define DEV_MODE_HPAV   1

PACKED(
struct SGhnSetDeviceMode
{
	UINT8      dev_mode;
	UINT8      pad[3];
}); typedef struct SGhnSetDeviceMode SGhnSetDeviceMode;  /* IPCM_msg_cp_pcu_SetMode_t */;


// G.hn PHY Start Line Sniffing
PACKED(
struct SGhnPhyStartLineSniffer
{
	UINT32  offset;     //offset (in usec) of first measurement from ZC
	UINT32  gap;        //gap (in usec) between 2 consecutive measurements (the program will add to this value AC cycle length: 20m)
	UINT8   gain_pn;    //(in db)
	UINT8   gain_ng;    //(in db)
	UINT16  Nbuff;      //number of measurements. each measurement fills a buffer of 64k.
	UINT8   Nrep;       //repeat number of measurements in the same time slot. each measurement fills a buffer of 64k.
}); typedef struct SGhnPhyStartLineSniffer SGhnPhyStartLineSniffer; /* IPCM_msg_phy_start_line_sniffing_t */

// G.HN Get Active FW State Data Structure
PACKED(
struct SGhnGetActiveFWState
{
	UINT8 launched;
	UINT8 pad[3];
	UINT8 FW1_valid;
	UINT8 FW1_active;
	UINT8 FW2_valid;
	UINT8 FW2_active;
}); typedef struct SGhnGetActiveFWState SGhnGetActiveFWState;  //VSM_msg_um_fw_active_state_t;

// VSM_MSG_GET_DC_CALIBRATION_VECTOR
PACKED(
struct VSM_msg_get_dc_calibration_t
{
	UINT8   dc_offset_hi_ch1;
	UINT8   dc_offset_hi_ch2;
	UINT8   dc_offset_lo_ch1;
	UINT8   dc_offset_lo_ch2;
}); typedef struct VSM_msg_get_dc_calibration_t VSM_msg_get_dc_calibration_t;


// VSM_MSG_DROP_PACKETS_REQ
PACKED(
struct SGhnDropPacketsReq
{
	UINT32		Value;						// 1 = Drop Packets
											// 0 = Forward Packets
}); typedef struct SGhnDropPacketsReq SGhnDropPacketsReq;

// VSM_MSG_GET_NTCLK
PACKED(
struct SGhnGetNTCLKDrop
{
	UINT32		NTCLK;						// 0x00000000 - 0xFFFFFFFF (With WrapAround)
}); typedef struct SGhnGetNTCLKDrop SGhnGetNTCLKDrop;

#define	BPL_MAX_SUPPORTED_DEVICES				64
#define	BPL_STATIC_MAX_SUPPORTED_DEVICES		32

PACKED(
struct SGhnStaticEntry
{
	UINT8		au8MACAddr[HMAC_LEN];			// Device MAC address
	UINT8		GhnDeviceID;					// Device ID
}); typedef struct SGhnStaticEntry SGhnStaticEntry;

// VSM_MSG_SET_DEVICES_DID
PACKED(
struct SGhnSetDevicesDID
{
	UINT8				GhnDMDeviceID;						// Device ID of the DM device
	UINT8				NumbersOfDevices;					// 1..BPL_MAX_SUPPORTED_DEVICES
	SGhnStaticEntry		Table[BPL_MAX_SUPPORTED_DEVICES];
}); typedef struct SGhnSetDevicesDID SGhnSetDevicesDID;

// VSM_MSG_SET_STATIC_ROUTING_TOPOLOGY
PACKED(
struct SGhnSetStaticRoutingTopology
{
	UINT8				Version;							// 1
	UINT8				Reset_On_Calibration_End;			// 1 means feature is enabled
	UINT32				Calibration_NTCLK;					// 0x00000000 - 0xFFFFFFFF (With WrapAround)
	UINT16				TTL;								// Seconds

	UINT8				NumbersOfDevices;					// 1..BPL_MAX_SUPPORTED_DEVICES
	SGhnStaticEntry		Table[BPL_MAX_SUPPORTED_DEVICES];
}); typedef struct SGhnSetStaticRoutingTopology SGhnSetStaticRoutingTopology;

PACKED(
struct SGhnStaticNetworkEntry
{
	UINT8		au8MACAddr[HMAC_LEN];			// Device MAC address
	UINT32		BitMask;						// Connectivity Bit-Mask
}); typedef struct SGhnStaticNetworkEntry SGhnStaticNetworkEntry;

// VSM_MSG_SET_STATIC_NETWORK_TOPOLOGY
PACKED(
struct SGhnSetStaticNetworkTopology
{
	UINT8					NumbersOfDevices;					// 1..BPL_SET_STATIC_NETWORK_TOPOLOGY_MAX_SUPPORTED_DEVICES
	SGhnStaticNetworkEntry	Table[BPL_STATIC_MAX_SUPPORTED_DEVICES];
}); typedef struct SGhnSetStaticNetworkTopology SGhnSetStaticNetworkTopology;

// VSM_MSG_SLOG_HANDSHAKE
PACKED(
struct SGhnSlogHandShake
{
	UINT8				reset_cause; // See table in "VSM_SLOG_HANDSHAKE_t"
	UINT8				pad[3];
	UINT32				reset_counter;
	UINT8				fw_version[16];
	UINT8				assert_file_name[32];
	UINT32				assert_line;
}); typedef struct SGhnSlogHandShake SGhnSlogHandShake;

// Type for slog handshake message (reset_cause)
typedef enum
{
	VSM_SLOG_HANDSHAKE_REGULAR = 0,     // Regular handshake, without any event in the network
	VSM_SLOG_HANDSHAKE_POWER_UP,
	VSM_SLOG_HANDSHAKE_SOFT_RESET,
	VSM_SLOG_HANDSHAKE_AFTER_CRASH,
	VSM_SLOG_HANDSHAKE_CRASH_IN_NETWORK,
	VSM_SLOG_HANDSHAKE_MANUALLY,
	VSM_SLOG_HANDSHAKE_MANUALLY_SLOG_TOOL
}VSM_SLOG_HANDSHAKE_t;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define MAX_SECTION_NAME_LEN	24
#define MAX_VERSION_NAME_LEN	24
#define MAX_SECTION_NUM			64

PACKED(
struct BIN_image_section_header_t
{
	UINT8	sectionName[MAX_SECTION_NAME_LEN];
	UINT32	address;        // address to load to (should be used by the SET_MEM command)
	UINT32	offset;         // offset from start of the image file
	UINT32	size;			// size of section (should be used by the SET_MEM command)
}); typedef struct BIN_image_section_header_t BIN_image_section_header_t;

PACKED(
struct BIN_image_header_t
{
	UINT8	version[MAX_VERSION_NAME_LEN];
	INT32	cpu_id;
	UINT32	numOfSections;
	UINT32	total_image_size;
	UINT32	checksum;
}); typedef struct BIN_image_header_t BIN_image_header_t;

PACKED(
struct BIN_image_loader_t
{
	BIN_image_header_t image_header;
	BIN_image_section_header_t image_sections[MAX_SECTION_NUM];
}); typedef struct BIN_image_loader_t BIN_image_loader_t;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
