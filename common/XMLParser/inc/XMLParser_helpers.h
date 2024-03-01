#ifndef XMLParser_helpers_h__
#define XMLParser_helpers_h__

#include "XMLParser.h"

#ifdef _WIN32
#define DllExport   __declspec( dllexport ) 
#else
#define DllExport
#endif

#ifdef __cplusplus
extern "C" {
#endif

DllExport int XMLParser_GetIntValue(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name);
DllExport unsigned int XMLParser_GetUIntValue(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name);
DllExport ULONG64 XMLParser_GetULONG64Value(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name);
DllExport float XMLParser_GetFloatValue(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name);

DllExport void XMLParser_PrintAllAttributes(XMLParserHandler Handler, XMLParserSubTree SubTree);
DllExport void XMLParser_PrintAllNodes(XMLParserHandler Handler, XMLParserSubTree SubTree);

#ifdef __cplusplus
}
#endif


#endif // XMLParser_helpers_h__
