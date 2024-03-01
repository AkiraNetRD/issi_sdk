#include "GHN_LIB_typedefs.h"
#include "GHN_LIB_int.h"
#include "GHN_LIB_int_consts.h"
#include "GHN_LIB_Layer2Connection.h"
#include "GHN_LIB_Flash.h"
#include "console.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

eGHN_LIB_STAT Ghn_Read_Flash_Information(sRead_Flash_Information* readFlashInformation)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	
	Layer2Connection			layer2Connection;
	bool						bHas_layer2Connection = FALSE;

	cg_stat_t					cg_stat;

	char						strTempFolder[GHN_LIB_MAX_PATH] = ".";
	char						TempFile[OS_MAX_PATH] = "";

	BIN_file_t*					BIN_file_Device_FW1;		// Current on device FW1
	BIN_file_t*					BIN_file_Device_FW2;		// Current on device FW2

	BIN_file_t*					BIN_file_Device_Active;		// Pointer

	BIN_file_t*					BIN_file_Pointer;			// Pointer
	
	sRead_Flash_Block			readFlashBlock;


	BIN_file_Device_FW1 = (BIN_file_t*)&readFlashInformation->BIN_file_Device_FW1_Buffer[0];
	BIN_file_Device_FW1->BIN_file_Sections_Max_Length = GHN_LIB_BIN_FILE_BUFFER_SIZE;
	BIN_file_Device_FW1->BIN_file_Sections_Length = 0;

	BIN_file_Device_FW2 = (BIN_file_t*)&readFlashInformation->BIN_file_Device_FW2_Buffer[0];
	BIN_file_Device_FW2->BIN_file_Sections_Max_Length = GHN_LIB_BIN_FILE_BUFFER_SIZE;
	BIN_file_Device_FW2->BIN_file_Sections_Length = 0;

	LOG_INFO("Started...");

	fflush(stdout);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Check Device-State
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("\n");
		printf("Query Device State...\t\t\t\t\t\t");
	}

	if (readFlashInformation->Connection.bIsBootCode)
	{
		readFlashInformation->DeviceState = eDeviceState_BootCode;
		if (readFlashInformation->bPrint_Process_Information)
		{
			Printf_Highlight("[BootCode]\n");
		}
	}
	else
	{
		readFlashInformation->DeviceState = eDeviceState_FW;
		if (readFlashInformation->bPrint_Process_Information)
		{
			Printf_Highlight("[FW]\n");
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if (readFlashInformation->DeviceState == eDeviceState_BootCode)
	{
		// No more information to read
		return eGHN_LIB_STAT_SUCCESS;
	}

	if ((GHN_LIB_STAT = Open_Layer2_Connection(	&readFlashInformation->Connection,
												ETHER_TYPE_GHN,
												&layer2Connection,
												readFlashInformation->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
		goto exit;
	}

	bHas_layer2Connection = TRUE;

	if (readFlashInformation->bPrint_Detail_Information)
	{
		printf("Working with MAC Address = "MAC_ADDR_FMT"\n",MAC_ADDR(layer2Connection.m_MAC_Address.macAddress));
	}

	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("\n");
	}

	// Support Windows 8
	Check_Write_Permission_And_Update_Output_Folder(strTempFolder);

	sprintf(TempFile,"%s\\"MAC_ADDR_NO_COLON".tmp", strTempFolder, MAC_ADDR(layer2Connection.m_MAC_Address.macAddress));
	remove(TempFile);

	fflush(stdout);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Check EBL Version
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("Get the EBL Version from the device...\t\t\t\t");
	}
	if (GetEBLVersion(&layer2Connection,&readFlashInformation->EBL_Version) == FALSE)
	{
		cg_stat = CG_STAT_FAILURE;
		sprintf(readFlashInformation->ErrorDescription,"Failed to read the EBL Version from the device (stat=%lu)",cg_stat);
		LOG_INFO(readFlashInformation->ErrorDescription);
		GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
		goto exit;
	}
	if (readFlashInformation->bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
	}

	if (readFlashInformation->bPrint_Detail_Information)
	{
		// Print the Parameters
		printf("\n");
		printf("EBL-Version              = %d\n",		readFlashInformation->EBL_Version);
	}

	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("\n");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// #define GOLAN_B1_VERSION (15) - Support the TLV feature (GA1)
	// #define GOLAN_B1_VERSION (16) - Support the GP-Protection feature (Checksum and Length)
	// #define GOLAN_B1_VERSION (17) - Support the FW Loader feature (Dual EBL)
	// #define GOLAN_B1_VERSION (18) - Support new flag (Payload contains CheckSum and Length) for the VSM command "VSM_MSG_WRITE_FLASH"

	if (readFlashInformation->EBL_Version < 15)
	{
		sprintf(readFlashInformation->ErrorDescription,"Your device has an older Boot-Loading version");
		LOG_INFO(readFlashInformation->ErrorDescription);
		GHN_LIB_STAT = eGHN_LIB_STAT_OLD_EBL_VERSION;
		goto exit;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get EBL AVL (Active/Valid/Launched) Status
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("Get the FW State from the device...\t\t\t\t");
	}
	if ((cg_stat=GetFwStateCommandFunction(&layer2Connection, &readFlashInformation->sGhnGetActiveFWState)) != CG_STAT_SUCCESS)
	{
		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}

		sprintf(readFlashInformation->ErrorDescription,"failed to get FW State (stat=%lu)",cg_stat);
		LOG_INFO(readFlashInformation->ErrorDescription);
		GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
		goto exit;
	}

	if (readFlashInformation->bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
	}


	if (readFlashInformation->bPrint_Detail_Information)
	{
		printf("FW1 Valid                = %d\n", readFlashInformation->sGhnGetActiveFWState.FW1_valid);
		printf("FW1 Active               = %d\n", readFlashInformation->sGhnGetActiveFWState.FW1_active);

		if (readFlashInformation->sGhnGetActiveFWState.launched == 1)
		{
			printf("FW1 Launched             = 1\n");
		}
		else
		{
			printf("FW1 Launched             = 0\n");
		}

		printf("FW2 Valid                = %d\n", readFlashInformation->sGhnGetActiveFWState.FW2_valid);
		printf("FW2 Active               = %d\n", readFlashInformation->sGhnGetActiveFWState.FW2_active);

		if (readFlashInformation->sGhnGetActiveFWState.launched != 1)
		{
			printf("FW2 Launched             = 1\n");
		}
		else
		{
			printf("FW2 Launched             = 0\n");
		}
	}

	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("\n");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


	memset(&readFlashBlock, 0x00, sizeof(readFlashBlock));
	readFlashBlock.Connection = readFlashInformation->Connection;
	readFlashBlock.bPrint_Process_Information = FALSE;
	strcpy(readFlashBlock.OutputFileName, TempFile);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the EBL SPI header from the device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("Get the EBL SPI header from the device...\t\t\t");
	}

	readFlashBlock.Offset = 0x00000000;
	readFlashBlock.Length = EBL_BCB_FLASH_SIZE(readFlashInformation->Device_ChipType);
	readFlashBlock.bHasChecksum_And_Length = FALSE;
	if ((GHN_LIB_STAT = Ghn_Read_Flash_Block(&readFlashBlock)) != eGHN_LIB_STAT_SUCCESS)
	{
		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}

		sprintf(readFlashInformation->ErrorDescription,"failed to get the EBL SPI header from the device");
		LOG_INFO(readFlashInformation->ErrorDescription);
		goto exit;
	}
	if (readFlashInformation->bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
	}

	AppendSection_From_File(BIN_file_Device_FW1, eBIN_Section_EBL_SPI_BCB, TempFile);
	AppendSection_From_File(BIN_file_Device_FW2, eBIN_Section_EBL_SPI_BCB, TempFile);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	BIN_file_Pointer = BIN_file_Device_FW1;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Parse the EBL SPI Offset Table
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("Parse the EBL SPI header from the device...\t\t\t");
	}
	if (Parse_EBL_Table(BIN_file_Pointer, readFlashInformation->Device_ChipType, &readFlashInformation->EBL_Offset_Table) == FALSE)
	{
		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}

		cg_stat = CG_STAT_FAILURE;
		sprintf(readFlashInformation->ErrorDescription,"failed to parse the EBL SPI header (stat=%lu)",cg_stat);
		LOG_INFO(readFlashInformation->ErrorDescription);
		GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
		goto exit;
	}
	if (readFlashInformation->bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
		Printf_Highlight("\n");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if (readFlashInformation->sGhnGetActiveFWState.launched == 1)
	{
		readFlashInformation->Offset_FW_Active_SPI = readFlashInformation->EBL_Offset_Table.Offset_FW1_SPI;

		BIN_file_Device_Active = BIN_file_Device_FW1;
	}
	else
	{
		readFlashInformation->Offset_FW_Active_SPI = readFlashInformation->EBL_Offset_Table.Offset_FW2_SPI;

		BIN_file_Device_Active = BIN_file_Device_FW2;
	}

	if (readFlashInformation->sGhnGetActiveFWState.FW1_valid)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the FW SPI header from the device
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("Get the FW SPI flash header (%s)...\t\t\t\t", "FW1");
		}

		if (readFlashInformation->EBL_Version == 15)
		{
			readFlashBlock.Offset = readFlashInformation->EBL_Offset_Table.Offset_FW1_SPI;
			readFlashBlock.Length = FW_SPI_FLASH_SIZE();
			readFlashBlock.bHasChecksum_And_Length = FALSE;
		}
		else
		{
			readFlashBlock.Offset = readFlashInformation->EBL_Offset_Table.Offset_FW1_SPI;
			readFlashBlock.bHasChecksum_And_Length = TRUE;
			readFlashBlock.bNeedNullifyBeforeChecksum = TRUE;
			readFlashBlock.NullifyOffset = Get_FW_SPI_ActiveBit_Offset();
		}

		if ((GHN_LIB_STAT = Ghn_Read_Flash_Block(&readFlashBlock)) != eGHN_LIB_STAT_SUCCESS)
		{
			if (readFlashInformation->bPrint_Process_Information)
			{
				printf("[FAILED]\n");
			}

			sprintf(readFlashInformation->ErrorDescription,"failed to get the FW SPI header from the device");
			LOG_INFO(readFlashInformation->ErrorDescription);
			goto exit;
		}
		if (readFlashInformation->bPrint_Process_Information)
		{
			Printf_Highlight("[OK]\n");
		}

		AppendSection_From_File(BIN_file_Device_FW1, eBIN_Section_FW_SPI, TempFile);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Parse the FW SPI Offset Table
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("Parse the FW SPI flash header (%s)...\t\t\t\t", "FW1");
		}
		if (Parse_FW_Table(BIN_file_Device_FW1, &readFlashInformation->FW_Offset_Table_FW1) == FALSE)
		{
			if (readFlashInformation->bPrint_Process_Information)
			{
				printf("[FAILED]\n");
			}

			cg_stat = CG_STAT_FAILURE;
			sprintf(readFlashInformation->ErrorDescription,"failed to parse the FW SPI header (stat=%lu)",cg_stat);
			LOG_INFO(readFlashInformation->ErrorDescription);
			GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
			goto exit;
		}
		if (readFlashInformation->bPrint_Process_Information)
		{
			Printf_Highlight("[OK]\n");
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (readFlashInformation->sGhnGetActiveFWState.FW1_valid)
	{
		if (readFlashInformation->FW_Offset_Table_FW1.BIN_FILE_HEADER.Offset != 0x00000000)
		{
			if (readFlashInformation->bPrint_Process_Information)
			{
				printf("Get the BIN File Header from the device (%s)...\t\t", "FW1");
			}

			if (readFlashInformation->EBL_Version == 15)
			{
				readFlashBlock.Offset = readFlashInformation->EBL_Offset_Table.Offset_FW1_SPI + readFlashInformation->FW_Offset_Table_FW1.BIN_FILE_HEADER.Offset;
				readFlashBlock.Length = FW_BIN_FILE_HEADER_FLASH_SIZE();
				readFlashBlock.bHasChecksum_And_Length = FALSE;
			}
			else
			{
				readFlashBlock.Offset = readFlashInformation->EBL_Offset_Table.Offset_FW1_SPI + readFlashInformation->FW_Offset_Table_FW1.BIN_FILE_HEADER.Offset;
				readFlashBlock.bHasChecksum_And_Length = TRUE;
				readFlashBlock.bNeedNullifyBeforeChecksum = FALSE;
			}

			if ((GHN_LIB_STAT = Ghn_Read_Flash_Block(&readFlashBlock)) != eGHN_LIB_STAT_SUCCESS)
			{
				if (readFlashInformation->bPrint_Process_Information)
				{
					printf("[FAILED]\n");
				}

				sprintf(readFlashInformation->ErrorDescription,"failed to get the BIN File Header from the device");
				LOG_INFO(readFlashInformation->ErrorDescription);
				goto exit;
			}

			if (readFlashInformation->bPrint_Process_Information)
			{
				Printf_Highlight("[OK]\n");
			}

			AppendSection_From_File(BIN_file_Device_FW1, eBIN_Section_BIN_FILE_HEADER, TempFile);
		}
	}

	if (readFlashInformation->sGhnGetActiveFWState.FW2_valid)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the FW SPI header from the device
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("Get the FW SPI flash header (%s)...\t\t\t\t", "FW2");
		}
		
		if (readFlashInformation->EBL_Version == 15)
		{
			readFlashBlock.Offset = readFlashInformation->EBL_Offset_Table.Offset_FW2_SPI;
			readFlashBlock.Length = FW_SPI_FLASH_SIZE();
			readFlashBlock.bHasChecksum_And_Length = FALSE;
		}
		else
		{
			readFlashBlock.Offset = readFlashInformation->EBL_Offset_Table.Offset_FW2_SPI;
			readFlashBlock.bHasChecksum_And_Length = TRUE;
			readFlashBlock.bNeedNullifyBeforeChecksum = TRUE;
			readFlashBlock.NullifyOffset = Get_FW_SPI_ActiveBit_Offset();
		}

		if ((GHN_LIB_STAT = Ghn_Read_Flash_Block(&readFlashBlock)) != eGHN_LIB_STAT_SUCCESS)
		{
			if (readFlashInformation->bPrint_Process_Information)
			{
				printf("[FAILED]\n");
			}

			sprintf(readFlashInformation->ErrorDescription,"failed to get the FW SPI header from the device");
			LOG_INFO(readFlashInformation->ErrorDescription);
			goto exit;
		}
		if (readFlashInformation->bPrint_Process_Information)
		{
			Printf_Highlight("[OK]\n");
		}

		AppendSection_From_File(BIN_file_Device_FW2, eBIN_Section_FW_SPI, TempFile);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Parse the FW SPI Offset Table
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("Parse the FW SPI flash header (%s)...\t\t\t\t", "FW2");
		}
		if (Parse_FW_Table(BIN_file_Device_FW2, &readFlashInformation->FW_Offset_Table_FW2) == FALSE)
		{
			if (readFlashInformation->bPrint_Process_Information)
			{
				printf("[FAILED]\n");
			}

			cg_stat = CG_STAT_FAILURE;
			sprintf(readFlashInformation->ErrorDescription,"failed to parse the FW SPI header (stat=%lu)",cg_stat);
			LOG_INFO(readFlashInformation->ErrorDescription);
			GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
			goto exit;
		}
		if (readFlashInformation->bPrint_Process_Information)
		{
			Printf_Highlight("[OK]\n");
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (readFlashInformation->sGhnGetActiveFWState.FW2_valid)
	{
		if (readFlashInformation->FW_Offset_Table_FW2.BIN_FILE_HEADER.Offset != 0x00000000)
		{

			if (readFlashInformation->bPrint_Process_Information)
			{
				printf("Get the BIN File Header from the device (%s)...\t\t", "FW2");
			}

			if (readFlashInformation->EBL_Version == 15)
			{
				readFlashBlock.Offset = readFlashInformation->EBL_Offset_Table.Offset_FW2_SPI + readFlashInformation->FW_Offset_Table_FW2.BIN_FILE_HEADER.Offset;
				readFlashBlock.Length = FW_BIN_FILE_HEADER_FLASH_SIZE();
				readFlashBlock.bHasChecksum_And_Length = FALSE;
			}
			else
			{
				readFlashBlock.Offset = readFlashInformation->EBL_Offset_Table.Offset_FW2_SPI + readFlashInformation->FW_Offset_Table_FW2.BIN_FILE_HEADER.Offset;
				readFlashBlock.bHasChecksum_And_Length = TRUE;
				readFlashBlock.bNeedNullifyBeforeChecksum = FALSE;
			}

			if ((GHN_LIB_STAT = Ghn_Read_Flash_Block(&readFlashBlock)) != eGHN_LIB_STAT_SUCCESS)
			{
				if (readFlashInformation->bPrint_Process_Information)
				{
					printf("[FAILED]\n");
				}

				sprintf(readFlashInformation->ErrorDescription,"failed to get the BIN File Header from the device");
				LOG_INFO(readFlashInformation->ErrorDescription);
				goto exit;
			}
			if (readFlashInformation->bPrint_Process_Information)
			{
				Printf_Highlight("[OK]\n");
			}

			AppendSection_From_File(BIN_file_Device_FW2, eBIN_Section_BIN_FILE_HEADER, TempFile);
		}
	}
	
	if (readFlashInformation->sGhnGetActiveFWState.launched == 1)
	{
		readFlashInformation->FW_Offset_Table_Active = readFlashInformation->FW_Offset_Table_FW1;
	}
	else
	{
		readFlashInformation->FW_Offset_Table_Active = readFlashInformation->FW_Offset_Table_FW2;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the Manufacturer Block from the device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("Get the Manufacturer Block from the device...\t\t\t");
	}

	if (readFlashInformation->EBL_Version == 15)
	{
		readFlashBlock.Offset = readFlashInformation->Offset_FW_Active_SPI + readFlashInformation->FW_Offset_Table_Active.Manufacturer_Block.Offset;
		readFlashBlock.Length = readFlashInformation->FW_Offset_Table_Active.Manufacturer_Block.MaxSize;
		readFlashBlock.bHasChecksum_And_Length = FALSE;
	}
	else
	{
		readFlashBlock.Offset = readFlashInformation->Offset_FW_Active_SPI + readFlashInformation->FW_Offset_Table_Active.Manufacturer_Block.Offset;
		readFlashBlock.bHasChecksum_And_Length = TRUE;
		readFlashBlock.bNeedNullifyBeforeChecksum = FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Read_Flash_Block(&readFlashBlock)) != eGHN_LIB_STAT_SUCCESS)
	{
		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}

		sprintf(readFlashInformation->ErrorDescription,"failed to parse the Manufacturer Block");
		LOG_INFO(readFlashInformation->ErrorDescription);
		goto exit;
	}

	if (readFlashInformation->bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
	}

	AppendSection_From_File(BIN_file_Device_Active, eBIN_Section_Manufacturer_Block, TempFile);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the Parameters Block from the device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("Get the Parameters Block from the device...\t\t\t");
	}

	if (readFlashInformation->EBL_Version == 15)
	{
		readFlashBlock.Offset = readFlashInformation->Offset_FW_Active_SPI + readFlashInformation->FW_Offset_Table_Active.Parameters_Block.Offset;
		readFlashBlock.Length = readFlashInformation->FW_Offset_Table_Active.Parameters_Block.MaxSize;
		readFlashBlock.bHasChecksum_And_Length = FALSE;
	}
	else
	{
		readFlashBlock.Offset = readFlashInformation->Offset_FW_Active_SPI + readFlashInformation->FW_Offset_Table_Active.Parameters_Block.Offset;
		readFlashBlock.bHasChecksum_And_Length = TRUE;
		readFlashBlock.bNeedNullifyBeforeChecksum = FALSE;
	}

	if ((GHN_LIB_STAT = Ghn_Read_Flash_Block(&readFlashBlock)) != eGHN_LIB_STAT_SUCCESS)
	{
		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}

		sprintf(readFlashInformation->ErrorDescription,"failed to parse the Parameters Block");
		LOG_INFO(readFlashInformation->ErrorDescription);
		goto exit;
	}

	if (readFlashInformation->bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
	}

	AppendSection_From_File(BIN_file_Device_Active, eBIN_Section_Parameters_Block, TempFile);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Try to get the Modified Parameters 1 Block from the device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("Try to get the Modified Parameters 1 Block from the device...\t");
	}
	
	if (readFlashInformation->EBL_Version == 15)
	{
		readFlashBlock.Offset = readFlashInformation->Offset_FW_Active_SPI + readFlashInformation->FW_Offset_Table_Active.Modified_Parameters_1_Block.Offset;
		readFlashBlock.Length = readFlashInformation->FW_Offset_Table_Active.Modified_Parameters_1_Block.MaxSize - FLASH_BLOCK_HEADER_SIZE;
		readFlashBlock.bHasChecksum_And_Length = FALSE;
	}
	else
	{
		readFlashBlock.Offset = readFlashInformation->Offset_FW_Active_SPI + readFlashInformation->FW_Offset_Table_Active.Modified_Parameters_1_Block.Offset;
		readFlashBlock.bHasChecksum_And_Length = TRUE;
		readFlashBlock.bNeedNullifyBeforeChecksum = TRUE;
		readFlashBlock.NullifyOffset = 0x00;
	}

	if ((GHN_LIB_STAT = Ghn_Read_Flash_Block(&readFlashBlock)) != eGHN_LIB_STAT_SUCCESS)
	{
		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}
		
		Append_TLV_Section(BIN_file_Device_Active, eBIN_Section_Modified_Parameters_1_Block, readFlashInformation->FW_Offset_Table_Active.Modified_Parameters_1_Block.MaxSize - FLASH_BLOCK_HEADER_SIZE);
		
		readFlashInformation->Modified_Parameters_1_Block_Is_Valid = FALSE;
	}
	else
	{
		if (readFlashInformation->bPrint_Process_Information)
		{
			Printf_Highlight("[OK]\n");
		}

		AppendSection_From_File(BIN_file_Device_Active, eBIN_Section_Modified_Parameters_1_Block, TempFile);

		readFlashInformation->Modified_Parameters_1_Block_Is_Valid = TRUE;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Try to get the Modified Parameters 2 Block from the device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("Try to get the Modified Parameters 2 Block from the device...\t");
	}
	
	if (readFlashInformation->EBL_Version == 15)
	{
		readFlashBlock.Offset = readFlashInformation->Offset_FW_Active_SPI + readFlashInformation->FW_Offset_Table_Active.Modified_Parameters_2_Block.Offset;
		readFlashBlock.Length = readFlashInformation->FW_Offset_Table_Active.Modified_Parameters_2_Block.MaxSize - FLASH_BLOCK_HEADER_SIZE;
		readFlashBlock.bHasChecksum_And_Length = FALSE;
	}
	else
	{
		readFlashBlock.Offset = readFlashInformation->Offset_FW_Active_SPI + readFlashInformation->FW_Offset_Table_Active.Modified_Parameters_2_Block.Offset;
		readFlashBlock.bHasChecksum_And_Length = TRUE;
		readFlashBlock.bNeedNullifyBeforeChecksum = TRUE;
		readFlashBlock.NullifyOffset = 0x00;
	}

	if ((GHN_LIB_STAT = Ghn_Read_Flash_Block(&readFlashBlock)) != eGHN_LIB_STAT_SUCCESS)
	{
		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}

		Append_TLV_Section(BIN_file_Device_Active, eBIN_Section_Modified_Parameters_2_Block, readFlashInformation->FW_Offset_Table_Active.Modified_Parameters_2_Block.MaxSize - FLASH_BLOCK_HEADER_SIZE);

		readFlashInformation->Modified_Parameters_2_Block_Is_Valid = FALSE;
	}
	else
	{
		if (readFlashInformation->bPrint_Process_Information)
		{
			Printf_Highlight("[OK]\n");
		}

		AppendSection_From_File(BIN_file_Device_Active, eBIN_Section_Modified_Parameters_2_Block, TempFile);

		readFlashInformation->Modified_Parameters_2_Block_Is_Valid = TRUE;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Decide which is the Active Modified Parameters Block
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashInformation->bPrint_Process_Information)
	{
		printf("Decide which is the Active Modified Parameters Block...\t\t");
	}

	if ((GHN_LIB_STAT = Ghn_Get_Modified_Parameters_Validity_Byte(BIN_file_Device_Active,
																&readFlashInformation->Modified_Parameters_Validity_Byte)) != eGHN_LIB_STAT_SUCCESS)
	{

		if (readFlashInformation->bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}

		GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;

		sprintf(readFlashInformation->ErrorDescription,"failed to decide which is the Active Modified Parameters Block");
		LOG_INFO(readFlashInformation->ErrorDescription);
		goto exit;
	}

	if (readFlashInformation->bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


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

	return GHN_LIB_STAT;
}

