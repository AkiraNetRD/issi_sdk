#ifndef GHN_LIB_Image_h__
#define GHN_LIB_Image_h__

#include "defs.h"

#include "common.h"


#define USE_NEW_FUUF_API (0)


#define swap16(n)                       ((UINT16)((((n)<<8)&0xFF00)|(((n)>>8)&0x00FF)))
#define swap32(n)                       (((swap16((n)&0xFFFF)<<16)&0xFFFF0000)| \
                                        (swap16(((n)>>16)&0xFFFF)&0x0000FFFF))


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
	sSection_Info	Modified_Parameters_Block;
	sSection_Info	BIN_FILE_HEADER;
	sSection_Info	SW_DB_XML;

	UINT8			active;
} FW_Offset_Table_t;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// The header of the final FW BIN file
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct  
{
	UINT16			HeaderVersion;				/* Header version number for future support  (current=1) */
	char			ModelName[6];				/* Model-Name. e.g. CG5210 */
	char			FW_Version[24];				/* FW Version */
	char			ConfigurationName[64];		/* Configuration Name (Profile Name) */
	char			BuildImageCreationTime[24];	/* The time the creation this BIN file */
	UINT32			TotalImageSize;				/* Total image size in bytes */
	unsigned char	FW_Signature[16];			/* MD5 on relevant FW sections */
} BIN_file_header_t;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define MIN_BIN_FILE_SIZE	64*1024			// 64Kb
#define MAX_BIN_FILE_SIZE	2*1024*1024		// 2Mb

typedef struct
{
	UINT32				BIN_file_Sections_Max_Length;			// Malloc length
	UINT32				BIN_file_Sections_Length;				// Actual Length
	UINT8				BIN_file_Sections[MAX_BIN_FILE_SIZE];	// Buffer
} BIN_file_t;

typedef enum
{
	eBIN_Section_EBL_SPI_BCB				= 1,          // EBL's SPI and BCB section
	eBIN_Section_EBL_B1						= 2,          // B1 Section
	eBIN_Section_FW_SPI						= 3,          // SPI header section
	eBIN_Section_FW_PCM						= 4,          // PCM section
	eBIN_Section_FW_HW						= 5,          // HW-Vector section                           
	eBIN_Section_FW_GP						= 6,          // Global Param Section
	eBIN_Section_FW_CP						= 7,          // FW CP section
	eBIN_Section_BIN_FILE_HEADER			= 0x4E484753, // BIN file Header (ASCII: "SGHN")
	eBIN_Section_FW_RW_GP					= 9,          // Read/Write GP section (NOT IN USE ANY MORE!!)
	eBIN_Section_SW_DB_XML					= 10,         // SW DB XML section
	eBIN_Section_Manufacturer_Block			= 11,         // Manufacturer Block
	eBIN_Section_Parameters_Block			= 12,         // Parameter Block
	eBIN_Section_Modified_Parameters_Block	= 13,         // Modified Parameter Block
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

#ifdef __cplusplus
extern "C" {
#endif

UINT32 CalculatePacketChecksum(UINT8 *pDataPacketBuffer, UINT32 u32Len);
	
BOOL GetHeader(BIN_file_t* BIN_file, BIN_file_header_t** BIN_file_header);
BOOL GetSection(BIN_file_t* BIN_file, eBIN_Section Section, BOOL IncludingHeader, UINT8** SectionBuffer, UINT32* SectionSize);
BOOL AppendHeader(BIN_file_t* BIN_file, BIN_file_header_t* BIN_file_header);

BOOL AppendSection_From_File(BIN_file_t* BIN_file, eBIN_Section Section, char* FileName);
void UpdateChecksum(BIN_file_t* BIN_file, eBIN_Section Section);
BOOL UpdateSignature(BIN_file_t* BIN_file);
BOOL ValidateSignature(BIN_file_t* BIN_file);
BOOL Validate_BIN_file(BIN_file_t* BIN_file);
BOOL Save_BIN_file(BIN_file_t* BIN_file,char* FileName);
BOOL Load_BIN_file(BIN_file_t* BIN_file,char* FileName);


UINT32 EBL_BCB_FLASH_SIZE();
UINT32 FW_SPI_FLASH_SIZE();
UINT32 FW_BIN_FILE_HEADER_FLASH_SIZE();

BOOL Parse_EBL_Table(	BIN_file_t* BIN_file,
						EBL_Offset_Table_t* EBL_SPI_Offset_Table);

BOOL Parse_FW_Table(BIN_file_t*			BIN_file,
					FW_Offset_Table_t*	FW_SPI_Offset_Table);

BOOL Update_EBL_SPI_BCB(	BIN_file_t* BIN_file,
							macStruct*	MAC_Address);

BOOL Update_EBL_SPI_BCB_Get_MAC_Address(BIN_file_t* BIN_file,
										macStruct*	MAC_Address);

BOOL Update_FW_SPI(	BIN_file_t* BIN_file,
					UINT8		ActiveBit);

int Get_FW_SPI_ActiveBit_Offset();

BOOL Extract_FW_Version(char* line, int size, char* FW_Version);
BOOL Get_BIN_FWVersion(char* BIN_FW_File, char* BIN_FW_Version);
	
void Print_BIN_file_Header(BIN_file_t* BIN_file, BOOL Highlighted);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // GHN_LIB_Image_h__
