#ifndef XMLParser_typedefs_h__
#define XMLParser_typedefs_h__

#include "XMLParser_defs.h"

#include "stdio.h"
#include <mxml.h>


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Defines
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define XML_PARSER_MAX_HANDLERS				20
#define XML_PARSER_MAX_SUBTREE				20
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

typedef struct 
{
	mxml_node_t*		subtree;									// Pointer to the root of the sub-tree
	mxml_node_t*		node;										// Pointer to the working node
	char				NodePath[XML_PARSER_MAX_STRING];			// Full node path from the root
	int					attrs;										// Number of attributes in the current node
} sXMLParserSubTree;

typedef struct 
{
	mxml_node_t*		tree;										// Pointer to the root of the XML tree
	sXMLParserSubTree*	SubTreeTable[XML_PARSER_MAX_SUBTREE+1];		// Pointer to sub-tree (need for searches)
																	// First Place is for default search
} sXMLParser;

extern sXMLParser* XMLParserHandlerTable[XML_PARSER_MAX_HANDLERS];

#endif // XMLParser_typedefs_h__