eGHN_LIB_STAT Ghn_Get_Modified_Parameters_Validity_Byte(	BIN_file_t* BIN_file,
															UINT8*		Modified_Parameters_Validity_Byte)
{
	UINT8						Modified_Parameters_1_Validity_Byte;
	UINT8						Modified_Parameters_2_Validity_Byte;

	*Modified_Parameters_Validity_Byte = 0;

	Modified_Parameters_1_Validity_Byte = 0;
	Modified_Parameters_2_Validity_Byte = 0;

	Get_TLV_Modified_Parameter_Validity_Byte(BIN_file, eBIN_Section_Modified_Parameters_1_Block, &Modified_Parameters_1_Validity_Byte);
	Get_TLV_Modified_Parameter_Validity_Byte(BIN_file, eBIN_Section_Modified_Parameters_2_Block, &Modified_Parameters_2_Validity_Byte);

	if (Modified_Parameters_1_Validity_Byte == Modified_Parameters_Block_Valid_and_Active)
	{
		*Modified_Parameters_Validity_Byte = 1;
	}
	else if (Modified_Parameters_2_Validity_Byte == Modified_Parameters_Block_Valid_and_Active)
	{
		*Modified_Parameters_Validity_Byte = 2;
	}
	else if (Modified_Parameters_1_Validity_Byte == Modified_Parameters_Block_Valid)
	{
		*Modified_Parameters_Validity_Byte = 1;
	}
	else if (Modified_Parameters_2_Validity_Byte == Modified_Parameters_Block_Valid)
	{
		*Modified_Parameters_Validity_Byte = 2;
	}
	else
	{
		return eGHN_LIB_STAT_FAILURE;
	}

	return eGHN_LIB_STAT_SUCCESS;
}

