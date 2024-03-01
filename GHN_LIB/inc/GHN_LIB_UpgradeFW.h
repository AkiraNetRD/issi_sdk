#ifndef GHN_LIB_UpgradeFW_h__
#define GHN_LIB_UpgradeFW_h__

#include "GHN_LIB_consts.h"
#include "GHN_LIB_int_consts.h"
#include "XMLParser.h"

#ifdef __cplusplus
extern "C" {
#endif

DllExport eGHN_LIB_STAT Ghn_Download_and_Execute_FUUF(	Layer2Connection*	layer2Connection,
														char*				FUUF_BIN_FileName,
														bool				bResetDeviceOnError,
														bool				bPrint_Process_Information,
														char*				ErrorDescription);

DllExport eGHN_LIB_STAT Ghn_Erase_Flash_Section(Layer2Connection*	layer2Connection,
												BIN_file_t*			BIN_file,
												eBIN_Section		Section,
												UINT32				Offset,
												UINT32				Size,					// -1 = Auto
												char*				str_Section_Name,
												char*				str_Programmed_FW,
												bool				Use_SPI_4KB_BLOCK_SIZE,
												bool				bPrint_Process_Information,
												char*				ErrorDescription);

DllExport eGHN_LIB_STAT Ghn_Erase_Flash_Sections(	Layer2Connection*	layer2Connection,
													BIN_file_t*			BIN_file,
													UINT32				FW_SPI_Offset_Programmed,
													FW_Offset_Table_t	FW_Offset_Table_Programmed,
													char*				str_Programmed_FW,
													bool				bErase_All_Sections,
													bool				bErase_Manufacturer_Block,
													bool				bErase_Parameters_Block,
													bool				bErase_Modified_Parameters_Block,
													bool				bPrint_Process_Information,
													char*				ErrorDescription);

DllExport eGHN_LIB_STAT Ghn_Erase_Flash_FW(	Layer2Connection*	layer2Connection,
											UINT32				FW_SPI_Offset_Programmed,
											UINT32				FW_Size_Programmed,
											char*				str_Programmed_FW,
											bool				Use_SPI_4KB_BLOCK_SIZE,
											bool				bPrint_Process_Information,
											char*				ErrorDescription);

DllExport eGHN_LIB_STAT Ghn_Erase_Flash_BootLoader(	Layer2Connection*	layer2Connection,
													UINT32				BootLoader_Size,
													bool				bPrint_Process_Information,
													char*				ErrorDescription);

DllExport eGHN_LIB_STAT Ghn_Program_Flash_Section(	Layer2Connection*	layer2Connection,
													BIN_file_t*			BIN_file,
													eBIN_Section		Section,
													UINT32				Offset,
													UINT32				Size,					// -1 = Auto
													UINT16				EBL_Version,
													char*				str_Section_Name,
													char*				str_Programmed_FW,
													bool				bPrint_Process_Information,
													char*				ErrorDescription);

DllExport eGHN_LIB_STAT Ghn_Program_Flash_Sections(	Layer2Connection*	layer2Connection,
													BIN_file_t*			BIN_file,
													UINT32				FW_SPI_Offset_Programmed,
													FW_Offset_Table_t	FW_Offset_Table_Programmed,
													UINT16				EBL_Version,
													EBL_Offset_Table_t	EBL_Offset_Table,
													UINT8				Modified_Parameters_Validity_Byte,
													char*				str_Programmed_FW,
													bool				bUpdate_EBL,
													bool				bProgram_All_Sections,
													bool				bProgram_Manufacturer_Block,
													bool				bProgram_Parameters_Block,
													bool				bProgram_Modified_Parameters_Block,
													bool				bPrint_Process_Information,
													char*				ErrorDescription);

DllExport eGHN_LIB_STAT Ghn_Update_Active_Bit(	Layer2Connection*	layer2Connection,
												UINT32				Address,
												UINT8				ActiveBit,
												char*				str_Active_FW,
												bool				bPrint_Process_Information,
												char*				ErrorDescription);

DllExport eGHN_LIB_STAT Ghn_QueryDeviceAfter_Upgrade_FW(Layer2Connection*	layer2Connection,
														bool				bPrint_Process_Information,
														char*				ErrorDescription);

DllExport eGHN_LIB_STAT Ghn_Override_TLV_Parameters(BIN_file_t*			BIN_file_Device_Active,
													BIN_file_t*			BIN_file_Programmed,
													eBIN_Section		Section);

DllExport eGHN_LIB_STAT Ghn_Copy_TLV_Parameters(BIN_file_t*			BIN_file_Device_Active,
												BIN_file_t*			BIN_file_Programmed,
												eBIN_Section		Section);


#ifdef __cplusplus
}
#endif

#endif // GHN_LIB_UpgradeFW_h__
