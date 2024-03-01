#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#include "console.h"

#include "GHN_LIB_netinf.h"
#include "GHN_LIB_int_consts.h"
#include "GHN_LIB_int.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Example for input "Wed 27 Feb 2013 13:16:35 GMT"
time_t CreateTimeT_From_TimeStamp(char* TimeStamp)
{
	struct tm			timeinfo;
	time_t				rawtime;

	strptime(TimeStamp,"%a %d %b %Y %H:%M:%S",&timeinfo);

	rawtime = mktime(&timeinfo);

	return rawtime;
}

void strcpy_Without_Comma(char* Destination, char* Source)
{
	while (*Source != '\0')
	{
		if (*Source != ',')
		{
			*Destination++ = *Source;
		}
		Source++;
	}

	*Destination = '\0';
}

eGHN_LIB_STAT Netinf_ReadCEParameters_AssociatedDevice(	XMLParserHandler	XMLHandler,
															XMLParserSubTree	SubTree,
															sNetinf*			Counters,
															sNetinf_Test*		Netinf_Test)
{
	mac_address_t		StaTxMAC;

	XMLParserSubTree	SubTree_CEParameters;

	char				Value[XML_PARSER_MAX_STRING];
	UINT32				Intervals;

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

		if (memcmp(Netinf_Test->StaTxMAC,StaTxMAC,sizeof(mac_address_t)) != 0)
		{
			continue;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		Counters->X_0023C7_Index = XMLParser_GetIntValue(XMLHandler, SubTree, "X_0023C7_Index");
		Counters->GhnDeviceID = XMLParser_GetIntValue(XMLHandler, SubTree, "GhnDeviceID");

		Counters->RxPowerChannel_0 = XMLParser_GetIntValue(XMLHandler, SubTree, "RxPowerChannel_0");
		Counters->RxPowerChannel_1 = XMLParser_GetIntValue(XMLHandler, SubTree, "RxPowerChannel_1");

		Counters->RxEnergyChannel_0 = XMLParser_GetIntValue(XMLHandler, SubTree, "RxEnergyChannel_0");
		Counters->RxEnergyChannel_1 = XMLParser_GetIntValue(XMLHandler, SubTree, "RxEnergyChannel_1");

		Counters->BytesReceived = XMLParser_GetULONG64Value(XMLHandler, SubTree, "X_00C5D9_BytesReceived");

		// Read the counters
		Counters->IntervalCEParametersNumberOfEntries = XMLParser_GetUIntValue(XMLHandler, SubTree, "IntervalCEParametersNumberOfEntries");

		XMLParser_DuplicateSubTree(XMLHandler, SubTree, &SubTree_CEParameters);

		Intervals=0;

		while (XMLParser_FindNode(XMLHandler, SubTree_CEParameters, "IntervalCEParameters") == eXMLPARSER_STAT_SUCCESS)
		{
			sNetinf_IntervalCEParameters* CE;

			Intervals++;

			// Sanity Check
			if (Intervals > PHY_MAX_INTERVALS)
			{
				XMLParser_FreeSubTree(XMLHandler, &SubTree_CEParameters);
				return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
			}

			CE = &Counters->IntervalCEParameters[Intervals-1];
			CE->TotalBitLoading	= XMLParser_GetUIntValue(XMLHandler, SubTree_CEParameters, "TotalBitLoading");
			CE->CodeRate		= XMLParser_GetUIntValue(XMLHandler, SubTree_CEParameters, "CodeRate");
			CE->RxTotalSegments	= XMLParser_GetUIntValue(XMLHandler, SubTree_CEParameters, "RxTotalSegments");
			CE->RxCRCSegments	= XMLParser_GetUIntValue(XMLHandler, SubTree_CEParameters, "RxCRCSegments");
			CE->AverageSNR		= XMLParser_GetIntValue(XMLHandler, SubTree_CEParameters, "AverageSNR");
			CE->GroupID			= XMLParser_GetUIntValue(XMLHandler, SubTree_CEParameters, "GroupID");

			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// BitAllocationTable
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			memset(&CE->BitAllocationTable[0],0x00,sizeof(CE->BitAllocationTable));
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		}

		XMLParser_FreeSubTree(XMLHandler, &SubTree_CEParameters);

		// Sanity Check
		if (Intervals != Counters->IntervalCEParametersNumberOfEntries)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}

		return eGHN_LIB_STAT_SUCCESS;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	return eGHN_LIB_STAT_FAILURE;
}

