#ifndef _HTTP_LIB_CONSTS_H
#define _HTTP_LIB_CONSTS_H

#define IN
#define OUT

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Parameter Length
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define HTTP_LIB_IP_ADDRESS_LEN 16				// 255.255.255.255

#define HTTP_LIB_ERROR_MESSAGE_SIZE 256			// #define CURL_ERROR_SIZE 256
#define HTTP_LIB_MAX_CONNECTTIMEOUT 20

#define HTTP_LIB_MAX_BRANCHS 10
#define HTTP_LIB_MAX_BRANCH_NAME_SIZE 256

#define HTTP_LIB_MAX_GET_DATA_MODEL_BUFFER_SIZE (2097152)	// 2MB
#define HTTP_LIB_MAX_SET_DATA_MODEL_BUFFER_SIZE (1024)		// 1Kb
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#endif // _HTTP_LIB_CONSTS_H
