#ifndef XMLParser_int_h__
#define XMLParser_int_h__

#include "XMLParser_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

eXMLPARSER_STAT XMLParser_AllocHandler(XMLParserHandler* Handler);
eXMLPARSER_STAT XMLParser_FreeHandler(XMLParserHandler Handler);

// Sub-Tree
eXMLPARSER_STAT XMLParser_InitSubTree(XMLParserHandler Handler);
eXMLPARSER_STAT XMLParser_AllocSubTree(XMLParserHandler Handler, XMLParserSubTree* SubTree);
eXMLPARSER_STAT XMLParser_FreeAllocSubTree(XMLParserHandler Handler, XMLParserSubTree* SubTree);

eXMLPARSER_STAT XMLParser_CalculateNodePath(XMLParserHandler Handler, XMLParserSubTree SubTree, char* NodePath);
eXMLPARSER_STAT XMLParser_UpdateNodeInformation(XMLParserHandler Handler, XMLParserSubTree SubTree, mxml_node_t* node, bool UpdateSubTreeNode);

eXMLPARSER_STAT XMLParser_CheckUserHandlers(XMLParserHandler Handler, XMLParserSubTree SubTree);


#ifdef __cplusplus
}
#endif

#endif // XMLParser_int_h__
