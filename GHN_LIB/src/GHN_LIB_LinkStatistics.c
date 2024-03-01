#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#include "console.h"

#include "GHN_LIB_LinkStatistics.h"
#include "GHN_LIB_int_consts.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************************************************************************
* Link_Statistics_ReadCEParameters()                                                               *
*                                                                                                  *
* 1. Check that RX-MAC is the same in the XML file                                                 *
* 2. Search for the TX-MAC in the AssociatedDevice list                                            *
* 3. Read the counters                                                                             *
* 4. Backup the previous counters and copy the current ones                                        *
***************************************************************************************************/
eGHN_LIB_STAT Link_Statistics_ReadCEParameters(XMLParserHandler XMLHandler,sLink_Statistics_Test* linkStatistics_Test, char* NodePrefix)
{
	sLink_Statistics	Counters;

	mac_address_t		StaRxMAC;
	mac_address_t		StaTxMAC;

	XMLParserSubTree	SubTree;
	XMLParserSubTree	SubTree_CEParameters;

	char				Value[XML_PARSER_MAX_STRING];
	UINT32				Intervals;
	char				NodePath[XML_PARSER_MAX_STRING];

	LOG_INFO("Started...");

	LOG_INFO("Link_Statistics_Test->StaRxMAC ("MAC_ADDR_FMT")", MAC_ADDR(linkStatistics_Test->StaRxMAC));
	LOG_INFO("Link_Statistics_Test->StaTxMAC ("MAC_ADDR_FMT")", MAC_ADDR(linkStatistics_Test->StaTxMAC));

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Check that RX-MAC is the same in the XML file
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface);
	if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath, "GhnMACAddress", &Value[0]) != eXMLPARSER_STAT_SUCCESS)
	{
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}

	str_to_mac(Value,&StaRxMAC);

	LOG_INFO("Read StaRxMAC GhnMACAddress (%s)",Value);

	if (memcmp(linkStatistics_Test->StaRxMAC,StaRxMAC,sizeof(mac_address_t)) != 0)
	{
		return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Create a SubTree for searching all AssociatedDevice under "Node_Interface"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface);
	if (XMLParser_CreateSubTree(XMLHandler, NodePath, &SubTree) != eXMLPARSER_STAT_SUCCESS)
	{
		return eGHN_LIB_STAT_FAILURE;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Search for the TX-MAC in the AssociatedDevice list
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	while (XMLParser_FindNode(XMLHandler, SubTree, "AssociatedDevice") == eXMLPARSER_STAT_SUCCESS)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Check that TX-MAC is the same
		// If not => continue with the loop
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (XMLParser_GetNodeAttributeByName(XMLHandler, SubTree, "GhnMACAddress",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			continue;
		}
		LOG_INFO("Read StaTxMAC GhnMACAddress (%s)",Value);

		str_to_mac(Value,&StaTxMAC);

		if (memcmp(linkStatistics_Test->StaTxMAC,StaTxMAC,sizeof(mac_address_t)) != 0)
		{
			continue;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		Counters.RxEnergyChannel_0 = XMLParser_GetIntValue(XMLHandler, SubTree, "RxEnergyChannel_0");
		Counters.RxEnergyChannel_1 = XMLParser_GetIntValue(XMLHandler, SubTree, "RxEnergyChannel_1");

		Counters.BytesReceived = XMLParser_GetULONG64Value(XMLHandler, SubTree, "X_00C5D9_BytesReceived");

		// Read the counters
		Counters.IntervalCEParametersNumberOfEntries = XMLParser_GetUIntValue(XMLHandler, SubTree, "IntervalCEParametersNumberOfEntries");

		XMLParser_DuplicateSubTree(XMLHandler, SubTree, &SubTree_CEParameters);

		Intervals=0;

		while (XMLParser_FindNode(XMLHandler, SubTree_CEParameters, "IntervalCEParameters") == eXMLPARSER_STAT_SUCCESS)
		{
			sLink_Statistics_IntervalCEParameters* CE;

			Intervals++;

			// Sanity Check
			if (Intervals > PHY_MAX_INTERVALS)
			{
				XMLParser_FreeSubTree(XMLHandler, &SubTree_CEParameters);
				XMLParser_FreeSubTree(XMLHandler, &SubTree);
				return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
			}

			CE = &Counters.IntervalCEParameters[Intervals-1];
			CE->TotalBitLoading	= XMLParser_GetUIntValue(XMLHandler, SubTree_CEParameters, "TotalBitLoading");
			CE->CodeRate		= XMLParser_GetUIntValue(XMLHandler, SubTree_CEParameters, "CodeRate");
			CE->AverageSNR		= XMLParser_GetIntValue(XMLHandler, SubTree_CEParameters, "AverageSNR");
		}

		XMLParser_FreeSubTree(XMLHandler, &SubTree_CEParameters);
		XMLParser_FreeSubTree(XMLHandler, &SubTree);

		// Sanity Check
		if (Intervals != Counters.IntervalCEParametersNumberOfEntries)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}

		// Copy counters
		memcpy(&linkStatistics_Test->counters,&Counters,sizeof(sLink_Statistics));

		return eGHN_LIB_STAT_SUCCESS;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// TX-MAC not found
	XMLParser_FreeSubTree(XMLHandler, &SubTree);

	return eGHN_LIB_STAT_FAILURE;
}

eGHN_LIB_STAT Link_Statistics_CalcCEParameters(sLink_Statistics_Test* Link_Statistics_Test,sLink_Statistics_Result* linkStatistics_Result)
{
	/*
		FEC_RATE		Interpretation
		value
		(b4b3b2)
		000				Reserved by ITU
		001				1/2
		010				2/3
		011				5/6
		100				16/18
		101				20/21
		110-111			Reserved by ITU
	*/
	double		CodeRateMultipleFactor[] =	{	0,						// Index 0
												(double)1/2,			// Index 1
												(double)2/3,			// Index 2
												(double)5/6,			// Index 3
												(double)16/18,			// Index 4
												(double)20/21,	 		// Index 5
											};

	sLink_Statistics*	Counters;
	UINT32				Interval;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Inits
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	Counters = &Link_Statistics_Test->counters;

	linkStatistics_Result->NumberOfInterval = Counters->IntervalCEParametersNumberOfEntries;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Sanity checks
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	for (Interval=0 ; Interval< linkStatistics_Result->NumberOfInterval ; Interval++)
	{
		sLink_Statistics_IntervalCEParameters* CE = &Counters->IntervalCEParameters[Interval];

		if (CE->CodeRate>5)
		{
			return eGHN_LIB_STAT_FAILED_PARSE_NETINF_RESULT;
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// PHY Rate
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	linkStatistics_Result->PHY = 0;

	for (Interval=0 ; Interval< linkStatistics_Result->NumberOfInterval ; Interval++)
	{
		sLink_Statistics_IntervalCEParameters* CE = &Counters->IntervalCEParameters[Interval];

		linkStatistics_Result->PHY += (CE->TotalBitLoading * CodeRateMultipleFactor[CE->CodeRate%6] / TOTAL_SYMBOL_DURATION_IN_MICRO_SECONDS);
	}

	linkStatistics_Result->PHY /= linkStatistics_Result->NumberOfInterval;
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// SNR
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	for (Interval=0 ; Interval< linkStatistics_Result->NumberOfInterval ; Interval++)
	{
		sLink_Statistics_IntervalCEParameters* CE = &Counters->IntervalCEParameters[Interval];

		linkStatistics_Result->SNR[Interval] = (SINT32)(CE->AverageSNR);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// RX-Power
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if ((Link_Statistics_Test->NodeTypeConfiguration == eLink_Statistics_NodeTypeConfiguration_MIMO) ||
		(Link_Statistics_Test->NodeTypeConfiguration == eLink_Statistics_NodeTypeConfiguration_SISO))
	{
		linkStatistics_Result->RXPower0 = Counters->RxEnergyChannel_0;
		linkStatistics_Result->RXPower1 = Counters->RxEnergyChannel_1;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	linkStatistics_Result->BytesReceived = Counters->BytesReceived;

	return eGHN_LIB_STAT_SUCCESS;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
