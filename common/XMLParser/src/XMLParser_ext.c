// This is the main DLL file.

#include "stdio.h"

#include "XMLParser_int.h"

#include "XMLParser_ext.h"
#include "XMLParser_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* XMLParser_OpenXMLFile()                                                                          *
*                                                                                                  *
* Open an instance of XMLParser from XML file                                                      *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_OpenXMLFile(char* FileName, XMLParserHandler* Handler)
{
	eXMLPARSER_STAT		status;
	FILE*				fp;
	mxml_node_t*		tree;
	XMLParserSubTree	SubTree;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Load XML
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	fp = fopen(FileName, "r");

	if (fp == NULL)
	{
		printf("file not found\n");
		return eXMLPARSER_STAT_FILE_NOT_FOUND;
	}

	tree = mxmlLoadFile(NULL, fp, MXML_TEXT_CALLBACK);
	fclose(fp);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// Failed to load/parse the file
	if (tree == NULL)
	{
		return eXMLPARSER_STAT_FAILURE;
	}

	if ((status = XMLParser_AllocHandler(Handler)) != eXMLPARSER_STAT_SUCCESS)
	{
		mxmlDelete(tree);
		return status;
	}

	XMLParserHandlerTable[*Handler-1]->tree = tree;

	XMLParser_InitSubTree(*Handler);
	XMLParser_AllocSubTree(*Handler,&SubTree);
	XMLParser_UpdateNodeInformation(*Handler, SubTree, tree, TRUE);

	return eXMLPARSER_STAT_SUCCESS;
}

const char *_xml_whitespace_callback(mxml_node_t *node, int where)
{
	const char *name = node->value.element.name;

	if (node->type != MXML_ELEMENT)
	{
		return NULL;
	}
	if (name == NULL)
	{
		return NULL;
	}

	switch (where)
	{
		case MXML_WS_BEFORE_OPEN:
		{
		} break;

		case MXML_WS_AFTER_OPEN:
		{
			return "\n";
		} break;
		
		case MXML_WS_BEFORE_CLOSE:
		{
			return NULL;
		} break;

		case MXML_WS_AFTER_CLOSE:
		{
			return "\n";
		} break;

		default:
			return NULL;
	}

	return NULL;
}