// Read Block from the flash according to:
// 1. Offset & Length	(Section does not contain Checksum & Length)
// 2. Offset			(Section does contain Checksum & Length)
eGHN_LIB_STAT Ghn_Read_Flash_Block(sRead_Flash_Block* readFlashBlock)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	
	Layer2Connection			layer2Connection;
	bool						bHas_layer2Connection = FALSE;

	cg_stat_t					cg_stat;

	UINT32						CheckSum;

	BIN_section_header_t		BIN_section_header;

	FILE*						file;
	UINT8*						Buffer = NULL;


	LOG_INFO("Started...");

	fflush(stdout);

	if ((GHN_LIB_STAT = Open_Layer2_Connection(	&readFlashBlock->Connection,
												ETHER_TYPE_GHN,
												&layer2Connection,
												readFlashBlock->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
		goto exit;
	}

	bHas_layer2Connection = TRUE;

	fflush(stdout);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the header of the section from the device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashBlock->bHasChecksum_And_Length)
	{
		if (readFlashBlock->bPrint_Process_Information)
		{
			printf("Get the header of the section from the device...\t");
		}

		if ((cg_stat = GetFlashCommandFunction(&layer2Connection,
												readFlashBlock->Offset,
												8,
												(UINT8*)&BIN_section_header.CheckSum)) != CG_STAT_SUCCESS)
		{
			if (readFlashBlock->bPrint_Process_Information)
			{
				printf("[FAILED]\n");
			}

			cg_stat = CG_STAT_FAILURE;
			sprintf(readFlashBlock->ErrorDescription,"failed to read the header of the section from the device (stat=%lu)",cg_stat);
			LOG_INFO(readFlashBlock->ErrorDescription);
			GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
			goto exit;
		}

		if ((letohl(BIN_section_header.CheckSum)	== 0xFFFFFFFF) &&
			(letohl(BIN_section_header.Length)		== 0xFFFFFFFF))
		{
			if (readFlashBlock->bPrint_Process_Information)
			{
				printf("[FAILED]\n");
			}

			cg_stat = CG_STAT_FAILURE;
			sprintf(readFlashBlock->ErrorDescription,"Section is not programmed on the device's flash (stat=%lu)",cg_stat);
			LOG_INFO(readFlashBlock->ErrorDescription);
			GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
			goto exit;
		}

		if (readFlashBlock->bPrint_Process_Information)
		{
			Printf_Highlight("[OK]\n");
		}

		readFlashBlock->Offset += FLASH_BLOCK_HEADER_SIZE;
		readFlashBlock->Length = letohl(BIN_section_header.Length);
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the Section from the device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashBlock->bPrint_Process_Information)
	{
		printf("Get the section from the device...\t");
	}
	if ((cg_stat = GetFlashIntoFileCommandFunction(	&layer2Connection,
													readFlashBlock->Offset,
													readFlashBlock->Length,
													readFlashBlock->OutputFileName,
													readFlashBlock->bTrim_Suffix_Of_0xFF)) != CG_STAT_SUCCESS)
	{
		if (readFlashBlock->bPrint_Process_Information)
		{
			printf("[FAILED]\n");
		}

		cg_stat = CG_STAT_FAILURE;
		sprintf(readFlashBlock->ErrorDescription,"failed to read the section from the device (stat=%lu)",cg_stat);
		LOG_INFO(readFlashBlock->ErrorDescription);
		GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
		goto exit;
	}

	if (readFlashBlock->bPrint_Process_Information)
	{
		Printf_Highlight("[OK]\n");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Check the checksum
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (readFlashBlock->bHasChecksum_And_Length)
	{
		if (readFlashBlock->bPrint_Process_Information)
		{
			printf("Checking the checksum...\t\t\t");
		}

		if ((Buffer = (UINT8*)malloc(readFlashBlock->Length)) == NULL)
		{
			if (readFlashBlock->bPrint_Process_Information)
			{
				printf("[FAILED]\n");
			}

			cg_stat = CG_STAT_FAILURE;
			sprintf(readFlashBlock->ErrorDescription,"failed to malloc memory");
			LOG_INFO(readFlashBlock->ErrorDescription);
			GHN_LIB_STAT = eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;
			goto exit;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Read from the file
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		file = fopen(readFlashBlock->OutputFileName,"rb");
		if (file == NULL)
		{
			if (readFlashBlock->bPrint_Process_Information)
			{
				printf("[FAILED]\n");
			}

			cg_stat = CG_STAT_FAILURE;
			sprintf(readFlashBlock->ErrorDescription,"failed to open the file");
			LOG_INFO(readFlashBlock->ErrorDescription);
			GHN_LIB_STAT = eGHN_LIB_STAT_FAILED_READ_FILE;
			goto exit;
		}

		fread(Buffer,1,readFlashBlock->Length,file);
		fclose(file);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		if (readFlashBlock->bNeedNullifyBeforeChecksum)
		{
			Buffer[readFlashBlock->NullifyOffset] = 0x00;
		}

		CheckSum = CalculatePacketChecksum((UINT8*)Buffer, readFlashBlock->Length);

		if (CheckSum != letohl(BIN_section_header.CheckSum))
		{
			if (readFlashBlock->bPrint_Process_Information)
			{
				printf("[FAILED]\n");
			}

			cg_stat = CG_STAT_FAILURE;
			sprintf(readFlashBlock->ErrorDescription,"CheckSum mismatch");
			LOG_INFO(readFlashBlock->ErrorDescription);
			GHN_LIB_STAT = eGHN_LIB_STAT_FAILED_READ_FILE;
			goto exit;
		}

		if (readFlashBlock->bPrint_Process_Information)
		{
			Printf_Highlight("[OK]\n");
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	GHN_LIB_STAT = eGHN_LIB_STAT_SUCCESS;

exit:

	if (Buffer != NULL)
	{
		free(Buffer);
	}

	if (bHas_layer2Connection == TRUE)
	{
		Close_Layer2_Connection(&layer2Connection);
	}
	
	return GHN_LIB_STAT;
}

eGHN_LIB_STAT Ghn_Read_Flash_Section(sRead_Flash_Section* readFlashSection)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	
	sRead_Flash_Block			readFlashBlock;

	LOG_INFO("Started...");

	fflush(stdout);

	memset(&readFlashBlock, 0x00, sizeof(readFlashBlock));

	readFlashBlock.bTrim_Suffix_Of_0xFF = readFlashSection->bTrim_Suffix_Of_0xFF;

	if (readFlashSection->readFlashInformation.EBL_Version == 15)
	{
		readFlashBlock.bHasChecksum_And_Length = FALSE;
	}
	else
	{
		readFlashBlock.bHasChecksum_And_Length = TRUE;
	}
	strcpy(readFlashBlock.OutputFileName, readFlashSection->OutputFileName);

	readFlashBlock.Connection = readFlashSection->readFlashInformation.Connection;

	fflush(stdout);

	switch (readFlashSection->Section)
	{
		case eBIN_Section_EBL_SPI_BCB:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.EBL_Offset_Table.Offset_EBL_SPI;
			readFlashBlock.Length = EBL_BCB_FLASH_SIZE(readFlashSection->readFlashInformation.Device_ChipType);
			readFlashBlock.bHasChecksum_And_Length = FALSE;
			break;
		}

		case eBIN_Section_EBL_B1:
		{
			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Offset = readFlashSection->readFlashInformation.EBL_Offset_Table.Offset_EBL_B1;
				readFlashBlock.Length = 16384; // Max B1 Size
			}
			else
			{
				readFlashBlock.Offset = readFlashSection->readFlashInformation.EBL_Offset_Table.Offset_EBL_B1 - FLASH_BLOCK_HEADER_SIZE;
			}
			break;
		}

		case eBIN_Section_FW_SPI:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.SPI.Offset;

			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Length = FW_SPI_FLASH_SIZE();
			}
			else
			{
				readFlashBlock.bNeedNullifyBeforeChecksum = TRUE;
				readFlashBlock.NullifyOffset = Get_FW_SPI_ActiveBit_Offset();
			}
			break;
		}

		case eBIN_Section_FW_PCM:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.PCM.Offset;

			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Length = readFlashSection->readFlashInformation.FW_Offset_Table_Active.PCM.MaxSize;
			}
			break;
		}

		case eBIN_Section_FW_HW:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.HW.Offset;

			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Length = readFlashSection->readFlashInformation.FW_Offset_Table_Active.HW.MaxSize;
			}
			break;
		}
		
		case eBIN_Section_FW_GP:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.GP.Offset;

			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Length = readFlashSection->readFlashInformation.FW_Offset_Table_Active.GP.MaxSize;
			}
			break;
		}

		case eBIN_Section_FW_CP:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.CP.Offset;

			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Length = readFlashSection->readFlashInformation.FW_Offset_Table_Active.CP.MaxSize;
			}
			break;
		}
		
		case eBIN_Section_FW_FWL:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.FWL.Offset;

			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Length = readFlashSection->readFlashInformation.FW_Offset_Table_Active.FWL.MaxSize;
			}
			break;
		}

		case eBIN_Section_BIN_FILE_HEADER:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.BIN_FILE_HEADER.Offset;

			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Length = FW_BIN_FILE_HEADER_FLASH_SIZE();
			}
			break;
		}

		case eBIN_Section_FW_RW_GP:
		{
			// NOT IN USE ANY MORE!!!
			break;
		}

		case eBIN_Section_SW_DB_XML:
		{
			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.SW_DB_XML.Offset + 12;
				readFlashBlock.Length = readFlashSection->readFlashInformation.FW_Offset_Table_Active.SW_DB_XML.MaxSize - 12;
			}
			else
			{
				readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.SW_DB_XML.Offset;
			}
			break;
		}
			
		case eBIN_Section_Manufacturer_Block:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.Manufacturer_Block.Offset;

			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Length = readFlashSection->readFlashInformation.FW_Offset_Table_Active.Manufacturer_Block.MaxSize;
			}
			break;
		}
		
		case eBIN_Section_Parameters_Block:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.Parameters_Block.Offset;

			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Length = readFlashSection->readFlashInformation.FW_Offset_Table_Active.Parameters_Block.MaxSize;
			}
			break;
		}
			
		case eBIN_Section_Modified_Parameters_1_Block:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.Modified_Parameters_1_Block.Offset;
			
			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Length = readFlashSection->readFlashInformation.FW_Offset_Table_Active.Modified_Parameters_1_Block.MaxSize;
			}
			else
			{
				readFlashBlock.bNeedNullifyBeforeChecksum = TRUE;
				readFlashBlock.NullifyOffset = 0x00;
			}
			break;
		}

		case eBIN_Section_Modified_Parameters_2_Block:
		{
			readFlashBlock.Offset = readFlashSection->readFlashInformation.FW_Offset_Table_Active.Modified_Parameters_2_Block.Offset;

			if (readFlashSection->readFlashInformation.EBL_Version == 15)
			{
				readFlashBlock.Length = readFlashSection->readFlashInformation.FW_Offset_Table_Active.Modified_Parameters_2_Block.MaxSize;
			}
			else
			{
				readFlashBlock.bNeedNullifyBeforeChecksum = TRUE;
				readFlashBlock.NullifyOffset = 0x00;
			}
			break;
		}

		default:
		{
			GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
			goto exit;
		}
	}

	if ((readFlashSection->Section != eBIN_Section_EBL_SPI_BCB) &&
		(readFlashSection->Section != eBIN_Section_EBL_B1))
	{
		if (readFlashSection->ReadFlash == eRead_Flash_Active_FW)
		{
			readFlashBlock.Offset += readFlashSection->readFlashInformation.Offset_FW_Active_SPI;
		}
		else if (readFlashSection->ReadFlash == eRead_Flash_FW1)
		{
			readFlashBlock.Offset += readFlashSection->readFlashInformation.EBL_Offset_Table.Offset_FW1_SPI;
		}
		else if (readFlashSection->ReadFlash == eRead_Flash_FW2)
		{
			readFlashBlock.Offset += readFlashSection->readFlashInformation.EBL_Offset_Table.Offset_FW2_SPI;
		}
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Read Flash Block
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	GHN_LIB_STAT = Ghn_Read_Flash_Block(&readFlashBlock);

	if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(readFlashSection->ErrorDescription, readFlashBlock.ErrorDescription);
		LOG_INFO(readFlashSection->ErrorDescription);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

exit:

	return GHN_LIB_STAT;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
