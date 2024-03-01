#ifndef GHN_LIB_Flash_h__
#define GHN_LIB_Flash_h__

#include "GHN_LIB_consts.h"
#include "GHN_LIB_typedefs.h"
#include "common.h"
#include "cdlib.h"

#include "Image_LIB_ext.h"

#ifdef __cplusplus
extern "C" {
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Read Flash Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	char					IN		Device_ChipType[GHN_LIB_MODEL_NAME_LEN];

	bool					IN		bPrint_Process_Information;
	bool					IN		bPrint_Detail_Information;

	UINT8					OUT		DeviceState;
	UINT16					OUT		EBL_Version;
	EBL_Offset_Table_t		OUT		EBL_Offset_Table;
	SGhnGetActiveFWState	OUT		sGhnGetActiveFWState;

	bool					OUT		Modified_Parameters_1_Block_Is_Valid;
	bool					OUT		Modified_Parameters_2_Block_Is_Valid;
	UINT8					OUT		Modified_Parameters_Validity_Byte;		// Should be 1 o 2 (First/Second section)

	UINT32					OUT		Offset_FW_Active_SPI;

	FW_Offset_Table_t		OUT		FW_Offset_Table_FW1;
	FW_Offset_Table_t		OUT		FW_Offset_Table_FW2;
	FW_Offset_Table_t		OUT		FW_Offset_Table_Active;

	UINT8					OUT		BIN_file_Device_FW1_Buffer[GHN_LIB_BIN_FILE_BUFFER_SIZE];
	UINT8					OUT		BIN_file_Device_FW2_Buffer[GHN_LIB_BIN_FILE_BUFFER_SIZE];

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sRead_Flash_Information;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

typedef enum
{
	eRead_Flash_Active_FW	= 0,
	eRead_Flash_FW1			= 1,
	eRead_Flash_FW2			= 2,
} eRead_Flash;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Read Flash Block
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sConnection_Information	IN		Connection;

	bool					IN		bPrint_Process_Information;

	UINT32					IN		Offset;
	UINT32					IN		Length;				// When "bHasChecksum_And_Length" is TRUE, Length is ignored
	bool					IN		bHasChecksum_And_Length;

	bool					IN		bNeedNullifyBeforeChecksum;
	UINT32					IN		NullifyOffset;

	bool					IN		bTrim_Suffix_Of_0xFF;

	char					OUT		OutputFileName[GHN_LIB_MAX_PATH];

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sRead_Flash_Block;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Read Flash Section
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	sRead_Flash_Information	IN		readFlashInformation;

	eRead_Flash				IN		ReadFlash;
	eBIN_Section			IN		Section;
	bool					IN		bTrim_Suffix_Of_0xFF;
	
	char					OUT		OutputFileName[GHN_LIB_MAX_PATH];

	char					OUT		ErrorDescription[GHN_LIB_ERROR_MESSAGE];
} sRead_Flash_Section;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

eGHN_LIB_STAT Ghn_Read_Flash_Information(sRead_Flash_Information* flashInformation);
eGHN_LIB_STAT Ghn_Read_Flash_Block(sRead_Flash_Block* readFlashBlock);
eGHN_LIB_STAT Ghn_Get_Modified_Parameters_Validity_Byte(BIN_file_t* BIN_file, UINT8* Modified_Parameters_Validity_Byte);
eGHN_LIB_STAT Ghn_Read_Flash_Section(sRead_Flash_Section* readFlashSection);

#ifdef __cplusplus
}
#endif

#endif // GHN_LIB_Flash_h__
