#ifndef XMLParser_ext_h__
#define XMLParser_ext_h__

#include "XMLParser_defs.h"

#ifdef _WIN32
#define DllExport   __declspec( dllexport ) 
#else
#define DllExport
#endif

#ifdef __cplusplus
extern "C" {
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// XMLParser API
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
DllExport eXMLPARSER_STAT XMLParser_OpenXMLFile(char* FileName, XMLParserHandler* Handler);
DllExport eXMLPARSER_STAT XMLParser_SaveXMLFile(XMLParserHandler Handler, char* FileName);
DllExport eXMLPARSER_STAT XMLParser_SaveXMLFile_SubTree(XMLParserHandler Handler, XMLParserSubTree SubTree, char* FileName);
DllExport eXMLPARSER_STAT XMLParser_OpenXMLBuffer(char* Buffer, XMLParserHandler* Handler);
DllExport eXMLPARSER_STAT XMLParser_CloseXML(XMLParserHandler* Handler);

// Sub-Tree
DllExport eXMLPARSER_STAT XMLParser_CreateSubTree(XMLParserHandler Handler, char* NodePath, XMLParserSubTree* SubTree);
DllExport eXMLPARSER_STAT XMLParser_DuplicateSubTree(XMLParserHandler Handler, XMLParserSubTree SubTree, XMLParserSubTree* SubTreeNew);
DllExport eXMLPARSER_STAT XMLParser_FreeSubTree(XMLParserHandler Handler, XMLParserSubTree* SubTree);

// Find a node
DllExport eXMLPARSER_STAT XMLParser_FindNodeByPath(XMLParserHandler Handler, XMLParserSubTree SubTree, char* NodePath);
DllExport eXMLPARSER_STAT XMLParser_FindNode(XMLParserHandler Handler, XMLParserSubTree SubTree, char* NodeName);

// Iterate through the XML node tree
DllExport eXMLPARSER_STAT XMLParser_GetNextNode(XMLParserHandler Handler, XMLParserSubTree SubTree);
DllExport eXMLPARSER_STAT XMLParser_GetChildNode(XMLParserHandler Handler, XMLParserSubTree SubTree);
DllExport eXMLPARSER_STAT XMLParser_GetParentNode(XMLParserHandler Handler, XMLParserSubTree SubTree);
DllExport eXMLPARSER_STAT XMLParser_GetWalkNode(XMLParserHandler Handler, XMLParserSubTree SubTree);

// Get node information
DllExport eXMLPARSER_STAT XMLParser_GetNodeElementName(XMLParserHandler Handler, XMLParserSubTree SubTree, char* ElementName);
DllExport eXMLPARSER_STAT XMLParser_GetNodePath(XMLParserHandler Handler, XMLParserSubTree SubTree, char* NodePath);
DllExport eXMLPARSER_STAT XMLParser_GetNodeNumOfAttributes(XMLParserHandler Handler, XMLParserSubTree SubTree, int* nAttributes);
DllExport eXMLPARSER_STAT XMLParser_GetNodeAttributeByIndex(XMLParserHandler Handler, XMLParserSubTree SubTree, int nAttribute, char* Name,char* Value);
DllExport eXMLPARSER_STAT XMLParser_GetNodeAttributeByName(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name,char* Value);
DllExport eXMLPARSER_STAT XMLParser_GetNodeAttributeByNodePath(XMLParserHandler Handler, char* NodePath, char* Name, char* Value);
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Set node information
DllExport eXMLPARSER_STAT XMLParser_SetNodeAttributeByName(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name, char* Value);

#ifdef __cplusplus
}
#endif


#endif // XMLParser_ext_h__
