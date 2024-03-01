#ifndef Image_LIB_typedefs_h__
#define Image_LIB_typedefs_h__

/*****************************************************************************
*                             TYPE DEFINITIONS
*****************************************************************************/

#define FLASH_BLOCK_HEADER_SIZE (8)

#define READ_U32_FROM_PACKET(x,y)	{\
	x = (*y++);\
	x += (*y++ << 8);\
	x += (*y++ << 16);\
	x += (*y++ << 24);\
}\

#define TWOS_COMPLEMENT(a)		((a)=~(a)+1)



typedef struct  
{
	UINT32		Offset_EBL_SPI;
	UINT32		Offset_EBL_BCB;
	
	UINT32		Offset_EBL_B1;
	UINT32		Launch_Addr_EBL_B1;

	UINT32		Offset_FW1_SPI;
	UINT32		Offset_FW2_SPI;
} EBL_Offset_Table_t;

typedef struct
{
	UINT32		Offset;
	UINT32		MaxSize;
} sSection_Info;

typedef struct
{
	sSection_Info	SPI;
	sSection_Info	CP;

	sSection_Info	PCM;
	sSection_Info	HW;
	sSection_Info	GP;
	sSection_Info	Manufacturer_Block;
	sSection_Info	Parameters_Block;
	sSection_Info	Modified_Parameters_1_Block;
	sSection_Info	Modified_Parameters_2_Block;
	sSection_Info	BIN_FILE_HEADER;
	sSection_Info	SW_DB_XML;
	sSection_Info	FWL;

	UINT8			active;
} FW_Offset_Table_t;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// The header of the final FW BIN file
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	UINT16			HeaderVersion;				/* Header version number for future support (current=2) */
	char			ModelName[6];				/* Model-Name. e.g. "CG5210" */
	char			FW_Version[24];				/* FW Version */
	char			ConfigurationName[64];		/* Configuration Name (Profile Name) */
	char			BuildImageCreationTime[24];	/* The time the creation this BIN file */
	UINT32			TotalImageSize;				/* Total image size in bytes */
	unsigned char	FW_Signature[16];			/* MD5 on relevant FW sections */
	char			ModelNameExt[(6+1)*5-1];	/* Support up to 5 Model-Name. e.g. "CG5210,CG5230" */
	UINT8			Padding[2];					/* 32Bits Alignment */
} BIN_file_header_t;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define MIN_BIN_FILE_SIZE	64*1024			// 64Kb
#define MAX_BIN_FILE_SIZE	2*1024*1024		// 2Mb
#define EBL_SIZE			64*1024			// 64Kb
#define MAX_FLASH_SIZE		4*1024*1024		// 4Mb

#define MAX_TLV_PARAMETER_NAME_SIZE 257				// 256 Bytes + 1 Byte
#define MAX_TLV_PARAMETER_VALUE_SIZE 8193			// 8Kb + 1 Byte

#define Modified_Parameters_Block_Valid 0x01
#define Modified_Parameters_Block_Valid_and_Active 0x11

typedef struct
{
	UINT32				BIN_file_Sections_Max_Length;			// Malloc length
	UINT32				BIN_file_Sections_Length;				// Actual Length
	UINT8				BIN_file_Sections[MAX_BIN_FILE_SIZE];	// Buffer
} BIN_file_t;

typedef enum
{
	eBIN_Section_EBL_SPI_BCB					= 1,          // EBL's SPI and BCB section
	eBIN_Section_EBL_B1							= 2,          // B1 Section
	eBIN_Section_FW_SPI							= 3,          // SPI header section
	eBIN_Section_FW_PCM							= 4,          // PCM section
	eBIN_Section_FW_HW							= 5,          // HW-Vector section                           
	eBIN_Section_FW_GP							= 6,          // Global Param Section
	eBIN_Section_FW_CP							= 7,          // FW CP section
	eBIN_Section_BIN_FILE_HEADER				= 0x4E484753, // BIN file Header (ASCII: "SGHN")
	eBIN_Section_FW_RW_GP						= 9,          // Read/Write GP section (NOT IN USE ANY MORE!!)
	eBIN_Section_SW_DB_XML						= 10,         // SW DB XML section
	eBIN_Section_Manufacturer_Block				= 11,         // Manufacturer Block
	eBIN_Section_Parameters_Block				= 12,         // Parameter Block
	eBIN_Section_Modified_Parameters_1_Block	= 13,         // Modified Parameter 1 Block 
	eBIN_Section_Modified_Parameters_2_Block	= 14,         // Modified Parameter 2 Block
	eBIN_Section_FW_FWL							= 15,         // Firmware Loader section
	eBIN_Section_MAX_SECTION
}eBIN_Section; // IMAGE_section_e

typedef struct
{
	eBIN_Section	Section;
	UINT32			CheckSum;
	UINT32			Length;
} BIN_section_header_t;


/* According to prm.h */
typedef enum
{
	ePhyType_PHY_MIMO = 0,
	ePhyType_PHY_MRC,
	ePhyType_PHY_SISO,
	ePhyType_PHY_HPAV,
	ePhyType_PHY_COAX,
}ePhyType;

typedef enum
{
	eTLV_Type_Undefined = 0,
	eTLV_Type_Boolean,
	eTLV_Type_Integer,
	eTLV_Type_String,
	eTLV_Type_Array,
}eTLV_Type;

#endif // Image_LIB_typedefs_h__
