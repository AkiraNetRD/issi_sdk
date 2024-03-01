#ifndef XMLParser_defs_h__
#define XMLParser_defs_h__

#include "defs.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Defines
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define XMLParserHandler					UINT32
#define XMLParserSubTree					UINT32
#define XMLParserDefaultSubTree				0
#define XMLParserInvalidSubTree				-1
#define XML_PARSER_MAX_STRING				16384
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Status code 
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
typedef enum 
{
	eXMLPARSER_STAT_SUCCESS                     = 0,		// general success
	eXMLPARSER_STAT_FAILURE						= 1,		// general failure
	
	eXMLPARSER_STAT_INVALID_PARAMETER			= 2,		// invalid parameter
	eXMLPARSER_STAT_NOT_SUPPORTED				= 3,		// requested not supported

	eXMLPARSER_STAT_NODE_NOT_FOUND				= 4,		// XML node not found
	eXMLPARSER_STAT_ATTR_NOT_FOUND				= 5,		// XML attribute not found

	eXMLPARSER_STAT_FILE_NOT_FOUND				= 6,		// file not found
	eXMLPARSER_STAT_FILE_NOT_ACCESS				= 7,		// file not access

} eXMLPARSER_STAT;
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#endif // XMLParser_defs_h__