/***************************************************************************************************
* Netinf_ReadCEParameters()                                                                        *
*                                                                                                  *
* 1. Check that RX-MAC is the same in the XML file                                                 *
* 2. Search for the TX-MAC in the AssociatedDevice list                                            *
* 3. Read the counters                                                                             *
* 4. Backup the previous counters and copy the current ones                                        *
***************************************************************************************************/
eGHN_LIB_STAT Netinf_ReadCEParameters(	XMLParserHandler		XMLHandler,
										sNetinf_Test*			Netinf_Test,
										eNetinf_ParsingMethod	Netinf_ParsingMethod,
										char*					NodePrefix,
										int						SampleSet_Index,
										char*					XMLFileName,
										bool					bOutputDateTimeWithMilliseconds)
{
	eGHN_LIB_STAT		RetVal;

	sNetinf				Counters;

	mac_address_t		StaRxMAC;

	XMLParserSubTree	SubTree;
	XMLParserSubTree	SubTree_SampleSet;
	XMLParserSubTree	SubTree_SampleSet_Stats;

	char				Value[XML_PARSER_MAX_STRING];
	char				NodePath[XML_PARSER_MAX_STRING];

	int					i;

	LOG_INFO("Started...");

	LOG_INFO("Netinf_Test->StaRxMAC ("MAC_ADDR_FMT")", MAC_ADDR(Netinf_Test->StaRxMAC));
	LOG_INFO("Netinf_Test->StaTxMAC ("MAC_ADDR_FMT")", MAC_ADDR(Netinf_Test->StaTxMAC));

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

	if (memcmp(Netinf_Test->StaRxMAC,StaRxMAC,sizeof(mac_address_t)) != 0)
	{
		return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if (Netinf_ParsingMethod == eNetinf_ParsingMethod_XML_Files_HD_RemoteMonitoring)
	{
		// Create a SubTree with the required SampleSet in PeridoicStatistics
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface_PeriodicStatistics);
		if (XMLParser_CreateSubTree(XMLHandler, NodePath, &SubTree_SampleSet) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILURE;
		}

		if (XMLParser_GetChildNode(XMLHandler, SubTree_SampleSet) != eXMLPARSER_STAT_SUCCESS)
		{
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
			return eGHN_LIB_STAT_FAILURE;
		}

		for (i=1;i<SampleSet_Index;i++)
		{
			if (XMLParser_GetNextNode(XMLHandler, SubTree_SampleSet) != eXMLPARSER_STAT_SUCCESS)
			{
				XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
				return eGHN_LIB_STAT_FAILURE;
			}
		}

		//XMLParser_GetNodeAttributeByIndex(XMLHandler, SubTree_SampleSet, 1, Name, Value);
	}


	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get "CurrentLocalTime"/"TimeStamp"/"ReportEndTime"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	switch (Netinf_ParsingMethod)
	{
		case eNetinf_ParsingMethod_Online:
		{
			strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Time);
			if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath,"Status",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
			{
				return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
			}

			// Device is missing an NTP server
			if (strcmp(Value, "Synchronized") == 0)
			{
				// Get the CurrentLocalTime from the Device
				
				strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Time);
				if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath,"CurrentLocalTime",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
				{
					return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
				}
			}
			else
			{
				// Get the CurrentLocalTime from the PC

				time_t				rawtime;
				struct tm*			timeinfo;
#ifdef _WIN32
				SYSTEMTIME st;
#endif

				time(&rawtime);
				timeinfo = localtime (&rawtime);

				// Example from Golan device - "Wed, 27 Feb 2013 13:16:35 GMT"
				strftime (Value,80,"%a, %d %b %Y %H:%M:%S",timeinfo);

#ifdef _WIN32
				if (bOutputDateTimeWithMilliseconds == TRUE)
				{
					GetSystemTime(&st);
					sprintf(&Value[strlen(Value)], ".%03d", st.wMilliseconds);
				}
#endif
			}
		} break;

		case eNetinf_ParsingMethod_XML_Files:
		{
			strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, "PeriodicCounterManadatoryInfo");
			if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath, "TimeStamp", &Value[0]) != eXMLPARSER_STAT_SUCCESS)
			{
				return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
			}
		} break;

		case eNetinf_ParsingMethod_XML_Files_HD_RemoteMonitoring:
		{
			if (XMLParser_GetNodeAttributeByName(XMLHandler, SubTree_SampleSet, "ReportEndTime", &Value[0]) != eXMLPARSER_STAT_SUCCESS)
			{
				XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
				return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
			}
		} break;

		case eNetinf_ParsingMethod_XML_Files_BPL:
		{
			// Use the timestamp from the filename itself (DataModel_20161122_161003.xml)
			struct tm			timeinfo;
			char*				ptr;

			ptr = strstr(XMLFileName, "DataModel_");

			if (ptr == NULL)
			{
				return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
			}

			strcpy(Value,ptr+sizeof("DataModel"));

			memset(&timeinfo, 0x00, sizeof(struct tm));

			// "20161122_161003.xml"
			sscanf(Value, "%4d%2d%2d_%2d%2d%2d",
								&timeinfo.tm_year,
								&timeinfo.tm_mon,
								&timeinfo.tm_mday,
								&timeinfo.tm_hour,
								&timeinfo.tm_min,
								&timeinfo.tm_sec);

			timeinfo.tm_year = timeinfo.tm_year - 1900;		// years since 1900
			timeinfo.tm_mon = timeinfo.tm_mon - 1;			// months since January - [0,11]

			// call mktime: timeinfo->tm_wday will be set
			mktime(&timeinfo);

			// Example from Golan device - "Wed, 27 Feb 2013 13:16:35 GMT"
			strftime (Value,80,"%a, %d %b %Y %H:%M:%S GMT",&timeinfo);
		} break;
	}

	// In case the TimeStamp is 1970 (Missing NTP), use the timestamp of the periodic file
	if ((Netinf_ParsingMethod == eNetinf_ParsingMethod_XML_Files) ||
		(Netinf_ParsingMethod == eNetinf_ParsingMethod_XML_Files_HD_RemoteMonitoring))
	{
		if (strstr(Value, "1970") != NULL)
		{
			time_t mtime;

			// Get the CurrentLocalTime from the file system
			if (file_get_Modified_Time(XMLFileName, &mtime) == TRUE)
			{
				struct tm*			timeinfo;

				timeinfo = gmtime (&mtime);

				// Example from Golan device - "Wed, 27 Feb 2013 13:16:35 GMT"
				strftime (Value,80,"%a, %d %b %Y %H:%M:%S GMT",timeinfo);
			}
			else
			{
				XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
				return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
			}
		}
	}

	strcpy_Without_Comma(Counters.TimeStamp, Value);
	Counters.Clock = CreateTimeT_From_TimeStamp(Counters.TimeStamp);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-



	if ((Netinf_ParsingMethod == eNetinf_ParsingMethod_Online) ||
		(Netinf_ParsingMethod == eNetinf_ParsingMethod_XML_Files) |
		(Netinf_ParsingMethod == eNetinf_ParsingMethod_XML_Files_BPL))
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the PacketsReceived
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface_Stats);
		if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath,"PacketsReceived",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.PacketsReceived = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the X_00C5D9_UpTimeMs
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_DeviceInfo);
		if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath,"X_00C5D9_UpTimeMs",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.X_00C5D9_UpTimeMs = strtoll(Value, NULL, 10);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the DiscardPacketsSent (PacketsDropped)
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface_Stats);
		if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath,"DiscardPacketsSent",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.DiscardPacketsSent = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the ReceivedFrames
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface_X_00C5D9_Stats);
		if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath,"ReceivedFrames",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.ReceivedFrames = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the ReceivedHCSErrors
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface_Stats);
		if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath, "ReceivedHCSErrors",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.ReceivedHCSErrors = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the FalseAlarms
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface_X_00C5D9_Stats);
		if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath,"FalseAlarms",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.FalseAlarms = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the BitAllocationTableSize
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface);
		if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath, "BitAllocationTableSize",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.BitAllocationTableSize = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the LastChange
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface);
		if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath,"LastChange",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.LastChange = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the GhnDeviceID (Interface)
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface);
		if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath,"GhnDeviceID",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.GhnDeviceID_Interface = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}
	else // eNetinf_ParsingMethod_XML_Files_HD_RemoteMonitoring
	{
		XMLParser_DuplicateSubTree(XMLHandler, SubTree_SampleSet, &SubTree_SampleSet_Stats);

		//XMLParser_GetNodeAttributeByIndex(XMLHandler, SubTree_SampleSet_Stats, 1, Name, Value);

		if (XMLParser_FindNode(XMLHandler,SubTree_SampleSet_Stats,"Stats") != eXMLPARSER_STAT_SUCCESS)
		{
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet_Stats);
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the PacketsReceived
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_SampleSet_Stats,"PacketsReceived",Value) != eXMLPARSER_STAT_SUCCESS)
		{
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet_Stats);
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.PacketsReceived = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the UpTimeMs
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_SampleSet,"UpTimeMs",Value) != eXMLPARSER_STAT_SUCCESS)
		{
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet_Stats);
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.X_00C5D9_UpTimeMs = strtoll(Value, NULL, 10);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the DiscardPacketsSent (PacketsDropped)
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_SampleSet_Stats,"DiscardPacketsSent",Value) != eXMLPARSER_STAT_SUCCESS)
		{
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet_Stats);
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.DiscardPacketsSent = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the ReceivedFrames
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_SampleSet_Stats,"ReceivedFrames",Value) != eXMLPARSER_STAT_SUCCESS)
		{
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet_Stats);
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.ReceivedFrames = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the ReceivedHCSErrors
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_SampleSet_Stats,"ReceivedHCSErrors",Value) != eXMLPARSER_STAT_SUCCESS)
		{
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet_Stats);
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.ReceivedHCSErrors = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the FalseAlarms
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_SampleSet_Stats,"FalseAlarms",Value) != eXMLPARSER_STAT_SUCCESS)
		{
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet_Stats);
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.FalseAlarms = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the BitAllocationTableSize
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface);
		if (XMLParser_GetNodeAttributeByNodePath(XMLHandler, NodePath, "BitAllocationTableSize",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		Counters.BitAllocationTableSize = atoi(Value);
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the LastChange (Support also old FW which doesn't have this field)
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_SampleSet,"LastChange",Value) != eXMLPARSER_STAT_SUCCESS)
		{
			Counters.LastChange = 0;
		}
		else
		{
			Counters.LastChange = atoi(Value);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get the GhnDeviceID (Interface) (Support also old FW which doesn't have this field)
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_SampleSet,"GhnDeviceID",Value) != eXMLPARSER_STAT_SUCCESS)
		{
			Counters.GhnDeviceID_Interface = 0;
		}
		else
		{
			Counters.GhnDeviceID_Interface = atoi(Value);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet_Stats);
	}

	if ((Netinf_ParsingMethod == eNetinf_ParsingMethod_Online) ||
		(Netinf_ParsingMethod == eNetinf_ParsingMethod_XML_Files) ||
		(Netinf_ParsingMethod == eNetinf_ParsingMethod_XML_Files_BPL))
	{
		// Create a SubTree for searching all AssociatedDevice under "Node_Interface"
		strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface);
		if (XMLParser_CreateSubTree(XMLHandler, NodePath, &SubTree) != eXMLPARSER_STAT_SUCCESS)
		{
			return eGHN_LIB_STAT_FAILURE;
		}
	}
	else // eNetinf_ParsingMethod_XML_Files_HD_RemoteMonitoring
	{
		// Create a SubTree for searching all AssociatedDevice under "SampleSet"

		if (XMLParser_DuplicateSubTree(XMLHandler, SubTree_SampleSet, &SubTree) != eXMLPARSER_STAT_SUCCESS)
		{
			XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
			return eGHN_LIB_STAT_FAILURE;
		}
	}

	RetVal = Netinf_ReadCEParameters_AssociatedDevice(XMLHandler, SubTree, &Counters, Netinf_Test);

	if (Netinf_ParsingMethod == eNetinf_ParsingMethod_XML_Files_HD_RemoteMonitoring)
	{
		XMLParser_FreeSubTree(XMLHandler, &SubTree_SampleSet);
	}

	XMLParser_FreeSubTree(XMLHandler, &SubTree);

	if (RetVal == eGHN_LIB_STAT_SUCCESS)
	{
		// Backup the previous counters and copy the current ones
		memcpy(&Netinf_Test->counters_prev,&Netinf_Test->counters_curr,sizeof(sNetinf));
		memcpy(&Netinf_Test->counters_curr,&Counters,sizeof(sNetinf));
	}

	return RetVal;
}

