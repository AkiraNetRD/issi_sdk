#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "windows.h"
#endif

#include "Image_LIB_ext.h"

#ifdef _WIN32
#define OS_STRICMP(str1,str2) _stricmp(str1,str2)
#elif __linux__
#define OS_STRICMP(str1,str2) strcasecmp(str1,str2)
#endif

bool Append_TLV_Section(BIN_file_t* BIN_file, eBIN_Section Section, UINT32 Length)
{
	BIN_section_header_t	BIN_section_header;

	BIN_section_header.Section = Section;
	BIN_section_header.Length = Length;

	if (BIN_file->BIN_file_Sections_Length + BIN_section_header.Length > BIN_file->BIN_file_Sections_Max_Length)
	{
		fprintf(stderr,"Append_TLV_Section() failed\n");
		return FALSE;
	}

	memcpy(&BIN_file->BIN_file_Sections[BIN_file->BIN_file_Sections_Length], &BIN_section_header, sizeof(BIN_section_header_t));
	BIN_file->BIN_file_Sections_Length += sizeof(BIN_section_header_t);

	// Fill with patten of 0xFF
	memset(&BIN_file->BIN_file_Sections[BIN_file->BIN_file_Sections_Length], 0xFF, BIN_section_header.Length);
	BIN_file->BIN_file_Sections_Length += BIN_section_header.Length;
	
	UpdateChecksum(BIN_file, Section);

	return TRUE;
}

