#ifndef GHN_LIB_CONSTS_H
#define GHN_LIB_CONSTS_H

#define IN
#define OUT


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Parameter Length
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define GHN_LIB_IP_ADDRESS_LEN 16					// 255.255.255.255 (15+1)
#define GHN_LIB_MAC_ADDRESS_LEN 18					// FF:FF:FF:FF:FF:FF (17+1)

#define GHN_LIB_DEVICE_STATE 64						// "Firmware" / "B1" / "BootCode" / "Unknown"
#define GHN_LIB_FW_VERSION_LEN 25					// 24+1
#define GHN_LIB_CONFIGURATION_NAME_LEN 65			// 64+1
#define GHN_LIB_MODEL_NAME_LEN 7					// 6+1
#define GHN_LIB_MODEL_NAME_REPETITION 5				// 5x
#define GHN_LIB_BUILD_IMAGE_CREATION_TIME_LEN 25	// 24+1

#define GHN_LIB_MAX_DOMAIN_NAME_LEN 33				// 32+1
#define GHN_LIB_MAX_PASSWORD_LEN 13					// 12+1
#define GHN_LIB_MAX_DEVICE_NAME_LEN 17				// 16+1
#define GHN_LIB_MAX_CUSTOMER_TRIAL_NAME_LEN 16		// 15+1
#define GHN_LIB_MAX_DCSM_MATRIXSIZE 15
#define GHN_LIB_MAX_VLAN_PORT 7
#define GHN_LIB_MAX_VLAN_VTU 10

#define GHN_LIB_MD5_SIGNATURE_LEN 49				// 48+1

#define GHN_LIB_ADAPTER_DESC_LEN 256
#define GHN_LIB_ERROR_MESSAGE 256

#define GHN_LIB_GOLAN_REV_LEN 7						// 6+1
#define GHN_LIB_MEDIA_TYPE_LEN 16
#define GHN_LIB_NODE_TYPE_LEN 5						// 4+1

#define GHN_LIB_MAX_SUPPORTED_DEVICES 128
#define GHN_LIB_MAX_ADAPTER_INFORMATION 64
#define GHN_LIB_MAX_LOCALDEVICE_INFORMATION GHN_LIB_MAX_SUPPORTED_DEVICES
#define GHN_LIB_MAX_GETSTATIONS GHN_LIB_MAX_SUPPORTED_DEVICES
#define GHN_LIB_MAX_LOCAL_HOST_MAC_ADDRESSES 128
#define GHN_LIB_MAX_NETWORK_SIZE 128

#define GHN_LIB_STRING_24 24
#define GHN_LIB_STRING_256 256
#define GHN_LIB_MAX_PATH 256
#define GHN_LIB_DEBUGINFO_MESSAGE 256
#define GHN_LIB_DEBUGINFO_RESPONSE 8192
#define GHN_LIB_DEBUGINFO_SLOG_RESPONSE 1048576 // 1Mb
#define GHN_LIB_READMEM_BUFFER_SIZE 65536		// 64Kb
#define GHN_LIB_WRITEMEM_BUFFER_SIZE 65536	// 64Kb
#define GHN_LIB_BIN_FILE_BUFFER_SIZE 65536	// 64Kb
#define GHN_LIB_MAX_TONEMAP_PARAMETER_LINE_LEN 1024


#define GHN_LIB_MAX_MANUFACTURER_BLOCK_INFORMATION 20
#define GHN_LIB_MAX_PARAMETERS_BLOCK_INFORMATION 150
#define GHN_LIB_MAX_MODIFIED_BLOCK_INFORMATION 20
#define GHN_LIB_MAX_PARAMETER_NAME_LEN 257				// MAX_TLV_PARAMETER_NAME_SIZE
#define GHN_LIB_MAX_PARAMETER_VALUE_LEN 24577			// MAX_TLV_PARAMETER_VALUE_SIZE
														// When printing "Array" in format "xx-xx-xx" we need tipple buffer size