/***************************************************************************************************
* Netinf_ReadCEParameters_BitLoading()                                                             *
*                                                                                                  *
* 1. Read the BitLoading counters                                                                  *
***************************************************************************************************/
eGHN_LIB_STAT Netinf_ReadCEParameters_BitLoading(XMLParserHandler XMLHandler,sNetinf_Test* Netinf_Test)
{
	sNetinf*			Counters = &Netinf_Test->counters_curr;

	mac_address_t		StaTxMAC;

	XMLParserSubTree	SubTree;
	XMLParserSubTree	SubTree_CEParameters;
	XMLParserSubTree	SubTree_CEParameters_BitLoadingTable;

	char				Value[XML_PARSER_MAX_STRING];
	UINT32				Intervals;

	bool				bHaveBitLoadingTable;


	LOG_INFO("Started...");

	LOG_INFO("Netinf_Test->StaRxMAC ("MAC_ADDR_FMT")", MAC_ADDR(Netinf_Test->StaRxMAC));
	LOG_INFO("Netinf_Test->StaTxMAC ("MAC_ADDR_FMT")", MAC_ADDR(Netinf_Test->StaTxMAC));

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Create a SubTree for searching all AssociatedDevice under "Node_Interface"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (XMLParser_CreateSubTree(XMLHandler, Node_Interface, &SubTree) != eXMLPARSER_STAT_SUCCESS)
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

		if (memcmp(Netinf_Test->StaTxMAC,StaTxMAC,sizeof(mac_address_t)) != 0)
		{
			continue;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		XMLParser_DuplicateSubTree(XMLHandler, SubTree, &SubTree_CEParameters);

		Intervals=0;

		while (XMLParser_FindNode(XMLHandler, SubTree_CEParameters, "IntervalCEParameters") == eXMLPARSER_STAT_SUCCESS)
		{
			sNetinf_IntervalCEParameters*	CE;
			int								GroupID_Decimated;
			int								i;

			bHaveBitLoadingTable = TRUE;
			XMLParser_DuplicateSubTree(XMLHandler, SubTree_CEParameters, &SubTree_CEParameters_BitLoadingTable);

			if (XMLParser_FindNode(XMLHandler, SubTree_CEParameters_BitLoadingTable, "BitLoadTable") != eXMLPARSER_STAT_SUCCESS)
			{
				bHaveBitLoadingTable = FALSE;
			}

			Intervals++;

			// Sanity Check
			if (Intervals > PHY_MAX_INTERVALS)
			{
				XMLParser_FreeSubTree(XMLHandler, &SubTree_CEParameters);
				XMLParser_FreeSubTree(XMLHandler, &SubTree);
				return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
			}

			CE = &Counters->IntervalCEParameters[Intervals-1];

			GroupID_Decimated = (int)pow(2,(double)(CE->GroupID));

			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// BitAllocationTable
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			memset(&CE->BitAllocationTable[0],0x00,sizeof(CE->BitAllocationTable));
			if (bHaveBitLoadingTable)
			{
				if (XMLParser_GetNodeAttributeByName(XMLHandler, SubTree_CEParameters_BitLoadingTable,"BitAllocationTable",&Value[0]) == eXMLPARSER_STAT_SUCCESS)
				{
					char*	ptr;
					int		Index=0;

					ptr = strtok(Value,",");
					while (ptr != NULL)
					{
						for(i=1;i<=GroupID_Decimated;i++)
						{
							CE->BitAllocationTable[Index++] = atoi(ptr);
						}
						ptr = strtok(NULL, ",");
					}
				}
			}
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

			XMLParser_FreeSubTree(XMLHandler, &SubTree_CEParameters_BitLoadingTable);
		}

		XMLParser_FreeSubTree(XMLHandler, &SubTree_CEParameters);
		XMLParser_FreeSubTree(XMLHandler, &SubTree);

		// Sanity Check
		if (Intervals != Counters->IntervalCEParametersNumberOfEntries)
		{
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}

		return eGHN_LIB_STAT_SUCCESS;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// TX-MAC not found
	XMLParser_FreeSubTree(XMLHandler, &SubTree);

	return eGHN_LIB_STAT_FAILURE;
}

eGHN_LIB_STAT Netinf_CalcCEParameters(sNetinf_Test* Netinf_Test,sNetinf_Result* Netinf_Result, bool FirstIteration)
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

	char		strCodeRateMultipleFactor[6][6] =	{	"0",						// Index 0
														"1/2",			// Index 1
														"2/3",			// Index 2
														"5/6",			// Index 3
														"16/18",			// Index 4
														"20/21",	 		// Index 5
													};

	sNetinf*		Counters_curr;
	sNetinf*		Counters_prev;
	UINT32			Interval;
	double			ElapsedTime;							// In seconds. Usage for normalize to 1 second
	double			ElapsedTime_According_To_TimeStamp;		// In seconds

	UINT32			TotalBLER_CRCSegments;
	UINT32			TotalBLER_TotalSegments;

	UINT32			Total_RxTotalSegments;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Inits
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	memset(Netinf_Result, 0x00, sizeof(sNetinf_Result));

	Counters_curr = &Netinf_Test->counters_curr;
	Counters_prev = &Netinf_Test->counters_prev;

	strcpy(Netinf_Result->TimeStamp,Counters_curr->TimeStamp);

	Netinf_Result->NumberOfInterval = Counters_curr->IntervalCEParametersNumberOfEntries;

	// Calc Delta Time
	ElapsedTime = Counters_curr->X_00C5D9_UpTimeMs - Counters_prev->X_00C5D9_UpTimeMs;
	ElapsedTime = ElapsedTime / 1000;		// Convert from Milliseconds to seconds

	ElapsedTime_According_To_TimeStamp = difftime(Counters_curr->Clock, Counters_prev->Clock); // In seconds

	/*
	sprintf(Buffer, "Counters_curr->X_00C5D9_UpTimeMs=%d, Counters_prev->X_00C5D9_UpTimeMs=%d, ElapsedTime:%f\n",
							Counters_curr->X_00C5D9_UpTimeMs,
							Counters_prev->X_00C5D9_UpTimeMs,
							ElapsedTime);
	OutputDebugStringA(Buffer);
	*/
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Sanity checks
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Not the first time & Number of entries has changed
	if ((Netinf_Test->counters_prev.IntervalCEParametersNumberOfEntries != 0) &&
		(Netinf_Test->counters_curr.IntervalCEParametersNumberOfEntries !=
		 Netinf_Test->counters_prev.IntervalCEParametersNumberOfEntries))
	{
		return eGHN_LIB_STAT_FAILURE;
	}

	for (Interval=0 ; Interval< Netinf_Result->NumberOfInterval ; Interval++)
	{
		sNetinf_IntervalCEParameters* CE = &Counters_curr->IntervalCEParameters[Interval];

		if (CE->TotalBitLoading > 12*Counters_curr->BitAllocationTableSize)  // 12 Bits per tone
		{
			return eGHN_LIB_STAT_FAILED_PARSE_NETINF_RESULT;
		}

		if (CE->CodeRate>5)
		{
			return eGHN_LIB_STAT_FAILED_PARSE_NETINF_RESULT;
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// Check that the delta of "UpTime" and "TimeStamp" is smaller than 60 seconds
	if (abs((int)ElapsedTime_According_To_TimeStamp - (int)ElapsedTime) > 60)
	{
		return eGHN_LIB_STAT_FAILURE;
	}

	// Check if the device joined to another network (another DM)
	if (Counters_curr->LastChange < Counters_prev->LastChange)
	{
		return eGHN_LIB_STAT_FAILURE;
	}

	// Check if the device rejoin and got another GhnDeviceID
	if (Counters_curr->GhnDeviceID_Interface != Counters_prev->GhnDeviceID_Interface)
	{
		return eGHN_LIB_STAT_FAILURE;
	}

	// Check if the device joined to another network (another DM)
	if (Counters_prev->GhnDeviceID != Counters_curr->GhnDeviceID)
	{
		return eGHN_LIB_STAT_FAILURE;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// CodeRateMultipleFactor
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	for (Interval=0 ; Interval< Netinf_Result->NumberOfInterval ; Interval++)
	{
		sNetinf_IntervalCEParameters* CE = &Counters_curr->IntervalCEParameters[Interval];

		strcpy(&Netinf_Result->strCodeRateMultipleFactor[Interval][0], strCodeRateMultipleFactor[CE->CodeRate%6]);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// PHY Rate
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	Netinf_Result->PHY = 0;

	for (Interval=0 ; Interval< Netinf_Result->NumberOfInterval ; Interval++)
	{
		sNetinf_IntervalCEParameters* CE = &Counters_curr->IntervalCEParameters[Interval];

		Netinf_Result->PHY += (CE->TotalBitLoading * CodeRateMultipleFactor[CE->CodeRate%6] / TOTAL_SYMBOL_DURATION_IN_MICRO_SECONDS);
	}

	Netinf_Result->PHY /= Netinf_Result->NumberOfInterval;
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// BLER
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	TotalBLER_CRCSegments = 0;
	TotalBLER_TotalSegments = 0;

	for (Interval=0 ; Interval< Netinf_Result->NumberOfInterval ; Interval++)
	{
		sNetinf_IntervalCEParameters* CE_prev = &Counters_prev->IntervalCEParameters[Interval];
		sNetinf_IntervalCEParameters* CE = &Counters_curr->IntervalCEParameters[Interval];

		// No Delta
		if ((CE->RxTotalSegments <= CE_prev->RxTotalSegments) ||
			(CE->RxCRCSegments <= CE_prev->RxCRCSegments))
		{
			Netinf_Result->BLER[Interval] = 0;
			continue;
		}
		else
		{
			Netinf_Result->BLER[Interval] =	(double)(CE->RxCRCSegments - CE_prev->RxCRCSegments) /
											(double)(CE->RxTotalSegments - CE_prev->RxTotalSegments);

			// BLER is higher than 15%
			if (Netinf_Result->BLER[Interval] >= 0.15) 
			{
				UINT32 EstimatedTXSegments = CE->RxTotalSegments - CE_prev->RxTotalSegments;

				// EstimatedTXSegments is smaller than the threshold
				if ((((double)EstimatedTXSegments)/ElapsedTime) <= Netinf_Test->ThresholdNumberOfSegments)
				{
					Netinf_Result->BLER[Interval] = 0;
					continue;
				}
			}

			// TotalBLER
			TotalBLER_CRCSegments += (CE->RxCRCSegments - CE_prev->RxCRCSegments);
			TotalBLER_TotalSegments += (CE->RxTotalSegments - CE_prev->RxTotalSegments);
		}

	}
	if (TotalBLER_TotalSegments == 0)
	{
		Netinf_Result->TotalBLER = 0;
	}
	else
	{
		Netinf_Result->TotalBLER = (double)(TotalBLER_CRCSegments) / (double)(TotalBLER_TotalSegments);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Effective PHY Rate
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	Netinf_Result->EffectivePHY = 0;

	for (Interval=0 ; Interval< Netinf_Result->NumberOfInterval ; Interval++)
	{
		sNetinf_IntervalCEParameters* CE = &Counters_curr->IntervalCEParameters[Interval];

		Netinf_Result->EffectivePHY += (CE->TotalBitLoading * CodeRateMultipleFactor[CE->CodeRate%6]
												/ TOTAL_SYMBOL_DURATION_IN_MICRO_SECONDS) * 
												(1 - Netinf_Result->BLER[Interval]);
	}

	Netinf_Result->EffectivePHY /= Netinf_Result->NumberOfInterval;
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Information Bits Per Symbol (IBPS)
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	for (Interval=0 ; Interval< Netinf_Result->NumberOfInterval ; Interval++)
	{
		sNetinf_IntervalCEParameters* CE = &Counters_curr->IntervalCEParameters[Interval];

		Netinf_Result->BitLoading[Interval] = (UINT32)(CE->TotalBitLoading * CodeRateMultipleFactor[CE->CodeRate%6]);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// SNR
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	for (Interval=0 ; Interval< Netinf_Result->NumberOfInterval ; Interval++)
	{
		sNetinf_IntervalCEParameters* CE = &Counters_curr->IntervalCEParameters[Interval];

		Netinf_Result->SNR[Interval] = (SINT32)(CE->AverageSNR);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Bytes Transmitted
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	{
		if (Counters_curr->BytesReceived == Counters_prev->BytesReceived)
		{
			Netinf_Result->TotalEstimatedTXBytes = 0;
		}
		else
		{
			if (Counters_curr->BytesReceived >= Counters_prev->BytesReceived)
			{
				// No Overflow
				Netinf_Result->TotalEstimatedTXBytes = Counters_curr->BytesReceived - Counters_prev->BytesReceived;
			}
			else
			{
				// Overflow
				if (Counters_prev->BytesReceived >= 0xFFFFFFFF)
				{
					// 64Bit WrapAround
					Netinf_Result->TotalEstimatedTXBytes = 0xFFFFFFFFFFFFFFFF - Counters_prev->BytesReceived + Counters_curr->BytesReceived;
				}
				else
				{
					// 32Bit WrapAround
					Netinf_Result->TotalEstimatedTXBytes = 0xFFFFFFFF - Counters_prev->BytesReceived + Counters_curr->BytesReceived;
				}
			}

			// Normalize to 1 second
			Netinf_Result->TotalEstimatedTXBytes = (ULONG64)(Netinf_Result->TotalEstimatedTXBytes / ElapsedTime);
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Summarize all the segments from all the intervals
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		Total_RxTotalSegments = 0;
		for (Interval=0 ; Interval< Netinf_Result->NumberOfInterval ; Interval++)
		{
			sNetinf_IntervalCEParameters* CE_prev = &Counters_prev->IntervalCEParameters[Interval];
			sNetinf_IntervalCEParameters* CE = &Counters_curr->IntervalCEParameters[Interval];

			if (CE->RxTotalSegments > CE_prev->RxTotalSegments)
			{
				Total_RxTotalSegments = Total_RxTotalSegments + (CE->RxTotalSegments - CE_prev->RxTotalSegments);
			}
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		for (Interval=0 ; Interval< Netinf_Result->NumberOfInterval ; Interval++)
		{
			sNetinf_IntervalCEParameters* CE_prev = &Counters_prev->IntervalCEParameters[Interval];
			sNetinf_IntervalCEParameters* CE = &Counters_curr->IntervalCEParameters[Interval];

			if (CE->RxTotalSegments <= CE_prev->RxTotalSegments)
			{
				Netinf_Result->EstimatedTXBytes[Interval] = 0;
			}
			else
			{
				// Calc the relative-part from the "Netinf_Result->TotalEstimatedTXBytes"
				Netinf_Result->EstimatedTXBytes[Interval] = Netinf_Result->TotalEstimatedTXBytes *
														((double)(CE->RxTotalSegments - CE_prev->RxTotalSegments) / Total_RxTotalSegments);
			}
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// ReceivedFrames / HeaderCheckSum / FalseAlarms
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	Netinf_Result->ReceivedFrames = ((double)Counters_curr->ReceivedFrames - (double)Counters_prev->ReceivedFrames) / ElapsedTime;
	Netinf_Result->ReceivedHCSErrors = ((double)Counters_curr->ReceivedHCSErrors - (double)Counters_prev->ReceivedHCSErrors) / ElapsedTime;
	Netinf_Result->FalseAlarms = ((double)Counters_curr->FalseAlarms - (double)Counters_prev->FalseAlarms) / ElapsedTime;
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// PER
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	
	if (((double)Counters_curr->PacketsReceived - (double)Counters_prev->PacketsReceived) == 0)
	{
		Netinf_Result->PER = 0;
	}
	else
	{
		Netinf_Result->PER = ((double)Counters_curr->DiscardPacketsSent - (double)Counters_prev->DiscardPacketsSent) /
							 ((double)Counters_curr->PacketsReceived - (double)Counters_prev->PacketsReceived) * 100;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// RX-Power
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if ((Netinf_Test->NodeTypeConfiguration == eNetinf_NodeTypeConfiguration_MIMO) ||
		(Netinf_Test->NodeTypeConfiguration == eNetinf_NodeTypeConfiguration_SISO))
	{
		Netinf_Result->RXPower0 = Counters_curr->RxEnergyChannel_0;
		Netinf_Result->RXPower1 = Counters_curr->RxEnergyChannel_1;
	}
 	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// BitAllocationTable
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	Netinf_Result->BitAllocationTableSize = Counters_curr->BitAllocationTableSize;

	for (Interval=0 ; Interval< Netinf_Result->NumberOfInterval ; Interval++)
	{
		sNetinf_IntervalCEParameters* CE = &Counters_curr->IntervalCEParameters[Interval];

		memcpy(&Netinf_Result->BitAllocationTable[Interval][0], &CE->BitAllocationTable[0], sizeof(CE->BitAllocationTable));
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	return eGHN_LIB_STAT_SUCCESS;
}


/***************************************************************************************************
* Netinf_PrintCEParameters()                                                                       *
*                                                                                                  *
* Print the current counters                                                                       *
***************************************************************************************************/
eGHN_LIB_STAT Netinf_PrintCEParameters(sNetinf_Test* Netinf_Test)
{
	sNetinf*			Counters = &Netinf_Test->counters_curr;
	UINT32				Intervals;

	LOG_INFO("Started...");

	LOG_INFO("StaRxMAC                            ("MAC_ADDR_FMT")", MAC_ADDR(Netinf_Test->StaRxMAC));
	LOG_INFO("StaTxMAC                            ("MAC_ADDR_FMT")", MAC_ADDR(Netinf_Test->StaTxMAC));

	LOG_INFO("ReceivedFrames                      (%d)",	Counters->ReceivedFrames);
	LOG_INFO("ReceivedHCSErrors                   (%d)",	Counters->ReceivedHCSErrors);
	LOG_INFO("FalseAlarms                         (%d)",	Counters->FalseAlarms);
	LOG_INFO("PacketsReceived                     (%d)",	Counters->PacketsReceived);
	LOG_INFO("DiscardPacketsSent                  (%d)",	Counters->DiscardPacketsSent);

	LOG_INFO("RxPowerChannel_0                    (%d)",	Counters->RxPowerChannel_0);
	LOG_INFO("RxPowerChannel_1                    (%d)",	Counters->RxPowerChannel_1);

	LOG_INFO("RxEnergyChannel_0                   (%d)",	Counters->RxEnergyChannel_0);
	LOG_INFO("RxEnergyChannel_1                   (%d)",	Counters->RxEnergyChannel_1);

	LOG_INFO("BytesReceived                       (%d)",	Counters->BytesReceived);

	LOG_INFO("IntervalCEParametersNumberOfEntries (%d)",	Counters->IntervalCEParametersNumberOfEntries);

	for (Intervals=1;Intervals<=Counters->IntervalCEParametersNumberOfEntries;Intervals++)
	{
		sNetinf_IntervalCEParameters* CE;

		CE = &Counters->IntervalCEParameters[Intervals-1];
		LOG_INFO("Interval(%d) TotalBitLoading         (%d)", Intervals, CE->TotalBitLoading);
		LOG_INFO("Interval(%d) CodeRate                (%d)", Intervals, CE->CodeRate);
		LOG_INFO("Interval(%d) RxTotalSegments         (%d)", Intervals, CE->RxTotalSegments);
		LOG_INFO("Interval(%d) RxCRCSegments           (%d)", Intervals, CE->RxCRCSegments);
		LOG_INFO("Interval(%d) AverageSNR              (%d)", Intervals, CE->AverageSNR);
		LOG_INFO("Interval(%d) GroupID                 (%d)", Intervals, CE->GroupID);

	}

	return eGHN_LIB_STAT_SUCCESS;
}

/***************************************************************************************************
* Netinf_PrintCalcCEParameters()                                                                   *
*                                                                                                  *
* Print the PHY graph details                                                                      *
***************************************************************************************************/
eGHN_LIB_STAT Netinf_PrintCalcCEParameters(sNetinf_Result* Netinf_Result)
{
	UINT32				Interval;

	LOG_INFO("Started...");

	LOG_INFO("NumberOfInterval                    (%d)",	Netinf_Result->NumberOfInterval);
	LOG_INFO("ReceivedFrames                      (%.2f)",	Netinf_Result->ReceivedFrames);
	LOG_INFO("HeaderCheckSum                      (%.2f)",	Netinf_Result->ReceivedHCSErrors);
	LOG_INFO("FalseAlarms                         (%.2f)",	Netinf_Result->FalseAlarms);
	LOG_INFO("PER                                 (%.2f)",	Netinf_Result->PER);

	LOG_INFO("RXPower0                            (%.2f)",	Netinf_Result->RXPower0);
	LOG_INFO("RXPower1                            (%.2f)",	Netinf_Result->RXPower1);
	LOG_INFO("PHY                                 (%.2f)",	Netinf_Result->PHY);
	LOG_INFO("EffectivePHY                        (%.2f)",	Netinf_Result->EffectivePHY);

	for (Interval=1; Interval<= Netinf_Result->NumberOfInterval ; Interval++)
	{
		LOG_INFO("EstimatedTXBytes                    (%d)",	Netinf_Result->EstimatedTXBytes[Interval-1]);
	}
	LOG_INFO("TotalEstimatedTXBytes               (%d)",	Netinf_Result->TotalEstimatedTXBytes);

	for (Interval=1; Interval<= Netinf_Result->NumberOfInterval ; Interval++)
	{
		LOG_INFO("Interval(%d) BitLoading             (%d)", Interval, Netinf_Result->BitLoading[Interval-1]);
	}

	for (Interval=1; Interval<= Netinf_Result->NumberOfInterval ; Interval++)
	{
		LOG_INFO("Interval(%d) BLER                   (%.4f)", Interval, Netinf_Result->BLER[Interval-1]);
	}

	LOG_INFO("TotalBLER                           (%.4f)", Netinf_Result->TotalBLER);
	 
	for (Interval=1; Interval<= Netinf_Result->NumberOfInterval ; Interval++)
	{
		LOG_INFO("Interval(%d) SNR                    (%d)", Interval, Netinf_Result->SNR[Interval-1]);
	}

	return eGHN_LIB_STAT_SUCCESS;
}

/***************************************************************************************************
* Netinf_SaveCalcCEParameters()                                                                    *
*                                                                                                  *
* Save the PHY graph details                                                                       *
***************************************************************************************************/
eGHN_LIB_STAT Netinf_SaveCalcCEParameters(sNetinf_Result* Netinf_Result, char* ReceiverTransmitterOutputFolder, bool bGenerateTraffic, bool bSavePERGraph, bool bSaveAdvancedGraphs, bool FirstIteration, bool LastIteration)
{
	FILE*				fp;
	char				FileName[OS_MAX_PATH];

	LOG_INFO("Started...");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// NumberOfInterval
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	sprintf(FileName,"%s/NumberOfInterval.txt",ReceiverTransmitterOutputFolder);
	fp = FOPEN(FileName,"a");
	if (fp != NULL)
	{
		FPRINTF(fp, "%s, %d\n",Netinf_Result->TimeStamp, Netinf_Result->NumberOfInterval);
		FCLOSE(fp);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// ReceivedFrames
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/ReceivedFrames.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			FPRINTF(fp, "%s, %5.2f\n",Netinf_Result->TimeStamp, Netinf_Result->ReceivedFrames);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// ReceivedHCSErrors & FalseAlarms
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/ReceivedHCSErrors_and_FalseAlarms.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			FPRINTF(fp, "%s, %5.2f\n",Netinf_Result->TimeStamp, Netinf_Result->ReceivedHCSErrors+
																Netinf_Result->FalseAlarms);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// ReceivedHCSErrors
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/ReceivedHCSErrors.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			FPRINTF(fp, "%s, %5.2f\n",Netinf_Result->TimeStamp, Netinf_Result->ReceivedHCSErrors);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// FalseAlarms
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/FalseAlarms.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			FPRINTF(fp, "%s, %5.2f\n",Netinf_Result->TimeStamp, Netinf_Result->FalseAlarms);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if ((bSavePERGraph) || (bSaveAdvancedGraphs))
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// PER (PacketsDropped)
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/PER.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			FPRINTF(fp, "%s, %8.2f\n",Netinf_Result->TimeStamp, Netinf_Result->PER);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// RXPower 0
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/RXPower_0.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			FPRINTF(fp, "%s, %5.2f\n",Netinf_Result->TimeStamp, Netinf_Result->RXPower0);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// RXPower 1
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/RXPower_1.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			FPRINTF(fp, "%s, %5.2f\n",Netinf_Result->TimeStamp, Netinf_Result->RXPower1);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// PHY
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	sprintf(FileName,"%s/PHY.txt",ReceiverTransmitterOutputFolder);
	fp = FOPEN(FileName,"a");
	if (fp != NULL)
	{
		if (FirstIteration)
		{
			FPRINTF(fp, "%s, --- Start Sampling ---\n",Netinf_Result->TimeStamp);
		}
		FPRINTF(fp, "%s, %6.2f\n",Netinf_Result->TimeStamp, Netinf_Result->PHY);
		FCLOSE(fp);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Effective PHY
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/EffectivePHY.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			if (FirstIteration)
			{
				FPRINTF(fp, "%s, --- Start Sampling ---\n",Netinf_Result->TimeStamp);
			}
			FPRINTF(fp, "%s, %6.2f\n",Netinf_Result->TimeStamp, Netinf_Result->EffectivePHY);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// EstimatedTXBytes
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/EstimatedTXBytes.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			if (FirstIteration)
			{
				FPRINTF(fp, "%s, --- Start Sampling ---\n", Netinf_Result->TimeStamp);
			}
			FPRINTF(fp, "%s, %15llu, %15llu, %15llu, %15llu, %15llu, %15llu, %15llu\n",
								Netinf_Result->TimeStamp,
								Netinf_Result->EstimatedTXBytes[0],
								Netinf_Result->EstimatedTXBytes[1],
								Netinf_Result->EstimatedTXBytes[2],
								Netinf_Result->EstimatedTXBytes[3],
								Netinf_Result->EstimatedTXBytes[4],
								Netinf_Result->EstimatedTXBytes[5],
								Netinf_Result->TotalEstimatedTXBytes);

			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if ((bSaveAdvancedGraphs == FALSE) && (bGenerateTraffic == FALSE))
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// UserThroughput
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/UserThroughput.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			if (FirstIteration)
			{
				FPRINTF(fp, "%s, --- Start Sampling ---\n", Netinf_Result->TimeStamp);
			}
			FPRINTF(fp, "%s, %6.2f\n",
				Netinf_Result->TimeStamp,
				((float)Netinf_Result->TotalEstimatedTXBytes) * 8 / (1000*1000)); // Normalize from Bytes to Mbits

			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// BitLoading
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/BitLoading.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			if (FirstIteration)
			{
				FPRINTF(fp, "%s, --- Start Sampling ---\n", Netinf_Result->TimeStamp);
			}
			FPRINTF(fp, "%s, %5d, %5d, %5d, %5d, %5d, %5d\n",
								Netinf_Result->TimeStamp,
								Netinf_Result->BitLoading[0],
								Netinf_Result->BitLoading[1],
								Netinf_Result->BitLoading[2],
								Netinf_Result->BitLoading[3],
								Netinf_Result->BitLoading[4],
								Netinf_Result->BitLoading[5]);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// SNR
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/SNR.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			if (FirstIteration)
			{
				FPRINTF(fp, "%s, --- Start Sampling ---\n", Netinf_Result->TimeStamp);
			}
			FPRINTF(fp, "%s, %5d, %5d, %5d, %5d, %5d, %5d\n",
				Netinf_Result->TimeStamp,
				Netinf_Result->SNR[0],
				Netinf_Result->SNR[1],
				Netinf_Result->SNR[2],
				Netinf_Result->SNR[3],
				Netinf_Result->SNR[4],
				Netinf_Result->SNR[5]);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bSaveAdvancedGraphs == FALSE)
	{
		/*
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// SNR
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/SNR.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			if (FirstIteration)
			{
				FPRINTF(fp, "%s, --- Start Sampling ---\n", Netinf_Result->TimeStamp);
			}
			FPRINTF(fp, "%s, %6.2f\n",
				Netinf_Result->TimeStamp,
				((float)(Netinf_Result->SNR[0] + Netinf_Result->SNR[1] + Netinf_Result->SNR[2] +
				Netinf_Result->SNR[3] + Netinf_Result->SNR[4] + Netinf_Result->SNR[5]))/Netinf_Result->NumberOfInterval);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		*/
	}

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// BLER
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/BLER.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			if (FirstIteration)
			{
				FPRINTF(fp, "%s, --- Start Sampling ---\n", Netinf_Result->TimeStamp);
			}
			FPRINTF(fp, "%s, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n",
								Netinf_Result->TimeStamp,
								Netinf_Result->BLER[0],
								Netinf_Result->BLER[1],
								Netinf_Result->BLER[2],
								Netinf_Result->BLER[3],
								Netinf_Result->BLER[4],
								Netinf_Result->BLER[5],
								Netinf_Result->TotalBLER);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// BitAllocationTable
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if ((FirstIteration) || (LastIteration))
		{
			UINT32 i;

			sprintf(FileName,"%s/BitAllocationTable.txt",ReceiverTransmitterOutputFolder);
			fp = FOPEN(FileName,"w");
			if (fp != NULL)
			{
				for (i=0 ; i < Netinf_Result->BitAllocationTableSize; i++)
				{
					FPRINTF(fp, "%3d, %3d, %3d, %3d, %3d, %3d\n",
								Netinf_Result->BitAllocationTable[0][i],
								Netinf_Result->BitAllocationTable[1][i],
								Netinf_Result->BitAllocationTable[2][i],
								Netinf_Result->BitAllocationTable[3][i],
								Netinf_Result->BitAllocationTable[4][i],
								Netinf_Result->BitAllocationTable[5][i]);
				}

				FCLOSE(fp);
			}
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	if (bSaveAdvancedGraphs)
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// CodeRateMultipleFactor
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		sprintf(FileName,"%s/CodeRateMultipleFactor.txt",ReceiverTransmitterOutputFolder);
		fp = FOPEN(FileName,"a");
		if (fp != NULL)
		{
			if (FirstIteration)
			{
				FPRINTF(fp, "%s, --- Start Sampling ---\n", Netinf_Result->TimeStamp);
			}
			FPRINTF(fp, "%s, %5s, %5s, %5s, %5s, %5s, %5s\n",
								Netinf_Result->TimeStamp,
								&Netinf_Result->strCodeRateMultipleFactor[0][0],
								&Netinf_Result->strCodeRateMultipleFactor[1][0],
								&Netinf_Result->strCodeRateMultipleFactor[2][0],
								&Netinf_Result->strCodeRateMultipleFactor[3][0],
								&Netinf_Result->strCodeRateMultipleFactor[4][0],
								&Netinf_Result->strCodeRateMultipleFactor[5][0]);
			FCLOSE(fp);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	return eGHN_LIB_STAT_SUCCESS;
}

/***************************************************************************************************
* Netinf_Update_Status_File()                                                                      *
*                                                                                                  *
* Update the status file                                                                           *
***************************************************************************************************/
eGHN_LIB_STAT Netinf_Update_Status_File(char* ReceiverTransmitterOutputFolder, char* SampleStatus)
{
	FILE*				fp;
	char				FileName[OS_MAX_PATH];

	LOG_INFO("Started...");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// NumberOfInterval
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	sprintf(FileName,"%s/SampleStatus.txt",ReceiverTransmitterOutputFolder);
	fp = fopen(FileName,"w");
	if (fp != NULL)
	{
		fprintf(fp, "%s\n", SampleStatus);
		fclose(fp);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	return eGHN_LIB_STAT_SUCCESS;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