bool Delete_TLV_Parameter(	BIN_file_t*		BIN_file,
							eBIN_Section	Section,
							char*			Delete_ParameterName)
{
	UINT8* SectionBuffer;
	UINT32 SectionSize;
	int Index1 = 0;
	int Index2 = 0;
	UINT8* SectionBuffer_Temp;

	char	ParameterName[MAX_TLV_PARAMETER_NAME_SIZE];
	UINT16	Data;
	UINT32	Length;
	UINT32	Type;

	int		StartIndex_FirstValue;

	if (GetSection(BIN_file, Section, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	if ((Section == eBIN_Section_Manufacturer_Block) || (Section == eBIN_Section_Parameters_Block))
	{
		StartIndex_FirstValue=0;
	}
	else if ((Section == eBIN_Section_Modified_Parameters_1_Block) ||
			 (Section == eBIN_Section_Modified_Parameters_2_Block))
	{
		if ((SectionBuffer[0] == Modified_Parameters_Block_Valid) ||
			(SectionBuffer[0] == Modified_Parameters_Block_Valid_and_Active))
		{
			StartIndex_FirstValue=1;
		}
		else
		{
			return FALSE;
		}
	}

	SectionBuffer_Temp = (UINT8*)malloc(SectionSize);

	if (SectionBuffer_Temp == NULL)
	{
		return FALSE;
	}

	memset(SectionBuffer_Temp, 0xFF, SectionSize);

	Index1=StartIndex_FirstValue;
	Index2=0;

	if ((Section == eBIN_Section_Modified_Parameters_1_Block) ||
		(Section == eBIN_Section_Modified_Parameters_2_Block))
	{
		// Copy the Active-Bit
		SectionBuffer_Temp[0] = SectionBuffer[0];
		Index2++;
	}
	
	while ((SectionBuffer[Index1]) != 0xFF)
	{
		if (strlen((char*)&SectionBuffer[Index1]) > MAX_TLV_PARAMETER_NAME_SIZE)
		{
			printf("TLV format is broken!\n");
			break;
		}

		// Read Parameter Name
		strcpy(ParameterName, (char*)&SectionBuffer[Index1]);

		if (strlen(ParameterName) == 0)
		{
			printf("TLV format is broken!\n");
			break;
		}

		Index1 += strlen(ParameterName) +1;

		// Read Parameter Length and Type
		memcpy(&Data, &SectionBuffer[Index1], 2);
		Data = letohs(Data);
		Index1 = Index1 +2;
		Length = Data >> 3;
		Type = Data & 0x07;

		if ((Type < eTLV_Type_Boolean) || (Type > eTLV_Type_Array) || (Length == 0))
		{
			printf("TLV format is broken!\n");
			break;
		}

		if (strcmp(ParameterName, Delete_ParameterName) == 0)
		{
			// Delete this Parameter
			Index1 += Length;
			continue;
		}

		// Copy this Parameter

		// Copy Parameter Name
		strcpy((char*)&SectionBuffer_Temp[Index2], ParameterName);
		Index2 += strlen(ParameterName) +1;
		
		// Copy Parameter Length and Type
		Data = htoles(Data);
		memcpy(&SectionBuffer_Temp[Index2], &Data, 2);
		Index2 = Index2 +2;

		// Copy Value
		memcpy(&SectionBuffer_Temp[Index2], &SectionBuffer[Index1], Length);

		Index1 += Length;
		Index2 += Length;
	}

	memcpy(&SectionBuffer[0], &SectionBuffer_Temp[0], SectionSize);

	free(SectionBuffer_Temp);

	return TRUE;
}

eTLV_Type Get_TVL_Type(char* strType)
{
	if (OS_STRICMP(strType, "Boolean") == 0) return eTLV_Type_Boolean;
	if (OS_STRICMP(strType, "Integer") == 0) return eTLV_Type_Integer;
	if (OS_STRICMP(strType, "String") == 0) return eTLV_Type_String;
	if (OS_STRICMP(strType, "Array") == 0) return eTLV_Type_Array;

	return eTLV_Type_Undefined;
}

bool Set_TLV_Parameter(	BIN_file_t*		BIN_file,
						eBIN_Section	Section,
						char*			New_ParameterName,
						UINT32			New_ParameterLength,
						eTLV_Type		New_ParameterType,
						void*			New_ParameterValue)
{
	UINT8* SectionBuffer;
	UINT32 SectionSize;
	int Index = 0;

	char	ParameterName[MAX_TLV_PARAMETER_NAME_SIZE];
	UINT16	Data;
	UINT32	Length;
	UINT32	Type;

	int		StartIndex_FirstValue;

	Delete_TLV_Parameter(BIN_file, Section, New_ParameterName);

	if (GetSection(BIN_file, Section, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	if ((Section == eBIN_Section_Manufacturer_Block) || (Section == eBIN_Section_Parameters_Block))
	{
		StartIndex_FirstValue=0;
	}
	else if ((Section == eBIN_Section_Modified_Parameters_1_Block) ||
			 (Section == eBIN_Section_Modified_Parameters_2_Block))
	{
		if ((SectionBuffer[0] == Modified_Parameters_Block_Valid) ||
			(SectionBuffer[0] == Modified_Parameters_Block_Valid_and_Active))
		{
			StartIndex_FirstValue=1;
		}
		else
		{
			return FALSE;
		}
	}

	Index=StartIndex_FirstValue;

	// Search for the end of the TLV structure
	while ((SectionBuffer[Index]) != 0xFF)
	{
		if (strlen((char*)&SectionBuffer[Index]) > MAX_TLV_PARAMETER_NAME_SIZE)
		{
			printf("TLV format is broken!\n");
			break;
		}

		// Read Parameter Name
		strcpy(ParameterName, (char*)&SectionBuffer[Index]);

		if (strlen(ParameterName) == 0)
		{
			printf("TLV format is broken!\n");
			break;
		}

		Index += strlen(ParameterName) +1;

		// Read Parameter Length and Type
		memcpy(&Data, &SectionBuffer[Index], 2);
		Data = letohs(Data);
		Index = Index +2;
		Length = Data >> 3;
		Type = Data & 0x07;

		if ((Type < eTLV_Type_Boolean) || (Type > eTLV_Type_Array) || (Length == 0))
		{
			printf("TLV format is broken!\n");
			break;
		}

		Index += Length;
	}

	// Check that we have enough space left
	if ((Index + strlen(New_ParameterName)+1 + 2 + New_ParameterLength) > SectionSize)
	{
		return FALSE;
	}

	// Add the new Parameter

	// Copy Parameter Name
	strcpy((char*)&SectionBuffer[Index], New_ParameterName);
	Index += strlen(New_ParameterName) +1;

	// Copy Parameter Length and Type
	Data = (New_ParameterLength << 3) + New_ParameterType;
	Data = htoles(Data);
	memcpy(&SectionBuffer[Index], &Data, 2);
	Index = Index +2;

	// Copy Parameter Value
	memcpy(&SectionBuffer[Index], New_ParameterValue, New_ParameterLength);

	Index += New_ParameterLength;

	UpdateChecksum(BIN_file, Section);

	return TRUE;
}

bool Set_TLV_Parameter_Boolean(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT8			Value)
{
	UINT32					ParameterLength;
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];

	ParameterLength = 1;
	ParameterType = eTLV_Type_Boolean;

	memcpy(&ParameterValue[0], &Value, 1);

	if (Set_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							ParameterLength,
							ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	return TRUE;
}


bool Set_TLV_Parameter_UINT8(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT8			Value)
{
	UINT32					ParameterLength;
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];

	ParameterLength = 1;
	ParameterType = eTLV_Type_Integer;

	memcpy(&ParameterValue[0], &Value, 1);

	if (Set_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							ParameterLength,
							ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	return TRUE;
}

bool Set_TLV_Parameter_UINT16(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT16			Value)
{
	UINT32					ParameterLength;
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];

	ParameterLength = 2;
	ParameterType = eTLV_Type_Integer;

	Value = htoles(Value);
	memcpy(&ParameterValue[0], &Value, 2);

	if (Set_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							ParameterLength,
							ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	return TRUE;
}

bool Set_TLV_Parameter_UINT32(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT32			Value)
{
	UINT32					ParameterLength;
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];

	ParameterLength = 4;
	ParameterType = eTLV_Type_Integer;

	Value = htolel(Value);
	memcpy(&ParameterValue[0], &Value, 4);

	if (Set_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							ParameterLength,
							ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	return TRUE;
}

bool Set_TLV_Parameter_String(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								char*			Value)
{
	UINT32					ParameterLength;
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];

	ParameterLength = strlen(Value)+1;
	ParameterType = eTLV_Type_String;

	strcpy(&ParameterValue[0], Value);

	if (Set_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							ParameterLength,
							ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	return TRUE;
}

bool Set_TLV_Parameter_Array(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT32			ParameterLength,
								UINT8*			Value)
{
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];

	ParameterType = eTLV_Type_Array;

	memcpy(&ParameterValue[0], &Value[0], ParameterLength);

	if (Set_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							ParameterLength,
							ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	return TRUE;
}


bool Get_TLV_Parameter(	BIN_file_t*		BIN_file,
						eBIN_Section	Section,
						char*			New_ParameterName,
						UINT32*			New_ParameterLength,
						eTLV_Type*		New_ParameterType,
						void*			New_ParameterValue)
{
	UINT8* SectionBuffer;
	UINT32 SectionSize;
	int Index = 0;

	char	ParameterName[MAX_TLV_PARAMETER_NAME_SIZE];
	UINT16	Data;
	UINT32	Length;
	UINT32	Type;

	int		StartIndex_FirstValue;

	if (GetSection(BIN_file, Section, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	if ((Section == eBIN_Section_Manufacturer_Block) || (Section == eBIN_Section_Parameters_Block))
	{
		StartIndex_FirstValue=0;
	}
	else if ((Section == eBIN_Section_Modified_Parameters_1_Block) ||
			 (Section == eBIN_Section_Modified_Parameters_2_Block))
	{
		if ((SectionBuffer[0] == Modified_Parameters_Block_Valid) ||
			(SectionBuffer[0] == Modified_Parameters_Block_Valid_and_Active))
		{
			StartIndex_FirstValue=1;
		}
		else
		{
			return FALSE;
		}
	}

	Index=StartIndex_FirstValue;
	
	while ((SectionBuffer[Index]) != 0xFF)
	{
		if (strlen((char*)&SectionBuffer[Index]) > MAX_TLV_PARAMETER_NAME_SIZE)
		{
			printf("TLV format is broken!\n");
			break;
		}
		
		// Read Parameter Name
		strcpy(ParameterName, (char*)&SectionBuffer[Index]);

		if (strlen(ParameterName) == 0)
		{
			printf("TLV format is broken!\n");
			break;
		}

		Index += strlen(ParameterName) +1;

		// Read Parameter Length and Type
		memcpy(&Data, &SectionBuffer[Index], 2);
		Data = letohs(Data);
		Index = Index +2;
		Length = Data >> 3;
		Type = Data & 0x07;

		if ((Type < eTLV_Type_Boolean) || (Type > eTLV_Type_Array) || (Length == 0))
		{
			printf("TLV format is broken!\n");
			break;
		}

		if (strcmp(ParameterName, New_ParameterName) == 0)
		{
			// Found
			*New_ParameterLength = Length;
			*New_ParameterType = (eTLV_Type)Type;
			memcpy(New_ParameterValue, &SectionBuffer[Index], Length);
			
			return TRUE;
		}

		Index += Length;
	}

	// Not Found
	return FALSE;
}

bool Get_TLV_Parameter_Boolean(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT8*			Value)
{
	UINT32					ParameterLength;
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];


	if (Get_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							&ParameterLength,
							&ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	if ((ParameterLength == 1) &&
		(ParameterType == eTLV_Type_Boolean))
	{
		memcpy(Value, &ParameterValue[0], 1);
		
		return TRUE;
	}

	return FALSE;
}


bool Get_TLV_Parameter_UINT8(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT8*			Value)
{
	UINT32					ParameterLength;
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];


	if (Get_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							&ParameterLength,
							&ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	if ((ParameterLength == 1) &&
		(ParameterType == eTLV_Type_Integer))
	{
		memcpy(Value, &ParameterValue[0], 1);
		
		return TRUE;
	}

	return FALSE;
}

bool Get_TLV_Parameter_UINT16(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT16*			Value)
{
	UINT32					ParameterLength;
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];


	if (Get_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							&ParameterLength,
							&ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	if ((ParameterLength == 2) &&
		((ParameterType == eTLV_Type_Integer) || (ParameterType == eTLV_Type_Boolean)))
	{
		memcpy(Value, &ParameterValue[0], 2);
		*Value = letohs(*Value);
		
		return TRUE;
	}

	return FALSE;
}

bool Get_TLV_Parameter_UINT32(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT32*			Value)
{
	UINT32					ParameterLength;
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];


	if (Get_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							&ParameterLength,
							&ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	if ((ParameterLength == 4) &&
		((ParameterType == eTLV_Type_Integer) || (ParameterType == eTLV_Type_Boolean)))
	{
		memcpy(Value, &ParameterValue[0], 4);
		*Value = letohl(*Value);
		
		return TRUE;
	}

	return FALSE;
}

bool Get_TLV_Parameter_String(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								char*			Value)
{
	UINT32					ParameterLength;
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];


	if (Get_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							&ParameterLength,
							&ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	if (ParameterType == eTLV_Type_String)
	{
		strcpy(Value, &ParameterValue[0]);
		
		return TRUE;
	}

	return FALSE;
}

bool Get_TLV_Parameter_Array(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT32			ParameterLength,
								UINT8*			Value)
{
	eTLV_Type				ParameterType;
	char					ParameterValue[MAX_TLV_PARAMETER_VALUE_SIZE];
	UINT32					ParameterLength_Current;

	if (Get_TLV_Parameter(	BIN_file,
							Section,
							ParameterName,
							&ParameterLength_Current,
							&ParameterType,
							&ParameterValue[0]) != TRUE)
	{
		return FALSE;
	}

	if ((ParameterType == eTLV_Type_Array) &&
		(ParameterLength == ParameterLength_Current))
	{
		memcpy(Value, &ParameterValue[0], ParameterLength);
		
		return TRUE;
	}

	return FALSE;
}


bool Get_TLV_Parameter_Boolean_Auto(BIN_file_t*		BIN_file,
									UINT8			Modified_Parameters_Validity_Byte,
									char*			ParameterName,
									UINT8*			Value)
{
	if (Modified_Parameters_Validity_Byte == 1)
	{
		if (Get_TLV_Parameter_Boolean(BIN_file, eBIN_Section_Modified_Parameters_1_Block, ParameterName, Value) == TRUE)
		{
			return TRUE;
		}
	}
	else
	{
		if (Get_TLV_Parameter_Boolean(BIN_file, eBIN_Section_Modified_Parameters_2_Block, ParameterName, Value) == TRUE)
		{
			return TRUE;
		}
	}

	if (Get_TLV_Parameter_Boolean(BIN_file, eBIN_Section_Parameters_Block, ParameterName, Value) == TRUE)
	{
		return TRUE;
	}

	if (Get_TLV_Parameter_Boolean(BIN_file, eBIN_Section_Manufacturer_Block, ParameterName, Value) == TRUE)
	{
		return TRUE;
	}

	return FALSE;
}

bool Get_TLV_Parameter_UINT8_Auto(	BIN_file_t*		BIN_file,
									UINT8			Modified_Parameters_Validity_Byte,
									char*			ParameterName,
									UINT8*			Value)
{
	if (Modified_Parameters_Validity_Byte == 1)
	{
		if (Get_TLV_Parameter_UINT8(BIN_file, eBIN_Section_Modified_Parameters_1_Block, ParameterName, Value) == TRUE)
		{
			return TRUE;
		}
	}
	else
	{
		if (Get_TLV_Parameter_UINT8(BIN_file, eBIN_Section_Modified_Parameters_2_Block, ParameterName, Value) == TRUE)
		{
			return TRUE;
		}
	}

	if (Get_TLV_Parameter_UINT8(BIN_file, eBIN_Section_Parameters_Block, ParameterName, Value) == TRUE)
	{
		return TRUE;
	}

	if (Get_TLV_Parameter_UINT8(BIN_file, eBIN_Section_Manufacturer_Block, ParameterName, Value) == TRUE)
	{
		return TRUE;
	}

	return FALSE;
}

bool Get_TLV_Parameter_UINT16_Auto(	BIN_file_t*		BIN_file,
									UINT8			Modified_Parameters_Validity_Byte,
									char*			ParameterName,
									UINT16*			Value)
{
	if (Modified_Parameters_Validity_Byte == 1)
	{
		if (Get_TLV_Parameter_UINT16(BIN_file, eBIN_Section_Modified_Parameters_1_Block, ParameterName, Value) == TRUE)
		{
			return TRUE;
		}
	}
	else
	{
		if (Get_TLV_Parameter_UINT16(BIN_file, eBIN_Section_Modified_Parameters_2_Block, ParameterName, Value) == TRUE)
		{
			return TRUE;
		}
	}

	if (Get_TLV_Parameter_UINT16(BIN_file, eBIN_Section_Parameters_Block, ParameterName, Value) == TRUE)
	{
		return TRUE;
	}

	if (Get_TLV_Parameter_UINT16(BIN_file, eBIN_Section_Manufacturer_Block, ParameterName, Value) == TRUE)
	{
		return TRUE;
	}

	return FALSE;
}

bool Get_TLV_Parameter_UINT32_Auto(	BIN_file_t*		BIN_file,
									UINT8			Modified_Parameters_Validity_Byte,
									char*			ParameterName,
									UINT32*			Value)
{
	if (Modified_Parameters_Validity_Byte == 1)
	{
		if (Get_TLV_Parameter_UINT32(BIN_file, eBIN_Section_Modified_Parameters_1_Block, ParameterName, Value) == TRUE)
		{
			return TRUE;
		}
	}
	else
	{
		if (Get_TLV_Parameter_UINT32(BIN_file, eBIN_Section_Modified_Parameters_2_Block, ParameterName, Value) == TRUE)
		{
			return TRUE;
		}
	}

	if (Get_TLV_Parameter_UINT32(BIN_file, eBIN_Section_Parameters_Block, ParameterName, Value) == TRUE)
	{
		return TRUE;
	}

	if (Get_TLV_Parameter_UINT32(BIN_file, eBIN_Section_Manufacturer_Block, ParameterName, Value) == TRUE)
	{
		return TRUE;
	}

	return FALSE;
}

bool Get_TLV_Parameter_String_Auto(	BIN_file_t*		BIN_file,
									UINT8			Modified_Parameters_Validity_Byte,
									char*			ParameterName,
									char*			Value)
{
	if (Modified_Parameters_Validity_Byte == 1)
	{
		if (Get_TLV_Parameter_String(BIN_file, eBIN_Section_Modified_Parameters_1_Block, ParameterName, Value) == TRUE)
		{
			return TRUE;
		}
	}
	else
	{
		if (Get_TLV_Parameter_String(BIN_file, eBIN_Section_Modified_Parameters_2_Block, ParameterName, Value) == TRUE)
		{
			return TRUE;
		}
	}

	if (Get_TLV_Parameter_String(BIN_file, eBIN_Section_Parameters_Block, ParameterName, Value) == TRUE)
	{
		return TRUE;
	}

	if (Get_TLV_Parameter_String(BIN_file, eBIN_Section_Manufacturer_Block, ParameterName, Value) == TRUE)
	{
		return TRUE;
	}

	return FALSE;
}

bool Get_TLV_Parameter_Array_Auto(	BIN_file_t*		BIN_file,
									UINT8			Modified_Parameters_Validity_Byte,
									char*			ParameterName,
									UINT32			ParameterLength,
									UINT8*			Value)
{
	if (Modified_Parameters_Validity_Byte == 1)
	{
		if (Get_TLV_Parameter_Array(BIN_file, eBIN_Section_Modified_Parameters_1_Block, ParameterName, ParameterLength, Value) == TRUE)
		{
			return TRUE;
		}
	}
	else
	{
		if (Get_TLV_Parameter_Array(BIN_file, eBIN_Section_Modified_Parameters_2_Block, ParameterName, ParameterLength, Value) == TRUE)
		{
			return TRUE;
		}
	}

	if (Get_TLV_Parameter_Array(BIN_file, eBIN_Section_Parameters_Block, ParameterName, ParameterLength, Value) == TRUE)
	{
		return TRUE;
	}

	if (Get_TLV_Parameter_Array(BIN_file, eBIN_Section_Manufacturer_Block, ParameterName, ParameterLength, Value) == TRUE)
	{
		return TRUE;
	}

	return FALSE;
}

bool Get_TLV_Parameter_By_Index(BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								UINT8			In_ParameterIndex,
								char*			Out_ParameterName,
								UINT32*			Out_ParameterLength,
								eTLV_Type*		Out_ParameterType,
								void*			Out_ParameterValue)
{
	UINT8* SectionBuffer;
	UINT32 SectionSize;
	int Index = 0;

	UINT8	ParameterIndex;

	char	ParameterName[MAX_TLV_PARAMETER_NAME_SIZE];
	UINT16	Data;
	UINT32	Length;
	UINT32	Type;

	int		StartIndex_FirstValue;

	if (GetSection(BIN_file, Section, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	if ((Section == eBIN_Section_Manufacturer_Block) || (Section == eBIN_Section_Parameters_Block))
	{
		StartIndex_FirstValue=0;
	}
	else if ((Section == eBIN_Section_Modified_Parameters_1_Block) ||
			 (Section == eBIN_Section_Modified_Parameters_2_Block))
	{
		if ((SectionBuffer[0] == Modified_Parameters_Block_Valid) ||
			(SectionBuffer[0] == Modified_Parameters_Block_Valid_and_Active))
		{
			StartIndex_FirstValue=1;
		}
		else
		{
			return FALSE;
		}
	}

	Index=StartIndex_FirstValue;
	ParameterIndex = 0;
	
	while ((SectionBuffer[Index]) != 0xFF)
	{
		if (strlen((char*)&SectionBuffer[Index]) > MAX_TLV_PARAMETER_NAME_SIZE)
		{
			printf("TLV format is broken!\n");
			break;
		}
		
		// Read Parameter Name
		strcpy(ParameterName, (char*)&SectionBuffer[Index]);

		if (strlen(ParameterName) == 0)
		{
			printf("TLV format is broken!\n");
			break;
		}

		Index += strlen(ParameterName) +1;

		// Read Parameter Length and Type
		memcpy(&Data, &SectionBuffer[Index], 2);
		Data = letohs(Data);
		Index = Index +2;
		Length = Data >> 3;
		Type = Data & 0x07;

		if ((Type < eTLV_Type_Boolean) || (Type > eTLV_Type_Array) || (Length == 0))
		{
			printf("TLV format is broken!\n");
			break;
		}

		if (Length > MAX_TLV_PARAMETER_VALUE_SIZE)
		{
			printf("TLV format is broken!\n");
			break;
		}

		ParameterIndex++;

		if (ParameterIndex == In_ParameterIndex)
		{
			// Found

			strcpy(Out_ParameterName, ParameterName);
			*Out_ParameterLength = Length;
			*Out_ParameterType = (eTLV_Type)Type;
			memcpy(Out_ParameterValue, &SectionBuffer[Index], Length);

			return TRUE;
		}

		Index += Length;
	}

	// Not Found
	return FALSE;
}

bool Get_TLV_Modified_Parameter_Validity_Byte(	BIN_file_t*		BIN_file,
												eBIN_Section	Section,
												UINT8*			Validity_Byte)
{
	UINT8* SectionBuffer;
	UINT32 SectionSize;

	if (GetSection(BIN_file, Section, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	*Validity_Byte = SectionBuffer[0];

	return TRUE;
}

bool Set_TLV_Modified_Parameter_Validity_Byte(	BIN_file_t*		BIN_file,
												eBIN_Section	Section,
												UINT8			Validity_Byte)
{
	UINT8* SectionBuffer;
	UINT32 SectionSize;

	if (GetSection(BIN_file, Section, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	SectionBuffer[0] = Validity_Byte;

	return TRUE;
}

bool Dump_TLV_Section(	BIN_file_t*		BIN_file,
						eBIN_Section	Section)
{
	UINT8* SectionBuffer;
	UINT32 SectionSize;
	UINT32 Index = 0;

	if (GetSection(BIN_file, Section, FALSE, &SectionBuffer, &SectionSize) == FALSE)
	{
		return FALSE;
	}

	SectionSize = 128;

	for (Index=1; Index < SectionSize; Index++)
	{
		printf("0x%02X, ", SectionBuffer[Index-1]);

		if ((Index % 16) == 0)
		{
			printf("\n");
		}
	}

	printf("\n");
	
	return TRUE;
}
