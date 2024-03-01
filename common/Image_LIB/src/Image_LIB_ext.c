#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "windows.h"
#endif

#include "spiboot.h" /* FW file */

#include "Image_LIB_ext.h"

#include "md5.h"
#include "common.h"

#define SIG_FMT                 "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"
#define SIG_ADDR(p)             (p)[0],(p)[1],(p)[2],(p)[3],(p)[4],(p)[5],(p)[6],(p)[7],(p)[8],(p)[9],(p)[10],(p)[11],(p)[12],(p)[13],(p)[14],(p)[15]

UINT32 CalculatePacketChecksum(UINT8 *pDataPacketBuffer, UINT32 u32Len)
{
	unsigned int dwCount = 0;
	unsigned int g_Check_SUM , g_CurrentDWORD = 0;
	/*
	 * DWORD Count
	 */
	dwCount = u32Len / 4;

	g_Check_SUM = 0;
	   
	//Check packet check sum here
	while(dwCount)
	{
		READ_U32_FROM_PACKET(g_CurrentDWORD, pDataPacketBuffer);

		g_Check_SUM += g_CurrentDWORD;

		dwCount--;            	
	}

	switch(u32Len%4)
	{
		case 3:
		g_Check_SUM += (UINT32)((*(pDataPacketBuffer+2)) << 16);
		case 2:
		g_Check_SUM += (UINT32)((*(pDataPacketBuffer+1)) << 8);
		case 1:
		g_Check_SUM += (UINT32)(*pDataPacketBuffer);
	}
	g_Check_SUM += u32Len;

	TWOS_COMPLEMENT(g_Check_SUM);

	return g_Check_SUM;
}

bool GetHeader(BIN_file_t* BIN_file, BIN_file_header_t** BIN_file_header)
{
	BIN_section_header_t BIN_section_header;
	UINT32 Size = 0;

	if (BIN_file->BIN_file_Sections_Length == 0)
	{
		return FALSE;
	}

	// Search for the Section
	while (Size < BIN_file->BIN_file_Sections_Length)
	{
		memcpy(&BIN_section_header, &BIN_file->BIN_file_Sections[Size], sizeof(BIN_section_header_t));

		// Support Big/Little endian machines
		BIN_section_header.Section = letohl(BIN_section_header.Section);
		BIN_section_header.Length = letohl(BIN_section_header.Length);
		BIN_section_header.CheckSum = letohl(BIN_section_header.CheckSum);

		Size += sizeof(BIN_section_header_t);

		if (BIN_section_header.Section == eBIN_Section_BIN_FILE_HEADER)
		{
			*BIN_file_header = (BIN_file_header_t*)&(BIN_file->BIN_file_Sections[Size]);

			return TRUE;

		}

		Size += BIN_section_header.Length;
	}

	return FALSE;
}

bool GetSection(BIN_file_t* BIN_file, eBIN_Section Section, bool IncludingHeader, UINT8** SectionBuffer, UINT32* SectionSize)
{
	BIN_section_header_t BIN_section_header;
	UINT32 Size = 0;

	if (BIN_file->BIN_file_Sections_Length == 0)
	{
		return FALSE;
	}

	// Search for the Section
	while (Size < BIN_file->BIN_file_Sections_Length)
	{
		memcpy(&BIN_section_header, &BIN_file->BIN_file_Sections[Size], sizeof(BIN_section_header_t));

		// Support Big/Little endian machines
		BIN_section_header.Section = letohl(BIN_section_header.Section);
		BIN_section_header.Length = letohl(BIN_section_header.Length);
		BIN_section_header.CheckSum = letohl(BIN_section_header.CheckSum);

		Size += sizeof(BIN_section_header_t);

		if (BIN_section_header.Section == Section)
		{
			*SectionSize = BIN_section_header.Length;
			*SectionBuffer = &(BIN_file->BIN_file_Sections[Size]);

			if (IncludingHeader)
			{
				// Include the "Checksum" and "Length" fields
				*SectionSize += 8;
				*SectionBuffer -= 8;
			}

			return TRUE;

		}

		Size += BIN_section_header.Length;
	}

	return FALSE;
}

bool AppendHeader(BIN_file_t* BIN_file, BIN_file_header_t* BIN_file_header)
{
	BIN_section_header_t	BIN_section_header;
	UINT8*					Buffer;

	BIN_section_header.Section = eBIN_Section_BIN_FILE_HEADER;
	BIN_section_header.Length = sizeof(BIN_file_header_t);

	if (BIN_file->BIN_file_Sections_Length + BIN_section_header.Length > BIN_file->BIN_file_Sections_Max_Length)
	{
		fprintf(stderr,"AppendHeader() failed\n");
		return FALSE;
	}

	Buffer = (UINT8*)BIN_file_header;

	BIN_section_header.CheckSum = CalculatePacketChecksum((UINT8*)Buffer, BIN_section_header.Length);

	memcpy(&BIN_file->BIN_file_Sections[BIN_file->BIN_file_Sections_Length], &BIN_section_header, sizeof(BIN_section_header_t));
	BIN_file->BIN_file_Sections_Length += sizeof(BIN_section_header_t);

	memcpy(&BIN_file->BIN_file_Sections[BIN_file->BIN_file_Sections_Length], &Buffer[0], BIN_section_header.Length);
	BIN_file->BIN_file_Sections_Length += BIN_section_header.Length;

	return TRUE;
}

