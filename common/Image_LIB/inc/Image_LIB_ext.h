#ifndef Image_LIB_ext_API_h__
#define Image_LIB_ext_API_h__

#include "defs.h"

#include "Image_LIB_typedefs.h"

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

	UINT32 CalculatePacketChecksum(UINT8 *pDataPacketBuffer, UINT32 u32Len);
	
	bool GetHeader(BIN_file_t* BIN_file, BIN_file_header_t** BIN_file_header);
	bool GetSection(BIN_file_t* BIN_file, eBIN_Section Section, bool IncludingHeader, UINT8** SectionBuffer, UINT32* SectionSize);
	bool AppendHeader(BIN_file_t* BIN_file, BIN_file_header_t* BIN_file_header);

	bool AppendSection_From_File(BIN_file_t* BIN_file, eBIN_Section Section, char* FileName);
	void UpdateChecksum(BIN_file_t* BIN_file, eBIN_Section Section);
	bool UpdateSignature(BIN_file_t* BIN_file);
	bool ValidateSignature(BIN_file_t* BIN_file);
	bool Validate_BIN_file(BIN_file_t* BIN_file);
	bool Save_BIN_file(BIN_file_t* BIN_file,char* FileName);
	bool Load_BIN_file(BIN_file_t* BIN_file,char* FileName);

	bool Validate_Chip_Type(BIN_file_t* BIN_file,char *Device_ChipType);

	UINT32 EBL_BCB_FLASH_SIZE(char* Device_ChipType);
	UINT32 FW_SPI_FLASH_SIZE();
	UINT32 FW_BIN_FILE_HEADER_FLASH_SIZE();

	bool Parse_EBL_Table(	BIN_file_t*			BIN_file,
							char*				Device_ChipType,
							EBL_Offset_Table_t* EBL_SPI_Offset_Table);

	bool Parse_FW_Table(BIN_file_t*			BIN_file,
						FW_Offset_Table_t*	FW_SPI_Offset_Table);

	bool Update_EBL_SPI_BCB(BIN_file_t*			BIN_file,
							char*				Device_ChipType,
							macStruct*			MAC_Address);

	bool Update_EBL_SPI_BCB_Get_MAC_Address(BIN_file_t*		BIN_file,
											char*			Device_ChipType,
											macStruct*		MAC_Address);
	bool Update_FW_SPI(	BIN_file_t* BIN_file,
						UINT8		ActiveBit);

	int Get_FW_SPI_ActiveBit_Offset();

	bool Extract_FW_Version(char* line, int size, char* FW_Version);
	bool Get_BIN_FWVersion(char* BIN_FW_File, char* BIN_FW_Version);
	
	void Print_BIN_file_Header(BIN_file_t* BIN_file, bool Highlighted);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Support the TLV Parameters sections
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	bool Append_TLV_Section(BIN_file_t* BIN_file, eBIN_Section Section, UINT32 Length);
	
	bool Delete_TLV_Parameter(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			Delete_ParameterName);

	eTLV_Type Get_TVL_Type(char* strType);




	bool Set_TLV_Parameter(	BIN_file_t*		BIN_file,
							eBIN_Section	Section,
							char*			New_ParameterName,
							UINT32			New_ParameterLength,
							eTLV_Type		New_ParameterType,
							void*			New_ParameterValue);

	bool Set_TLV_Parameter_Boolean(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									UINT8			Value);

	bool Set_TLV_Parameter_UINT8(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									UINT8			Value);

	bool Set_TLV_Parameter_UINT16(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									UINT16			Value);

	bool Set_TLV_Parameter_UINT32(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									UINT32			Value);

	bool Set_TLV_Parameter_String(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									char*			Value);

	bool Set_TLV_Parameter_Array(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									UINT32			ParameterLength,
									UINT8*			Value);




	bool Get_TLV_Parameter(	BIN_file_t*		BIN_file,
							eBIN_Section	Section,
							char*			New_ParameterName,
							UINT32*			New_ParameterLength,
							eTLV_Type*		New_ParameterType,
							void*			New_ParameterValue);

	bool Get_TLV_Parameter_Boolean(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									UINT8*			Value);

	bool Get_TLV_Parameter_UINT8(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									UINT8*			Value);

	bool Get_TLV_Parameter_UINT16(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									UINT16*			Value);

	bool Get_TLV_Parameter_UINT32(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									UINT32*			Value);
	
	bool Get_TLV_Parameter_String(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									char*			Value);

	bool Get_TLV_Parameter_Array(	BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									char*			ParameterName,
									UINT32			ParameterLength,
									UINT8*			Value);


	bool Get_TLV_Parameter_Boolean_Auto(BIN_file_t*		BIN_file,
										UINT8			Modified_Parameters_Validity_Byte,
										char*			ParameterName,
										UINT8*			Value);

	bool Get_TLV_Parameter_UINT8_Auto(	BIN_file_t*		BIN_file,
										UINT8			Modified_Parameters_Validity_Byte,
										char*			ParameterName,
										UINT8*			Value);
	
	bool Get_TLV_Parameter_UINT16_Auto(	BIN_file_t*		BIN_file,
										UINT8			Modified_Parameters_Validity_Byte,
										char*			ParameterName,
										UINT16*			Value);

	bool Get_TLV_Parameter_UINT32_Auto(	BIN_file_t*		BIN_file,
										UINT8			Modified_Parameters_Validity_Byte,
										char*			ParameterName,
										UINT32*			Value);

	bool Get_TLV_Parameter_String_Auto(	BIN_file_t*		BIN_file,
										UINT8			Modified_Parameters_Validity_Byte,
										char*			ParameterName,
										char*			Value);

	bool Get_TLV_Parameter_Array_Auto(	BIN_file_t*		BIN_file,
										UINT8			Modified_Parameters_Validity_Byte,
										char*			ParameterName,
										UINT32			ParameterLength,
										UINT8*			Value);

	bool Get_TLV_Parameter_By_Index(BIN_file_t*		BIN_file,
									eBIN_Section	Section,
									UINT8			In_ParameterIndex,
									char*			Out_ParameterName,
									UINT32*			Out_ParameterLength,
									eTLV_Type*		Out_ParameterType,
									void*			Out_ParameterValue);

	bool Get_TLV_Modified_Parameter_Validity_Byte(	BIN_file_t*		BIN_file,
													eBIN_Section	Section,
													UINT8*			Validity_Byte);

	bool Set_TLV_Modified_Parameter_Validity_Byte(	BIN_file_t*		BIN_file,
													eBIN_Section	Section,
													UINT8			Validity_Byte);

	bool Dump_TLV_Section(	BIN_file_t*		BIN_file,
							eBIN_Section	Section);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // Image_LIB_ext_API_h__
