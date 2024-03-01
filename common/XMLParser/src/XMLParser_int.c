
#include "XMLParser_int.h"
#include "XMLParser_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* XMLParser_AllocHandler()                                                                         *
*                                                                                                  *
* Allocate a free Handler                                                                          *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_AllocHandler(XMLParserHandler* Handler)
{
	*Handler = 1;

	while (*Handler <= XML_PARSER_MAX_HANDLERS)
	{
		if (XMLParserHandlerTable[*Handler-1] == NULL)
		{
			// Found empty place
			XMLParserHandlerTable[*Handler-1] = (sXMLParser*)malloc(sizeof(sXMLParser));

			XMLParserHandlerTable[*Handler-1]->tree = NULL;

			return eXMLPARSER_STAT_SUCCESS;
		}

		(*Handler)++;
	}

	return eXMLPARSER_STAT_FAILURE;
}

/***************************************************************************************************
* XMLParser_FreeHandler()                                                                          *
*                                                                                                  *
* Free a Handler                                                                                   *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_FreeHandler(XMLParserHandler Handler)
{
	if ((Handler < 1) || (Handler > XML_PARSER_MAX_HANDLERS))
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	if (XMLParserHandlerTable[Handler-1] == NULL)
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	free(XMLParserHandlerTable[Handler-1]);
	XMLParserHandlerTable[Handler-1] = NULL;

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_AllocSubTree()                                                                         *
*                                                                                                  *
* Allocate a free SubTree                                                                          *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_InitSubTree(XMLParserHandler Handler)
{
	int i;

	if ((Handler < 1) || (Handler > XML_PARSER_MAX_HANDLERS))
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	if (XMLParserHandlerTable[Handler-1] == NULL)
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	for (i=0;i<(XML_PARSER_MAX_SUBTREE+1);i++)
	{
		XMLParserHandlerTable[Handler-1]->SubTreeTable[i] = NULL;
	}

	return eXMLPARSER_STAT_SUCCESS;
}


/***************************************************************************************************
* XMLParser_AllocSubTree()                                                                         *
*                                                                                                  *
* Allocate a free ubTree                                                                          *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_AllocSubTree(XMLParserHandler Handler, XMLParserSubTree* SubTree)
{
	*SubTree = 0;

	while (*SubTree <= XML_PARSER_MAX_SUBTREE)
	{
		if (XMLParserHandlerTable[Handler-1]->SubTreeTable[*SubTree] == NULL)
		{
			// Found empty place
			XMLParserHandlerTable[Handler-1]->SubTreeTable[*SubTree] = (sXMLParserSubTree*)malloc(sizeof(sXMLParserSubTree));

			return eXMLPARSER_STAT_SUCCESS;
		}

		(*SubTree)++;
	}

	return eXMLPARSER_STAT_FAILURE;
}

/***************************************************************************************************
* XMLParser_FreeAllocSubTree()                                                                     *
*                                                                                                  *
* Free a SubTree                                                                                   *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_FreeAllocSubTree(XMLParserHandler Handler, XMLParserSubTree* SubTree)
{
	// Check Handler
	if ((Handler < 1) || (Handler > XML_PARSER_MAX_HANDLERS))
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	if (XMLParserHandlerTable[Handler-1] == NULL)
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	// Check SubTree
	if ((*SubTree < 0) || (*SubTree > XML_PARSER_MAX_SUBTREE))
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	if (XMLParserHandlerTable[Handler-1]->SubTreeTable[*SubTree] == NULL)
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	free(XMLParserHandlerTable[Handler-1]->SubTreeTable[*SubTree]);
	XMLParserHandlerTable[Handler-1]->SubTreeTable[*SubTree] = NULL;

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_CalculateNodePath()                                                                    *
*                                                                                                  *
* Calculate the node path of the current node                                                      *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_CalculateNodePath(XMLParserHandler Handler, XMLParserSubTree SubTree, char* NodePath)
{
	mxml_node_t*		node;

	char				NodeTemp[XML_PARSER_MAX_STRING];

	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	strcpy(NodePath, "");

	while ((node != NULL) && (node->parent != NULL))
	{
		// Ignore comments
		if (strncmp(node->value.element.name,"!--",3) != 0)
		{
			strcpy(NodeTemp,node->value.element.name);

			if (strlen(NodePath)>0)
			{
				strcat(NodeTemp,".");
				strcat(NodeTemp,NodePath);
			}

			strcpy(NodePath,NodeTemp);
		}
		node = node->parent;
	}

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_UpdateNodeInformation()                                                                *
*                                                                                                  *
* Update the structure "sXMLParserSubTree" in the "XMLParserHandlerTable"                          *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_UpdateNodeInformation(XMLParserHandler Handler, XMLParserSubTree SubTree, mxml_node_t* node, bool UpdateSubTreeNode)
{
	char				UpdatedNodePath[XML_PARSER_MAX_STRING];

	if (node == NULL)
	{
		XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node = NULL;
		XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->subtree = NULL;
		strcpy(XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->NodePath,"");
		XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->attrs = 0;

		return eXMLPARSER_STAT_SUCCESS;
	}

	XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node = node;

	if (UpdateSubTreeNode)
	{
		XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->subtree = node;
	}

	XMLParser_CalculateNodePath(Handler, SubTree, UpdatedNodePath);
	strcpy(XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->NodePath,UpdatedNodePath);
	XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->attrs = node->value.element.num_attrs;

	return eXMLPARSER_STAT_SUCCESS;
}


/***************************************************************************************************
* XMLParser_CheckUserHandlers()                                                                    *
*                                                                                                  *
* Check that user provide valid "XMLParserHandler" and "XMLParserSubTree"                          *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_CheckUserHandlers(XMLParserHandler Handler, XMLParserSubTree SubTree)
{
	// Check Handler
	if ((Handler < 1) || (Handler > XML_PARSER_MAX_HANDLERS))
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	if (XMLParserHandlerTable[Handler-1] == NULL)
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	// Check SubTree
	if ((SubTree < 1) || (SubTree > XML_PARSER_MAX_SUBTREE))
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	if (XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree] == NULL)
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	return eXMLPARSER_STAT_SUCCESS;
}



#ifdef __cplusplus
}
#endif
