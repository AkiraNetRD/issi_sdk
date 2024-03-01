/* 
*****************************************************************************
*
* spiboot.h
* Definitions of the SPI BOOT process
*****************************************************************************
*/
#ifndef __SPIBOOT_H__
#define __SPIBOOT_H__

#include "defs.h"

#include "flashboot.h"

/*****************************************************************************
*                               DEFINITIONS
*****************************************************************************/


/* Magic numbers of sections in the flash. These magic
Numbers are also in use by the tools (imager/prog) */
#define APP_MAGIC_NUM               0xABCDEFAB
#define HW_VECT_MAGIC_NUM           0xAABBAABB
#define GLOBAL_PARAMS_MAGIC_NUM     0xAABBDDCC
#define PCM_CONFIG_MAGIC_NUM        0xAABBEECC
#define FW1_MAGIC_NUM               0xEEFFBBAA
#define FW2_MAGIC_NUM               0xEEFFBBBB
#define BIN_FILE_HEADER_MAGIC_NUM   0xAABBFFEE
#define MANUFACTURER_BLOCK_MAGIC_NUM    0xAABBEE00
#define CONFIG_BLOCK_MAGIC_NUM           0xAABBEE11
#define MODIFIED_CONFIG_1_BLOCK_MAGIC_NUM  0xAABBEE22
#define MODIFIED_CONFIG_2_BLOCK_MAGIC_NUM  0xAABBEE33
#define FW_XML_MAGIC_NUM                   0xAABBFFCC
#define FWL_MAGIC_NUM                   0X2222AACD




/* Max number of section that can be added to the SPI.
Does not include BCB/ESCAPE/B1 and application sections. */
#define SPI_MAX_SECTIONS            12

#define SPI_FW_ACTIVE_INVALID           0xFF
#define SPI_FW_ACTIVE_MARK              0x01
#define SPI_FW_NOT_ACTIVE_MARK          0x00

/*****************************************************************************
*                             TYPE DEFINITIONS
*****************************************************************************/


/* Use to define an SPI section which contains a specific BIN file */
typedef struct  
{
    UINT32 magic_num;       /* Identifier the section type: app/B1/BCB... */
    UINT32 offset;          /* Section's offset in the flash */
    UINT32 max_size;        /* The maximum size of the section */
}SPI_flash_section_t;

typedef struct 
{
	UINT32 SPI_header_magic_num;
	UINT32 SPI_header_version;

	UINT32 FWL_magic_num;       /* Identifier the section type: app/B1/BCB... */
	UINT32 FWL_spi_offset;          /* Section's offset in the flash */
	UINT32 FWL_launch_addr;        /* The maximum size of the section */
	UINT16 FWL_max_size;        /* The maximum size of the section */
	UINT16 Active_byte_offset;        

	UINT8 reserved[8];

	/* Common section to the Application,B1 and the Flash Upgrade */
	UINT32 app_magic_num;
	UINT32 app_spi_offset; 
	UINT32 app_launch_addr;
	UINT8  num_sections;
	UINT8  reserved3[3];
	SPI_flash_section_t flash_sections[SPI_MAX_SECTIONS];
	UINT8 reserved2[8];
	UINT8 active;
	UINT8 pad[3];
} APP_flash_header_t;

/* Extended boot loader SPI header 
 * This is the SPI header that sits at the beginning of the flash */
typedef struct 
{
    /* Common fields with the BOOT ROM */
    FLASH_root_header_t flash_root_header;
    /* End of BOOT ROM fields */
    /* Common section to the Application,B1 and the Flash Upgrade */
    UINT32 FW1_magic_num;
    UINT32 FW1_flash_offset; 
    UINT32 FW2_magic_num;
    UINT32 FW2_flash_offset;
} EBL_flash_header_t;

#endif