bool AppendSection_From_File(BIN_file_t* BIN_file, eBIN_Section Section, char* FileName)
{
	BIN_section_header_t	BIN_section_header;
	char*					Buffer;
	FILE*					f_in;
	UINT32					Length;

	UINT8					Validity_Byte;

	Length = file_get_size(FileName);

	if (Length == 0)
	{
		return FALSE;
	}

	if (BIN_file->BIN_file_Sections_Length + Length > BIN_file->BIN_file_Sections_Max_Length)
	{
		fprintf(stderr,"AppendSection_From_File() failed\n");
		return FALSE;
	}

	Buffer = (char*)malloc(Length);

	f_in = fopen(FileName, "rb");
	fread(Buffer, 1, Length, f_in);
	fclose(f_in);

	if (Section == eBIN_Section_FW_SPI)
	{
		Buffer[Get_FW_SPI_ActiveBit_Offset()] = 0x00;
	}
	else if ((Section == eBIN_Section_Modified_Parameters_1_Block) ||
		(Section == eBIN_Section_Modified_Parameters_2_Block))
	{
		Validity_Byte = Buffer[0];
		Buffer[0] = 0x00;
	}

	// Support Big/Little endian machines
	BIN_section_header.Section = htolel(Section);
	BIN_section_header.Length = htolel(Length);
	BIN_section_header.CheckSum = htolel(CalculatePacketChecksum((UINT8*)Buffer, Length));

	if (Section == eBIN_Section_FW_SPI)
	{
		Buffer[Get_FW_SPI_ActiveBit_Offset()] = 0xFF;
	}
	else if ((Section == eBIN_Section_Modified_Parameters_1_Block) ||
		(Section == eBIN_Section_Modified_Parameters_2_Block))
	{
		Buffer[0] = Validity_Byte;
	}

	memcpy(&BIN_file->BIN_file_Sections[BIN_file->BIN_file_Sections_Length], &BIN_section_header, sizeof(BIN_section_header_t));
	BIN_file->BIN_file_Sections_Length += sizeof(BIN_section_header_t);

	memcpy(&BIN_file->BIN_file_Sections[BIN_file->BIN_file_Sections_Length], &Buffer[0], Length);
	BIN_file->BIN_file_Sections_Length += Length;

	free(Buffer);

	return TRUE;
}

