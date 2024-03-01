#ifndef console_Helpers_h__
#define console_Helpers_h__

#include "console_typedefs.h"

#include "cdlib.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

unsigned long console_read_int();
long console_get_msectime();
void LOG_INFO_Highlight(char* Message);

// Reset
bool Firmware_Reset_Device(Layer2Connection* layer2Connection, int NewState);
bool Hardware_Reset_Device(Layer2Connection* layer2Connection, int NewState);

bool Console_Debug_Info(Layer2Connection* layer2Connection, char* Message, char* Response, UINT32 MaxResponseTimeOut);

bool Console_Debug_Info_Post_Mortem_Analysis(	Layer2Connection* layer2Connection,
												char* Response,
												char* SLOG_Response,
												int* SLOG_Response_Size,
												bool bIsEtherBoot);

bool GetEBLVersion(Layer2Connection* layer2Connection, UINT16* EBLVersion);
bool GetChipType(Layer2Connection* layer2Connection, char* Device_ChipType);
unsigned long Get_FUUF_First_Address(char* FileName, UINT32* Address);
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define PBLOCK_MAGIC_NUMBER         0xCEBAB    /* PBLOCK magic number */
#define PBLOCK_VERSION              2          /* The version of the PBLOCK. Should be increased with each change to the pblock's structure */

// Reset Causes
#define PBLOCK_ASSERT_REBOOT        0x00000001 /* Reset due to assert */ 
#define PBLOCK_WATCHDOG_REBOOT	    0x00000002 /* Watchdog reset */ 

#define PBLOCK_UART_CYCLIC_BUF_LEN (4000)

#define PBLOCK_MAX_RESET_ENTRIES (10)
#define MAX_ASSERT_FILE_NAME_LEN (32)

// virtual addresses:
#define VIRTUAL_ADDRESS_UART            (0xa0000000)
#define VIRTUAL_ADDRESS_UART_COMMAND    (0xa0100000)
#define VIRTUAL_ADDRESS_PBLOCK          (0xa0200000)
#define VIRTUAL_ADDRESS_PBLOCK_UART_BUF (0xa0300000)
#define VIRTUAL_ADDRESS_PBLOCK_SLOG_BUF (0xa0400000)
#define VIRTUAL_ADDRESS_GLOBAL_PARAMS	(0xa0500000)
#define VIRTUAL_ADDRESS_SLOG_BUF_SIZE   (0xa0B00000)

// Reading the PBLOCK in EtherBoot
#define ABSOLUTE_ADDRESS_PBLOCK               (0x00020000)
#define ABSOLUTE_ADDRESS_PBLOCK_SLOG_BUF_SIZE (0x00020244)
#define ABSOLUTE_ADDRESS_PBLOCK_UART_BUF      (0x00020248)
#define ABSOLUTE_ADDRESS_PBLOCK_SLOG_BUF      (0x000211e8)

#define VERSION_REG                      (0x6000000C)		// the chip version from the software team
#define ROM_REG                          (0x00018004)		// ROM version address to identify Golan2 or Golan3 
#define TOP_RC_BONDING_OPTION_REG        (0x610300DC)		// The chip Bonding address (bit field see below) some values are the same for golan2 and golan3

typedef struct 
{    
	UINT32 line;
	char file[MAX_ASSERT_FILE_NAME_LEN];
} ASSERT_info_s;

typedef struct  
{
	UINT8 launched;
	UINT16 version;
	UINT8 pad;
	UINT8 FW1_valid;
	UINT8 FW1_active;
	UINT8 FW2_valid;
	UINT8 FW2_active;
	UINT32 fw_offset;
}PBLOCK_ebl_section_t;

typedef struct 
{
	UINT32 reset_cause;
	UINT32 system_up_time_os_tick;

	union
	{
		ASSERT_info_s assert;

		// Add here additional structures that can supply useful information regarding the 
		// reset. 
		//Note that this is a union structure!!!.

	} detail;

} PBLOCK_reset_info_s;

typedef struct
{
	UINT32 magic_num;       // Magic number that indicates whether the pblock is valid    
	UINT16 version;         // The version of the pblock, in order to parse it correctly in the tools    
	UINT8 pad;
	UINT8 reset_index;
	UINT32 reset_counter;    
	PBLOCK_reset_info_s reset_entries[PBLOCK_MAX_RESET_ENTRIES];
	UINT32 continues_reset_accumulator;		// Accumulated time of continues resets
	UINT32 total_inits;             // total init times. watchdog reset counter == total init times - reset_counter    
	PBLOCK_ebl_section_t  ebl; 
} PBLOCK_s;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
