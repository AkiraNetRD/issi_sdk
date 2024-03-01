// This is the main DLL file.

#include "GHN_LIB_typedefs.h"
#include "GHN_LIB_int.h"
#include "GHN_LIB_int_consts.h"
#include "GHN_LIB_Layer2Connection.h"
#include "GHN_LIB_ext.h"
#include "GHN_LIB_netinf.h"
#include "GHN_LIB_LinkStatistics.h"
#include "GHN_LIB_Flash.h"
#include "GHN_LIB_UpgradeFW.h"
#include "console.h"
#include "common.h"


#include "HTTP_LIB_ext.h"

#include "Image_LIB_ext.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

eGHN_LIB_STAT Ghn_Erase_Flash_Section(	Layer2Connection*	layer2Connection,
										BIN_file_t*			BIN_file,
										eBIN_Section		Section,
										UINT32				Offset,
										UINT32				Size,					// -1 = Auto
										char*				str_Section_Name,
										char*				str_Programmed_FW,
										bool				Use_SPI_4KB_BLOCK_SIZE,
										bool				bPrint_Process_Information,
										char*				ErrorDescription)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	cg_stat_t					cg_stat;

	UINT8*				SectionBuffer = NULL;
	UINT32				SectionSize;
	UINT32				SectionSize_Erase;

	char				strTemp[1024];
	int					LineSize;

	if (bPrint_Process_Information)
	{
		sprintf(strTemp,"Erasing Flash %s (%s)...", str_Section_Name, str_Programmed_FW);
		printf("%s", strTemp);

		LineSize = strlen(strTemp);
		while (LineSize < 64)
		{
			printf("\t");
			LineSize = (LineSize + 8) / 8 * 8;
		}

		fflush(stdout);
	}

	if (Section == eBIN_Section_MAX_SECTION)
	{
		// Erase entire flash
		SectionSize = Size;
	}
	else
	{
		GetSection(BIN_file, Section, TRUE, &SectionBuffer, &SectionSize);

		if (Size != -1)
		{
			SectionSize = Size;
		}
	}

	while (SectionSize > 0)
	{
		if (Use_SPI_4KB_BLOCK_SIZE == TRUE)
		{
			SectionSize_Erase = min(SectionSize,0x1000); // 4KB
		}
		else
		{
			SectionSize_Erase = SectionSize;
		}


		if ((cg_stat = EraseFlashCommandFunction(layer2Connection, Offset, SectionSize_Erase)) != CG_STAT_SUCCESS)
		{
			if (bPrint_Process_Information)
			{
				Printf_Highlight("[FAILED]\n");
			}

			sprintf(ErrorDescription,"failed to erase the %s (programmed) (stat=%lu)", str_Section_Name, cg_stat);
			LOG_INFO(ErrorDescription);
			GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
			return GHN_LIB_STAT;
		}

		SectionSize = SectionSize - SectionSize_Erase;
		Offset = Offset + SectionSize_Erase;
	}


	if (bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
	}
	
	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Erase_Flash_Sections(	Layer2Connection*	layer2Connection,
										BIN_file_t*			BIN_file,
										UINT32				FW_SPI_Offset_Programmed,
										FW_Offset_Table_t	FW_Offset_Table_Programmed,
										char*				str_Programmed_FW,
										bool				bErase_All_Sections,
										bool				bErase_Manufacturer_Block,
										bool				bErase_Parameters_Block,
										bool				bErase_Modified_Parameters_Block,
										bool				bPrint_Process_Information,
										char*				ErrorDescription)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	UINT8*				SectionBuffer = NULL;
	UINT32				SectionSize;

	if (bErase_All_Sections == TRUE)
	{
		if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
													BIN_file,
													eBIN_Section_FW_SPI,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.SPI.Offset,
													-1,
													"FW SPI",
													str_Programmed_FW,
													FALSE,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}

		if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
													BIN_file,
													eBIN_Section_FW_PCM,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.PCM.Offset,
													-1,
													"FW PCM",
													str_Programmed_FW,
													FALSE,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}

		if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
													BIN_file,
													eBIN_Section_FW_HW,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.HW.Offset,
													-1,
													"FW HW",
													str_Programmed_FW,
													FALSE,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
			
		if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
													BIN_file,
													eBIN_Section_FW_GP,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.GP.Offset,
													-1,
													"FW GP",
													str_Programmed_FW,
													FALSE,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}

		// Add support for the SLOG tool
		if (GetSection(BIN_file, eBIN_Section_SW_DB_XML, TRUE, &SectionBuffer, &SectionSize) == TRUE)
		{
			// The section "eBIN_Section_SW_DB_XML" exists
			if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
														BIN_file,
														eBIN_Section_SW_DB_XML,
														FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.SW_DB_XML.Offset,
														-1,
														"SW DB XML Section",
														str_Programmed_FW,
														FALSE,
														bPrint_Process_Information,
														ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
			{
				return GHN_LIB_STAT;
			}
		}

		if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
													BIN_file,
													eBIN_Section_BIN_FILE_HEADER,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.BIN_FILE_HEADER.Offset,
													-1,
													"BIN File Header",
													str_Programmed_FW,
													FALSE,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}

		if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
													BIN_file,
													eBIN_Section_FW_CP,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.CP.Offset,
													-1,
													"FW CP",
													str_Programmed_FW,
													FALSE,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
		
		if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
													BIN_file,
													eBIN_Section_FW_FWL,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.FWL.Offset,
													-1,
													"FW FWL",
													str_Programmed_FW,
													FALSE,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if ((bErase_All_Sections == TRUE) || (bErase_Manufacturer_Block == TRUE))
	{
		if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
													BIN_file,
													eBIN_Section_Manufacturer_Block,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.Manufacturer_Block.Offset,
													-1,
													"Manufacturer Block",
													str_Programmed_FW,
													FALSE,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if ((bErase_All_Sections == TRUE) || (bErase_Parameters_Block == TRUE))
	{
		if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
													BIN_file,
													eBIN_Section_Parameters_Block,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.Parameters_Block.Offset,
													-1,
													"Parameters Block",
													str_Programmed_FW,
													FALSE,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if ((bErase_All_Sections == TRUE) || (bErase_Modified_Parameters_Block == TRUE))
	{
		if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
													BIN_file,
													eBIN_Section_Modified_Parameters_1_Block,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.Modified_Parameters_1_Block.Offset,
													-1,
													"Modified Parameters 1 Block",
													str_Programmed_FW,
													FALSE,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}

		if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
													BIN_file,
													eBIN_Section_Modified_Parameters_2_Block,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.Modified_Parameters_2_Block.Offset,
													-1,
													"Modified Parameters 2 Block",
													str_Programmed_FW,
													FALSE,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Erase_Flash_FW(	Layer2Connection*	layer2Connection,
									UINT32				FW_SPI_Offset_Programmed,
									UINT32				FW_Size_Programmed,
									char*				str_Programmed_FW,
									bool				Use_SPI_4KB_BLOCK_SIZE,
									bool				bPrint_Process_Information,
									char*				ErrorDescription)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
												NULL,
												eBIN_Section_MAX_SECTION,
												FW_SPI_Offset_Programmed,
												FW_Size_Programmed,
												"entire FW",
												str_Programmed_FW,
												Use_SPI_4KB_BLOCK_SIZE,
												bPrint_Process_Information,
												ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		return GHN_LIB_STAT;
	}

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Erase_Flash_BootLoader(	Layer2Connection*	layer2Connection,
											UINT32				BootLoader_Size,
											bool				bPrint_Process_Information,
											char*				ErrorDescription)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	if ((GHN_LIB_STAT = Ghn_Erase_Flash_Section(	layer2Connection,
												NULL,
												eBIN_Section_MAX_SECTION,
												0x00000000,
												BootLoader_Size,
												"Boot Loader",
												"Boot Loader",
												FALSE,
												bPrint_Process_Information,
												ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		return GHN_LIB_STAT;
	}

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Program_Flash_Section(Layer2Connection*	layer2Connection,
										BIN_file_t*			BIN_file,
										eBIN_Section		Section,
										UINT32				Offset,
										UINT32				Size,					// -1 = Auto
										UINT16				EBL_Version,
										char*				str_Section_Name,
										char*				str_Programmed_FW,
										bool				bPrint_Process_Information,
										char*				ErrorDescription)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	cg_stat_t					cg_stat;

	UINT8*				SectionBuffer = NULL;
	UINT32				SectionSize;

	char				strTemp[1024];
	int					LineSize;

	bool				bUpdateProgressIndication = FALSE;
	bool				bContainChecksumLength; // for VSM command "VSM_MSG_WRITE_FLASH"

	if (bPrint_Process_Information)
	{
		sprintf(strTemp,"Programming Flash %s (%s)...", str_Section_Name, str_Programmed_FW);
		printf("%s", strTemp);

		LineSize = strlen(strTemp);
		while (LineSize < 64)
		{
			printf("\t");
			LineSize = (LineSize + 8) / 8 * 8;
		}

		fflush(stdout);
	}

	if (Section == eBIN_Section_EBL_SPI_BCB)
	{
		GetSection(BIN_file, Section, FALSE, &SectionBuffer, &SectionSize);
		
		bContainChecksumLength = FALSE;
	}
	else // All Other sections
	{
		GetSection(BIN_file, Section, TRUE, &SectionBuffer, &SectionSize);

		if (EBL_Version < 18)
		{
			bContainChecksumLength = FALSE;
		}
		else
		{
			bContainChecksumLength = TRUE;
		}
	}

	if (Size != -1)
	{
		SectionSize = Size;
	}

	if (Section == eBIN_Section_FW_CP)
	{
		bUpdateProgressIndication = TRUE;
	}

	if ((cg_stat = SetFlashFromBufferCommandFunction(layer2Connection, SectionBuffer, SectionSize, Offset, bContainChecksumLength, bUpdateProgressIndication)) != CG_STAT_SUCCESS)
	{
		if (bPrint_Process_Information)
		{
			Printf_Highlight("[FAILED]\n");
		}

		sprintf(ErrorDescription,"failed to program the %s (programmed) (stat=%lu)", str_Section_Name, cg_stat);
		LOG_INFO(ErrorDescription);
		GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
		return GHN_LIB_STAT;
	}

	if (bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
	}
	
	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Program_Flash_Sections(	Layer2Connection*	layer2Connection,
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
											char*				ErrorDescription)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	UINT8*				SectionBuffer = NULL;
	UINT32				SectionSize;

	if (bUpdate_EBL == TRUE)
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_EBL_SPI_BCB,
													EBL_Offset_Table.Offset_EBL_SPI,
													-1,
													EBL_Version,
													"EBL SPI",
													"Share",
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}
	
	if (bUpdate_EBL == TRUE)
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_EBL_B1,
 													EBL_Offset_Table.Offset_EBL_B1 - FLASH_BLOCK_HEADER_SIZE,
													-1,
													EBL_Version,
													"EBL B1",
													"Share",
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if (bProgram_All_Sections == TRUE)
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_FW_SPI,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.SPI.Offset,
													-1,
													EBL_Version,
													"FW SPI",
													str_Programmed_FW,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if (bProgram_All_Sections == TRUE)
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_FW_PCM,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.PCM.Offset,
													-1,
													EBL_Version,
													"FW PCM",
													str_Programmed_FW,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if (bProgram_All_Sections == TRUE)
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_FW_HW,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.HW.Offset,
													-1,
													EBL_Version,
													"FW HW",
													str_Programmed_FW,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if (bProgram_All_Sections == TRUE)
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_FW_GP,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.GP.Offset,
													-1,
													EBL_Version,
													"FW GP",
													str_Programmed_FW,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if (bProgram_All_Sections == TRUE)
	{
		// Add support for the SLOG tool
		if (GetSection(BIN_file, eBIN_Section_SW_DB_XML, TRUE, &SectionBuffer, &SectionSize) == TRUE)
		{
			// The section "eBIN_Section_SW_DB_XML" exists
			if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
														BIN_file,
														eBIN_Section_SW_DB_XML,
														FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.SW_DB_XML.Offset,
														-1,
														EBL_Version,
														"SW DB XML",
														str_Programmed_FW,
														bPrint_Process_Information,
														ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
			{
				return GHN_LIB_STAT;
			}
		}
	}

	if (bProgram_All_Sections == TRUE)
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_BIN_FILE_HEADER,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.BIN_FILE_HEADER.Offset,
													-1,
													EBL_Version,
													"BIN File Header",
													str_Programmed_FW,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if (bProgram_All_Sections == TRUE)
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_FW_CP,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.CP.Offset,
													-1,
													EBL_Version,
													"FW CP",
													str_Programmed_FW,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if (bProgram_All_Sections == TRUE)
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_FW_FWL,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.FWL.Offset,
													-1,
													EBL_Version,
													"FW FWL",
													str_Programmed_FW,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if ((bProgram_All_Sections == TRUE) || (bProgram_Manufacturer_Block == TRUE))
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_Manufacturer_Block,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.Manufacturer_Block.Offset,
													-1,
													EBL_Version,
													"Manufacturer Block",
													str_Programmed_FW,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if ((bProgram_All_Sections == TRUE) || (bProgram_Parameters_Block == TRUE))
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_Parameters_Block,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.Parameters_Block.Offset,
													-1,
													EBL_Version,
													"Parameters Block",
													str_Programmed_FW,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	if ((bProgram_All_Sections == TRUE) || (bProgram_Modified_Parameters_Block == TRUE))
	{
		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_Modified_Parameters_1_Block,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.Modified_Parameters_1_Block.Offset,
													-1,
													EBL_Version,
													"Modified Parameters 1 Block",
													str_Programmed_FW,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}

		if ((GHN_LIB_STAT = Ghn_Program_Flash_Section(layer2Connection,
													BIN_file,
													eBIN_Section_Modified_Parameters_2_Block,
													FW_SPI_Offset_Programmed + FW_Offset_Table_Programmed.Modified_Parameters_2_Block.Offset,
													-1,
													EBL_Version,
													"Modified Parameters 2 Block",
													str_Programmed_FW,
													bPrint_Process_Information,
													ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			return GHN_LIB_STAT;
		}
	}

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Update_Active_Bit(Layer2Connection*	layer2Connection,
									UINT32				Address,
									UINT8				ActiveBit,
									char*				str_Active_FW,
									bool				bPrint_Process_Information,
									char*				ErrorDescription)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	cg_stat_t					cg_stat;


	if (bPrint_Process_Information)
	{
		printf("Updating FW SPI (%s)...\t\t\t\t\t",str_Active_FW);
		fflush(stdout);
	}

	if ((cg_stat = SetFlashCommandFunction(	layer2Connection,
											Address,
											1,
											&ActiveBit,
											FALSE)) != CG_STAT_SUCCESS)
	{
		if (bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}

		sprintf(ErrorDescription,"failed to prog FW SPI (current FW) (stat=%lu)",cg_stat);
		LOG_INFO(ErrorDescription);
		GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
		return GHN_LIB_STAT;
	}
	if (bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
	}

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_QueryDeviceAfter_Upgrade_FW(	Layer2Connection*	layer2Connection,
												bool				bPrint_Process_Information,
												char*				ErrorDescription)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	cg_stat_t					cg_stat;

	SGhnQueryDevice				QueryDevice;

	if (bPrint_Process_Information)
	{
		printf("\n");
		printf("Try to connect to the device after upgrade FW...\t\t");
	}

	if ((cg_stat = QueryDeviceCommandFunction(layer2Connection, &QueryDevice, FALSE)) != CG_STAT_SUCCESS)
	{
		if (bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}

		sprintf(ErrorDescription,"failed to query the device");
		LOG_INFO(ErrorDescription);
		GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
		return GHN_LIB_STAT;
	}

	if (QueryDevice.DeviceState != eDeviceState_FW)
	{
		if (bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}

		sprintf(ErrorDescription,"failed to upgrade the FW");
		LOG_INFO(ErrorDescription);
		GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
		return GHN_LIB_STAT;
	}

	if (bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
	}

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Override_TLV_Parameters(	BIN_file_t*			BIN_file_Device_Active,
											BIN_file_t*			BIN_file_Programmed,
											eBIN_Section		Section)
{
	UINT8					ParameterIndex;
	char					ParameterName[MAX_TLV_PARAMETER_NAME_SIZE];
	UINT32					ParameterLength;
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];

	ParameterIndex = 0;
	while (Get_TLV_Parameter_By_Index(	BIN_file_Device_Active,
										Section,
										ParameterIndex+1,
										ParameterName,
										&ParameterLength,
										&ParameterType,
										ParameterValue) == TRUE)
	{
		if (ParameterType == eTLV_Type_Boolean)
		{
			UINT8 u8;

			memcpy(&u8, &ParameterValue[0], 1);

			Set_TLV_Parameter_Boolean(BIN_file_Programmed, Section, ParameterName, u8);
		}
		else if (ParameterType == eTLV_Type_Integer)
		{
			UINT8 u8;
			UINT16 u16;
			UINT32 u32;

			if (ParameterLength == 1)
			{
				memcpy(&u8, &ParameterValue[0], 1);
				Set_TLV_Parameter_UINT8(BIN_file_Programmed, Section, ParameterName, u8);
			}
			if (ParameterLength == 2)
			{
				memcpy(&u16, &ParameterValue[0], 2);
				Set_TLV_Parameter_UINT16(BIN_file_Programmed, Section, ParameterName, u16);
			}
			if (ParameterLength == 4)
			{
				memcpy(&u32, &ParameterValue[0], 4);
				Set_TLV_Parameter_UINT32(BIN_file_Programmed, Section, ParameterName, u32);
			}
		}
		else if (ParameterType == eTLV_Type_String)
		{
			Set_TLV_Parameter_String(BIN_file_Programmed, Section, ParameterName, ParameterValue);
		}
		else if (ParameterType == eTLV_Type_Array)
		{
			Set_TLV_Parameter_Array(BIN_file_Programmed, Section, ParameterName, ParameterLength, (UINT8*)ParameterValue);
		}

		ParameterIndex++;
	}

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Copy_TLV_Parameters(	BIN_file_t*			BIN_file_Device_Active,
										BIN_file_t*			BIN_file_Programmed,
										eBIN_Section		Section)
{
	UINT8* SectionBuffer_Device_Active;
	UINT32 SectionSize_Device_Active;

	UINT8* SectionBuffer_Programmed;
	UINT32 SectionSize_Programmed;

	if (GetSection(BIN_file_Device_Active, Section, TRUE, &SectionBuffer_Device_Active, &SectionSize_Device_Active) == FALSE)
	{
		return eGHN_LIB_STAT_FAILURE;
	}

	if (GetSection(BIN_file_Programmed, Section, TRUE, &SectionBuffer_Programmed, &SectionSize_Programmed) == FALSE)
	{
		return eGHN_LIB_STAT_FAILURE;
	}

	if (SectionSize_Programmed < SectionSize_Device_Active)
	{
		return eGHN_LIB_STAT_FAILURE;
	}

	memcpy(SectionBuffer_Programmed, SectionBuffer_Device_Active, SectionSize_Device_Active);
	
	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Upgrade_Firmware(sUpgrade_Firmware_Information* firmware)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	
	Layer2Connection			layer2Connection;
	bool						bHas_layer2Connection = FALSE;

	cg_stat_t					cg_stat;

	sGet_Chip_Type_Information	getChipTypeInformation;
	sRead_Flash_Information		readFlashInformation;

	UINT8						DeviceState;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Offset Calculations
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// EBL Offset Table
	EBL_Offset_Table_t			EBL_Offset_Table;				// From Device or parameter "FW_BIN_FileName"

	// Offset of FW1 or FW2
	UINT32						FW_SPI_Offset_Device_Active;	// From Device
	UINT32						FW_SPI_Offset_Programmed;		// From Device or parameter "FW_BIN_FileName"
	UINT32						FW_Size_Programmed;				// Size of the programmed area in the flash
	UINT32						FW_Size_Active_FW;				// Size of the erase area in the flash

	// FW Offset Table
	FW_Offset_Table_t			FW_Offset_Table_Device_Active;	// From Device
	FW_Offset_Table_t			FW_Offset_Table_BIN_FILE;		// From parameter "FW_BIN_FileName"
	FW_Offset_Table_t			FW_Offset_Table_Programmed;		// Pointer to "FW_Offset_Table_Device_Active"
																//         or "FW_Offset_Table_BIN_FILE"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// "BIN_file_t"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	BIN_file_t*			BIN_file_Device_FW1;		// Current on device FW1
	BIN_file_t*			BIN_file_Device_FW2;		// Current on device FW2
	BIN_file_t*			BIN_file_Device_Active;		// Pointer to "BIN_file_Device_FW1" or "BIN_file_Device_FW2"
	BIN_file_t*			BIN_file_BIN_FILE;			// Provided in parameter "FW_BIN_FileName"
	BIN_file_t*			BIN_file_Programmed;		// Pointer to "BIN_file_Device_Active" or "BIN_file_BIN_FILE"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


	UINT8						ActiveBit;

	mac_address_t				mac;

	macStruct					DeviceMac;							// MAC-Address of the device

	char						str_Active_FW[10];
	char						str_Inactive_FW[10];
	char						str_Programmed_FW[10];
	
	char						strTempFolder[GHN_LIB_MAX_PATH] = ".";
	char						TempFile[OS_MAX_PATH] = "";

	BIN_file_BIN_FILE = (BIN_file_t*)malloc(2*sizeof(UINT32) + MAX_BIN_FILE_SIZE);
	BIN_file_BIN_FILE->BIN_file_Sections_Max_Length = MAX_BIN_FILE_SIZE;
	BIN_file_BIN_FILE->BIN_file_Sections_Length = 0;

	LOG_INFO("Started...");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Check FW BIN file
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if ((firmware->bUpgradeFirmware == TRUE) || (firmware->bUpdateActiveBit == TRUE))
	{
		LOG_INFO("Programming FW BIN file: %s\n", firmware->FW_BIN_FileName);

		LOG_INFO("\n");
		LOG_INFO("Checking FW BIN file\t\t\t\t\t\t\t");

		// Load FW BIN file into 
		if (Load_BIN_file(BIN_file_BIN_FILE, firmware->FW_BIN_FileName) == FALSE)
		{
			LOG_INFO("[Failed to read the FW BIN File]\n");

			sprintf(firmware->ErrorDescription,"Failed to read the FW BIN File");
			LOG_INFO(firmware->ErrorDescription);
			GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
			goto exit;
		}

		if (Validate_BIN_file(BIN_file_BIN_FILE) == FALSE)
		{
			LOG_INFO("[FW BIN file is corrupted]\n");

			sprintf(firmware->ErrorDescription,"FW BIN file is corrupted");
			LOG_INFO(firmware->ErrorDescription);
			GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
			goto exit;
		}

		LOG_INFO("[OK]\n");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	fflush(stdout);


	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get Chip-Type Information
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	memset(&getChipTypeInformation, 0x00, sizeof(getChipTypeInformation));
	getChipTypeInformation.Connection = firmware->Connection;

	GHN_LIB_STAT = Ghn_Get_Chip_Type_Information(&getChipTypeInformation);

	if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(firmware->ErrorDescription, getChipTypeInformation.ErrorDescription);
		LOG_INFO(firmware->ErrorDescription);
		goto exit;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if ((firmware->bUpgradeFirmware == TRUE) || (firmware->bUpdateActiveBit == TRUE))
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Validate Chip-Type 
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (Validate_Chip_Type(BIN_file_BIN_FILE, getChipTypeInformation.Device_ChipType) == FALSE)
		{
			LOG_INFO("[Chip-Type mismatch between bin file and device]\n");
			sprintf(firmware->ErrorDescription,"Chip-Type mismatch");
			LOG_INFO(firmware->ErrorDescription);
			GHN_LIB_STAT = eGHN_LIB_STAT_CHIP_TYPE_MISMATCH;
			goto exit;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if ((firmware->bUpgradeFirmware == TRUE) || (firmware->bUpdateActiveBit == TRUE) ||
		(firmware->bPrint_Detail_Information == TRUE))
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Read Flash Information
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		memset(&readFlashInformation, 0x00, sizeof(sRead_Flash_Information));
		readFlashInformation.Connection = firmware->Connection;
		strcpy(readFlashInformation.Device_ChipType, getChipTypeInformation.Device_ChipType);
		readFlashInformation.bPrint_Process_Information = FALSE;
		readFlashInformation.bPrint_Detail_Information = firmware->bPrint_Detail_Information;

		GHN_LIB_STAT = Ghn_Read_Flash_Information(&readFlashInformation);

		if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
		{
			strcpy(firmware->ErrorDescription, readFlashInformation.ErrorDescription);
			LOG_INFO(firmware->ErrorDescription);
			goto exit;
		}

		if (readFlashInformation.EBL_Version < 18)
		{
			if (firmware->bUpdate_Boot_Loader_Version == FALSE)
			{
				sprintf(firmware->ErrorDescription,"Your device has an older Boot-Loading version");
				LOG_INFO(firmware->ErrorDescription);
				GHN_LIB_STAT = eGHN_LIB_STAT_OLD_EBL_VERSION;
				goto exit;
			}
		}

		DeviceState = readFlashInformation.DeviceState;

		BIN_file_Device_FW1 = (BIN_file_t*)&readFlashInformation.BIN_file_Device_FW1_Buffer[0];
		BIN_file_Device_FW2 = (BIN_file_t*)&readFlashInformation.BIN_file_Device_FW2_Buffer[0];

		if ((firmware->bUpgradeFirmware == TRUE) && (firmware->bUpdate_Boot_Loader_Version == TRUE))
		{
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// Parse the EBL SPI Offset Table from the BIN FILE
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			if (Parse_EBL_Table(BIN_file_BIN_FILE, readFlashInformation.Device_ChipType, &EBL_Offset_Table) == FALSE)
			{
				cg_stat = CG_STAT_FAILURE;
				sprintf(firmware->ErrorDescription,"failed to parse the EBL SPI header (stat=%lu)",cg_stat);
				LOG_INFO(firmware->ErrorDescription);
				GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
				goto exit;
			}
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		}
		else
		{
			EBL_Offset_Table = readFlashInformation.EBL_Offset_Table;
		}

		if (readFlashInformation.sGhnGetActiveFWState.launched == 1)
		{
			strcpy(str_Active_FW,"FW1");
			strcpy(str_Inactive_FW,"FW2");
			FW_SPI_Offset_Device_Active = EBL_Offset_Table.Offset_FW1_SPI;

			BIN_file_Device_Active = BIN_file_Device_FW1;
			FW_Offset_Table_Device_Active = readFlashInformation.FW_Offset_Table_FW1;
		}
		else
		{
			strcpy(str_Active_FW,"FW2");
			strcpy(str_Inactive_FW,"FW1");
			FW_SPI_Offset_Device_Active = EBL_Offset_Table.Offset_FW2_SPI;

			BIN_file_Device_Active = BIN_file_Device_FW2;
			FW_Offset_Table_Device_Active = readFlashInformation.FW_Offset_Table_FW2;
		}

		FW_Offset_Table_Programmed = FW_Offset_Table_Device_Active;
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if ((firmware->bUpgradeFirmware == TRUE) || (firmware->bUpdateActiveBit == TRUE))
	{
		BIN_file_Programmed = BIN_file_BIN_FILE;
	}
	else
	{
		BIN_file_Programmed = BIN_file_Device_Active;
	}

	if (((firmware->bUpgradeFirmware == TRUE) || (firmware->bUpdateActiveBit == TRUE)) ||
		(firmware->bPrint_Detail_Information == TRUE))
	{
		if (DeviceState == eDeviceState_BootCode)
		{
			if (firmware->bUpgradeFirmware == FALSE)
			{
				sprintf(firmware->ErrorDescription,"Device is in BootCode. Please program with the local programming tool");
				LOG_INFO(firmware->ErrorDescription);
				GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
				goto exit;
			}
		}
	}

	/*
	if (DeviceState == eDeviceState_B1)
	{
		if (firmware->bUpgradeFirmware == FALSE)
		{
			sprintf(firmware->ErrorDescription,"Device is in B1 mode. Please program with the local programming tool");
			LOG_INFO(firmware->ErrorDescription);
			GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
			goto exit;
		}
	}
	*/

	if ((GHN_LIB_STAT = Open_Layer2_Connection(	&firmware->Connection,
												ETHER_TYPE_GHN,
												&layer2Connection,
												firmware->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
		goto exit;
	}

	bHas_layer2Connection = TRUE;

	if (firmware->bUpgradeFirmware == TRUE)
	{
		// Update progress to 1%
		UpdateProgressIndication(layer2Connection.m_MAC_Address,1);
	}


	memcpy(DeviceMac.macAddress,layer2Connection.m_MAC_Address.macAddress,HMAC_LEN);

	if (firmware->bPrint_Detail_Information)
	{
		//printf("Working with MAC Address = "MAC_ADDR_FMT"\n",MAC_ADDR(layer2Connection.m_MAC_Address.macAddress));
	}

	// Support Windows 8
	Check_Write_Permission_And_Update_Output_Folder(strTempFolder);

	sprintf(TempFile,"%s\\"MAC_ADDR_NO_COLON".tmp", strTempFolder, MAC_ADDR(layer2Connection.m_MAC_Address.macAddress));
	remove(TempFile);

	fflush(stdout);

	if (firmware->bPrint_Detail_Information)
	{
		if (readFlashInformation.sGhnGetActiveFWState.FW1_valid)
		{
			if (readFlashInformation.FW_Offset_Table_FW1.BIN_FILE_HEADER.Offset != 0x00000000)
			{
				printf("\n");
				if (readFlashInformation.sGhnGetActiveFWState.launched == 1)
				{
					Printf_Highlight("FW1 BIN file header (ACTIVE):\n");
				}
				else
				{
					printf("FW1 BIN file header:\n");
				}
				Print_BIN_file_Header(BIN_file_Device_FW1, (readFlashInformation.sGhnGetActiveFWState.launched == 1));
				//printf("\n");
			}
		}

		if (readFlashInformation.sGhnGetActiveFWState.FW2_valid)
		{
			if (readFlashInformation.FW_Offset_Table_FW2.BIN_FILE_HEADER.Offset != 0x00000000)
			{
				printf("\n");
				if (readFlashInformation.sGhnGetActiveFWState.launched != 1)
				{
					Printf_Highlight("FW2 BIN file header (ACTIVE):\n");
				}
				else
				{
					printf("FW2 BIN file header:\n");
				}
				Print_BIN_file_Header(BIN_file_Device_FW2, (readFlashInformation.sGhnGetActiveFWState.launched != 1));
				//printf("\n");
			}
		}
	}

	if (firmware->bUpgradeFirmware == TRUE)
	{
		UINT8 Status;

		if ((cg_stat=Enter_Remote_FW_UpgradeCommandFunction(	&layer2Connection,
																&Status)) != CG_STAT_SUCCESS)
		{
			sprintf(firmware->ErrorDescription,"failed to enter remote FW upgrade (stat=%lu)",cg_stat);
			LOG_INFO(firmware->ErrorDescription);
			GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
			goto exit;
		}

		if (Status != eUM_Status_Success)
		{
			sprintf(firmware->ErrorDescription,"failed to enter remote FW upgrade (Status=%d)", Status);
			LOG_INFO(firmware->ErrorDescription);
			GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
			goto exit;
		}
	}

	if ((firmware->bUpgradeFirmware == TRUE) || (firmware->bUpdateActiveBit == TRUE))
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Decide which Image Primary/Secondary to upgrade
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		{
			if (readFlashInformation.sGhnGetActiveFWState.launched == 1)
			{
				strcpy(str_Programmed_FW,"FW2");
				FW_SPI_Offset_Programmed = EBL_Offset_Table.Offset_FW2_SPI;
				FW_Size_Programmed = MAX_FLASH_SIZE - EBL_Offset_Table.Offset_FW2_SPI;
				FW_Size_Active_FW = EBL_Offset_Table.Offset_FW2_SPI - EBL_Offset_Table.Offset_FW1_SPI;
			}
			else
			{
				strcpy(str_Programmed_FW,"FW1");
				FW_SPI_Offset_Programmed = EBL_Offset_Table.Offset_FW1_SPI;
				FW_Size_Programmed = EBL_Offset_Table.Offset_FW2_SPI - EBL_Offset_Table.Offset_FW1_SPI;
				FW_Size_Active_FW = MAX_FLASH_SIZE - EBL_Offset_Table.Offset_FW2_SPI;
			}
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Parse the FW SPI Offset Table
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (Parse_FW_Table(BIN_file_BIN_FILE, &FW_Offset_Table_BIN_FILE) == FALSE)
		{
			if (firmware->bResetDeviceOnError)
			{
				Firmware_Reset_Device(&layer2Connection, 0);
			}

			cg_stat = CG_STAT_FAILURE;
			sprintf(firmware->ErrorDescription,"failed to parse the FW SPI header (stat=%lu)",cg_stat);
			LOG_INFO(firmware->ErrorDescription);
			GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
			goto exit;
		}

		FW_Offset_Table_Programmed = FW_Offset_Table_BIN_FILE;
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		if (firmware->bUpgradeFirmware == TRUE)
		{
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// Erase Flash Section(s)
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			if ((GHN_LIB_STAT = Ghn_Erase_Flash_FW(	&layer2Connection,
													FW_SPI_Offset_Programmed,
													FW_Size_Programmed,
													str_Programmed_FW,
													FALSE,
													FALSE/*firmware->bPrint_Process_Information*/,
													firmware->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
			{
				sprintf(firmware->ErrorDescription,"Failed to erase the flash");
				LOG_INFO(firmware->ErrorDescription);
				goto exit;
			}
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		}

		if (firmware->bUpgradeFirmware == TRUE)
		{
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// Update the following Blocks
			//   "eBIN_Section_Manufacturer_Block"
			//   "eBIN_Section_Parameters_Block"
			//   "eBIN_Section_Modified_Parameters_1_Block"
			//   "eBIN_Section_Modified_Parameters_2_Block"
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			{
				// Update Manufacturer Block Parameters (Device MAC-Address)
				macStruct					UpdateMacAddress;
				
				UINT8						Modified_Parameters_1_Validity_Byte;
				UINT8						Modified_Parameters_2_Validity_Byte;

				if (firmware->bUpdateMacAddress)
				{
					// Override the MAC-Address with the one provided by the user
					str_to_mac(firmware->UpdateDeviceMAC,&mac);
					memcpy(UpdateMacAddress.macAddress,mac,HMAC_LEN);
				}
				else 
				{
					// Use the MAC-Address of the device
					memcpy(&UpdateMacAddress,&DeviceMac,sizeof(macStruct));
				}

				if (Ghn_Override_TLV_Parameters(BIN_file_Device_Active, BIN_file_Programmed, eBIN_Section_Manufacturer_Block) != eGHN_LIB_STAT_SUCCESS)
				{
					sprintf(firmware->ErrorDescription,"Failed to update the Manufacturer_Block section");
					LOG_INFO(firmware->ErrorDescription);
					GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
					goto exit;
				}

				if (Set_TLV_Parameter_Array(BIN_file_Programmed, eBIN_Section_Manufacturer_Block, "dev_mac", 6, UpdateMacAddress.macAddress) == FALSE)
				{
					sprintf(firmware->ErrorDescription,"Set_TLV_Parameter_Array() failed to update MAC-Address");
					LOG_INFO(firmware->ErrorDescription);
					GHN_LIB_STAT = ConvertReturnStatus(CG_STAT_FAILURE);
					goto exit;
				}

				if (firmware->bKeepDeviceParameterSettings)
				{
					if (Ghn_Override_TLV_Parameters(BIN_file_Device_Active, BIN_file_Programmed, eBIN_Section_Parameters_Block) != eGHN_LIB_STAT_SUCCESS)
					{
						sprintf(firmware->ErrorDescription,"Failed to update the Parameters_Block section");
						LOG_INFO(firmware->ErrorDescription);
						GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
						goto exit;
					}

					UpdateSignature(BIN_file_Programmed);
				}

				// Override the Remote Monitoring default setting
				if (firmware->bHas_RemoteMonitoring == TRUE)
				{
					if (Set_TLV_Parameter_Boolean(BIN_file_Programmed, eBIN_Section_Parameters_Block, "remote_monitoring", firmware->RemoteMonitoring) == FALSE)
					{
						sprintf(firmware->ErrorDescription,"Failed to update the Parameters_Block section");
						LOG_INFO(firmware->ErrorDescription);
						GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
						goto exit;
					}

					UpdateSignature(BIN_file_Programmed);
				}

				// Copy the Validity Byte of "eBIN_Section_Modified_Parameters_1_Block" and "eBIN_Section_Modified_Parameters_2_Block" from the Active FW
				Get_TLV_Modified_Parameter_Validity_Byte(BIN_file_Device_Active,	eBIN_Section_Modified_Parameters_1_Block, &Modified_Parameters_1_Validity_Byte);
				Get_TLV_Modified_Parameter_Validity_Byte(BIN_file_Device_Active,	eBIN_Section_Modified_Parameters_2_Block, &Modified_Parameters_2_Validity_Byte);
				Set_TLV_Modified_Parameter_Validity_Byte(BIN_file_Programmed,		eBIN_Section_Modified_Parameters_1_Block, Modified_Parameters_1_Validity_Byte);
				Set_TLV_Modified_Parameter_Validity_Byte(BIN_file_Programmed,		eBIN_Section_Modified_Parameters_2_Block, Modified_Parameters_2_Validity_Byte);

				// Support copying the content of the Modified-Blocks into a smaller block length
				if ((Ghn_Override_TLV_Parameters(BIN_file_Device_Active, BIN_file_Programmed, eBIN_Section_Modified_Parameters_1_Block) != eGHN_LIB_STAT_SUCCESS) ||
					(Ghn_Override_TLV_Parameters(BIN_file_Device_Active, BIN_file_Programmed, eBIN_Section_Modified_Parameters_2_Block) != eGHN_LIB_STAT_SUCCESS))
				{
					sprintf(firmware->ErrorDescription,"Failed to update the Modified_Parameters_Block section");
					LOG_INFO(firmware->ErrorDescription);
					GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
					goto exit;
				}
			}
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		}

		if (firmware->bUpgradeFirmware == TRUE)
		{
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// Program the FW relevant sections
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			if ((GHN_LIB_STAT = Ghn_Program_Flash_Sections(	&layer2Connection,
															BIN_file_Programmed,
															FW_SPI_Offset_Programmed,
															FW_Offset_Table_Programmed,
															readFlashInformation.EBL_Version,
															EBL_Offset_Table,
															readFlashInformation.Modified_Parameters_Validity_Byte,
															str_Programmed_FW,
															FALSE/*bUpdate_EBL*/,
															firmware->bUpgradeFirmware/*bProgram_All_Sections*/,
															FALSE/*bProgram_Manufacturer_Block*/,
															FALSE/*bProgram_Parameters_Block*/,
															FALSE/*bProgram_Modified_Parameters_Block*/,
															FALSE/*firmware->bPrint_Process_Information*/,
															firmware->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
			{
				goto exit;
			}
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Set the new FW as the Active FW
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (firmware->bUpdateActiveBit == TRUE)
		{
			ActiveBit = 0x01; // Set as the Active FW

			if ((GHN_LIB_STAT = Ghn_Update_Active_Bit(&layer2Connection,
													FW_SPI_Offset_Programmed +
													FW_Offset_Table_Programmed.SPI.Offset +
													FLASH_BLOCK_HEADER_SIZE + 
													Get_FW_SPI_ActiveBit_Offset(),
													ActiveBit,
													str_Active_FW,
													FALSE/*firmware->bPrint_Process_Information*/,
													firmware->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
			{
				goto exit;
			}
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Updating the EBL and Erase the Active-FW
		// (or)
		// Turn off the "active bit" on the Active-FW
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (firmware->bUpdate_Boot_Loader_Version == TRUE)
		{
			if ((GHN_LIB_STAT = Ghn_Erase_Flash_BootLoader(	&layer2Connection,
															EBL_Offset_Table.Offset_FW1_SPI,
															FALSE/*firmware->bPrint_Process_Information*/,
															firmware->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
			{
				sprintf(firmware->ErrorDescription,"Failed to erase the flash");
				LOG_INFO(firmware->ErrorDescription);
				goto exit;
			}

			if ((GHN_LIB_STAT = Ghn_Program_Flash_Sections(	&layer2Connection,
															BIN_file_Programmed,
															FW_SPI_Offset_Programmed,
															FW_Offset_Table_Programmed,
															readFlashInformation.EBL_Version,
															EBL_Offset_Table,
															readFlashInformation.Modified_Parameters_Validity_Byte,
															str_Programmed_FW,
															TRUE/*bUpdate_EBL*/,
															FALSE/*bProgram_All_Sections*/,
															FALSE/*bProgram_Manufacturer_Block*/,
															FALSE/*bProgram_Parameters_Block*/,
															FALSE/*bProgram_Modified_Parameters_Block*/,
															FALSE/*firmware->bPrint_Process_Information*/,
															firmware->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
			{
				goto exit;
			}

			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// Erase the Active-FW
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			if ((GHN_LIB_STAT = Ghn_Erase_Flash_FW(	&layer2Connection,
													FW_SPI_Offset_Device_Active,
													FW_Size_Active_FW,
													str_Active_FW,
													TRUE/*Use_SPI_4KB_BLOCK_SIZE*/,
													FALSE/*firmware->bPrint_Process_Information*/,
													firmware->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
			{
				sprintf(firmware->ErrorDescription,"Failed to erase the flash");
				LOG_INFO(firmware->ErrorDescription);
				goto exit;
			}
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		}
		else
		{
			if (firmware->bUpdateActiveBit == TRUE)
			{
				ActiveBit = 0x00; // Set as the not Active FW

				if ((GHN_LIB_STAT = Ghn_Update_Active_Bit(&layer2Connection,
														FW_SPI_Offset_Device_Active +
														FW_Offset_Table_Device_Active.SPI.Offset +
														FLASH_BLOCK_HEADER_SIZE + 
														Get_FW_SPI_ActiveBit_Offset(),
														ActiveBit,
														str_Active_FW,
														FALSE/*firmware->bPrint_Process_Information*/,
														firmware->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
				{
					goto exit;
				}
			}
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (firmware->bQueryDeviceAfterReset)
	{
		memcpy(&layer2Connection.m_MAC_Address,&DeviceMac,sizeof(macStruct));

		if ((GHN_LIB_STAT = Ghn_QueryDeviceAfter_Upgrade_FW(&layer2Connection,
															FALSE/*firmware->bPrint_Process_Information*/,
															firmware->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			goto exit;
		}
	}

	GHN_LIB_STAT = eGHN_LIB_STAT_SUCCESS;

exit:

	if (bHas_layer2Connection == TRUE)
	{
		Close_Layer2_Connection(&layer2Connection);
	}
	
	if (strlen(TempFile) > 0)
	{
		remove(TempFile);
	}

	free(BIN_file_BIN_FILE);

	return GHN_LIB_STAT;
}

eGHN_LIB_STAT Ghn_Validate_Chip_Type(sValidate_Chip_Type_Information* chipType)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	sGet_Chip_Type_Information	getChipTypeInformation;

	BIN_file_t*					BIN_file_BIN_FILE;			// Provided in parameter "FW_BIN_FileName"

	BIN_file_BIN_FILE = (BIN_file_t*)malloc(2*sizeof(UINT32) + MAX_BIN_FILE_SIZE);
	BIN_file_BIN_FILE->BIN_file_Sections_Max_Length = MAX_BIN_FILE_SIZE;
	BIN_file_BIN_FILE->BIN_file_Sections_Length = 0;

	LOG_INFO("Started...");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Check FW BIN file
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	{
		LOG_INFO("Checking FW BIN file\t\t\t\t\t\t\t");

		// Load FW BIN file into 
		if (Load_BIN_file(BIN_file_BIN_FILE, chipType->FW_BIN_FileName) == FALSE)
		{
			LOG_INFO("[Failed to read the FW BIN File]\n");

			sprintf(chipType->ErrorDescription,"Failed to read the FW BIN File");
			LOG_INFO(chipType->ErrorDescription);
			GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
			goto exit;
		}

		if (Validate_BIN_file(BIN_file_BIN_FILE) == FALSE)
		{
			LOG_INFO("[FW BIN file is corrupted]\n");

			sprintf(chipType->ErrorDescription,"FW BIN file is corrupted");
			LOG_INFO(chipType->ErrorDescription);
			GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
			goto exit;
		}

		LOG_INFO("[OK]\n");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	fflush(stdout);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get Chip-Type Information
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	memset(&getChipTypeInformation, 0x00, sizeof(getChipTypeInformation));
	getChipTypeInformation.Connection = chipType->Connection;

	GHN_LIB_STAT = Ghn_Get_Chip_Type_Information(&getChipTypeInformation);

	if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(chipType->ErrorDescription, getChipTypeInformation.ErrorDescription);
		LOG_INFO(chipType->ErrorDescription);
		goto exit;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Validate Chip-Type 
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (Validate_Chip_Type(BIN_file_BIN_FILE, getChipTypeInformation.Device_ChipType) == FALSE)
	{
		LOG_INFO("[Chip-Type mismatch between bin file and device]\n");
		sprintf(chipType->ErrorDescription,"Chip-Type mismatch");
		LOG_INFO(chipType->ErrorDescription);
		GHN_LIB_STAT = eGHN_LIB_STAT_CHIP_TYPE_MISMATCH;
		goto exit;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

exit:

	free(BIN_file_BIN_FILE);

	return GHN_LIB_STAT;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