/***************************************************************************************************
* XMLParser_SaveXMLFile()                                                                          *
*                                                                                                  *
* Save an instance of XMLParser into XML file                                                      *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_SaveXMLFile(XMLParserHandler Handler, char* FileName)
{
	FILE*				fp;
	mxml_node_t*		tree;

	// Check Handler
	if ((Handler < 1) || (Handler > XML_PARSER_MAX_HANDLERS))
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	if (XMLParserHandlerTable[Handler-1] == NULL)
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	// Disable wrapping - Save space - all data in one line
	mxmlSetWrapMargin(0);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Save XML
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	fp = fopen(FileName, "w");

	if (fp == NULL)
	{
		printf("failed to create the file\n");
		return eXMLPARSER_STAT_FILE_NOT_ACCESS;
	}

	tree = XMLParserHandlerTable[Handler-1]->tree;

	//mxmlSaveFile(tree, fp, MXML_NO_CALLBACK);
	mxmlSaveFile(tree, fp, _xml_whitespace_callback);
	fclose(fp);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_SaveXMLFile_SubTree()                                                                  *
*                                                                                                  *
* Save an instance of XMLParser SubTree into XML file                                              *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_SaveXMLFile_SubTree(XMLParserHandler Handler, XMLParserSubTree SubTree, char* FileName)
{
	eXMLPARSER_STAT		status;
	FILE*				fp;
	mxml_node_t*		tree;
	mxml_node_t*		node_next_backup;

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	// Disable wrapping - Save space - all data in one line
	//mxmlSetWrapMargin(0);

	// Set the margin to 30 columns
	//mxmlSetWrapMargin(30);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Save XML
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	fp = fopen(FileName, "w");

	if (fp == NULL)
	{
		printf("failed to create the file\n");
		return eXMLPARSER_STAT_FILE_NOT_ACCESS;
	}

	tree = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	node_next_backup = tree->next;
	tree->next = NULL;

	mxmlSaveFile(tree, fp, MXML_NO_CALLBACK);
	fclose(fp);

	tree->next = node_next_backup;
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_OpenXMLBuffer()                                                                        *
*                                                                                                  *
* Open an instance of XMLParser from XML buffer                                                    *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_OpenXMLBuffer(char* Buffer, XMLParserHandler* Handler)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		tree;
	XMLParserSubTree	SubTree;

	tree = mxmlLoadString(NULL, Buffer, MXML_TEXT_CALLBACK);

	// Failed to load/parse the file
	if (tree == NULL)
	{
		return eXMLPARSER_STAT_FAILURE;
	}

	if ((status = XMLParser_AllocHandler(Handler)) != eXMLPARSER_STAT_SUCCESS)
	{
		mxmlDelete(tree);
		return status;
	}

	XMLParserHandlerTable[*Handler-1]->tree = tree;

	XMLParser_InitSubTree(*Handler);
	XMLParser_AllocSubTree(*Handler,&SubTree);
	XMLParser_UpdateNodeInformation(*Handler, SubTree, tree, TRUE);

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_CloseXML()                                                                             *
*                                                                                                  *
* Close an instance of XMLParser                                                                   *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_CloseXML(XMLParserHandler* Handler)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		tree;
	int					i;

	if ((*Handler < 1) || (*Handler > XML_PARSER_MAX_HANDLERS))
	{
		return eXMLPARSER_STAT_INVALID_PARAMETER;
	}

	tree = XMLParserHandlerTable[*Handler-1]->tree;

	if (tree != NULL)
	{
		mxmlDelete(tree);
	}

	// Free all Sub-Trees
	for (i=0;i<(XML_PARSER_MAX_SUBTREE+1);i++)
	{
		XMLParserSubTree SubTree = i;

		XMLParser_FreeAllocSubTree(*Handler,&SubTree);
	}

	if ((status = XMLParser_FreeHandler(*Handler)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	*Handler = 0x00;

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_FindNodePath()                                                                         *
*                                                                                                  *
* Find the exact path "NodePath" in the XML tree                                                   *
*                                                                                                  *
* Update sub-tree                                                                                  *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_FindNodeByPath(XMLParserHandler Handler, XMLParserSubTree SubTree, char* NodePath)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		tree;
	mxml_node_t*		node;

	char				m_DesiredPath[XML_PARSER_MAX_STRING];
	char				m_NodePath[XML_PARSER_MAX_STRING];

	char*				NodeName = NULL;
	char				Delim[2] = ".";

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	tree = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->subtree;
	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Build the "m_DesiredPath"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	strcpy(m_DesiredPath,XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->NodePath);
	if (strlen(m_DesiredPath)>0)
	{
		strcat(m_DesiredPath,".");
	}
	strcat(m_DesiredPath,NodePath);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


	strcpy(m_NodePath,NodePath);
	NodeName = strtok(m_NodePath,Delim);
	while (NodeName != NULL)
	{
		//printf("NodeName = \"%s\"\n",NodeName);

		node = mxmlFindElement(	node,
								tree,
								NodeName,				// node name
								NULL,					// attr name
								NULL,					// value
								MXML_DESCEND);

		if (node == NULL)
		{
			XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node = NULL;
			return eXMLPARSER_STAT_NODE_NOT_FOUND;
		}

		NodeName = strtok(NULL, Delim);
	}

	if (node->type != MXML_ELEMENT)
	{
		XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node = NULL;
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	XMLParser_UpdateNodeInformation(Handler, SubTree, node, TRUE);

	if (strcmp(m_DesiredPath,XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->NodePath) != 0)
	{
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_FindNode()                                                                             *
*                                                                                                  *
* Find the next node with "NodeName"                                                               *
*                                                                                                  *
* Doesn't Update sub-tree                                                                          *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_FindNode(XMLParserHandler Handler, XMLParserSubTree SubTree, char* NodeName)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		tree;
	mxml_node_t*		node;

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	tree = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->subtree;
	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	do 
	{
		node = mxmlFindElement(	node,
								tree,
								NodeName,				// node name
								NULL,					// attr name
								NULL,					// value
								MXML_DESCEND);

		if (node == NULL)
		{
			XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node = NULL;
			return eXMLPARSER_STAT_NODE_NOT_FOUND;
		}

		if (node->type != MXML_ELEMENT)
		{
			XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node = NULL;
			return eXMLPARSER_STAT_NODE_NOT_FOUND;
		}
	}
	while (node->parent != tree);

	XMLParser_UpdateNodeInformation(Handler, SubTree, node, FALSE);

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_CreateSubTree()                                                                        *
*                                                                                                  *
* Duplicate a SubTree                                                                              *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_CreateSubTree(XMLParserHandler Handler, char* NodePath, XMLParserSubTree* SubTree)
{
	mxml_node_t*		node;

	XMLParser_AllocSubTree(Handler,SubTree);

	node = XMLParserHandlerTable[Handler-1]->tree;

	XMLParser_UpdateNodeInformation(Handler, *SubTree, node, TRUE);

	if (XMLParser_FindNodeByPath(Handler,*SubTree,NodePath) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_FreeAllocSubTree(Handler,SubTree);

		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	return eXMLPARSER_STAT_SUCCESS;
}


/***************************************************************************************************
* XMLParser_DuplicateSubTree()                                                                     *
*                                                                                                  *
* Duplicate a SubTree                                                                              *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_DuplicateSubTree(XMLParserHandler Handler, XMLParserSubTree SubTree, XMLParserSubTree* SubTreeNew)
{
	mxml_node_t*		node;

	XMLParser_AllocSubTree(Handler,SubTreeNew);

	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	XMLParser_UpdateNodeInformation(Handler, *SubTreeNew, node, TRUE);

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_FreeSubTree()                                                                         *
*                                                                                                  *
* Close a SubTree                                                                                  *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_FreeSubTree(XMLParserHandler Handler, XMLParserSubTree* SubTree)
{
	eXMLPARSER_STAT		status;

	if ((status = XMLParser_CheckUserHandlers(Handler,*SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	XMLParser_FreeAllocSubTree(Handler,SubTree);

	*SubTree = XMLParserInvalidSubTree;

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_GetNodeNumOfAttributes()                                                               *
*                                                                                                  *
* Get the number of attributes at the current node                                                 *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_GetNodeNumOfAttributes(XMLParserHandler Handler, XMLParserSubTree SubTree, int* nAttributes)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		node;

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	if (node == NULL)
	{
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	*nAttributes = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->attrs;

	return eXMLPARSER_STAT_SUCCESS;
}


/***************************************************************************************************
* XMLParser_GetNodeAttributeByIndex()                                                              *
*                                                                                                  *
* Get the attributes from the current node with specific index                                     *
*                                                                                                  *
* nAttributeIndex - start from 1                                                                   *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_GetNodeAttributeByIndex(XMLParserHandler Handler, XMLParserSubTree SubTree, int nAttributeIndex, char* Name,char* Value)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		node;
	int					attrs;

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;
	attrs = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->attrs;

	if (node == NULL)
	{
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	if (nAttributeIndex <= attrs)
	{
		strcpy(Name,node->value.element.attrs[nAttributeIndex-1].name);
		strcpy(Value,node->value.element.attrs[nAttributeIndex-1].value);

		return eXMLPARSER_STAT_SUCCESS;
	}

	return eXMLPARSER_STAT_ATTR_NOT_FOUND;
}


/***************************************************************************************************
* XMLParser_GetNodeAttributeByName()                                                               *
*                                                                                                  *
* Search for a specific attribute name in the current node                                         *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_GetNodeAttributeByName(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name, char* Value)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		node;
	int					attrs;
	int					i;

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;
	attrs = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->attrs;

	if (node == NULL)
	{
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	for (i=1 ; i<=attrs; i++)
	{
		if (strcmp(Name,node->value.element.attrs[i-1].name) == 0)
		{
			strcpy(Value,node->value.element.attrs[i-1].value);
			
			return eXMLPARSER_STAT_SUCCESS;
		}
	}

	return eXMLPARSER_STAT_ATTR_NOT_FOUND;
}

/***************************************************************************************************
* XMLParser_SetNodeAttributeByName()                                                               *
*                                                                                                  *
* Update a specific attribute value in the current node                                            *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_SetNodeAttributeByName(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name, char* Value)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		node;

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	if (node == NULL)
	{
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	//mxmlNewText(node, 0, Value);
	mxmlElementSetAttr(node, Name, Value);

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_GetNodeAttributeByNodePath()                                                           *
*                                                                                                  *
* Get a specific attribute from Node Path                                                          *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_GetNodeAttributeByNodePath(XMLParserHandler Handler, char* NodePath, char* Name, char* Value)
{
	XMLParserSubTree	SubTree;
	eXMLPARSER_STAT		RetVal = eXMLPARSER_STAT_ATTR_NOT_FOUND;

	// Search for the node
	if (XMLParser_CreateSubTree(Handler,NodePath,&SubTree) == eXMLPARSER_STAT_SUCCESS)
	{
		// Search for the Attribute Name
		if (XMLParser_GetNodeAttributeByName(Handler, SubTree ,Name, Value) == eXMLPARSER_STAT_SUCCESS)
		{
			RetVal = eXMLPARSER_STAT_SUCCESS;
		}

		XMLParser_FreeSubTree(Handler,&SubTree);
	}

	return RetVal;
}


/***************************************************************************************************
* XMLParser_Get_Element_Name()                                                                     *
*                                                                                                  *
* Get the element name of a current node                                                           *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_GetNodeElementName(XMLParserHandler Handler, XMLParserSubTree SubTree, char* ElementName)
{
	mxml_node_t*		node;

	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	strcpy(ElementName,node->value.element.name);

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_GetNodePath()                                                                          *
*                                                                                                  *
* Get the "NodePath" from the the XML tree                                                         *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_GetNodePath(XMLParserHandler Handler, XMLParserSubTree SubTree, char* NodePath)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		node;

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	if (node == NULL)
	{
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	strcpy(NodePath,XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->NodePath);

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_GetNextNode()                                                                          *
*                                                                                                  *
* Get the next node (the next node in the same tree hierarchy)                                     *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_GetNextNode(XMLParserHandler Handler, XMLParserSubTree SubTree)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		node;

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	if ((node == NULL) || (node->next == NULL))
	{
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}
	
	node = node->next;

	// Fix '\n' between two XML TAGs
	if (node->type == MXML_TEXT)
	{
		node = node->next;
	}

	XMLParser_UpdateNodeInformation(Handler, SubTree, node, FALSE);

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_GetChildNode()                                                                         *
*                                                                                                  *
* Get the first child node (the first child node one level down from the current node hierarchy)   *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_GetChildNode(XMLParserHandler Handler, XMLParserSubTree SubTree)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		node;

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	if ((node == NULL) || (node->child == NULL))
	{
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	node = node->child;

	// Fix '\n' between two XML TAGs
	if (node->type == MXML_TEXT)
	{
		node = node->next;
	}

	XMLParser_UpdateNodeInformation(Handler, SubTree,node, FALSE);

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_GetParentNode()                                                                        *
*                                                                                                  *
* Get the parent node (the parent node of the current node)                                        *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_GetParentNode(XMLParserHandler Handler, XMLParserSubTree SubTree)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		node;

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	if ((node == NULL) || (node->parent == NULL))
	{
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	node = node->parent;

	XMLParser_UpdateNodeInformation(Handler, SubTree, node, FALSE);

	return eXMLPARSER_STAT_SUCCESS;
}

/***************************************************************************************************
* XMLParser_GetWalkNode()                                                                          *
*                                                                                                  *
* Get the child/next node (Iterate through the XML node tree)                                      *
***************************************************************************************************/
eXMLPARSER_STAT XMLParser_GetWalkNode(XMLParserHandler Handler, XMLParserSubTree SubTree)
{
	eXMLPARSER_STAT		status;
	mxml_node_t*		tree;
	mxml_node_t*		node;

	if ((status = XMLParser_CheckUserHandlers(Handler,SubTree)) != eXMLPARSER_STAT_SUCCESS)
	{
		return status;
	}

	tree = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->subtree;
	node = XMLParserHandlerTable[Handler-1]->SubTreeTable[SubTree]->node;

	if (node == NULL)
	{
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	do 
	{
		node = mxmlWalkNext(node, tree, MXML_DESCEND);
	} while ((node != NULL) && (node->type != MXML_ELEMENT));

	if (node == NULL)
	{
		return eXMLPARSER_STAT_NODE_NOT_FOUND;
	}

	XMLParser_UpdateNodeInformation(Handler, SubTree, node, FALSE);

	return eXMLPARSER_STAT_SUCCESS;
}

#ifdef __cplusplus
}
#endif