// HTTP Get BBT Data-Model
#define GHN_LIB_MAX_BRANCHS 10						// #define HTTP_LIB_MAX_BRANCHS 10
#define GHN_LIB_MAX_BRANCH_NAME_SIZE 256				// #define HTTP_LIB_MAX_BRANCH_NAME_SIZE 256
#define GHN_LIB_MAX_DATA_MODEL_BUFFER_SIZE (2097152)	// #define HTTP_LIB_MAX_DATA_MODEL_BUFFER_SIZE (2097152) // 2MB

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Status code 
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef enum 
{
	eGHN_LIB_STAT_SUCCESS                   = 0,		// general success
	eGHN_LIB_STAT_FAILURE					= 1,		// general failure
	eGHN_LIB_STAT_INVALID_PARAMETER			= 2,		// invalid parameter
	eGHN_LIB_STAT_TIMEOUT					= 3,		// timeout expired 
	eGHN_LIB_STAT_MISSING_FILE				= 4,		// integral file is missing
	eGHN_LIB_STAT_DEVICE_NOT_FOUND			= 5,		// requested device not found
	eGHN_LIB_STAT_NOT_SUPPORTED				= 6,		// requested not supported
	eGHN_LIB_STAT_TEST_WAS_INTERRUPT		= 7,		// test was interrupt
	eGHN_LIB_STAT_FAILED_TO_PARSE_XML		= 8,		// failed to parse XML file
	eGHN_LIB_STAT_HTTP_REQUEST_FAILED		= 9,		// HTTP request was failed

	eGHN_LIB_STAT_FAILED_SWITCH_2_ETHERBOOT	= 10,		// failed to switch to EtherBoot
	eGHN_LIB_STAT_FAILED_SWITCH_2_FLASHBOOT	= 11,		// failed to switch to FlashBoot
	eGHN_LIB_STAT_FAILED_UPDATEFIRMWARE		= 12,		// failed to update firmware
	eGHN_LIB_STAT_FAILED_LOAD_MAIN_FIRMWARE	= 13,		// failed to load main firmware
	eGHN_LIB_STAT_FW_BOOT_CODE				= 14,		// device is in BootCode mode
	eGHN_LIB_STAT_FW_B1						= 15,		// device is in B1 mode
	eGHN_LIB_STAT_FW_FIRMWARE				= 16,		// device is in Firmware mode
	eGHN_LIB_STAT_DISABLED_NVM				= 17,		// device was power-up when "Disabled-NVM" bit is on
	eGHN_LIB_STAT_OLD_EBL_VERSION			= 18,		// device has an older EBL version

	eGHN_LIB_STAT_FAILED_READ_FILE			= 20,		// failed to read from a file
	eGHN_LIB_STAT_FAILED_WRITE_FILE			= 21,		// failed to write to a file
	eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION	= 22,		// failed to allocate memory
	eGHN_LIB_STAT_CHIP_TYPE_MISMATCH		= 23,		// Chip-Type mismatch

	eGHN_LIB_STAT_FAILED_START_NETINF			= 30,		// failed to start netinf test
	eGHN_LIB_STAT_FAILED_STOP_NETINF			= 31,		// failed to stop netinf test
	eGHN_LIB_STAT_FAILED_PARSE_NETINF_RESULT	= 32,		// failed to parse netinf result
	eGHN_LIB_STAT_NETINF_ALREADY_RUNNING		= 33,		// netinf is already running

	eGHN_LIB_STAT_ADAPTERS_QUERY_FAILED       = 101,		// adapter query/processing failed
	eGHN_LIB_STAT_BIND_ADAPTER_FAILED         = 102,		// failed to bind adapter
	eGHN_LIB_STAT_SUBNET_MASK_MISMATCH        = 103,		// Subnet mask is incorrect
	eGHN_LIB_STAT_IP_ADDRESS_INVALID          = 104,		// IP Address is invalid
} eGHN_LIB_STAT;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#endif // GHN_LIB_CONSTS_H