void UpdateChecksum(BIN_file_t* BIN_file, eBIN_Section Section)
{
	UINT8*					SectionBuffer = NULL;
	UINT32					SectionSize;

	BIN_section_header_t	BIN_section_header;
	UINT8*					SectionHeader = NULL;
	
	UINT8					Validity_Byte;

	if (GetSection(BIN_file, Section, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		printf("GetSection() Failed\n");
		return;
	}

	SectionHeader = SectionBuffer - sizeof(BIN_section_header_t);
	memcpy(&BIN_section_header, SectionHeader, sizeof(BIN_section_header_t));

	if (Section == eBIN_Section_FW_SPI)
	{
		SectionBuffer[Get_FW_SPI_ActiveBit_Offset()] = 0x00;
	}
	else if ((Section == eBIN_Section_Modified_Parameters_1_Block) ||
			 (Section == eBIN_Section_Modified_Parameters_2_Block))
	{
		Validity_Byte = SectionBuffer[0];
		SectionBuffer[0] = 0x00;
	}

	BIN_section_header.CheckSum = CalculatePacketChecksum((UINT8*)SectionBuffer, SectionSize);

	BIN_section_header.CheckSum = htolel(BIN_section_header.CheckSum);

	if (Section == eBIN_Section_FW_SPI)
	{
		SectionBuffer[Get_FW_SPI_ActiveBit_Offset()] = 0xFF;
	}
	else if ((Section == eBIN_Section_Modified_Parameters_1_Block) ||
			 (Section == eBIN_Section_Modified_Parameters_2_Block))
	{
		SectionBuffer[0] = Validity_Byte;
	}

	memcpy(SectionHeader, &BIN_section_header , sizeof(BIN_section_header_t));
}

bool ValidateChecksum(BIN_file_t* BIN_file, eBIN_Section Section)
{
	UINT8*					SectionBuffer = NULL;
	UINT32					SectionSize;

	BIN_section_header_t*	BIN_section_header;
	BIN_section_header_t	BIN_section_header_Aligned32Bit;
	UINT32					CheckSum_BIN_file;
	UINT32					CheckSum_Calculated;
	
	UINT8					Validity_Byte;

	if (GetSection(BIN_file, Section, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		printf("GetSection() Failed\n");
		return FALSE;
	}

	BIN_section_header = (BIN_section_header_t*)(SectionBuffer - sizeof(BIN_section_header_t));
	memcpy(&BIN_section_header_Aligned32Bit, BIN_section_header, sizeof(BIN_section_header_t));

	CheckSum_BIN_file	= letohl(BIN_section_header_Aligned32Bit.CheckSum);

	if (Section == eBIN_Section_FW_SPI)
	{
		SectionBuffer[Get_FW_SPI_ActiveBit_Offset()] = 0x00;
	}
	else if ((Section == eBIN_Section_Modified_Parameters_1_Block) ||
		(Section == eBIN_Section_Modified_Parameters_2_Block))
	{
		Validity_Byte = SectionBuffer[0];
		SectionBuffer[0] = 0x00;
	}

	CheckSum_Calculated	= CalculatePacketChecksum((UINT8*)SectionBuffer, SectionSize);

	if (Section == eBIN_Section_FW_SPI)
	{
		SectionBuffer[Get_FW_SPI_ActiveBit_Offset()] = 0xFF;
	}
	else if ((Section == eBIN_Section_Modified_Parameters_1_Block) ||
		(Section == eBIN_Section_Modified_Parameters_2_Block))
	{
		SectionBuffer[0] = Validity_Byte;
	}

	//printf("CheckSum_BIN_file=0x%08x CheckSum_Calculated=0x%08x\n", CheckSum_BIN_file, CheckSum_Calculated);

	if (CheckSum_BIN_file == CheckSum_Calculated)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

bool UpdateSignature(BIN_file_t* BIN_file)
{
	BIN_section_header_t BIN_section_header;
	BIN_file_header_t*	BIN_file_header;

	MD5Context MD5_FW;

	BYTE FW_Signature[16];

	UINT32 Size = 0;

	if (GetHeader(BIN_file, &BIN_file_header) == FALSE)
	{
		return FALSE;
	}

	MD5Init(&MD5_FW);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// calculate the signature
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	while (Size < BIN_file->BIN_file_Sections_Length)
	{
		memcpy(&BIN_section_header, &BIN_file->BIN_file_Sections[Size], sizeof(BIN_section_header_t));
		Size += sizeof(BIN_section_header_t);

		// Calculating the signature only for relevant sections
		if ((BIN_section_header.Section == eBIN_Section_EBL_SPI_BCB) ||
			(BIN_section_header.Section == eBIN_Section_EBL_B1) ||
			(BIN_section_header.Section == eBIN_Section_FW_SPI) ||
			(BIN_section_header.Section == eBIN_Section_FW_PCM) ||
			(BIN_section_header.Section == eBIN_Section_FW_HW) ||
			(BIN_section_header.Section == eBIN_Section_FW_GP) ||
			(BIN_section_header.Section == eBIN_Section_FW_CP) ||
			(BIN_section_header.Section == eBIN_Section_Parameters_Block) ||
			(BIN_section_header.Section == eBIN_Section_FW_FWL))
		{
			MD5Update(&MD5_FW, &BIN_file->BIN_file_Sections[Size], BIN_section_header.Length);
		}

		Size += BIN_section_header.Length;
	}

	MD5Final(&FW_Signature[0], &MD5_FW);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Update the signature
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	memcpy(BIN_file_header->FW_Signature, FW_Signature, 16);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	UpdateChecksum(BIN_file, eBIN_Section_BIN_FILE_HEADER);

	return TRUE;
}

bool ValidateSignature(BIN_file_t* BIN_file)
{
	BIN_section_header_t BIN_section_header;
	BIN_file_header_t*	BIN_file_header;

	MD5Context MD5_FW;

	BYTE FW_Signature[16];

	UINT32 Size = 0;

	if (GetHeader(BIN_file, &BIN_file_header) == FALSE)
	{
		return FALSE;
	}

	MD5Init(&MD5_FW);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// calculate the signature
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	while (Size < BIN_file->BIN_file_Sections_Length)
	{
		memcpy(&BIN_section_header, &BIN_file->BIN_file_Sections[Size], sizeof(BIN_section_header_t));

		// Support Big/Little endian machines
		BIN_section_header.Section = letohl(BIN_section_header.Section);
		BIN_section_header.Length = letohl(BIN_section_header.Length);
		BIN_section_header.CheckSum = letohl(BIN_section_header.CheckSum);

		Size += sizeof(BIN_section_header_t);

		// Calculating the signature only for relevant sections
		if ((BIN_section_header.Section == eBIN_Section_EBL_SPI_BCB) ||
			(BIN_section_header.Section == eBIN_Section_EBL_B1) ||
			(BIN_section_header.Section == eBIN_Section_FW_SPI) ||
			(BIN_section_header.Section == eBIN_Section_FW_PCM) ||
			(BIN_section_header.Section == eBIN_Section_FW_HW) ||
			(BIN_section_header.Section == eBIN_Section_FW_GP) ||
			(BIN_section_header.Section == eBIN_Section_FW_CP) ||
			(BIN_section_header.Section == eBIN_Section_Parameters_Block) ||
			(BIN_section_header.Section == eBIN_Section_FW_FWL))
		{
			MD5Update(&MD5_FW, &BIN_file->BIN_file_Sections[Size], BIN_section_header.Length);
		}

		Size += BIN_section_header.Length;
	}

	MD5Final(&FW_Signature[0], &MD5_FW);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Check the signature
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (memcmp(BIN_file_header->FW_Signature, FW_Signature, 16) != 0)
	{
		return FALSE;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	return TRUE;
}

bool Validate_BIN_file(BIN_file_t* BIN_file)
{
	BIN_file_header_t*	BIN_file_header;

	if (GetHeader(BIN_file, &BIN_file_header) == FALSE)
	{
		return FALSE;
	}

	if (ValidateSignature(BIN_file) == FALSE)
	{
		return FALSE;
	}

	// Check the CheckSum in all sections
	//printf("Checking Checksum for eBIN_Section_BIN_FILE_HEADER:\n");
	if (ValidateChecksum(BIN_file, eBIN_Section_BIN_FILE_HEADER) == FALSE)
	{
		return FALSE;
	}

	//printf("Checking Checksum for eBIN_Section_EBL_SPI_BCB:\n");
	if (ValidateChecksum(BIN_file, eBIN_Section_EBL_SPI_BCB) == FALSE)
	{
		return FALSE;
	}
	
	//printf("Checking Checksum for eBIN_Section_EBL_B1:\n");
	if (ValidateChecksum(BIN_file, eBIN_Section_EBL_B1) == FALSE)
	{
		return FALSE;
	}

	//printf("Checking Checksum for eBIN_Section_FW_SPI:\n");
	if (ValidateChecksum(BIN_file, eBIN_Section_FW_SPI) == FALSE)
	{
		return FALSE;
	}

	//printf("Checking Checksum for eBIN_Section_FW_PCM:\n");
	if (ValidateChecksum(BIN_file, eBIN_Section_FW_PCM) == FALSE)
	{
		return FALSE;
	}

	//printf("Checking Checksum for eBIN_Section_FW_HW:\n");
	if (ValidateChecksum(BIN_file, eBIN_Section_FW_HW) == FALSE)
	{
		return FALSE;
	}

	//printf("Checking Checksum for eBIN_Section_FW_GP:\n");
	if (ValidateChecksum(BIN_file, eBIN_Section_FW_GP) == FALSE)
	{
		return FALSE;
	}

	//printf("Checking Checksum for eBIN_Section_FW_CP:\n");
	if (ValidateChecksum(BIN_file, eBIN_Section_FW_CP) == FALSE)
	{
		return FALSE;
	}

	//printf("Checking Checksum for eBIN_Section_Parameters_Block:\n");
	if (ValidateChecksum(BIN_file, eBIN_Section_Parameters_Block) == FALSE)
	{
		return FALSE;
	}

	//printf("Checking Checksum for eBIN_Section_FW_FWL:\n");
	if (ValidateChecksum(BIN_file, eBIN_Section_FW_FWL) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}


bool Validate_Chip_Type(BIN_file_t* BIN_file,char *Device_ChipType)
{
	BIN_file_header_t*	BIN_file_header;
	
	char				ModelName[7*5]; /* Model-Name. e.g. "CG5210" or "CG5210,CG5230" */
	char*				ptr;

	if (GetHeader(BIN_file, &BIN_file_header) == FALSE)
	{
		return FALSE;
	}
	
	memset(ModelName, 0x00, sizeof(ModelName));

	if (letohs(BIN_file_header->HeaderVersion) < 2)
	{
		strncpy(ModelName, BIN_file_header->ModelName, sizeof(BIN_file_header->ModelName));
	}
	else
	{
		strncpy(ModelName, BIN_file_header->ModelNameExt, sizeof(BIN_file_header->ModelNameExt));
	}

	// Check match between bin file to device chip type 
	ptr = strtok(ModelName, ",");
	while (ptr != NULL)
	{
		if (strcmp(ptr, Device_ChipType) == 0)
		{
			return TRUE;
		}
		ptr = strtok(NULL, ",");
	}

	return FALSE;
}

bool Save_BIN_file(BIN_file_t* BIN_file,char* FileName)
{
	FILE *f_out;

	f_out = fopen(FileName,"wb");

	if (f_out == NULL)
	{
		return FALSE;
	}

	fwrite(&BIN_file->BIN_file_Sections, 1, BIN_file->BIN_file_Sections_Length, f_out);
	fclose(f_out);

	return TRUE;
}

bool Load_BIN_file(BIN_file_t* BIN_file,char* FileName)
{
	FILE *f_in;
	long FileSize;

	FileSize = file_get_size(FileName);

	if (FileSize < sizeof(BIN_file_header_t))
	{
		return FALSE;
	}

	f_in = fopen(FileName,"rb");

	if (f_in == NULL)
	{
		return FALSE;
	}

	if (BIN_file->BIN_file_Sections_Length + FileSize > BIN_file->BIN_file_Sections_Max_Length)
	{
		fprintf(stderr,"Load_BIN_file() failed\n");
		return FALSE;
	}

	BIN_file->BIN_file_Sections_Length = FileSize;

	fread(&BIN_file->BIN_file_Sections, 1, BIN_file->BIN_file_Sections_Length, f_in);

	fclose(f_in);

	return TRUE;
}

// Device_ChipType format is "CG5200"
UINT32 EBL_BCB_FLASH_SIZE(char*	Device_ChipType)
{
	if (Device_ChipType[3] == '2')
	{
		return sizeof(EBL_flash_header_t) + sizeof(BCB_data_t_Golan2);
	}
	else if (Device_ChipType[3] == '3')
	{
		return sizeof(EBL_flash_header_t) + sizeof(BCB_data_t_Golan3);
	}
	else
	{
		return 0;
	}
}
UINT32 FW_SPI_FLASH_SIZE()
{
	return sizeof(APP_flash_header_t);
}

UINT32 FW_BIN_FILE_HEADER_FLASH_SIZE()
{
	return sizeof(BIN_file_header_t);
}

bool Parse_EBL_Table_From_EBL_Header(	EBL_flash_header_t*		EBL_flash_header,
										EBL_Offset_Table_t*		EBL_SPI_Offset_Table)
{
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Sanity Check
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if ((letohl(EBL_flash_header->flash_root_header.BCB_magic_num)		!= BCB_MAGIC_NUM) ||
		(letohl(EBL_flash_header->flash_root_header.B1_magic_num)		!= B1_MAGIC_NUM) ||
		(letohl(EBL_flash_header->FW1_magic_num)						!= FW1_MAGIC_NUM) ||
		(letohl(EBL_flash_header->FW2_magic_num)						!= FW2_MAGIC_NUM))
	{
		return FALSE;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get Values
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	EBL_SPI_Offset_Table->Offset_EBL_SPI		= letohl(0x00000000);
	EBL_SPI_Offset_Table->Offset_EBL_BCB		= letohl(EBL_flash_header->flash_root_header.BCB_flash_offset);
	
	EBL_SPI_Offset_Table->Offset_EBL_B1			= letohl(EBL_flash_header->flash_root_header.B1_flash_offset);
	EBL_SPI_Offset_Table->Launch_Addr_EBL_B1	= letohl(EBL_flash_header->flash_root_header.B1_launch_addr);
	
	EBL_SPI_Offset_Table->Offset_FW1_SPI		= letohl(EBL_flash_header->FW1_flash_offset);
	EBL_SPI_Offset_Table->Offset_FW2_SPI		= letohl(EBL_flash_header->FW2_flash_offset);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	return TRUE;
}

bool Parse_FW_Table_From_FW_Header(	APP_flash_header_t*		APP_flash_header,
									FW_Offset_Table_t*		FW_Offset_Table)
{
	int				i;

	bool			bFound_HW_Section						= FALSE;
	bool			bFound_GLOBAL_PARAMS_Section			= FALSE;
	bool			bFound_Manufacturer_Section				= FALSE;
	bool			bFound_Configuration_Section			= FALSE;
	bool			bFound_Modified_Configuration_1_Section	= FALSE;
	bool			bFound_Modified_Configuration_2_Section	= FALSE;
	bool			bFound_PCM_Section						= FALSE;
	bool			bFound_BIN_FILE_HEADER_Section			= FALSE;
	bool			bFound_SW_DB_XML_Section				= FALSE;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Sanity Check
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (letohl(APP_flash_header->app_magic_num)			!= APP_MAGIC_NUM)
	{
		return FALSE;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get Values
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	FW_Offset_Table->SPI.Offset = letohl(0x00000000);
	FW_Offset_Table->SPI.MaxSize = sizeof(APP_flash_header_t);
	FW_Offset_Table->CP.Offset = letohl(APP_flash_header->app_spi_offset);
	FW_Offset_Table->CP.MaxSize = MAX_BIN_FILE_SIZE - EBL_SIZE - letohl(APP_flash_header->app_spi_offset);

	// Will be NULL when (EBL_Version < 17)
	FW_Offset_Table->FWL.Offset = letohl(APP_flash_header->FWL_spi_offset);
	FW_Offset_Table->FWL.MaxSize = letohl(APP_flash_header->FWL_max_size);

	for (i=1;i<=APP_flash_header->num_sections;i++)
	{
		if (letohl(APP_flash_header->flash_sections[i-1].magic_num) == HW_VECT_MAGIC_NUM)
		{
			FW_Offset_Table->HW.Offset = letohl(APP_flash_header->flash_sections[i-1].offset);
			FW_Offset_Table->HW.MaxSize = letohl(APP_flash_header->flash_sections[i-1].max_size);
			bFound_HW_Section = TRUE;
		}

		if (letohl(APP_flash_header->flash_sections[i-1].magic_num) == GLOBAL_PARAMS_MAGIC_NUM)
		{
			FW_Offset_Table->GP.Offset = letohl(APP_flash_header->flash_sections[i-1].offset);
			FW_Offset_Table->GP.MaxSize = letohl(APP_flash_header->flash_sections[i-1].max_size);
			bFound_GLOBAL_PARAMS_Section = TRUE;
		}

		if (letohl(APP_flash_header->flash_sections[i-1].magic_num) == MANUFACTURER_BLOCK_MAGIC_NUM)
		{
			FW_Offset_Table->Manufacturer_Block.Offset = letohl(APP_flash_header->flash_sections[i-1].offset);
			FW_Offset_Table->Manufacturer_Block.MaxSize = letohl(APP_flash_header->flash_sections[i-1].max_size);
			bFound_Manufacturer_Section = TRUE;
		}

		if (letohl(APP_flash_header->flash_sections[i-1].magic_num) == CONFIG_BLOCK_MAGIC_NUM)
		{
			FW_Offset_Table->Parameters_Block.Offset = letohl(APP_flash_header->flash_sections[i-1].offset);
			FW_Offset_Table->Parameters_Block.MaxSize = letohl(APP_flash_header->flash_sections[i-1].max_size);
			bFound_Configuration_Section = TRUE;
		}

		if (letohl(APP_flash_header->flash_sections[i-1].magic_num) == MODIFIED_CONFIG_1_BLOCK_MAGIC_NUM)
		{
			FW_Offset_Table->Modified_Parameters_1_Block.Offset = letohl(APP_flash_header->flash_sections[i-1].offset);
			FW_Offset_Table->Modified_Parameters_1_Block.MaxSize = letohl(APP_flash_header->flash_sections[i-1].max_size);
			bFound_Modified_Configuration_1_Section = TRUE;
		}
		
		if (letohl(APP_flash_header->flash_sections[i-1].magic_num) == MODIFIED_CONFIG_2_BLOCK_MAGIC_NUM)
		{
			FW_Offset_Table->Modified_Parameters_2_Block.Offset = letohl(APP_flash_header->flash_sections[i-1].offset);
			FW_Offset_Table->Modified_Parameters_2_Block.MaxSize = letohl(APP_flash_header->flash_sections[i-1].max_size);
			bFound_Modified_Configuration_2_Section = TRUE;
		}

		if (letohl(APP_flash_header->flash_sections[i-1].magic_num) == PCM_CONFIG_MAGIC_NUM)
		{
			FW_Offset_Table->PCM.Offset = letohl(APP_flash_header->flash_sections[i-1].offset);
			FW_Offset_Table->PCM.MaxSize = letohl(APP_flash_header->flash_sections[i-1].max_size);
			bFound_PCM_Section = TRUE;
		}

		if (letohl(APP_flash_header->flash_sections[i-1].magic_num) == BIN_FILE_HEADER_MAGIC_NUM)
		{
			FW_Offset_Table->BIN_FILE_HEADER.Offset = letohl(APP_flash_header->flash_sections[i-1].offset);
			FW_Offset_Table->BIN_FILE_HEADER.MaxSize = letohl(APP_flash_header->flash_sections[i-1].max_size);
			bFound_BIN_FILE_HEADER_Section = TRUE;
		}

		if (letohl(APP_flash_header->flash_sections[i-1].magic_num) == FW_XML_MAGIC_NUM)
		{
			FW_Offset_Table->SW_DB_XML.Offset= letohl(APP_flash_header->flash_sections[i-1].offset);
			FW_Offset_Table->SW_DB_XML.MaxSize= letohl(APP_flash_header->flash_sections[i-1].max_size);
			bFound_SW_DB_XML_Section = TRUE;
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	FW_Offset_Table->active = APP_flash_header->active;

	if ((bFound_HW_Section) &&
		(bFound_GLOBAL_PARAMS_Section) &&
		(bFound_Manufacturer_Section) &&
		(bFound_Configuration_Section) &&
		(bFound_Modified_Configuration_1_Section) &&
		(bFound_Modified_Configuration_2_Section) &&
		(bFound_PCM_Section) &&
		(bFound_BIN_FILE_HEADER_Section) &&
		(bFound_SW_DB_XML_Section) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

bool Parse_EBL_Table(	BIN_file_t*			BIN_file,
						char*				Device_ChipType,
						EBL_Offset_Table_t* EBL_SPI_Offset_Table)
{
	UINT8*					SectionBuffer = NULL;
	UINT32					SectionSize;
	EBL_flash_header_t*		EBL_flash_header;

	if (GetSection(BIN_file, eBIN_Section_EBL_SPI_BCB, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	if (SectionSize < EBL_BCB_FLASH_SIZE(Device_ChipType))
	{
		printf("\n");
		printf("***************************\n");
		printf("* Incorrect SPI file size *\n");
		printf("***************************\n");
		printf("\n");

		return FALSE;
	}

	EBL_flash_header = (EBL_flash_header_t*)SectionBuffer;

	if (Parse_EBL_Table_From_EBL_Header(EBL_flash_header, EBL_SPI_Offset_Table) == FALSE)
	{
		//LOG_INFO("Call Parse_EBL_Table() Failed");
		return FALSE;
	}

	return TRUE;
}

bool Parse_FW_Table(BIN_file_t*			BIN_file,
					FW_Offset_Table_t*	FW_SPI_Offset_Table)
{
	UINT8*					SectionBuffer = NULL;
	UINT32					SectionSize;
	APP_flash_header_t*		FW_APP_flash_header;

	memset(FW_SPI_Offset_Table, 0x00, sizeof(FW_Offset_Table_t));

	if (GetSection(BIN_file, eBIN_Section_FW_SPI, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	if (SectionSize < sizeof(APP_flash_header_t))
	{
		printf("\n");
		printf("******************************\n");
		printf("* Incorrect FW SPI file size *\n");
		printf("******************************\n");
		printf("\n");

		return FALSE;
	}

	FW_APP_flash_header = (APP_flash_header_t*)SectionBuffer;

	if (Parse_FW_Table_From_FW_Header(FW_APP_flash_header, FW_SPI_Offset_Table) == FALSE)
	{
		//LOG_INFO("Call Parse_FW_Table() Failed");
		return FALSE;
	}

	return TRUE;
}

bool Update_EBL_SPI_BCB(BIN_file_t*			BIN_file,
						char*				Device_ChipType,
						macStruct*			MAC_Address)
{
	UINT8*					SectionBuffer = NULL;
	UINT32					SectionSize;

	EBL_flash_header_t*		EBL_flash_header;
	EBL_Offset_Table_t		EBL_SPI_Offset_Table;
	BCB_data_t_Shared*		BCB_data;

	if (GetSection(BIN_file, eBIN_Section_EBL_SPI_BCB, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	if (SectionSize < EBL_BCB_FLASH_SIZE(Device_ChipType))
	{
		printf("\n");
		printf("*******************************\n");
		printf("* Incorrect EBL SPI file size *\n");
		printf("*******************************\n");
		printf("\n");
		
		return FALSE;
	}

	EBL_flash_header = (EBL_flash_header_t*)SectionBuffer;

	if (Parse_EBL_Table_From_EBL_Header(EBL_flash_header, &EBL_SPI_Offset_Table) == FALSE)
	{
		//LOG_INFO("Call Parse_SPI_Offset_Table() Failed");
		return FALSE;
	}

	BCB_data = (BCB_data_t_Shared*)&SectionBuffer[EBL_SPI_Offset_Table.Offset_EBL_BCB];

	// Update MAC-Address
	memcpy(BCB_data->mac_addr,MAC_Address->macAddress,HMAC_LEN);

	UpdateChecksum(BIN_file, eBIN_Section_EBL_SPI_BCB);

	return TRUE;
}

bool Update_EBL_SPI_BCB_Get_MAC_Address(BIN_file_t*		BIN_file,
										char*			Device_ChipType,
										macStruct*		MAC_Address)
{
	UINT8*					SectionBuffer = NULL;
	UINT32					SectionSize;

	EBL_flash_header_t*		EBL_flash_header;
	EBL_Offset_Table_t		EBL_SPI_Offset_Table;
	BCB_data_t_Shared*		BCB_data;

	if (GetSection(BIN_file, eBIN_Section_EBL_SPI_BCB, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	if (SectionSize < EBL_BCB_FLASH_SIZE(Device_ChipType))
	{
		printf("\n");
		printf("*******************************\n");
		printf("* Incorrect EBL SPI file size *\n");
		printf("*******************************\n");
		printf("\n");
		
		return FALSE;
	}

	EBL_flash_header = (EBL_flash_header_t*)SectionBuffer;

	if (Parse_EBL_Table_From_EBL_Header(EBL_flash_header, &EBL_SPI_Offset_Table) == FALSE)
	{
		//LOG_INFO("Call Parse_SPI_Offset_Table() Failed");
		return FALSE;
	}

	BCB_data = (BCB_data_t_Shared*)&SectionBuffer[EBL_SPI_Offset_Table.Offset_EBL_BCB];

	// Get the MAC-Address
	memcpy(MAC_Address->macAddress, BCB_data->mac_addr, HMAC_LEN);

	UpdateChecksum(BIN_file, eBIN_Section_EBL_SPI_BCB);

	return TRUE;
}

bool Update_FW_SPI(	BIN_file_t* BIN_file,
					UINT8		ActiveBit)
{
	UINT8*					SectionBuffer = NULL;
	UINT32					SectionSize;
	APP_flash_header_t*		FW_APP_flash_header;

	if (GetSection(BIN_file, eBIN_Section_FW_SPI, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	if (SectionSize < FW_SPI_FLASH_SIZE())
	{
		printf("\n");
		printf("******************************\n");
		printf("* Incorrect FW SPI file size *\n");
		printf("******************************\n");
		printf("\n");
		
		return FALSE;
	}

	FW_APP_flash_header = (APP_flash_header_t*)SectionBuffer;

	// Update Active-Bit
	FW_APP_flash_header->active = ActiveBit;

	UpdateChecksum(BIN_file, eBIN_Section_FW_SPI);

	return TRUE;
}

int Get_FW_SPI_ActiveBit_Offset()
{
	APP_flash_header_t FW_APP_flash_header;
	int ActiveBit_Offset;

	ActiveBit_Offset = (int)&FW_APP_flash_header.active - (int)&FW_APP_flash_header;

	return ActiveBit_Offset;
}


// Search for the FW version in a Buffer sized "size"
// "Running version: 01.02.000.1100"
bool Extract_FW_Version(char* line, int size, char* FW_Version)
{
	char	SearchString[256] = "Running version: ";
	int		SearchStringsize;
	char	FW_Version_Template[256] = "01.02.000.1100";
	int		i;

	SearchStringsize = strlen(SearchString);

	for(i=0;i<size;i++)
	{
		if ((strncmp(&(line[i]),SearchString,SearchStringsize)) == 0)
		{
			strncpy(FW_Version, &line[i+SearchStringsize], strlen(FW_Version_Template));
			return TRUE;
		}
	}

	return FALSE;
}

// Search for the FW-Version in BIN file
bool Get_BIN_FWVersion(char* BIN_FW_File, char* BIN_FW_Version)
{
	FILE*	binfile;
	char	line[4096];

	memset(line,0x00,sizeof(line));

	strcpy(BIN_FW_Version,"N/A");

	// Empty file
	if (strlen(BIN_FW_File)==0)
	{
		return FALSE;
	}

	binfile = fopen(BIN_FW_File,"rb");

	// File is not accessible
	if (binfile == NULL)
	{
		return FALSE;
	}

	while (fgets(line, sizeof(line), binfile) != NULL)
	{
		Extract_FW_Version(line, sizeof(line), BIN_FW_Version);
	}

	fclose(binfile);

	if (strcmp(BIN_FW_Version,"N/A") == 0)
	{
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
*                                                                                                  *
***************************************************************************************************/
void Print_BIN_file_Header(BIN_file_t* BIN_file, bool Highlighted)
{
	BIN_file_header_t*	BIN_file_header_p;

	char				ModelName[7*5];					/* Model-Name. e.g. "CG5210" or "CG5210,CG5230" */
	char				FW_Version[24+1];				/* FW Version */
	char				ConfigurationName[64+1];		/* Configuration Name (Profile Name) */
	char				BuildImageCreationTime[24+1];	/* The time the creation this BIN file */

	if (GetHeader(BIN_file, &BIN_file_header_p) == FALSE)
	{
		printf("Print_BIN_file_Header() Failed\n");
		return;
	}

	memset(ModelName, 0x00, sizeof(ModelName));
	memset(FW_Version, 0x00, sizeof(FW_Version));
	memset(ConfigurationName, 0x00, sizeof(ConfigurationName));
	memset(BuildImageCreationTime, 0x00, sizeof(BuildImageCreationTime));

	if (letohs(BIN_file_header_p->HeaderVersion) < 2)
	{
		strncpy(ModelName, BIN_file_header_p->ModelName, sizeof(BIN_file_header_p->ModelName));
	}
	else
	{
		strncpy(ModelName, BIN_file_header_p->ModelNameExt, sizeof(BIN_file_header_p->ModelNameExt));
	}
	strncpy(FW_Version, BIN_file_header_p->FW_Version, sizeof(BIN_file_header_p->FW_Version));
	strncpy(ConfigurationName, BIN_file_header_p->ConfigurationName, sizeof(BIN_file_header_p->ConfigurationName));
	strncpy(BuildImageCreationTime, BIN_file_header_p->BuildImageCreationTime, sizeof(BIN_file_header_p->BuildImageCreationTime));

	if (Highlighted)
	{
		char Temp[1024];
		
		// Print the Parameters
		sprintf(Temp, "ModelName                = %s\n", ModelName); Printf_Highlight(Temp);
		sprintf(Temp, "FW_Version               = %s\n", FW_Version); Printf_Highlight(Temp);
		sprintf(Temp, "ConfigurationName        = %s\n", ConfigurationName); Printf_Highlight(Temp);
		sprintf(Temp, "BuildImageCreationTime   = %s\n", BuildImageCreationTime); Printf_Highlight(Temp);
		sprintf(Temp, "TotalImageSize           = %d bytes\n", letohl(BIN_file_header_p->TotalImageSize)); Printf_Highlight(Temp);
		sprintf(Temp, "FW_Signature             = " SIG_FMT "\n", SIG_ADDR(BIN_file_header_p->FW_Signature)); Printf_Highlight(Temp);
	}
	else
	{
		// Print the Parameters
		printf("ModelName                = %s\n", ModelName);
		printf("FW_Version               = %s\n", FW_Version);
		printf("ConfigurationName        = %s\n", ConfigurationName);
		printf("BuildImageCreationTime   = %s\n", BuildImageCreationTime);
		printf("TotalImageSize           = %d bytes\n", letohl(BIN_file_header_p->TotalImageSize));
		printf("FW_Signature             = " SIG_FMT "\n", SIG_ADDR(BIN_file_header_p->FW_Signature));
	}
}
