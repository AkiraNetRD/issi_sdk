
#include "XMLParser_defs.h"
#include "XMLParser_ext.h"
#include "XMLParser_helpers.h"

#include "stdio.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
* XMLParser_GetIntValue()                                                                          *
*                                                                                                  *
* Get the integer value of a specific attribute from the current node                              *
***************************************************************************************************/
int XMLParser_GetIntValue(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name)
{
	char				Value[XML_PARSER_MAX_STRING];

	if (XMLParser_GetNodeAttributeByName(Handler,SubTree,Name,Value) == eXMLPARSER_STAT_SUCCESS)
	{
		int RetVal;

		RetVal = strtol(Value, NULL, 10);

		return RetVal;
	}

	return 0;
}

/***************************************************************************************************
* XMLParser_GetUIntValue()                                                                         *
*                                                                                                  *
* Get the unsigned integer value of a specific attribute from the current node                     *
***************************************************************************************************/
unsigned int XMLParser_GetUIntValue(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name)
{
	char				Value[XML_PARSER_MAX_STRING];

	if (XMLParser_GetNodeAttributeByName(Handler,SubTree,Name,Value) == eXMLPARSER_STAT_SUCCESS)
	{
		unsigned int RetVal;

		RetVal = strtoul(Value, NULL, 10);

		return RetVal;
	}

	return 0;
}

/***************************************************************************************************
* XMLParser_GetULONG64Value()                                                                      *
*                                                                                                  *
* Get the unsigned integer value of a specific attribute from the current node                     *
***************************************************************************************************/
ULONG64 XMLParser_GetULONG64Value(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name)
{
	char				Value[XML_PARSER_MAX_STRING];

	if (XMLParser_GetNodeAttributeByName(Handler,SubTree,Name,Value) == eXMLPARSER_STAT_SUCCESS)
	{
		ULONG64 RetVal;

		RetVal = strtoll(Value, NULL, 10);

		return RetVal;
	}

	return 0;
}

/***************************************************************************************************
* XMLParser_GetFloatValue()                                                                        *
*                                                                                                  *
* Get a float value of a specific attribute from the current node                                  *
***************************************************************************************************/
float XMLParser_GetFloatValue(XMLParserHandler Handler, XMLParserSubTree SubTree, char* Name)
{
	char				Value[XML_PARSER_MAX_STRING];

	if (XMLParser_GetNodeAttributeByName(Handler,SubTree,Name,Value) == eXMLPARSER_STAT_SUCCESS)
	{
		return (float)atof(Value);
	}

	return 0;
}

/***************************************************************************************************
* XMLParser_PrintAllAttributes()                                                                   *
*                                                                                                  *
* Print all the attributes in the current node                                                     *
***************************************************************************************************/
void XMLParser_PrintAllAttributes(XMLParserHandler Handler, XMLParserSubTree SubTree)
{
	char				Name[XML_PARSER_MAX_STRING];
	char				Value[XML_PARSER_MAX_STRING];
	char				NodePath[XML_PARSER_MAX_STRING];

	int					nAttributes;
	int					i;

	if (XMLParser_GetNodePath(Handler,SubTree,NodePath) != eXMLPARSER_STAT_SUCCESS)
	{
		return;
	}

	if (XMLParser_GetNodeNumOfAttributes(Handler,SubTree,&nAttributes) != eXMLPARSER_STAT_SUCCESS)
	{
		return;
	}

	for (i=1; i<=nAttributes; i++)
	{
		XMLParser_GetNodeAttributeByIndex(Handler,SubTree,i,Name,Value);
		{
			printf("%-70s (%2i) %-40s = %s\n", NodePath, i, Name, Value);
		}
	}
}

/***************************************************************************************************
* XMLParser_PrintAllNodes()                                                                        *
*                                                                                                  *
* Print all the nodes in the current subtree                                                       *
***************************************************************************************************/
void XMLParser_PrintAllNodes(XMLParserHandler Handler, XMLParserSubTree SubTree)
{
	XMLParserSubTree SubTree_Duplicated;

	XMLParser_DuplicateSubTree(Handler,SubTree,&SubTree_Duplicated);

	do 
	{
		XMLParser_PrintAllAttributes(Handler,SubTree_Duplicated);
	}
	while (XMLParser_GetWalkNode(Handler,SubTree_Duplicated) == eXMLPARSER_STAT_SUCCESS);

	XMLParser_FreeSubTree(Handler,&SubTree_Duplicated);
}

#ifdef __cplusplus
}
#endif
