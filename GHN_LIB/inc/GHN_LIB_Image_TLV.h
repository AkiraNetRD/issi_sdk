#ifndef GHN_LIB_Image_TLV_h__
#define GHN_LIB_Image_TLV_h__

#include "defs.h"

#include "GHN_LIB_Image.h"

#include "common.h"


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Support the TLV Parameters sections
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

typedef enum
{
	eTLV_Type_Undefined = 0,
	eTLV_Type_Boolean,
	eTLV_Type_Integer,
	eTLV_Type_String,
	eTLV_Type_Array,
}eTLV_Type;


#ifdef __cplusplus
extern "C" {
#endif

BOOL Append_TLV_Section(BIN_file_t* BIN_file, eBIN_Section Section, UINT32 Length);
	
BOOL Delete_TLV_Parameter(	BIN_file_t*		BIN_file,
							eBIN_Section	Section,
							char*			Delete_ParameterName);

eTLV_Type Get_TVL_Type(char* strType);




BOOL Set_TLV_Parameter(	BIN_file_t*		BIN_file,
						eBIN_Section	Section,
						char*			New_ParameterName,
						UINT32			New_ParameterLength,
						eTLV_Type		New_ParameterType,
						void*			New_ParameterValue);

BOOL Set_TLV_Parameter_Boolean(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT8			Value);

BOOL Set_TLV_Parameter_UINT8(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT8			Value);

BOOL Set_TLV_Parameter_UINT16(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT16			Value);

BOOL Set_TLV_Parameter_UINT32(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT32			Value);

BOOL Set_TLV_Parameter_String(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								char*			Value);

BOOL Set_TLV_Parameter_Array(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT32			ParameterLength,
								UINT8*			Value);




BOOL Get_TLV_Parameter(	BIN_file_t*		BIN_file,
						eBIN_Section	Section,
						char*			New_ParameterName,
						UINT32*			New_ParameterLength,
						eTLV_Type*		New_ParameterType,
						void*			New_ParameterValue);

BOOL Get_TLV_Parameter_Boolean(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT8*			Value);

BOOL Get_TLV_Parameter_UINT8(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT8*			Value);

BOOL Get_TLV_Parameter_UINT16(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT16*			Value);

BOOL Get_TLV_Parameter_UINT32(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT32*			Value);
	
BOOL Get_TLV_Parameter_String(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								char*			Value);

BOOL Get_TLV_Parameter_Array(	BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								char*			ParameterName,
								UINT32			ParameterLength,
								UINT8*			Value);


BOOL Get_TLV_Parameter_Boolean_Auto(BIN_file_t*		BIN_file,
									char*			ParameterName,
									UINT8*			Value);

BOOL Get_TLV_Parameter_UINT8_Auto(	BIN_file_t*		BIN_file,
									char*			ParameterName,
									UINT8*			Value);
	
BOOL Get_TLV_Parameter_UINT16_Auto(	BIN_file_t*		BIN_file,
									char*			ParameterName,
									UINT16*			Value);

BOOL Get_TLV_Parameter_UINT32_Auto(	BIN_file_t*		BIN_file,
									char*			ParameterName,
									UINT32*			Value);

BOOL Get_TLV_Parameter_String_Auto(	BIN_file_t*		BIN_file,
									char*			ParameterName,
									char*			Value);

BOOL Get_TLV_Parameter_Array_Auto(	BIN_file_t*		BIN_file,
									char*			ParameterName,
									UINT32			ParameterLength,
									UINT8*			Value);

BOOL Get_TLV_Parameter_By_Index(BIN_file_t*		BIN_file,
								eBIN_Section	Section,
								UINT8			In_ParameterIndex,

								char*			Out_ParameterName,
								UINT32*			Out_ParameterLength,
								eTLV_Type*		Out_ParameterType,
								void*			Out_ParameterValue);

BOOL Set_TLV_Validity_Byte(	BIN_file_t*		BIN_file,
							UINT8			Configuration1,
							UINT8			Configuration2);

BOOL Dump_TLV_Section(	BIN_file_t*		BIN_file,
						eBIN_Section	Section);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // GHN_LIB_Image_TLV_h__
