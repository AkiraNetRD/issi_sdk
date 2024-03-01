#ifndef HTTP_LIB_typedef_h__
#define HTTP_LIB_typedef_h__

#include "HTTP_LIB_common.h"
#include "HTTP_LIB_consts.h"

typedef struct
{
	char					IN		Name[HTTP_LIB_MAX_BRANCH_NAME_SIZE];
} sBranch;

typedef struct
{
	char					IN		Name[HTTP_LIB_MAX_BRANCH_NAME_SIZE];
	char					IN		Value[HTTP_LIB_MAX_BRANCH_NAME_SIZE];
} sParameter;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Get Data-Mode Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	char					IN		DeviceIP[HTTP_LIB_IP_ADDRESS_LEN];

	// This sets the interface name to use as outgoing network interface
	bool					IN		bHasNetworkCardIP;
	char					IN		NetworkCardIP[HTTP_LIB_IP_ADDRESS_LEN]; 

	UINT32					IN		IncludeBranch_Size;
	sBranch					IN		IncludeBranch_Array[HTTP_LIB_MAX_BRANCHS];

	UINT32					IN		ExcludeBranch_Size;
	sBranch					IN		ExcludeBranch_Array[HTTP_LIB_MAX_BRANCHS];

	UINT32					OUT		DataModel_Size;
	char					OUT		DataModel_Buffer[HTTP_LIB_MAX_GET_DATA_MODEL_BUFFER_SIZE];

	char					OUT		ErrorDescription[HTTP_LIB_ERROR_MESSAGE_SIZE];
} sGet_Data_Mode_lnformation;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Set Data-Mode Information
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef struct
{
	char					IN		DeviceIP[HTTP_LIB_IP_ADDRESS_LEN];

	// This sets the interface name to use as outgoing network interface 
	bool					IN		bHasNetworkCardIP;
	char					IN		NetworkCardIP[HTTP_LIB_IP_ADDRESS_LEN]; 

	UINT32					IN		SetParameter_Size;
	sParameter				IN		SetParameter_Array[HTTP_LIB_MAX_BRANCHS];

	UINT32					OUT		DataModel_Size;
	char					OUT		DataModel_Buffer[HTTP_LIB_MAX_SET_DATA_MODEL_BUFFER_SIZE];

	char					OUT		ErrorDescription[HTTP_LIB_ERROR_MESSAGE_SIZE];
} sSet_Data_Mode_lnformation;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#endif // HTTP_LIB_typedef_h__
