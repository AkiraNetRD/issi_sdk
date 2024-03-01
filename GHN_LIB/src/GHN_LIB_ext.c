// This is the main DLL file.

#include "GHN_LIB_typedefs.h"
#include "GHN_LIB_int.h"
#include "GHN_LIB_int_consts.h"
#include "GHN_LIB_Layer2Connection.h"
#include "GHN_LIB_ext.h"
#include "GHN_LIB_netinf.h"
#include "GHN_LIB_LinkStatistics.h"
#include "GHN_LIB_Flash.h"
#include "console.h"
#include "common.h"
#include "math.h"								// ceilf()

#include "HTTP_LIB_ext.h"

#include "Image_LIB_ext.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

eGHN_LIB_STAT Ghn_Get_Status_Description(eGHN_LIB_STAT status, char* StatusDescription)
{
	switch (status)
	{
		case eGHN_LIB_STAT_SUCCESS:						strcpy(StatusDescription, "General success");						break;
		case eGHN_LIB_STAT_FAILURE:						strcpy(StatusDescription, "General failure");						break;
		case eGHN_LIB_STAT_INVALID_PARAMETER:			strcpy(StatusDescription, "Invalid parameter");						break;
		case eGHN_LIB_STAT_TIMEOUT:						strcpy(StatusDescription, "Timeout expired");						break;
		case eGHN_LIB_STAT_MISSING_FILE:				strcpy(StatusDescription, "Integral file is missing");				break;
		case eGHN_LIB_STAT_DEVICE_NOT_FOUND:			strcpy(StatusDescription, "Requested device not found");			break;
		case eGHN_LIB_STAT_NOT_SUPPORTED:				strcpy(StatusDescription, "Requested not supported");				break;
		case eGHN_LIB_STAT_TEST_WAS_INTERRUPT:			strcpy(StatusDescription, "Test was interrupt");					break;
		case eGHN_LIB_STAT_FAILED_TO_PARSE_XML:			strcpy(StatusDescription, "Failed to parse XML file");				break;
		case eGHN_LIB_STAT_HTTP_REQUEST_FAILED:			strcpy(StatusDescription, "HTTP request was failed");				break;

		case eGHN_LIB_STAT_FAILED_SWITCH_2_ETHERBOOT:	strcpy(StatusDescription, "Failed to switch to EtherBoot");			break;
		case eGHN_LIB_STAT_FAILED_SWITCH_2_FLASHBOOT:	strcpy(StatusDescription, "Failed to switch to FlashBoot");			break;
		case eGHN_LIB_STAT_FAILED_UPDATEFIRMWARE:		strcpy(StatusDescription, "Failed to update firmware");				break;
		case eGHN_LIB_STAT_FAILED_LOAD_MAIN_FIRMWARE:	strcpy(StatusDescription, "Failed to load main firmware");			break;
		case eGHN_LIB_STAT_FW_BOOT_CODE:				strcpy(StatusDescription, "Device is in BootCode mode");			break;
		case eGHN_LIB_STAT_FW_FIRMWARE:					strcpy(StatusDescription, "Device is in Firmware mode");			break;
		case eGHN_LIB_STAT_DISABLED_NVM:				strcpy(StatusDescription, "Device was power-up with Disabled-NVM");	break;
		case eGHN_LIB_STAT_OLD_EBL_VERSION:				strcpy(StatusDescription, "Device has an older EBL version");		break;

		case eGHN_LIB_STAT_FAILED_READ_FILE:			strcpy(StatusDescription, "Failed to read from a file");			break;
		case eGHN_LIB_STAT_FAILED_WRITE_FILE:			strcpy(StatusDescription, "Failed to write to a file");				break;
		case eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION:	strcpy(StatusDescription, "failed to allocate memory");				break;
		case eGHN_LIB_STAT_CHIP_TYPE_MISMATCH:			strcpy(StatusDescription, "Chip-Type mismatch");					break;

		case eGHN_LIB_STAT_FAILED_START_NETINF:			strcpy(StatusDescription, "Failed to start netinf test");			break;
		case eGHN_LIB_STAT_FAILED_STOP_NETINF:			strcpy(StatusDescription, "Failed to stop netinf test");			break;
		case eGHN_LIB_STAT_FAILED_PARSE_NETINF_RESULT:	strcpy(StatusDescription, "Failed to parse netinf result");			break;

		case eGHN_LIB_STAT_NETINF_ALREADY_RUNNING:		strcpy(StatusDescription, "Netinf is already running");				break;

		case eGHN_LIB_STAT_ADAPTERS_QUERY_FAILED:		strcpy(StatusDescription, "Adapter query/processing failed");		break;
		case eGHN_LIB_STAT_BIND_ADAPTER_FAILED:			strcpy(StatusDescription, "Failed to bind adapter");				break;
		case eGHN_LIB_STAT_SUBNET_MASK_MISMATCH:		strcpy(StatusDescription, "Subnet mask is incorrect");				break;
		case eGHN_LIB_STAT_IP_ADDRESS_INVALID:			strcpy(StatusDescription, "IP Address is invalid");					break;

		default:										strcpy(StatusDescription, "General failure");
	}

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Adapter_List(sAdapter_Information* adapterinformation)
{
	#define DEF_ARR_SIZE 64

	int		      nAdapterCount = 0;

	int size,idx; 
	AdapterInfo inf[DEF_ARR_SIZE];
	cg_stat_t   status;

	size = DEF_ARR_SIZE;

	status = NIC_get_adapters(inf,&size);
	if (status != CG_STAT_SUCCESS)
	{
		return ConvertReturnStatus(status);
	}

	for(idx=0;idx<size;idx++)
	{
		// Skip 127.0.0.1
		if (inf[idx].ip == 0x0100007F)
		{
			continue;
		}

		// Fill in IP and description
		sprintf(adapterinformation->Array[nAdapterCount].IP, "%u.%u.%u.%u",
				((BYTE*)&inf[idx].ip)[0],
				((BYTE*)&inf[idx].ip)[1],
				((BYTE*)&inf[idx].ip)[2],
				((BYTE*)&inf[idx].ip)[3]);

		strcpy(adapterinformation->Array[nAdapterCount].Description, inf[idx].name);

		nAdapterCount++;

	}
	
	adapterinformation->Size = nAdapterCount;

	return eGHN_LIB_STAT_SUCCESS;
}


bool Update_sAttribute_Info(XMLParserHandler Handler,char* NodePath,char* Name,sAttribute_Info* adapterInfo)
{
	char				Value[XML_PARSER_MAX_STRING] = "N/A";
	bool				FoundAttribute = FALSE;
	XMLParserSubTree	SubTree;

	// Search for the node
	if (XMLParser_CreateSubTree(Handler,NodePath,&SubTree) == eXMLPARSER_STAT_SUCCESS)
	{
		// Search for the Attribute Name
		if (XMLParser_GetNodeAttributeByName(Handler,SubTree,Name,Value) == eXMLPARSER_STAT_SUCCESS)
		{
			FoundAttribute = TRUE;
		}

		XMLParser_FreeSubTree(Handler,&SubTree);
	}

	// Copy Name & Value
	strcpy(adapterInfo->Name,Name);
	strcpy(adapterInfo->Value,Value);

	return FoundAttribute;
}

eGHN_LIB_STAT Ghn_Get_Network_Interface_By_Device_IP(ip_address_t xi_DeviceIP,ip_address_t* xo_NetworkCard)
{
#define DEF_ARR_SIZE 64

	int size,idx; 
	AdapterInfo inf[DEF_ARR_SIZE];
	cg_stat_t status;

	size = DEF_ARR_SIZE;
	status = NIC_get_adapters(inf,&size);
	if (status != CG_STAT_SUCCESS)
	{
		LOG_INFO("failed to query network interface list (stat=%lu)",status);
		return eGHN_LIB_STAT_FAILURE;    
	}

	for (idx=0;idx < size;idx++)
	{
		if (inf[idx].ip != 0x0100007F)
		{
			if (Is_Ip_In_The_Subnet(xi_DeviceIP,inf[idx].ip,inf[idx].SubnetMask))
			{
				*xo_NetworkCard = inf[idx].ip;
				return eGHN_LIB_STAT_SUCCESS;
			}
		}
	}

	return eGHN_LIB_STAT_FAILURE;
}

eGHN_LIB_STAT Ghn_Get_Stations(sGet_stations_Information* getstation);

eGHN_LIB_STAT Ghn_Get_Device_Information(sDevice_Information* devinf)
{
	sGet_stations_Information	getstation;
	eGHN_LIB_STAT				GHN_LIB_STAT;
	char						DeviceIP[HTTP_LIB_IP_ADDRESS_LEN];
	char						SelectedNetworkCardIP[HTTP_LIB_IP_ADDRESS_LEN]; 

	sGet_Data_Mode_lnformation*	getDataModel;
	XMLParserHandler			XMLHandler;
	UINT32						Size;
	
	XMLParserSubTree			SubTree;
	XMLParserSubTree			SubTree_IPv4Address;
	XMLParserSubTree			SubTree_IPv6Address;
	
	UINT32						InterfaceNumberOfEntries;
	UINT32						IPv4AddressNumberOfEntries = 0;
	UINT32						IPv6AddressNumberOfEntries = 0;

	int							i;

	char						Value[XML_PARSER_MAX_STRING];
	char						Value_IPAddress[XML_PARSER_MAX_STRING];
	char						Value_Alias[XML_PARSER_MAX_STRING];
	UINT32						UpTime = 0;

	bool						bDMDevice = FALSE;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// We have to get the IP-Address of the device from "AdapterIP" and "DeviceMAC" before we can continue
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (devinf->Connection.bHasAdapterIP == TRUE)
	{
		memset(&getstation, 0x00, sizeof(sGet_stations_Information));

		// Duplicate the "sConnection_Information" structure
		memcpy(&getstation.Connection, &devinf->Connection, sizeof(sConnection_Information));

		if ((GHN_LIB_STAT = Ghn_Get_Stations(&getstation)) != eGHN_LIB_STAT_SUCCESS)
		{
			strcpy(devinf->ErrorDescription,"Failed to get the stations list");
			return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
		}

		if ((GHN_LIB_STAT = Get_IP_Address_From_Station_List(&getstation, devinf->Connection.DeviceMAC, DeviceIP)) != eGHN_LIB_STAT_SUCCESS)
		{
			strcpy(devinf->ErrorDescription,"Failed to get the IP Address of the device");
			return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
		}

		strcpy(SelectedNetworkCardIP, getstation.Connection.SelectedNetworkCardIP);
	}
	else
	{
		strcpy(DeviceIP, devinf->Connection.DeviceIP);
		strcpy(SelectedNetworkCardIP, devinf->Connection.SelectedNetworkCardIP);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


	if (strcmp(DeviceIP, "0.0.0.0") == 0)
	{
		// FW is not supporting the IP-Stack
		// Get the information via VSM commands - TBD

		Size = 0x00;

		strcpy(devinf->AttributeArray[Size].Name,"Information");
		strcpy(devinf->AttributeArray[Size].Value, "N/A");
		Size++;

		devinf->Size = Size;

		return eGHN_LIB_STAT_SUCCESS;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "getDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	getDataModel = (sGet_Data_Mode_lnformation*)malloc(sizeof(sGet_Data_Mode_lnformation));
	if (getDataModel == NULL)
	{
		strcpy(devinf->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(getDataModel,0x00,sizeof(sGet_Data_Mode_lnformation));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// Create the structure for GetDataModel()
	strcpy(getDataModel->DeviceIP,DeviceIP);
	getDataModel->bHasNetworkCardIP = strlen(SelectedNetworkCardIP)>0;
	strcpy(getDataModel->NetworkCardIP, SelectedNetworkCardIP);
	getDataModel->IncludeBranch_Size = 4;
	getDataModel->ExcludeBranch_Size = 7;
	strcpy(getDataModel->IncludeBranch_Array[0].Name,Node_IP);
	strcpy(getDataModel->IncludeBranch_Array[1].Name,Node_LANConfigSecurity);
	strcpy(getDataModel->IncludeBranch_Array[2].Name,Node_DeviceInfo);
	strcpy(getDataModel->IncludeBranch_Array[3].Name,Node_Interface);

	strcpy(getDataModel->ExcludeBranch_Array[0].Name, Nodes_Interface_Stats);
	strcpy(getDataModel->ExcludeBranch_Array[1].Name, Nodes_Interface_X_00C5D9_Stats);
	strcpy(getDataModel->ExcludeBranch_Array[2].Name, Nodes_Interface_AssociatedDevice);
	strcpy(getDataModel->ExcludeBranch_Array[3].Name, Nodes_Interface_PeriodicStatistics);
	strcpy(getDataModel->ExcludeBranch_Array[4].Name, Nodes_Interface_PhyDiagConfig);
	strcpy(getDataModel->ExcludeBranch_Array[5].Name, Nodes_Interface_Alarms);
	strcpy(getDataModel->ExcludeBranch_Array[6].Name, Nodes_Interface_Debug);

	if (Get_Data_Model(getDataModel) == FALSE)
	{
		// Error getting the XML file from the device
		sprintf(devinf->ErrorDescription,"Failed to run Get_Data_Model(). %s",getDataModel->ErrorDescription);
		LOG_INFO(devinf->ErrorDescription);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}

	if (XMLParser_OpenXMLBuffer((char*)getDataModel->DataModel_Buffer, &XMLHandler) != eXMLPARSER_STAT_SUCCESS)
	{
		sprintf(devinf->ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
		LOG_INFO(devinf->ErrorDescription);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}

	Size = 0x00;
	
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Create a SubTree for searching all IP-Address under "Node_IP"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (XMLParser_CreateSubTree(XMLHandler, Node_IP, &SubTree) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_CloseXML(&XMLHandler);

		free(getDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}

	InterfaceNumberOfEntries = XMLParser_GetUIntValue(XMLHandler, SubTree, "InterfaceNumberOfEntries");

	/*
	// For very old FW-Version
	if (InterfaceNumberOfEntries == 1)
	{
		Update_sAttribute_Info(XMLHandler,Node_IP_Interface,"IPAddress",&devinf->AttributeArray[Size++]);
		Update_sAttribute_Info(XMLHandler,Node_IP_Interface,"DynamicIPAddress",&devinf->AttributeArray[Size++]);
	}
	*/

	// For FW-Version GA4
	if (InterfaceNumberOfEntries == 2)
	{
		// All "Interface" under "Device.IP.Interface"
		while (XMLParser_FindNode(XMLHandler,SubTree,"Interface") == eXMLPARSER_STAT_SUCCESS)
		{
			if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree,"Enable",Value) != eXMLPARSER_STAT_SUCCESS)
			{
				continue;
			}

			if (strcmp(Value,"true") != 0)
			{
				continue;
			}

			if (XMLParser_GetUIntValue(XMLHandler, SubTree, "IPv4AddressNumberOfEntries") != 1)
			{
				continue;
			}

			XMLParser_DuplicateSubTree(XMLHandler, SubTree, &SubTree_IPv4Address);

			if (XMLParser_FindNode(XMLHandler,SubTree_IPv4Address,"IPv4Address") == eXMLPARSER_STAT_SUCCESS)
			{
				if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_IPv4Address,"IPAddress",Value_IPAddress) != eXMLPARSER_STAT_SUCCESS)
				{
					XMLParser_FreeSubTree(XMLHandler,&SubTree);
					continue;
				}

				if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_IPv4Address,"Alias",Value_Alias) != eXMLPARSER_STAT_SUCCESS)
				{
					XMLParser_FreeSubTree(XMLHandler,&SubTree);
					continue;
				}

				sprintf(devinf->AttributeArray[Size].Name, "IPv4Address (%s)", Value_Alias);
				strcpy(devinf->AttributeArray[Size].Value, Value_IPAddress);
				Size++;
			}
			XMLParser_FreeSubTree(XMLHandler,&SubTree_IPv4Address);
		}

	}
	else if (InterfaceNumberOfEntries == 1)
	{
		// For FW-Version Support IPv6 (TR-181)

		if (XMLParser_FindNode(XMLHandler,SubTree,"Interface") == eXMLPARSER_STAT_SUCCESS)
		{
			IPv4AddressNumberOfEntries = XMLParser_GetUIntValue(XMLHandler, SubTree, "IPv4AddressNumberOfEntries");
			IPv6AddressNumberOfEntries = XMLParser_GetUIntValue(XMLHandler, SubTree, "IPv6AddressNumberOfEntries");

			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// IPv4 Section
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			XMLParser_DuplicateSubTree(XMLHandler, SubTree, &SubTree_IPv4Address);

			for (i=1;i<=IPv4AddressNumberOfEntries;i++)
			{
				if (XMLParser_FindNode(XMLHandler,SubTree_IPv4Address,"IPv4Address") != eXMLPARSER_STAT_SUCCESS)
				{
					break;
				}

				if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_IPv4Address,"Enable",Value) != eXMLPARSER_STAT_SUCCESS)
				{
					continue;
				}

				if (strcmp(Value,"true") != 0)
				{
					continue;
				}

				if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_IPv4Address,"Status",Value) != eXMLPARSER_STAT_SUCCESS)
				{
					continue;
				}

				if (strcmp(Value,"Enabled") != 0)
				{
					continue;
				}

				if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_IPv4Address,"IPAddress",Value_IPAddress) != eXMLPARSER_STAT_SUCCESS)
				{
					XMLParser_FreeSubTree(XMLHandler,&SubTree);
					continue;
				}

				if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_IPv4Address,"Alias",Value_Alias) != eXMLPARSER_STAT_SUCCESS)
				{
					XMLParser_FreeSubTree(XMLHandler,&SubTree);
					continue;
				}
				
				sprintf(devinf->AttributeArray[Size].Name, "IPv4Address (%s)", Value_Alias);
				strcpy(devinf->AttributeArray[Size].Value, Value_IPAddress);
				Size++;
			}

			XMLParser_FreeSubTree(XMLHandler,&SubTree_IPv4Address);
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// IPv6 Section
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			XMLParser_DuplicateSubTree(XMLHandler, SubTree, &SubTree_IPv6Address);

			for (i=1;i<=IPv6AddressNumberOfEntries;i++)
			{
				if (XMLParser_FindNode(XMLHandler,SubTree_IPv6Address,"IPv6Address") != eXMLPARSER_STAT_SUCCESS)
				{
					break;
				}

				if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_IPv6Address,"Enable",Value) != eXMLPARSER_STAT_SUCCESS)
				{
					continue;
				}

				if (strcmp(Value,"true") != 0)
				{
					continue;
				}

				if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_IPv6Address,"Status",Value) != eXMLPARSER_STAT_SUCCESS)
				{
					continue;
				}

				if (strcmp(Value,"Enabled") != 0)
				{
					continue;
				}

				if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_IPv6Address,"IPAddress",Value_IPAddress) != eXMLPARSER_STAT_SUCCESS)
				{
					XMLParser_FreeSubTree(XMLHandler,&SubTree);
					continue;
				}

				if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree_IPv6Address,"Alias",Value_Alias) != eXMLPARSER_STAT_SUCCESS)
				{
					XMLParser_FreeSubTree(XMLHandler,&SubTree);
					continue;
				}
				
				sprintf(devinf->AttributeArray[Size].Name, "IPv6Address (%s)", Value_Alias);
				strcpy(devinf->AttributeArray[Size].Value, Value_IPAddress);
				Size++;
			}

			XMLParser_FreeSubTree(XMLHandler,&SubTree_IPv6Address);
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		}
	}

	// Didn't found any information
	if (Size == 0)
	{
		strcpy(devinf->AttributeArray[Size].Name, "IPv4Address (StaticIP)");
		strcpy(devinf->AttributeArray[Size].Value, "N/A");
		Size++;
		strcpy(devinf->AttributeArray[Size].Name, "IPv4Address (DynamicIP)");
		strcpy(devinf->AttributeArray[Size].Value, "N/A");
		Size++;
	}

	XMLParser_FreeSubTree(XMLHandler,&SubTree);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	Update_sAttribute_Info(XMLHandler,Node_Interface,"GhnMACAddress",&devinf->AttributeArray[Size++]);

	Update_sAttribute_Info(XMLHandler,Node_DeviceInfo,"X_00C5D9_DeviceName",&devinf->AttributeArray[Size++]);
	strcpy(devinf->AttributeArray[Size-1].Name, "Device Name");

	Update_sAttribute_Info(XMLHandler,Node_Interface,"FirmwareVersion",&devinf->AttributeArray[Size++]);
	
	Update_sAttribute_Info(XMLHandler,Node_Interface,"NodeTypeDMStatus",&devinf->AttributeArray[Size++]);
	strcpy(devinf->AttributeArray[Size-1].Name, "Node Type");
	if (strcmp(devinf->AttributeArray[Size-1].Value,"TRUE") == 0)
	{
		strcpy(devinf->AttributeArray[Size-1].Value, "DM");
		bDMDevice = TRUE;
	}
	else
	{
		strcpy(devinf->AttributeArray[Size-1].Value, "RN");
		bDMDevice = FALSE;
	}

	if (devinf->bAdvanced == TRUE)
	{
		Update_sAttribute_Info(XMLHandler,Node_Interface,"GhnDeviceID",&devinf->AttributeArray[Size++]);
	}

	Update_sAttribute_Info(XMLHandler,Node_Interface,"NodeTypeActiveMedium",&devinf->AttributeArray[Size++]);
	strcpy(devinf->AttributeArray[Size-1].Name, "Active Medium");

	Update_sAttribute_Info(XMLHandler,Node_Interface,"NodeTypeConfiguration",&devinf->AttributeArray[Size++]);

	//Update_sAttribute_Info(XMLHandler,Node_DeviceInfo,"ModelName",&devinf->AttributeArray[Size++]);
	Update_sAttribute_Info(XMLHandler,Node_DeviceInfo,"Manufacturer",&devinf->AttributeArray[Size++]);
	//Update_sAttribute_Info(XMLHandler,Node_DeviceInfo,"ManufacturerOUI",&devinf->AttributeArray[Size++]);
	//Update_sAttribute_Info(XMLHandler,Node_DeviceInfo,"HardwareVersion",&devinf->AttributeArray[Size++]);


	if (devinf->bAdvanced == TRUE)
	{
		Update_sAttribute_Info(XMLHandler,Node_DeviceInfo,"DeviceNearDHCP",&devinf->AttributeArray[Size++]);
	}
	Update_sAttribute_Info(XMLHandler,Node_DeviceInfo,"X_00C5D9_ChipsetNum",&devinf->AttributeArray[Size++]);
	strcpy(devinf->AttributeArray[Size-1].Name, "Chipset");

	if (devinf->bAdvanced == TRUE)
	{
		Update_sAttribute_Info(XMLHandler,Node_Interface,"DomainId",&devinf->AttributeArray[Size++]);
		strcpy(devinf->AttributeArray[Size-1].Name, "DomainID");
	}

	Update_sAttribute_Info(XMLHandler,Node_DeviceInfo,"X_00C5D9_Bandwidth",&devinf->AttributeArray[Size++]);
	strcpy(devinf->AttributeArray[Size-1].Name, "Bandwidth");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Uptime
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	Update_sAttribute_Info(XMLHandler,Node_DeviceInfo,"UpTime",&devinf->AttributeArray[Size]);

	UpTime = atoi(devinf->AttributeArray[Size].Value);

	if (UpTime > 0)
	{
		sprintf(devinf->AttributeArray[Size].Value,"%dD %dH %dM",
				UpTime / 60 / 60 / 24,
				UpTime / 60 / 60 % 24,
				UpTime / 60 % 60);
	}
	Size++;
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	Update_sAttribute_Info(XMLHandler,Node_Interface,"DomainName",&devinf->AttributeArray[Size++]);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Encryption Status & Password
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	Update_sAttribute_Info(XMLHandler,Node_Interface,Node_X_00C5D9_EncryptionStatus,&devinf->AttributeArray[Size]);
	if (strcmp(devinf->AttributeArray[Size].Value, "false") == 0)
	{
		strcpy(devinf->AttributeArray[Size].Name,"Encryption Status");
		strcpy(devinf->AttributeArray[Size].Value,"Off");
		Size++;

		strcpy(devinf->AttributeArray[Size].Name,"Encryption Password");
		strcpy(devinf->AttributeArray[Size].Value,"");
		Size++;
	}
	else
	{
		strcpy(devinf->AttributeArray[Size].Name,"Encryption Status");
		strcpy(devinf->AttributeArray[Size].Value,"On");
		Size++;

		Update_sAttribute_Info(XMLHandler,Node_Interface,"GhnNetworkPassword",&devinf->AttributeArray[Size]);
		strcpy(devinf->AttributeArray[Size].Name,"Encryption Password");
		Size++;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if (devinf->bAdvanced == TRUE)
	{
		Update_sAttribute_Info(XMLHandler,Node_DeviceInfo,"TxPort",&devinf->AttributeArray[Size++]);
		Update_sAttribute_Info(XMLHandler,Node_DeviceInfo,"RxPort",&devinf->AttributeArray[Size++]);
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// BPL-Support
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (XMLParser_GetNodeAttributeByNodePath(XMLHandler,Node_Interface,"X_00C5D9_Hop_Count",&Value[0]) == eXMLPARSER_STAT_SUCCESS)
	{
		if (bDMDevice == TRUE)
		{
			strcpy(devinf->AttributeArray[Size].Name,"Number of MPRs");
		}
		else
		{
			strcpy(devinf->AttributeArray[Size].Name,"Hops from DM");
		}
		strcpy(devinf->AttributeArray[Size].Value, &Value[0]);
		Size++;
	}

	if (XMLParser_GetNodeAttributeByNodePath(XMLHandler,Node_Interface,"X_00C5D9_MPR_Indication",&Value[0]) == eXMLPARSER_STAT_SUCCESS)
	{
		// Only for EP devices
		if (bDMDevice == FALSE)
		{
			strcpy(devinf->AttributeArray[Size].Name,"MPR");

			if (OS_STRICMP(Value,"true") == 0)
			{
				strcpy(devinf->AttributeArray[Size].Value, "TRUE");
			}
			else
			{
				strcpy(devinf->AttributeArray[Size].Value, "FALSE");
			}
			Size++;
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	devinf->Size = Size;


	XMLParser_CloseXML(&XMLHandler);

	free(getDataModel);
	return eGHN_LIB_STAT_SUCCESS;
}

/***************************************************************************************************
* Ghn_Query_Device_State()                                                                         *
*                                                                                                  *
* Get Device State of a specific G.hn device                                                       *
*                                                                                                  *
* Input:                                                                                           *
*        device.AdapterIP - The user must specify the IP address of the network-card               *
*        device.DeviceMAC - The user must specify the MAC address of the G.hn device               *
*                                                                                                  *
***************************************************************************************************/
eGHN_LIB_STAT Ghn_Query_Device_State(sGet_Device_State_Information* deviceState)
{
	sConnection_Information		Connection;
	eGHN_LIB_STAT				GHN_LIB_STAT;
	Layer2Connection			layer2Connection;

	cg_stat_t					cg_stat;
	SGhnQueryDevice				QueryDevice;
	
	LOG_INFO("Started...");

	memset(&Connection,0x00,sizeof(sConnection_Information));

	Connection.bHasAdapterIP = TRUE;
	strcpy(Connection.AdapterIP,deviceState->AdapterIP);
	strcpy(Connection.DeviceMAC, deviceState->DeviceMAC);

	if ((GHN_LIB_STAT = Open_Layer2_Connection(	&Connection,
												ETHER_TYPE_GHN,
												&layer2Connection,
												deviceState->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		return GHN_LIB_STAT;
	}

	if ((cg_stat = QueryDeviceCommandFunction(&layer2Connection, &QueryDevice, FALSE)) != CG_STAT_SUCCESS)
	{
		sprintf(deviceState->ErrorDescription,"failed to query device (stat=%lu)",cg_stat);
		LOG_INFO(deviceState->ErrorDescription);
		Close_Layer2_Connection(&layer2Connection);
		return ConvertReturnStatus(cg_stat);
	}

	switch(QueryDevice.DeviceState)
	{
		case eDeviceState_FW:		strcpy(deviceState->DeviceState,"Firmware"); break;
		case eDeviceState_B1:		strcpy(deviceState->DeviceState,"B1"); break;
		case eDeviceState_BootCode:	strcpy(deviceState->DeviceState,"BootCode"); break;
		default:					strcpy(deviceState->DeviceState,"Unknown"); break;
	}

	Close_Layer2_Connection(&layer2Connection);
	return eGHN_LIB_STAT_SUCCESS;
}

/***************************************************************************************************
* Query_Local_Devices()                                                                            *
*                                                                                                  *
* Get the MAC,IP and the state of all the local connected G.hn devices                             *
*                                                                                                  *
* Input:                                                                                           *
*        localdevices.AdapterIP - The user must specify the IP address of the network-card         *
*                                                                                                  *
***************************************************************************************************/
eGHN_LIB_STAT Ghn_Query_Local_Devices(sGet_Local_Devices_Information* localdevices)
{
	sConnection_Information		Connection;
	eGHN_LIB_STAT				GHN_LIB_STAT;
	Layer2Connection			layer2Connection;

	cg_stat_t			cg_stat;

	ip_address_t		NetworkCardIP=0;


	sDevice				devices[64];
	int					Size = 64;
	int					i;

	LOG_INFO("Started...");

	memset(&Connection,0x00,sizeof(sConnection_Information));

	Connection.bHasAdapterIP = TRUE;
	strcpy(Connection.AdapterIP,localdevices->AdapterIP);
	strcpy(Connection.DeviceMAC, "FF:FF:FF:FF:FF:FF"); // Broadcast

	if ((GHN_LIB_STAT = Open_Layer2_Connection(	&Connection,
												ETHER_TYPE_GHN,
												&layer2Connection,
												localdevices->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		return GHN_LIB_STAT;
	}

	NetworkCardIP = str_to_ip(localdevices->AdapterIP);

	if ((cg_stat = Query_Local_DevicesCommandFunction(&layer2Connection, &Size, devices)) != CG_STAT_SUCCESS)
	{
		sprintf(localdevices->ErrorDescription,"failed to query local devices (stat=%lu)",cg_stat);
		LOG_INFO(localdevices->ErrorDescription);
		Close_Layer2_Connection(&layer2Connection);
		return ConvertReturnStatus(cg_stat);
	}

	localdevices->Size = Size;

	for (i=1;i<=Size;i++)
	{
		localdevices->sStationArray[i-1].bLocalDevice = TRUE;

		sprintf(localdevices->sStationArray[i-1].DeviceMAC,MAC_ADDR_FMT,MAC_ADDR(devices[i-1].DeviceMac.macAddress));

		// Check if one of the IPs is relevant for this network card
		if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(devices[i-1].DeviceIP1,NetworkCardIP))
		{
			ip_to_str(devices[i-1].DeviceIP1,localdevices->sStationArray[i-1].DeviceIP);
		}
		else if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(devices[i-1].DeviceIP2,NetworkCardIP))
		{
			ip_to_str(devices[i-1].DeviceIP2,localdevices->sStationArray[i-1].DeviceIP);
		}
		else
		{
			// None of the 
			ip_to_str(0x000000,localdevices->sStationArray[i-1].DeviceIP);
		}

		switch(devices[i-1].DeviceState)
		{
			case eDeviceState_FW:		strcpy(localdevices->sStationArray[i-1].DeviceState,"Firmware"); break;
			case eDeviceState_B1:		strcpy(localdevices->sStationArray[i-1].DeviceState,"B1"); break;
			case eDeviceState_BootCode:	strcpy(localdevices->sStationArray[i-1].DeviceState,"BootCode"); break;
			default:					strcpy(localdevices->sStationArray[i-1].DeviceState,"Unknown"); break;
		}
	}

	Close_Layer2_Connection(&layer2Connection);
	return eGHN_LIB_STAT_SUCCESS;
}

/***************************************************************************************************
* Query_Network_Devices()                                                                          *
*                                                                                                  *
* Get the MAC,IP and the state of the local and remote G.hn devices                                *
*                                                                                                  *
* Input:                                                                                           *
*        localdevices.AdapterIP - The user must specify the IP address of the network-card         *
*                                                                                                  *
***************************************************************************************************/
eGHN_LIB_STAT Ghn_Query_Network_Devices(sGet_Local_Devices_Information* localdevices)
{
	sConnection_Information		Connection;
	eGHN_LIB_STAT				CG5XXXLIB_STAT;
	Layer2Connection			layer2Connection;

	cg_stat_t			cg_stat;

	ip_address_t		NetworkCardIP=0;


	sDevice				devices[64];
	int					Size = 64;
	int					i;

	LOG_INFO("Started...");

	memset(&Connection,0x00,sizeof(sConnection_Information));

	Connection.bHasAdapterIP = TRUE;
	strcpy(Connection.AdapterIP,localdevices->AdapterIP);
	strcpy(Connection.DeviceMAC, "FF:FF:FF:FF:FF:FF"); // Broadcast

	if ((CG5XXXLIB_STAT = Open_Layer2_Connection(	&Connection,
													ETHER_TYPE_GHN,
													&layer2Connection,
													localdevices->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		return CG5XXXLIB_STAT;
	}

	NetworkCardIP = str_to_ip(localdevices->AdapterIP);

	if ((cg_stat = Query_Network_DevicesCommandFunction(&layer2Connection, &Size, devices)) != CG_STAT_SUCCESS)
	{
		sprintf(localdevices->ErrorDescription,"failed to query local devices (stat=%lu)",cg_stat);
		LOG_INFO(localdevices->ErrorDescription);
		Close_Layer2_Connection(&layer2Connection);
		return ConvertReturnStatus(cg_stat);
	}

	localdevices->Size = Size;

	for (i=1;i<=Size;i++)
	{
		sprintf(localdevices->sStationArray[i-1].DeviceMAC,MAC_ADDR_FMT,MAC_ADDR(devices[i-1].DeviceMac.macAddress));

		// Check if one of the IPs is relevant for this network card
		if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(devices[i-1].DeviceIP1,NetworkCardIP))
		{
			ip_to_str(devices[i-1].DeviceIP1,localdevices->sStationArray[i-1].DeviceIP);
		}
		else if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(devices[i-1].DeviceIP2,NetworkCardIP))
		{
			ip_to_str(devices[i-1].DeviceIP2,localdevices->sStationArray[i-1].DeviceIP);
		}
		else
		{
			// None of the 
			ip_to_str(0x000000,localdevices->sStationArray[i-1].DeviceIP);
		}

		switch(devices[i-1].DeviceState)
		{
			case eDeviceState_FW:		strcpy(localdevices->sStationArray[i-1].DeviceState,"Firmware"); break;
			case eDeviceState_B1:		strcpy(localdevices->sStationArray[i-1].DeviceState,"B1"); break;
			case eDeviceState_BootCode:	strcpy(localdevices->sStationArray[i-1].DeviceState,"BootCode"); break;
			default:					strcpy(localdevices->sStationArray[i-1].DeviceState,"Unknown"); break;
		}
	}

	Close_Layer2_Connection(&layer2Connection);
	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Stations(sGet_stations_Information* getstation)
{
 	eGHN_LIB_STAT				GHN_LIB_STAT;
 	Layer2Connection			layer2Connection;

	sGet_Data_Mode_lnformation*	getDataModel;

	cg_stat_t			cg_stat;
	eXMLPARSER_STAT		status;
	XMLParserHandler	Handler;
	XMLParserSubTree	SubTree;
	mac_address_t		mac;
	macStruct			LocalDeviceMac;
	char				Value[XML_PARSER_MAX_STRING];
	UINT32				Size = 0x00;

	ip_address_t		DeviceIP=0;

	ip_address_t		DeviceIP1=0;
	ip_address_t		DeviceIP2=0;
	char				strDeviceIP1[GHN_LIB_IP_ADDRESS_LEN];
	char				strDeviceIP2[GHN_LIB_IP_ADDRESS_LEN];

	ip_address_t		NetworkCardIP=0;
	char				strDeviceIP[256];
	eDeviceState		DeviceState;
	bool				bLocalDevice;

	LOG_INFO("Started...");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "getDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	getDataModel = (sGet_Data_Mode_lnformation*)malloc(sizeof(sGet_Data_Mode_lnformation));
	if (getDataModel == NULL)
	{
		strcpy(getstation->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(getDataModel,0x00,sizeof(sGet_Data_Mode_lnformation));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if ((GHN_LIB_STAT = Open_Layer2_Connection(	&getstation->Connection,
												ETHER_TYPE_GHN,
												&layer2Connection,
												getstation->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		free(getDataModel);
		return GHN_LIB_STAT;
	}

	NetworkCardIP = HandleToConn(layer2Connection.m_eth_handle_t)->adapter_address;

	if (getstation->Connection.bHasDeviceIP == TRUE)
	{
		// The user specify the IP address of the device 
		DeviceIP = str_to_ip(getstation->Connection.DeviceIP);
	}

	if (getstation->Connection.bHasAdapterIP == TRUE)
	{
		// Get the IP addresses of the local device
		if ((cg_stat=QueryDeviceIP(&layer2Connection, &layer2Connection.m_MAC_Address,&DeviceState, &bLocalDevice, &DeviceIP1, &DeviceIP2)) != CG_STAT_SUCCESS)
		{
			sprintf(getstation->ErrorDescription,"Failed to query device "MAC_ADDR_FMT,MAC_ADDR(layer2Connection.m_MAC_Address.macAddress));
			LOG_INFO(getstation->ErrorDescription);
			Close_Layer2_Connection(&layer2Connection);
			free(getDataModel);
			return ConvertReturnStatus(cg_stat);
		}

		if (DeviceState == eDeviceState_BootCode)
		{
			sprintf(getstation->ErrorDescription,"Failed to query device "MAC_ADDR_FMT,MAC_ADDR(layer2Connection.m_MAC_Address.macAddress));
			LOG_INFO(getstation->ErrorDescription);
			Close_Layer2_Connection(&layer2Connection);
			free(getDataModel);
			return eGHN_LIB_STAT_FW_BOOT_CODE;
		}

		/*
		if (DeviceState == eDeviceState_B1)
		{
			sprintf(getstation->ErrorDescription,"Failed to query device "MAC_ADDR_FMT,MAC_ADDR(layer2Connection.m_MAC_Address.macAddress));
			LOG_INFO(getstation->ErrorDescription);
			Close_Layer2_Connection(&layer2Connection);
			free(getDataModel);
			return eGHN_LIB_STAT_FW_B1;
		}
		*/

		Size = 1;
		sprintf(getstation->sStationArray[Size-1].DeviceMAC,MAC_ADDR_FMT,MAC_ADDR(layer2Connection.m_MAC_Address.macAddress));
		getstation->sStationArray[Size-1].bLocalDevice = (bLocalDevice == TRUE);
		switch(DeviceState)
		{
			case eDeviceState_FW:		strcpy(getstation->sStationArray[Size-1].DeviceState,"Firmware"); break;
			case eDeviceState_B1:		strcpy(getstation->sStationArray[Size-1].DeviceState,"B1"); break;
			case eDeviceState_BootCode:	strcpy(getstation->sStationArray[Size-1].DeviceState,"BootCode"); break;
			default:					strcpy(getstation->sStationArray[Size-1].DeviceState,"Unknown"); break;
		}

		// Check if FW supports the IP-Stack
		if ((DeviceIP1 != 0x00) || (DeviceIP2 != 0x00))
		{
			// Check if one of the IPs is relevant for this network card
			if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(DeviceIP1,NetworkCardIP))
			{
				DeviceIP = DeviceIP1;
			}
			else if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(DeviceIP2,NetworkCardIP))
			{
				DeviceIP = DeviceIP2;
			}
			else
			{
				// None of the IPs in the Subnet-Mask of the select network interface
				ip_to_str(DeviceIP1,strDeviceIP1);
				ip_to_str(DeviceIP2,strDeviceIP2);

				if (DeviceIP2 == 0x00)
				{
					sprintf(getstation->ErrorDescription,"The device %s with IP=%s is not in the Subnet-Mask of the select network interface",
													getstation->sStationArray[Size-1].DeviceMAC,
													strDeviceIP1);
				}
				else if (DeviceIP1 == 0x00)
				{
					sprintf(getstation->ErrorDescription,"The device %s with IP=%s is not in the Subnet-Mask of the select network interface",
													getstation->sStationArray[Size-1].DeviceMAC,
													strDeviceIP2);
				}
				else
				{
					sprintf(getstation->ErrorDescription,"The device %s with IP1=%s and IP2=%s is not in the Subnet-Mask of the select network interface",
													getstation->sStationArray[Size-1].DeviceMAC,
													strDeviceIP1,
													strDeviceIP2);
				}

				LOG_INFO(getstation->ErrorDescription);
				Close_Layer2_Connection(&layer2Connection);
				free(getDataModel);
				return eGHN_LIB_STAT_SUBNET_MASK_MISMATCH;
			}
		}

		ip_to_str(DeviceIP,getstation->sStationArray[Size-1].DeviceIP);
	}
	
	ip_to_str(DeviceIP,strDeviceIP);

	if (strcmp(strDeviceIP, "0.0.0.0") == 0)
	{
		// FW is not supporting the IP-Stack
		// Get the information via VSM commands - TBD
	}
	else
	{
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get Station-List From LocalDevice
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		LOG_INFO("Call GetDataModel() strIP=%s",strDeviceIP);

		// Create the structure for GetDataModel()
		strcpy(getDataModel->DeviceIP, strDeviceIP);
		getDataModel->bHasNetworkCardIP = TRUE;
		strcpy(getDataModel->NetworkCardIP, getstation->Connection.SelectedNetworkCardIP);
		getDataModel->IncludeBranch_Size = 4;
		getDataModel->ExcludeBranch_Size = 0;
		strcpy(getDataModel->IncludeBranch_Array[0].Name,Nodes_Interface_GhnMACAddress);
		strcpy(getDataModel->IncludeBranch_Array[1].Name,Nodes_AssociatedDevice_GhnMACAddress);
		strcpy(getDataModel->IncludeBranch_Array[2].Name,Nodes_Interface_GhnDeviceID);
		strcpy(getDataModel->IncludeBranch_Array[3].Name,Nodes_AssociatedDevice_GhnDeviceID);

		if (Get_Data_Model(getDataModel) == FALSE)
		{
			// Error getting the XML file from the device
			sprintf(getstation->ErrorDescription,"Failed to run Get_Data_Model(). %s",getDataModel->ErrorDescription);
			LOG_INFO(getstation->ErrorDescription);
			Close_Layer2_Connection(&layer2Connection);
			free(getDataModel);
			return eGHN_LIB_STAT_HTTP_REQUEST_FAILED;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Parsing the BBT.xml file
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		LOG_INFO("Parsing the BBT Data-Model");

		if ((status = XMLParser_OpenXMLBuffer((char*)getDataModel->DataModel_Buffer,&Handler)) != eXMLPARSER_STAT_SUCCESS)
		{
			sprintf(getstation->ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
			LOG_INFO(getstation->ErrorDescription);
			Close_Layer2_Connection(&layer2Connection);
			free(getDataModel);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}

		if (XMLParser_CreateSubTree(Handler, Node_Interface, &SubTree) == eXMLPARSER_STAT_SUCCESS)
		{
			UINT8 GhnDeviceID = 0;

			GhnDeviceID = XMLParser_GetIntValue(Handler, SubTree, "GhnDeviceID");

			// Get MAC of Local-Device
			if (Size == 0)
			{
				if (XMLParser_GetNodeAttributeByName(Handler,SubTree,"GhnMACAddress",Value) == eXMLPARSER_STAT_SUCCESS)
				{
					str_to_mac(Value,&mac);
					memcpy(LocalDeviceMac.macAddress,mac,HMAC_LEN);
				}


				// Get the IP addresses of the local device
				if ((cg_stat=QueryDeviceIP(&layer2Connection, &LocalDeviceMac,&DeviceState, &bLocalDevice, &DeviceIP1, &DeviceIP2)) != CG_STAT_SUCCESS)
				{
					LOG_INFO("failed to query device "MAC_ADDR_FMT,MAC_ADDR(LocalDeviceMac.macAddress));
					bLocalDevice = FALSE;
				}

				// Add to "g_StationListTable"
				Size = 1;
				sprintf(getstation->sStationArray[Size-1].DeviceMAC,MAC_ADDR_FMT,MAC_ADDR(LocalDeviceMac.macAddress));
				getstation->sStationArray[Size-1].GhnDeviceID = GhnDeviceID;
				getstation->sStationArray[Size-1].bLocalDevice = (bLocalDevice == TRUE);
				ip_to_str(DeviceIP,getstation->sStationArray[Size-1].DeviceIP);
				switch(DeviceState)
				{
					case eDeviceState_FW:		strcpy(getstation->sStationArray[Size-1].DeviceState,"Firmware"); break;
					case eDeviceState_B1:		strcpy(getstation->sStationArray[Size-1].DeviceState,"B1"); break;
					default:					strcpy(getstation->sStationArray[Size-1].DeviceState,"Unknown"); break;
				}
			}
			else // (Size == 1)
			{
				getstation->sStationArray[Size-1].GhnDeviceID = GhnDeviceID;
			}

			// All "AssociatedDevice" under "Device.Ghn.Interface"
			while ((status = XMLParser_FindNode(Handler,SubTree,"AssociatedDevice")) == eXMLPARSER_STAT_SUCCESS)
			{
				UINT8 GhnDeviceID = 0;

				GhnDeviceID = XMLParser_GetIntValue(Handler, SubTree, "GhnDeviceID");

				if (XMLParser_GetNodeAttributeByName(Handler,SubTree,"GhnMACAddress",Value) == eXMLPARSER_STAT_SUCCESS)
				{
					macStruct		RemoteDeviceMac;
					ip_address_t	RemoteDeviceIP=0;
					ip_address_t	RemoteDeviceIP1=0;
					ip_address_t	RemoteDeviceIP2=0;

					str_to_mac(Value,&mac);

					memcpy(&RemoteDeviceMac,mac,HMAC_LEN);

					if (getstation->bQueryOnlyLocalDevice == FALSE)
					{
						if ((cg_stat=QueryDeviceIP(&layer2Connection, &RemoteDeviceMac,&DeviceState, &bLocalDevice, &RemoteDeviceIP1,&RemoteDeviceIP2)) != CG_STAT_SUCCESS)
						{
							LOG_INFO("failed to query device "MAC_ADDR_FMT,MAC_ADDR(RemoteDeviceMac.macAddress));
							bLocalDevice = FALSE;
						}
					}

					// Add to "g_StationListTable"
					Size++;
					sprintf(getstation->sStationArray[Size-1].DeviceMAC,MAC_ADDR_FMT,MAC_ADDR(RemoteDeviceMac.macAddress));
					getstation->sStationArray[Size-1].GhnDeviceID = GhnDeviceID;
					getstation->sStationArray[Size-1].bLocalDevice = (bLocalDevice == TRUE);
					switch(DeviceState)
					{
						case eDeviceState_FW:		strcpy(getstation->sStationArray[Size-1].DeviceState,"Firmware"); break;
						case eDeviceState_B1:		strcpy(getstation->sStationArray[Size-1].DeviceState,"B1"); break;
						case eDeviceState_BootCode:	strcpy(getstation->sStationArray[Size-1].DeviceState,"BootCode"); break;
						default:					strcpy(getstation->sStationArray[Size-1].DeviceState,"Unknown"); break;
					}

					// Check if one of the IPs is relevant for this network card
					if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(RemoteDeviceIP1,NetworkCardIP))
					{
						RemoteDeviceIP = RemoteDeviceIP1;
					}
					else if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(RemoteDeviceIP2,NetworkCardIP))
					{
						RemoteDeviceIP = RemoteDeviceIP2;
					}
					else
					{
						RemoteDeviceIP = 0x00000000;
					}

					ip_to_str(RemoteDeviceIP,getstation->sStationArray[Size-1].DeviceIP);
				}
			}

			XMLParser_FreeSubTree(Handler,&SubTree);
		}

		if ((status = XMLParser_CloseXML(&Handler)) != eXMLPARSER_STAT_SUCCESS)
		{
			LOG_INFO("Call XMLParser_CloseXML() Failed");
			Close_Layer2_Connection(&layer2Connection);
			free(getDataModel);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	getstation->Size = Size;

	Close_Layer2_Connection(&layer2Connection);
	
	free(getDataModel);
	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Reset(sReset_Information* reset)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	Layer2Connection			layer2Connection;

	LOG_INFO("Started...");

	if ((GHN_LIB_STAT = Open_Layer2_Connection(	&reset->Connection,
												ETHER_TYPE_GHN,
												&layer2Connection,
												reset->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		return GHN_LIB_STAT;
	}

	if (reset->HardwareReset)
	{
		if (Hardware_Reset_Device(&layer2Connection, reset->ResetMode) == FALSE)
		{
			LOG_INFO("Call Hardware_Reset_Device() Failed");
			Close_Layer2_Connection(&layer2Connection);
			return eGHN_LIB_STAT_FAILURE;
		}
	}
	else
	{
		if (Firmware_Reset_Device(&layer2Connection, reset->ResetMode) == FALSE)
		{
			LOG_INFO("Call Firmware_Reset_Device() Failed");
			Close_Layer2_Connection(&layer2Connection);
			return eGHN_LIB_STAT_FAILURE;
		}
	}

	Close_Layer2_Connection(&layer2Connection);

	return eGHN_LIB_STAT_SUCCESS;
}

bool g_bStopNetinf = FALSE;

eGHN_LIB_STAT Ghn_Stop_Netinf()
{
	if (g_bStopNetinf == FALSE)
	{
		g_bStopNetinf = TRUE;
		return eGHN_LIB_STAT_SUCCESS;
	}
	else
	{
		return eGHN_LIB_STAT_FAILURE;
	}
}


bool g_bIncludeBitLoadTableInTheNextIteration = FALSE;

DllExport eGHN_LIB_STAT Ghn_Netinf_Refresh_The_Bit_Load_Table_In_The_Next_Iteration()
{
	g_bIncludeBitLoadTableInTheNextIteration = TRUE;
	
	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Phy_Diag_Config_Information(sPhy_Diag_Config_Information* PhyDiagConfig)
{
	sGet_Data_Mode_lnformation*	getDataModel;
	XMLParserHandler			Handler;

	XMLParserSubTree			SubTree;
	char						Value[XML_PARSER_MAX_STRING] = "N/A";

	eXMLPARSER_STAT				status;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "getDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	getDataModel = (sGet_Data_Mode_lnformation*)malloc(sizeof(sGet_Data_Mode_lnformation));
	if (getDataModel == NULL)
	{
		strcpy(PhyDiagConfig->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(getDataModel,0x00,sizeof(sGet_Data_Mode_lnformation));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// Create the structure for GetDataModel()
	strcpy(getDataModel->DeviceIP,PhyDiagConfig->DeviceIP);
	getDataModel->bHasNetworkCardIP = PhyDiagConfig->bHasNetworkCardIP;
	strcpy(getDataModel->NetworkCardIP, PhyDiagConfig->NetworkCardIP);
	getDataModel->IncludeBranch_Size = 1;
	getDataModel->ExcludeBranch_Size = 0;
	strcpy(getDataModel->IncludeBranch_Array[0].Name,Nodes_Interface_PhyDiagConfig);

	if (Get_Data_Model(getDataModel) == FALSE)
	{
		// Error getting the XML file from the device
		sprintf(PhyDiagConfig->ErrorDescription,"Failed to run Get_Data_Model(). %s",getDataModel->ErrorDescription);
		LOG_INFO(PhyDiagConfig->ErrorDescription);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}

	if (XMLParser_OpenXMLBuffer((char*)getDataModel->DataModel_Buffer, &Handler) != eXMLPARSER_STAT_SUCCESS)
	{
		sprintf(PhyDiagConfig->ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
		LOG_INFO(PhyDiagConfig->ErrorDescription);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}


	if (XMLParser_CreateSubTree(Handler, Node_Interface_X_00C5D9_PhyDiagConfig, &SubTree) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_CloseXML(&Handler);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get PhyDiagReceiverMAC
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (XMLParser_GetNodeAttributeByName(Handler,SubTree, "PhyDiagReceiverMAC", Value) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_FreeSubTree(Handler,&SubTree);
		XMLParser_CloseXML(&Handler);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}
	strcpy(PhyDiagConfig->PhyDiagReceiverMAC,Value);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get PhyDiagTrafficBurstSize
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (XMLParser_GetNodeAttributeByName(Handler,SubTree, "PhyDiagTrafficBurstSize", Value) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_FreeSubTree(Handler,&SubTree);
		XMLParser_CloseXML(&Handler);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}
	PhyDiagConfig->PhyDiagTrafficBurstSize = atoi(Value);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get PhyDiagTrafficTimeOut
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (XMLParser_GetNodeAttributeByName(Handler,SubTree, "PhyDiagTrafficTimeOut", Value) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_FreeSubTree(Handler,&SubTree);
		XMLParser_CloseXML(&Handler);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}
	PhyDiagConfig->PhyDiagTrafficTimeOut = atoi(Value);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get TrafficIsAcked
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (XMLParser_GetNodeAttributeByName(Handler,SubTree, "TrafficIsAcked", Value) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_FreeSubTree(Handler,&SubTree);
		XMLParser_CloseXML(&Handler);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}

	if (strcmp(Value,"true")==0)
	{
		PhyDiagConfig->TrafficIsAcked = TRUE;
	}
	else
	{
		PhyDiagConfig->TrafficIsAcked = FALSE;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get StartPhyDiagTest
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (XMLParser_GetNodeAttributeByName(Handler,SubTree, "StartPhyDiagTest", Value) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_FreeSubTree(Handler,&SubTree);
		XMLParser_CloseXML(&Handler);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}
	if (strcmp(Value,"true")==0)
	{
		PhyDiagConfig->StartPhyDiagTest = TRUE;
	}
	else
	{
		PhyDiagConfig->StartPhyDiagTest = FALSE;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get OperationResult
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (XMLParser_GetNodeAttributeByName(Handler,SubTree, "OperationResult", Value) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_FreeSubTree(Handler,&SubTree);
		XMLParser_CloseXML(&Handler);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}
	PhyDiagConfig->OperationResult = atoi(Value);

	XMLParser_FreeSubTree(Handler,&SubTree);


	if ((status = XMLParser_CloseXML(&Handler)) != eXMLPARSER_STAT_SUCCESS)
	{
		LOG_INFO("Call XMLParser_CloseXML() Failed");
		free(getDataModel);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}

	free(getDataModel);
	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Netinf(sNetinf_Information* netinf)
{
	sGet_stations_Information		getstation;
	eGHN_LIB_STAT					GHN_LIB_STAT;
	char							DeviceIP[HTTP_LIB_IP_ADDRESS_LEN];
	char							SelectedNetworkCardIP[HTTP_LIB_IP_ADDRESS_LEN]; 

	sGet_Data_Mode_lnformation*		getDataModel;
	sSet_Data_Mode_lnformation		setDataModel;

	sPhy_Diag_Config_Information	PhyDiagConfig;

	XMLParserHandler				XMLHandler;
	
	sNetinf_Test					Netinf_Test;
	sNetinf_Result					Netinf_Result;

	char							TransmitterDeviceIP[GHN_LIB_IP_ADDRESS_LEN];
	char							ReceiverDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	UINT32							SampleIteration;
	eSamplingState					SamplingState;
	bool							bLastIteration;
	bool							bIncludeBitLoadTable;
	UINT32							SamplingUpTime = 0;
	UINT32							SamplingCurrentUpTime = 0;

	char							TestOutputFolder[OS_MAX_PATH];

	clock_t							Clock_StartNetinf;
	clock_t							Clock_EndNetinf;
	clock_t							ClockLast;
	
	char							Value[XML_PARSER_MAX_STRING];
	UINT32							Intervals;

	FILE*							f;
	int								ThresholdNumberOfSegments = 0;
	bool							bNeedContinue;

	LOG_INFO("Started...");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// We have to get the IP-Address of the device from "AdapterIP" and "DeviceMAC" before we can continue
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (netinf->Connection.bHasAdapterIP == TRUE)
	{
		memset(&getstation, 0x00, sizeof(sGet_stations_Information));

		// Duplicate the "sConnection_Information" structure
		memcpy(&getstation.Connection, &netinf->Connection, sizeof(sConnection_Information));

		if ((GHN_LIB_STAT = Ghn_Get_Stations(&getstation)) != eGHN_LIB_STAT_SUCCESS)
		{
			strcpy(netinf->ErrorDescription,"Failed to get the stations list");
			return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
		}

		if ((GHN_LIB_STAT = Get_IP_Address_From_Station_List(&getstation, netinf->Connection.DeviceMAC, DeviceIP)) != eGHN_LIB_STAT_SUCCESS)
		{
			strcpy(netinf->ErrorDescription,"Failed to get the IP Address of the device");
			return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
		}

		strcpy(SelectedNetworkCardIP, getstation.Connection.SelectedNetworkCardIP);
	}
	else
	{
		strcpy(DeviceIP, netinf->Connection.DeviceIP);
		strcpy(SelectedNetworkCardIP, netinf->Connection.SelectedNetworkCardIP);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "getDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	getDataModel = (sGet_Data_Mode_lnformation*)malloc(sizeof(sGet_Data_Mode_lnformation));
	if (getDataModel == NULL)
	{
		strcpy(netinf->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(getDataModel,0x00,sizeof(sGet_Data_Mode_lnformation));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if ((f = fopen("NumberOfSegments.txt","r")) != NULL)
	{
		fscanf(f, "%d", &ThresholdNumberOfSegments);
		fclose(f);

		LOG_INFO("ThresholdNumberOfSegments=%d", ThresholdNumberOfSegments);
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get Station-List From Local-Device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	LOG_INFO("Get Station-List From Local-Device");
	memset(&getstation,0x00,sizeof(sGet_stations_Information));
	getstation.Connection.bHasDeviceIP = TRUE;
	strcpy(getstation.Connection.DeviceIP, DeviceIP);
	strcpy(getstation.Connection.SelectedNetworkCardIP, SelectedNetworkCardIP);

	getstation.bQueryOnlyLocalDevice = netinf->bQueryOnlyLocalDevice;

	if ((GHN_LIB_STAT = Ghn_Get_Stations(&getstation)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(netinf->ErrorDescription,getstation.ErrorDescription);
		LOG_INFO("Call Get_Stations() Failed");
		free(getDataModel);
		return GHN_LIB_STAT;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the IP address of the transmitter device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (netinf->bGenerateTraffic) 
	{
		if ((GHN_LIB_STAT = Get_IP_Address_From_Station_List(&getstation, netinf->TransmitterDeviceMAC, TransmitterDeviceIP)) != eGHN_LIB_STAT_SUCCESS)
		{
			sprintf(netinf->ErrorDescription,"Failed to get the IP Address of the transmitter device");
			LOG_INFO(netinf->ErrorDescription);
			free(getDataModel);
			return GHN_LIB_STAT;
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the IP address of the receiver device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if ((GHN_LIB_STAT = Get_IP_Address_From_Station_List(&getstation, netinf->ReceiverDeviceMAC, ReceiverDeviceIP)) != eGHN_LIB_STAT_SUCCESS)
	{
		sprintf(netinf->ErrorDescription,"Failed to get the IP Address of the receiver device");
		LOG_INFO(netinf->ErrorDescription);
		free(getDataModel);
		return GHN_LIB_STAT;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Check if there is already running netinf on that channel
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (netinf->bGenerateTraffic)
	{
		strcpy(PhyDiagConfig.DeviceIP, TransmitterDeviceIP);
		PhyDiagConfig.bHasNetworkCardIP = strlen(SelectedNetworkCardIP)>0;
		strcpy(PhyDiagConfig.NetworkCardIP, SelectedNetworkCardIP);
		if ((GHN_LIB_STAT = Ghn_Phy_Diag_Config_Information(&PhyDiagConfig)) != eGHN_LIB_STAT_SUCCESS)
		{
			if (netinf->bGenerateTraffic == FALSE)
			{
				// Workaround to support old FW (TBD 2011)
				PhyDiagConfig.StartPhyDiagTest = FALSE;
			}
			else
			{
				sprintf(netinf->ErrorDescription,"Failed to get the PhyDiagConfigInformation of the transmitter device");
				LOG_INFO(netinf->ErrorDescription);
				free(getDataModel);
				return GHN_LIB_STAT;
			}
		}
		if ((PhyDiagConfig.StartPhyDiagTest)&& (netinf->bOverideTraffic == FALSE))
		{
			// Error getting the XML file from the device
			sprintf(netinf->ErrorDescription,"netinf is already running");
			LOG_INFO(netinf->ErrorDescription);
			free(getDataModel);
			return eGHN_LIB_STAT_NETINF_ALREADY_RUNNING;
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Stop the Netinf
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (netinf->bGenerateTraffic) 
	{
		strcpy(setDataModel.DeviceIP, TransmitterDeviceIP);
		setDataModel.bHasNetworkCardIP = strlen(SelectedNetworkCardIP)>0;
		strcpy(setDataModel.NetworkCardIP, SelectedNetworkCardIP);
		setDataModel.SetParameter_Size = 2;

		strcpy(setDataModel.SetParameter_Array[0].Name, Node_StartPhyDiagTest);
		strcpy(setDataModel.SetParameter_Array[0].Value, "false");

		strcpy(setDataModel.SetParameter_Array[1].Name, Node_PhyDiagReceiverMAC);
		strcpy(setDataModel.SetParameter_Array[1].Value, "");

		if (Set_Data_Model(&setDataModel) == FALSE)
		{
			// Error getting the XML file from the device
			sprintf(netinf->ErrorDescription,"Failed to stop netinf %s",setDataModel.ErrorDescription);
			LOG_INFO(netinf->ErrorDescription);
			free(getDataModel);
			return eGHN_LIB_STAT_FAILED_START_NETINF;
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Start the Netinf
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (netinf->bGenerateTraffic)
	{
		strcpy(setDataModel.DeviceIP, TransmitterDeviceIP);
		setDataModel.SetParameter_Size = 4;

		strcpy(setDataModel.SetParameter_Array[0].Name, Node_PhyDiagReceiverMAC);
		strcpy(setDataModel.SetParameter_Array[0].Value, netinf->ReceiverDeviceMAC);

		strcpy(setDataModel.SetParameter_Array[1].Name, Node_PhyDiagTrafficBurstSize);
		sprintf(setDataModel.SetParameter_Array[1].Value, "%d", netinf->TrafficBurstSize);

		strcpy(setDataModel.SetParameter_Array[2].Name, Node_PhyDiagTrafficTimeOut);
		sprintf(setDataModel.SetParameter_Array[2].Value, "%d", netinf->TestDuration);

		strcpy(setDataModel.SetParameter_Array[3].Name, Node_StartPhyDiagTest);
		strcpy(setDataModel.SetParameter_Array[3].Value, "true");

		if (Set_Data_Model(&setDataModel) == FALSE)
		{
			// Error getting the XML file from the device
			if (netinf->bStopOnReadingError)
			{
				sprintf(netinf->ErrorDescription,"Failed to start netinf %s",setDataModel.ErrorDescription);
				LOG_INFO(netinf->ErrorDescription);
				free(getDataModel);
				return eGHN_LIB_STAT_FAILED_START_NETINF;
			}
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Prepare the Netinf-Test Setup
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	memset(&Netinf_Test,0x00,sizeof(sNetinf_Test));
	str_to_mac(netinf->TransmitterDeviceMAC,&Netinf_Test.StaTxMAC);
	str_to_mac(netinf->ReceiverDeviceMAC,&Netinf_Test.StaRxMAC);

	Prepare_Netinf_Output_Folder(netinf->BaseFolder, netinf->TestFolder, netinf->ReceiverDeviceMAC, netinf->TransmitterDeviceMAC, TestOutputFolder);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	printf("\n\n");
	
	Clock_StartNetinf = console_get_msectime();
	ClockLast = Clock_StartNetinf;

	// When TestDuration is 0 (indefinitely), we stop the test only on "g_StopNetinf"
	if (netinf->TestDuration != 0)
	{
		Clock_EndNetinf = Clock_StartNetinf + (netinf->TestDuration * 1000);
	}

	g_bStopNetinf = FALSE;
	
	SampleIteration = 0;
	SamplingState = eSamplingState_Recover;
	bLastIteration = FALSE;

	
	while (bLastIteration == FALSE)
	{
		SampleIteration++;
		bLastIteration	= (((netinf->TestDuration != 0) && (console_get_msectime() > Clock_EndNetinf)) ||
							(g_bStopNetinf));

		fprintf(stdout,"\rgetting counters sample #%d...   ",SampleIteration);
		fflush(stdout);

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get Netinf results from the Receiver Device
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		LOG_INFO("Get Netinf results from the Receiver Device strIP=%s",ReceiverDeviceIP);

		// Create the structure for GetDataModel()
		strcpy(getDataModel->DeviceIP, ReceiverDeviceIP);
		getDataModel->bHasNetworkCardIP = strlen(SelectedNetworkCardIP)>0;
		strcpy(getDataModel->NetworkCardIP, SelectedNetworkCardIP);
		getDataModel->IncludeBranch_Size = 3;
		getDataModel->ExcludeBranch_Size = 5;
		
		strcpy(getDataModel->IncludeBranch_Array[0].Name, Node_DeviceInfo);
		strcpy(getDataModel->IncludeBranch_Array[1].Name, Node_Time);
		strcpy(getDataModel->IncludeBranch_Array[2].Name, Node_Interface);

		strcpy(getDataModel->ExcludeBranch_Array[0].Name, Nodes_Interface_PeriodicStatistics);
		strcpy(getDataModel->ExcludeBranch_Array[1].Name, Nodes_Interface_PhyDiagConfig);
		strcpy(getDataModel->ExcludeBranch_Array[2].Name, Nodes_Interface_Alarms);
		strcpy(getDataModel->ExcludeBranch_Array[3].Name, Nodes_Interface_Debug);
		strcpy(getDataModel->ExcludeBranch_Array[4].Name, Nodes_BitLoadingTable);

		if (Get_Data_Model(getDataModel) == FALSE)
		{
			// Error getting the XML file from the device
			if (netinf->bStopOnReadingError)
			{
				sprintf(netinf->ErrorDescription,"Failed to run Get_Data_Model(). %s",getDataModel->ErrorDescription);
				LOG_INFO(netinf->ErrorDescription);
				free(getDataModel);
				return eGHN_LIB_STAT_HTTP_REQUEST_FAILED;
			}

			Netinf_Update_Status_File(TestOutputFolder, "Failed to read the channel information from the device");

			SamplingState = eSamplingState_Recover;
			continue;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		//SaveDataModelBufferIntoFile(ReceiverOutputFolder, getDataModel->DataModel_Buffer);

		if (XMLParser_OpenXMLBuffer((char*)getDataModel->DataModel_Buffer, &XMLHandler) != eXMLPARSER_STAT_SUCCESS)
		{
			if (netinf->bStopOnReadingError)
			{
				sprintf(netinf->ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
				LOG_INFO(netinf->ErrorDescription);
				free(getDataModel);
				return eGHN_LIB_STAT_FAILURE;
			}

			Netinf_Update_Status_File(TestOutputFolder, "Failed to parse the channel information");

			SamplingState = eSamplingState_Recover;
			continue;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Check if the device was reset from last sampling
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if (XMLParser_GetNodeAttributeByNodePath(XMLHandler,Node_DeviceInfo,"UpTime",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
		{
			if (netinf->bStopOnReadingError)
			{
				sprintf(netinf->ErrorDescription,"Failed to read the device up-time");
				LOG_INFO(netinf->ErrorDescription);
				XMLParser_CloseXML(&XMLHandler);
				free(getDataModel);
				return eGHN_LIB_STAT_FAILURE;
			}

			Netinf_Update_Status_File(TestOutputFolder, "Failed to parse the channel information");

			XMLParser_CloseXML(&XMLHandler);
			SamplingState = eSamplingState_Recover;
			continue;
		}
		SamplingCurrentUpTime = atoi(Value);

		if (SamplingCurrentUpTime < SamplingUpTime)
		{

			Netinf_Update_Status_File(TestOutputFolder, "Device had reset");

			XMLParser_CloseXML(&XMLHandler);
			SamplingUpTime = 0;
			SamplingState = eSamplingState_Recover;
			continue;
		}
		SamplingUpTime = SamplingCurrentUpTime;
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get NodeTypeActiveMedium
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if ((XMLParser_GetNodeAttributeByNodePath(XMLHandler,Node_Interface,"NodeTypeActiveMedium",&Value[0]) == eXMLPARSER_STAT_SUCCESS) &&
			(strcmp(Value,"Coax")==0))
		{
			Netinf_Test.ActiveMedium = eNetinf_ActiveMedium_Coax;
		}
		else
		{
			Netinf_Test.ActiveMedium = eNetinf_ActiveMedium_PowerLine;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get NodeTypeConfiguration
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		if ((XMLParser_GetNodeAttributeByNodePath(XMLHandler,Node_Interface,"NodeTypeConfiguration",&Value[0]) == eXMLPARSER_STAT_SUCCESS) &&
			(strcmp(Value,"SISO")==0))
		{
			Netinf_Test.NodeTypeConfiguration = eNetinf_NodeTypeConfiguration_SISO;
		}
		else
		{
			Netinf_Test.NodeTypeConfiguration = eNetinf_NodeTypeConfiguration_MIMO;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		GHN_LIB_STAT = Netinf_ReadCEParameters(XMLHandler,&Netinf_Test, eNetinf_ParsingMethod_Online, NULL , 0, NULL, (ceilf(netinf->SamplingPeriods) != netinf->SamplingPeriods));

		XMLParser_CloseXML(&XMLHandler);
	
		if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
		{
			if (netinf->bStopOnReadingError)
			{
				sprintf(netinf->ErrorDescription,"Failed to read the Channel Estimation Parameters");
				LOG_INFO(netinf->ErrorDescription);
				free(getDataModel);
				return eGHN_LIB_STAT_FAILURE;
			}

			Netinf_Update_Status_File(TestOutputFolder, "Failed to read the Channel-Estimation Parameters");

			SamplingState = eSamplingState_Recover;
			continue;
		}

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Check if the device entry was init from last sampling (CodeRate=0 or TotalBitLoading=0)
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		bNeedContinue = FALSE;
		
		if (netinf->bQueryOnlyLocalDevice == FALSE)
		{
			for (Intervals=1;Intervals<=Netinf_Test.counters_curr.IntervalCEParametersNumberOfEntries;Intervals++)
			{
				if ((Netinf_Test.counters_curr.IntervalCEParameters[Intervals-1].CodeRate == 0) ||
					(Netinf_Test.counters_curr.IntervalCEParameters[Intervals-1].TotalBitLoading == 0))
				{
					bNeedContinue = TRUE;
				}
			}
		}

		if (bNeedContinue == TRUE)
		{
			Netinf_Update_Status_File(TestOutputFolder, "Not enough traffic on this channel");

			SamplingState = eSamplingState_Recover;
			OS_Sleep(1000); // After a failure, take two samples with 1 sec gap
			continue;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		Netinf_PrintCEParameters(&Netinf_Test);

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// BitLoading Table
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		bIncludeBitLoadTable = ((g_bIncludeBitLoadTableInTheNextIteration) ||
								((netinf->BitLoadingTableSamplingPeriods > 0) && ((SampleIteration % netinf->BitLoadingTableSamplingPeriods)==0)));
		// or ==1 and % n == 0

		if (bIncludeBitLoadTable == TRUE)
		{
			g_bIncludeBitLoadTableInTheNextIteration = FALSE;

			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			// Get Netinf Bit Loading Table from the Receiver Device
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
			LOG_INFO("Get Netinf Bit Loading Table from the Receiver Device strIP=%s",ReceiverDeviceIP);

			// Create the structure for GetDataModel()
			strcpy(getDataModel->DeviceIP, ReceiverDeviceIP);
			getDataModel->bHasNetworkCardIP = strlen(SelectedNetworkCardIP)>0;
			strcpy(getDataModel->NetworkCardIP, SelectedNetworkCardIP);
			getDataModel->IncludeBranch_Size = 1;
			getDataModel->ExcludeBranch_Size = 0;

			sprintf(getDataModel->IncludeBranch_Array[0].Name, "%s.%d",
								Nodes_Interface_AssociatedDevice,
								Netinf_Test.counters_curr.X_0023C7_Index);

			if (Get_Data_Model(getDataModel) == FALSE)
			{
				// Error getting the XML file from the device
				if (netinf->bStopOnReadingError)
				{
					sprintf(netinf->ErrorDescription,"Failed to run Get_Data_Model(). %s",getDataModel->ErrorDescription);
					LOG_INFO(netinf->ErrorDescription);
					free(getDataModel);
					return eGHN_LIB_STAT_HTTP_REQUEST_FAILED;
				}

				Netinf_Update_Status_File(TestOutputFolder, "Failed to read the Bit-Loading Parameters from the device");

				SamplingState = eSamplingState_Recover;
				continue;
			}
			// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

			//SaveDataModelBufferIntoFile(ReceiverOutputFolder, getDataModel->DataModel_Buffer);

			if (XMLParser_OpenXMLBuffer((char*)getDataModel->DataModel_Buffer, &XMLHandler) != eXMLPARSER_STAT_SUCCESS)
			{
				if (netinf->bStopOnReadingError)
				{
					sprintf(netinf->ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
					LOG_INFO(netinf->ErrorDescription);
					free(getDataModel);
					return eGHN_LIB_STAT_FAILURE;
				}

				Netinf_Update_Status_File(TestOutputFolder, "Failed to parse the Bit-Loading Parameters");

				SamplingState = eSamplingState_Recover;
				continue;
			}

			GHN_LIB_STAT = Netinf_ReadCEParameters_BitLoading(XMLHandler,&Netinf_Test);

			if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
			{
				bIncludeBitLoadTable = FALSE;
			}
			
			XMLParser_CloseXML(&XMLHandler);
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		if (SamplingState != eSamplingState_Recover)
		{
			Netinf_Test.ThresholdNumberOfSegments = ThresholdNumberOfSegments;

			if ((GHN_LIB_STAT = Netinf_CalcCEParameters(&Netinf_Test,&Netinf_Result, (SamplingState == eSamplingState_After_Recover))) != eGHN_LIB_STAT_SUCCESS)
			{
				if (netinf->bStopOnReadingError)
				{
					sprintf(netinf->ErrorDescription,"Failed to calculate the Channel Estimation Parameters");
					LOG_INFO(netinf->ErrorDescription);
					free(getDataModel);
					return GHN_LIB_STAT;
				}

				Netinf_Update_Status_File(TestOutputFolder, "Failed to calculate the Channel Estimation Parameters");

				continue;
			}

			Netinf_PrintCalcCEParameters(&Netinf_Result);

			Netinf_SaveCalcCEParameters(&Netinf_Result, TestOutputFolder, netinf->bGenerateTraffic, netinf->bSavePERGraph, netinf->bSaveAdvancedGraphs, (SamplingState == eSamplingState_After_Recover), bIncludeBitLoadTable);
			Netinf_Update_Status_File(TestOutputFolder, "OK");
		}

		if (SamplingState == eSamplingState_Recover)
		{
			SamplingState = eSamplingState_After_Recover;
		}
		else
		{
			SamplingState = eSamplingState_Normal;
		}

		
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Sleep for "netinf->SamplingPeriods" sec between two iterations
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		while ((g_bStopNetinf == FALSE) &&
				(console_get_msectime() - ClockLast < (long)(netinf->SamplingPeriods*(1000-200))))
		{
			OS_Sleep(100);
		}
		while ((g_bStopNetinf == FALSE) &&
				(console_get_msectime() - ClockLast < (long)(netinf->SamplingPeriods*(1000-20))))
		{
			OS_Sleep(10);
		}
		while ((g_bStopNetinf == FALSE) &&
			(console_get_msectime() - ClockLast < (long)(netinf->SamplingPeriods*(1000)-5)))
		{
		}
		ClockLast = console_get_msectime();
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Stop the Netinf
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (netinf->bGenerateTraffic)
	{
		strcpy(setDataModel.DeviceIP, TransmitterDeviceIP);
		setDataModel.SetParameter_Size = 2;

		strcpy(setDataModel.SetParameter_Array[0].Name, Node_StartPhyDiagTest);
		strcpy(setDataModel.SetParameter_Array[0].Value, "false");

		strcpy(setDataModel.SetParameter_Array[1].Name, Node_PhyDiagReceiverMAC);
		strcpy(setDataModel.SetParameter_Array[1].Value, "");

		if (Set_Data_Model(&setDataModel) == FALSE)
		{
			// Error getting the XML file from the device
			if (netinf->bStopOnReadingError)
			{
				sprintf(netinf->ErrorDescription,"Failed to stop netinf %s",setDataModel.ErrorDescription);
				LOG_INFO(netinf->ErrorDescription);
				free(getDataModel);
				return eGHN_LIB_STAT_FAILED_STOP_NETINF;
			}
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	free(getDataModel);
	return eGHN_LIB_STAT_SUCCESS;
}

DllExport eGHN_LIB_STAT Ghn_Get_BBT_Data_Model(sGet_BBT_Data_Model_Information* getBBTDataModel)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	ip_address_t				NetworkCardIP=0;

	ip_address_t				DeviceIP=0;

	ip_address_t				DeviceIP1=0;
	ip_address_t				DeviceIP2=0;
	char						strDeviceIP1[GHN_LIB_IP_ADDRESS_LEN];
	char						strDeviceIP2[GHN_LIB_IP_ADDRESS_LEN];

	eDeviceState				DeviceState;
	bool						bLocalDevice;
	cg_stat_t					cg_stat;

	sGet_Data_Mode_lnformation*	getDataModel;
	Layer2Connection			layer2Connection;
	UINT32						i;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "getDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	getDataModel = (sGet_Data_Mode_lnformation*)malloc(sizeof(sGet_Data_Mode_lnformation));
	if (getDataModel == NULL)
	{
		strcpy(getBBTDataModel->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(getDataModel,0x00,sizeof(sGet_Data_Mode_lnformation));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if (getBBTDataModel->Connection.bHasAdapterIP)
	{
		if ((GHN_LIB_STAT = Open_Layer2_Connection(	&getBBTDataModel->Connection,
													ETHER_TYPE_GHN,
													&layer2Connection,
													getBBTDataModel->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			free(getDataModel);
			return GHN_LIB_STAT;
		}

		// Get the IP address of the local device
		if ((cg_stat=QueryDeviceIP(&layer2Connection, &layer2Connection.m_MAC_Address,&DeviceState, &bLocalDevice, &DeviceIP1,&DeviceIP2)) != CG_STAT_SUCCESS)
		{
			sprintf(getBBTDataModel->ErrorDescription,"Failed to query device "MAC_ADDR_FMT,MAC_ADDR(layer2Connection.m_MAC_Address.macAddress));
			LOG_INFO(getBBTDataModel->ErrorDescription);
			Close_Layer2_Connection(&layer2Connection);
			free(getDataModel);
			return ConvertReturnStatus(cg_stat);
		}

		Close_Layer2_Connection(&layer2Connection);

		NetworkCardIP = str_to_ip(getBBTDataModel->Connection.AdapterIP);

		// Check if one of the IPs is relevant for this network card
		if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(DeviceIP1,NetworkCardIP))
		{
			DeviceIP = DeviceIP1;
		}
		else if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(DeviceIP2,NetworkCardIP))
		{
			DeviceIP = DeviceIP2;
		}
		else
		{
			// None of the IPs in the Subnet-Mask of the select network interface
			ip_to_str(DeviceIP1,strDeviceIP1);
			ip_to_str(DeviceIP2,strDeviceIP2);

			if (DeviceIP2 == 0x00)
			{
				sprintf(getBBTDataModel->ErrorDescription,"The device %s with IP=%s is not in the Subnet-Mask of the select network interface",
					getBBTDataModel->Connection.DeviceMAC,
					strDeviceIP1);
			}
			else if (DeviceIP1 == 0x00)
			{
				sprintf(getBBTDataModel->ErrorDescription,"The device %s with IP=%s is not in the Subnet-Mask of the select network interface",
					getBBTDataModel->Connection.DeviceMAC,
					strDeviceIP2);
			}
			else
			{
				sprintf(getBBTDataModel->ErrorDescription,"The device %s with IP1=%s and IP2=%s is not in the Subnet-Mask of the select network interface",
					getBBTDataModel->Connection.DeviceMAC,
					strDeviceIP1,
					strDeviceIP2);
			}

			LOG_INFO(getBBTDataModel->ErrorDescription);
			free(getDataModel);
			return eGHN_LIB_STAT_SUBNET_MASK_MISMATCH;
		}

		ip_to_str(DeviceIP,getDataModel->DeviceIP);
	}
	else
	{
		strcpy(getDataModel->DeviceIP,getBBTDataModel->Connection.DeviceIP);
	}

	getDataModel->bHasNetworkCardIP = strlen(getBBTDataModel->Connection.SelectedNetworkCardIP)>0;
	strcpy(getDataModel->NetworkCardIP, getBBTDataModel->Connection.SelectedNetworkCardIP);

	getDataModel->IncludeBranch_Size = getBBTDataModel->IncludeBranch_Size;
	for(i=0;i<getBBTDataModel->IncludeBranch_Size;i++)
	{
		memcpy(&getDataModel->IncludeBranch_Array[i], &getBBTDataModel->IncludeBranch_Array[i], sizeof(sBranch_Info));
	}

	getDataModel->ExcludeBranch_Size = getBBTDataModel->ExcludeBranch_Size;
	for(i=0;i<getBBTDataModel->ExcludeBranch_Size;i++)
	{
		memcpy(&getDataModel->ExcludeBranch_Array[i], &getBBTDataModel->ExcludeBranch_Array[i], sizeof(sBranch_Info));
	}

	LOG_INFO("Started...");

	if (Get_Data_Model(getDataModel) == FALSE)
	{
		// Error getting the XML file from the device
		sprintf(getBBTDataModel->ErrorDescription,"Failed to run Get_Data_Model(). %s",getDataModel->ErrorDescription);
		LOG_INFO(getBBTDataModel->ErrorDescription);
		free(getDataModel);
		return eGHN_LIB_STAT_HTTP_REQUEST_FAILED;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// Copy the Data-Model results
	getBBTDataModel->DataModel_Size = getDataModel->DataModel_Size;
	memcpy(getBBTDataModel->DataModel_Buffer, getDataModel->DataModel_Buffer, getDataModel->DataModel_Size);

	free(getDataModel);
	return eGHN_LIB_STAT_SUCCESS;
}

DllExport eGHN_LIB_STAT Ghn_Set_BBT_Data_Model(sSet_BBT_Data_Model_Information* setBBTDataModel)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	ip_address_t				NetworkCardIP=0;

	ip_address_t				DeviceIP=0;

	ip_address_t				DeviceIP1=0;
	ip_address_t				DeviceIP2=0;
	char						strDeviceIP1[GHN_LIB_IP_ADDRESS_LEN];
	char						strDeviceIP2[GHN_LIB_IP_ADDRESS_LEN];

	eDeviceState				DeviceState;
	bool						bLocalDevice;
	cg_stat_t					cg_stat;

	sSet_Data_Mode_lnformation*	setDataModel;
	Layer2Connection			layer2Connection;
	UINT32						i;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "setDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	setDataModel = (sSet_Data_Mode_lnformation*)malloc(sizeof(sSet_Data_Mode_lnformation));
	if (setDataModel == NULL)
	{
		strcpy(setBBTDataModel->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(setDataModel,0x00,sizeof(sSet_Data_Mode_lnformation));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if (setBBTDataModel->Connection.bHasAdapterIP)
	{
		if ((GHN_LIB_STAT = Open_Layer2_Connection(	&setBBTDataModel->Connection,
													ETHER_TYPE_GHN,
													&layer2Connection,
													setBBTDataModel->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
		{
			free(setDataModel);
			return GHN_LIB_STAT;
		}

		// Get the IP address of the local device
		if ((cg_stat=QueryDeviceIP(&layer2Connection, &layer2Connection.m_MAC_Address,&DeviceState, &bLocalDevice, &DeviceIP1,&DeviceIP2)) != CG_STAT_SUCCESS)
		{
			sprintf(setBBTDataModel->ErrorDescription,"Failed to query device "MAC_ADDR_FMT,MAC_ADDR(layer2Connection.m_MAC_Address.macAddress));
			LOG_INFO(setBBTDataModel->ErrorDescription);
			Close_Layer2_Connection(&layer2Connection);
			free(setDataModel);
			return ConvertReturnStatus(cg_stat);
		}

		Close_Layer2_Connection(&layer2Connection);

		NetworkCardIP = str_to_ip(setBBTDataModel->Connection.AdapterIP);

		// Check if one of the IPs is relevant for this network card
		if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(DeviceIP1,NetworkCardIP))
		{
			DeviceIP = DeviceIP1;
		}
		else if (Is_DeviceIP_In_The_Subnet_Of_NetworkInterface(DeviceIP2,NetworkCardIP))
		{
			DeviceIP = DeviceIP2;
		}
		else
		{
			// None of the IPs in the Subnet-Mask of the select network interface
			ip_to_str(DeviceIP1,strDeviceIP1);
			ip_to_str(DeviceIP2,strDeviceIP2);

			if (DeviceIP2 == 0x00)
			{
				sprintf(setBBTDataModel->ErrorDescription,"The device %s with IP=%s is not in the Subnet-Mask of the select network interface",
					setBBTDataModel->Connection.DeviceMAC,
					strDeviceIP1);
			}
			else if (DeviceIP1 == 0x00)
			{
				sprintf(setBBTDataModel->ErrorDescription,"The device %s with IP=%s is not in the Subnet-Mask of the select network interface",
					setBBTDataModel->Connection.DeviceMAC,
					strDeviceIP2);
			}
			else
			{
				sprintf(setBBTDataModel->ErrorDescription,"The device %s with IP1=%s and IP2=%s is not in the Subnet-Mask of the select network interface",
					setBBTDataModel->Connection.DeviceMAC,
					strDeviceIP1,
					strDeviceIP2);
			}

			LOG_INFO(setBBTDataModel->ErrorDescription);
			free(setDataModel);
			return eGHN_LIB_STAT_SUBNET_MASK_MISMATCH;
		}

		ip_to_str(DeviceIP,setDataModel->DeviceIP);
	}
	else
	{
		strcpy(setDataModel->DeviceIP,setBBTDataModel->Connection.DeviceIP);
	}

	setDataModel->bHasNetworkCardIP = strlen(setBBTDataModel->Connection.SelectedNetworkCardIP)>0;
	strcpy(setDataModel->NetworkCardIP, setBBTDataModel->Connection.SelectedNetworkCardIP);

	setDataModel->SetParameter_Size = setBBTDataModel->SetParameter_Size;
	for(i=0;i<setBBTDataModel->SetParameter_Size;i++)
	{
		memcpy(&setDataModel->SetParameter_Array[i], &setBBTDataModel->SetParameter_Array[i], sizeof(sParameter_Info));
	}

	LOG_INFO("Started...");

	if (Set_Data_Model(setDataModel) == FALSE)
	{
		// Error set the data model
		sprintf(setBBTDataModel->ErrorDescription,"Failed to run Set_Data_Model(). %s",setDataModel->ErrorDescription);
		LOG_INFO(setBBTDataModel->ErrorDescription);
		free(setDataModel);
		return eGHN_LIB_STAT_HTTP_REQUEST_FAILED;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// Copy the Data-Model results
	setBBTDataModel->DataModel_Size = setDataModel->DataModel_Size;
	memcpy(setBBTDataModel->DataModel_Buffer, setDataModel->DataModel_Buffer, setDataModel->DataModel_Size);

	free(setDataModel);
	return eGHN_LIB_STAT_SUCCESS;
}

// Add ".1" After the "NodePrefix"
// For Example (When NodePrefix is "Device.Ghn.Interface"): "Device.Ghn.Interface.AttributeName" -> "Device.Ghn.Interface.1.AttributeName"
void GetBBT_FixPath(char* strInput, char* strOutput, char *NodePrefix)
{
	if (strncmp(strInput, NodePrefix, strlen(NodePrefix)) == 0)
	{
		sprintf(strOutput, "%s.1", NodePrefix);

		if (strInput[strlen(NodePrefix)] != '\0')
		{
			// Copy the rest of the path (after "NodePrefix")
			strcat(strOutput, &strInput[strlen(NodePrefix)]);
		}
	}
	else
	{
		strcpy(strOutput, strInput);
	}
}

DllExport eGHN_LIB_STAT Ghn_Get_BBT_Data_Model_One_Parameter(sBBT_Data_Model_One_Parameter_Information* BBTDataModelOneParameter)
{
	sGet_BBT_Data_Model_Information*	getBBTDataModel;
	eGHN_LIB_STAT						GHN_LIB_STAT;

	XMLParserHandler					XMLHandler;
	char								Value[XML_PARSER_MAX_STRING];

	char								AttributePath_Temp[GHN_LIB_STRING_256];
	char								AttributePath_Fixed[GHN_LIB_STRING_256];

	LOG_INFO("Started...");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "getBBTDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	getBBTDataModel = (sGet_BBT_Data_Model_Information*)malloc(sizeof(sGet_BBT_Data_Model_Information));
	if (getBBTDataModel == NULL)
	{
		strcpy(BBTDataModelOneParameter->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(getBBTDataModel,0x00,sizeof(sGet_BBT_Data_Model_Information));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	getBBTDataModel->Connection = BBTDataModelOneParameter->Connection;

	getBBTDataModel->IncludeBranch_Size = 1;

	strcpy(AttributePath_Temp, BBTDataModelOneParameter->AttributePath);
	GetBBT_FixPath(AttributePath_Temp, AttributePath_Fixed, Node_Interface);
	
	strcpy(AttributePath_Temp, AttributePath_Fixed);
	GetBBT_FixPath(AttributePath_Temp, AttributePath_Fixed, Node_Interface_1_AssociatedDevice);

	sprintf(getBBTDataModel->IncludeBranch_Array[0].Name,
			"%s.%s",
			AttributePath_Fixed,
			BBTDataModelOneParameter->AttributeName);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model(getBBTDataModel)) != eGHN_LIB_STAT_SUCCESS)
	{
		// Error getting the XML file from the device
		sprintf(BBTDataModelOneParameter->ErrorDescription,"Failed to run Get_BBT_Data_Model(). %s",getBBTDataModel->ErrorDescription);
		LOG_INFO(BBTDataModelOneParameter->ErrorDescription);
		free(getBBTDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}

	if (XMLParser_OpenXMLBuffer((char*)getBBTDataModel->DataModel_Buffer, &XMLHandler) != eXMLPARSER_STAT_SUCCESS)
	{
		sprintf(BBTDataModelOneParameter->ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
		LOG_INFO(BBTDataModelOneParameter->ErrorDescription);
		free(getBBTDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the result
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (XMLParser_GetNodeAttributeByNodePath(	XMLHandler,
												BBTDataModelOneParameter->AttributePath,
												BBTDataModelOneParameter->AttributeName,
												&Value[0]) != eXMLPARSER_STAT_SUCCESS)
	{
		sprintf(BBTDataModelOneParameter->ErrorDescription,"Call XMLParser_GetNodeAttributeByNodePath() Failed");
		LOG_INFO(BBTDataModelOneParameter->ErrorDescription);
		free(getBBTDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	strcpy(BBTDataModelOneParameter->AttributeValue, Value);

	XMLParser_CloseXML(&XMLHandler);
	free(getBBTDataModel);

	return eGHN_LIB_STAT_SUCCESS;
}

DllExport eGHN_LIB_STAT Ghn_Set_BBT_Data_Model_One_Parameter(sBBT_Data_Model_One_Parameter_Information* BBTDataModelOneParameter)
{
	sSet_BBT_Data_Model_Information*	setBBTDataModel;
	eGHN_LIB_STAT						GHN_LIB_STAT;

	XMLParserHandler					XMLHandler;
	char								Value[XML_PARSER_MAX_STRING];
	int									ResponseStatus_Code;

	char								AttributePath_Temp[GHN_LIB_STRING_256];
	char								AttributePath_Fixed[GHN_LIB_STRING_256];

	LOG_INFO("Started...");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "setBBTDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	setBBTDataModel = (sSet_BBT_Data_Model_Information*)malloc(sizeof(sSet_BBT_Data_Model_Information));
	if (setBBTDataModel == NULL)
	{
		strcpy(BBTDataModelOneParameter->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(setBBTDataModel,0x00,sizeof(sSet_BBT_Data_Model_Information));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	setBBTDataModel->Connection = BBTDataModelOneParameter->Connection;

	setBBTDataModel->SetParameter_Size = 1;

	strcpy(AttributePath_Temp, BBTDataModelOneParameter->AttributePath);
	GetBBT_FixPath(AttributePath_Temp, AttributePath_Fixed, Node_Interface);

	strcpy(AttributePath_Temp, AttributePath_Fixed);
	GetBBT_FixPath(AttributePath_Temp, AttributePath_Fixed, Node_Interface_1_AssociatedDevice);

	sprintf(setBBTDataModel->SetParameter_Array[0].Name,
			"%s.%s",
			AttributePath_Fixed,
			BBTDataModelOneParameter->AttributeName);

	sprintf(setBBTDataModel->SetParameter_Array[0].Value,
			"%s",
			BBTDataModelOneParameter->AttributeValue);

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model(setBBTDataModel)) != eGHN_LIB_STAT_SUCCESS)
	{
		// Error setting the parameter on the device
		sprintf(BBTDataModelOneParameter->ErrorDescription,"Failed to run Set_BBT_Data_Model(). %s",setBBTDataModel->ErrorDescription);
		LOG_INFO(BBTDataModelOneParameter->ErrorDescription);
		free(setBBTDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}

	if (XMLParser_OpenXMLBuffer((char*)setBBTDataModel->DataModel_Buffer, &XMLHandler) != eXMLPARSER_STAT_SUCCESS)
	{
		sprintf(BBTDataModelOneParameter->ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
		LOG_INFO(BBTDataModelOneParameter->ErrorDescription);
		free(setBBTDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the result
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (XMLParser_GetNodeAttributeByNodePath(XMLHandler,Node_ap_stat_ResponseStatus,"Code",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
	{
		sprintf(BBTDataModelOneParameter->ErrorDescription,"Call XMLParser_GetNodeAttributeByNodePath() Failed");
		LOG_INFO(BBTDataModelOneParameter->ErrorDescription);
		XMLParser_CloseXML(&XMLHandler);
		free(setBBTDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Check the Status code of the HTTP-Request
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	ResponseStatus_Code = atoi(Value);

	if (ResponseStatus_Code == 0)
	{
		BBTDataModelOneParameter->bNeedApplyParameterSetting = FALSE;
	}
	else if (ResponseStatus_Code == 1)
	{
		BBTDataModelOneParameter->bNeedApplyParameterSetting = TRUE;
	}
	else if (ResponseStatus_Code > 1)
	{
		sprintf(BBTDataModelOneParameter->ErrorDescription,"The device reply with error status code(%d)", ResponseStatus_Code);
		LOG_INFO(BBTDataModelOneParameter->ErrorDescription);
		XMLParser_CloseXML(&XMLHandler);
		free(setBBTDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	
	XMLParser_CloseXML(&XMLHandler);
	free(setBBTDataModel);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_GetImageHeaderFrom_BIN_File_t(BIN_file_t* BIN_file, sImage_Header_Information* ImageHeaderInformation)
{
	BIN_file_header_t*	BIN_file_header_p;

	if (GetHeader(BIN_file, &BIN_file_header_p) == FALSE)
	{
		return eGHN_LIB_STAT_FAILURE;
	}

	ImageHeaderInformation->HeaderVersion = letohs(BIN_file_header_p->HeaderVersion);
	memset(ImageHeaderInformation->ModelName, 0x00, sizeof(ImageHeaderInformation->ModelName));
	memset(ImageHeaderInformation->FW_Version, 0x00, sizeof(ImageHeaderInformation->FW_Version));
	memset(ImageHeaderInformation->ConfigurationName, 0x00, sizeof(ImageHeaderInformation->ConfigurationName));
	memset(ImageHeaderInformation->BuildImageCreationTime, 0x00, sizeof(ImageHeaderInformation->BuildImageCreationTime));
	
	if (ImageHeaderInformation->HeaderVersion < 2)
	{
		strncpy(ImageHeaderInformation->ModelName, BIN_file_header_p->ModelName, sizeof(BIN_file_header_p->ModelName));
	}
	else
	{
		strncpy(ImageHeaderInformation->ModelName, BIN_file_header_p->ModelNameExt, sizeof(BIN_file_header_p->ModelNameExt));
	}
	
	strncpy(ImageHeaderInformation->FW_Version, BIN_file_header_p->FW_Version, sizeof(BIN_file_header_p->FW_Version));
	strncpy(ImageHeaderInformation->ConfigurationName, BIN_file_header_p->ConfigurationName, sizeof(BIN_file_header_p->ConfigurationName));
	strncpy(ImageHeaderInformation->BuildImageCreationTime, BIN_file_header_p->BuildImageCreationTime, sizeof(BIN_file_header_p->BuildImageCreationTime));
	ImageHeaderInformation->TotalImageSize = letohl(BIN_file_header_p->TotalImageSize);
	
	sprintf(ImageHeaderInformation->FW_Signature, SIG_FMT, SIG_ADDR(BIN_file_header_p->FW_Signature));

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Image_Header_From_File(sGet_Image_Header_From_File_Information* getImageHeader)
{
	BIN_file_t*			BIN_file;

	// Init
	BIN_file = (BIN_file_t*)malloc(2*sizeof(UINT32) + MAX_BIN_FILE_SIZE);
	BIN_file->BIN_file_Sections_Max_Length = MAX_BIN_FILE_SIZE;
	BIN_file->BIN_file_Sections_Length = 0;

	LOG_INFO("Started...");

	// Load FW BIN file into 
	if (Load_BIN_file(BIN_file, getImageHeader->FW_BIN_FileName) == FALSE)
	{
		sprintf(getImageHeader->ErrorDescription,"Failed to open the FW BIN file: %s", getImageHeader->FW_BIN_FileName);
		LOG_INFO(getImageHeader->ErrorDescription);
		free(BIN_file);
		return eGHN_LIB_STAT_FAILURE;
	}

	if (Validate_BIN_file(BIN_file) == FALSE)
	{
		sprintf(getImageHeader->ErrorDescription,"FW BIN file is corrupted: %s", getImageHeader->FW_BIN_FileName);
		LOG_INFO(getImageHeader->ErrorDescription);
		free(BIN_file);
		return eGHN_LIB_STAT_FAILURE;
	}

	if (Ghn_GetImageHeaderFrom_BIN_File_t(BIN_file, &(getImageHeader->ImageHeaderInformation)) != eGHN_LIB_STAT_SUCCESS)
	{
		sprintf(getImageHeader->ErrorDescription,"GetHeader() failed");
		LOG_INFO(getImageHeader->ErrorDescription);
		free(BIN_file);
		return eGHN_LIB_STAT_FAILURE;
	}

	free(BIN_file);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Network_Encryption_Mode(sNetwork_Encryption_Mode_Information* NetworkEncryptionMode)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = NetworkEncryptionMode->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, Node_X_00C5D9_EncryptionStatus);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(NetworkEncryptionMode->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	if (OS_STRICMP(BBTDataModelOneParameter.AttributeValue, "TRUE") == 0)
	{
		NetworkEncryptionMode->Mode = TRUE;
	}
	else
	{
		NetworkEncryptionMode->Mode = FALSE;
	}

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Set_Network_Encryption_Mode(sNetwork_Encryption_Mode_Information* NetworkEncryptionMode)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = NetworkEncryptionMode->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, Node_X_00C5D9_EncryptionStatus);

	if (NetworkEncryptionMode->Mode == TRUE)
	{
		strcpy(BBTDataModelOneParameter.AttributeValue, "true");
	}
	else
	{
		strcpy(BBTDataModelOneParameter.AttributeValue, "false");
	}

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(NetworkEncryptionMode->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	NetworkEncryptionMode->bNeedApplyParameterSetting = BBTDataModelOneParameter.bNeedApplyParameterSetting;

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Network_Device_Password(sNetwork_Device_Password_Information* NetworkDevicePassword)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = NetworkDevicePassword->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, Node_GhnNetworkPassword);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(NetworkDevicePassword->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	strcpy(NetworkDevicePassword->DevicePassword, BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Set_Network_Device_Password(sNetwork_Device_Password_Information* NetworkDevicePassword)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = NetworkDevicePassword->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, Node_GhnNetworkPassword);

	strcpy(BBTDataModelOneParameter.AttributeValue, NetworkDevicePassword->DevicePassword);

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(NetworkDevicePassword->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	NetworkDevicePassword->bNeedApplyParameterSetting = BBTDataModelOneParameter.bNeedApplyParameterSetting;

	return eGHN_LIB_STAT_SUCCESS;
}


eGHN_LIB_STAT Ghn_Get_Network_Domain_Name(sNetwork_Domain_Name_Information* NetworkDomainName)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = NetworkDomainName->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, Node_DomainName);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(NetworkDomainName->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	strcpy(NetworkDomainName->DomainName, BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Set_Network_Domain_Name(sNetwork_Domain_Name_Information* NetworkDomainName)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = NetworkDomainName->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, Node_DomainName);

	strcpy(BBTDataModelOneParameter.AttributeValue, NetworkDomainName->DomainName);

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(NetworkDomainName->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	NetworkDomainName->bNeedApplyParameterSetting = BBTDataModelOneParameter.bNeedApplyParameterSetting;

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Power_Save_Mode_Status(sPower_Save_Mode_Status_Information* PowerSaveModeStatus)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = PowerSaveModeStatus->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_PowerSaveModeStatus);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(PowerSaveModeStatus->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	if (OS_STRICMP(BBTDataModelOneParameter.AttributeValue, "TRUE") == 0)
	{
		PowerSaveModeStatus->Status = TRUE;
	}
	else
	{
		PowerSaveModeStatus->Status = FALSE;
	}

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Set_Power_Save_Mode_Status(sPower_Save_Mode_Status_Information* PowerSaveModeStatus)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = PowerSaveModeStatus->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_PowerSaveModeStatus);

	if (PowerSaveModeStatus->Status == TRUE)
	{
		strcpy(BBTDataModelOneParameter.AttributeValue, "true");
	}
	else
	{
		strcpy(BBTDataModelOneParameter.AttributeValue, "false");
	}

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(PowerSaveModeStatus->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	PowerSaveModeStatus->bNeedApplyParameterSetting = BBTDataModelOneParameter.bNeedApplyParameterSetting;

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Power_Save_Mode_Link_Down_Timer(sPower_Save_Mode_Link_Down_Timer_Information* PowerSaveModeLinkDownTimer)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = PowerSaveModeLinkDownTimer->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_PowerSaveModeLinkDownTimer);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(PowerSaveModeLinkDownTimer->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	PowerSaveModeLinkDownTimer->Timer = atoi(BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Set_Power_Save_Mode_Link_Down_Timer(sPower_Save_Mode_Link_Down_Timer_Information* PowerSaveModeLinkDownTimer)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = PowerSaveModeLinkDownTimer->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_PowerSaveModeLinkDownTimer);

	sprintf(BBTDataModelOneParameter.AttributeValue, "%d", PowerSaveModeLinkDownTimer->Timer);

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(PowerSaveModeLinkDownTimer->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	PowerSaveModeLinkDownTimer->bNeedApplyParameterSetting = BBTDataModelOneParameter.bNeedApplyParameterSetting;

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Power_Save_Mode_No_Traffic_Timer(sPower_Save_Mode_No_Traffic_Timer_Information* PowerSaveModeNoTrafficTimer)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = PowerSaveModeNoTrafficTimer->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_PowerSaveModeNoTrafficTimer);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(PowerSaveModeNoTrafficTimer->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	PowerSaveModeNoTrafficTimer->Timer = atoi(BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Set_Power_Save_Mode_No_Traffic_Timer(sPower_Save_Mode_No_Traffic_Timer_Information* PowerSaveModeNoTrafficTimer)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = PowerSaveModeNoTrafficTimer->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_PowerSaveModeNoTrafficTimer);

	sprintf(BBTDataModelOneParameter.AttributeValue, "%d", PowerSaveModeNoTrafficTimer->Timer);

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(PowerSaveModeNoTrafficTimer->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	PowerSaveModeNoTrafficTimer->bNeedApplyParameterSetting = BBTDataModelOneParameter.bNeedApplyParameterSetting;

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Master_Selection_Mode(sMaster_Selection_Mode_Information* MasterSelectionMode)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = MasterSelectionMode->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_AMSMode);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(MasterSelectionMode->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	MasterSelectionMode->Mode = atoi(BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Set_Master_Selection_Mode(sMaster_Selection_Mode_Information* MasterSelectionMode)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = MasterSelectionMode->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_AMSMode);

	sprintf(BBTDataModelOneParameter.AttributeValue, "%d", MasterSelectionMode->Mode);

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(MasterSelectionMode->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	MasterSelectionMode->bNeedApplyParameterSetting = BBTDataModelOneParameter.bNeedApplyParameterSetting;

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Device_Name(sDevice_Name_Information* DeviceName)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = DeviceName->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_DeviceInfo);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_DeviceName);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(DeviceName->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	strcpy(DeviceName->DeviceName, BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Set_Device_Name(sDevice_Name_Information* DeviceName)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = DeviceName->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_DeviceInfo);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_DeviceName);

	strcpy(BBTDataModelOneParameter.AttributeValue, DeviceName->DeviceName);

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(DeviceName->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	DeviceName->bNeedApplyParameterSetting = BBTDataModelOneParameter.bNeedApplyParameterSetting;

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Coexistence_Mode(sCoexistence_Mode_Information* CoexistenceMode)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = CoexistenceMode->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface_Debug_Coex);
	strcpy(BBTDataModelOneParameter.AttributeName, Node_Coex_Enable);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(CoexistenceMode->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	CoexistenceMode->Mode = atoi(BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}


eGHN_LIB_STAT Ghn_Set_Coexistence_Mode(sCoexistence_Mode_Information* CoexistenceMode)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = CoexistenceMode->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface_Debug_Coex);
	strcpy(BBTDataModelOneParameter.AttributeName, Node_Coex_Enable);

	sprintf(BBTDataModelOneParameter.AttributeValue, "%d", CoexistenceMode->Mode);

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(CoexistenceMode->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	CoexistenceMode->bNeedApplyParameterSetting = BBTDataModelOneParameter.bNeedApplyParameterSetting;

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Coexistence_Threshold(sCoexistence_Threshold_Information* CoexistenceThreshold)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = CoexistenceThreshold->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface_Debug_Coex);
	strcpy(BBTDataModelOneParameter.AttributeName, Node_Coex_Active_Threshold);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(CoexistenceThreshold->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	CoexistenceThreshold->Threshold = atoi(BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}


eGHN_LIB_STAT Ghn_Set_Coexistence_Threshold(sCoexistence_Threshold_Information* CoexistenceThreshold)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = CoexistenceThreshold->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface_Debug_Coex);
	strcpy(BBTDataModelOneParameter.AttributeName, Node_Coex_Active_Threshold);

	sprintf(BBTDataModelOneParameter.AttributeValue, "%d", CoexistenceThreshold->Threshold);

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(CoexistenceThreshold->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	CoexistenceThreshold->bNeedApplyParameterSetting = BBTDataModelOneParameter.bNeedApplyParameterSetting;

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Utilization_Field(sUtilization_Field_Information* utilization_Field)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = utilization_Field->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_Utilization_Field);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(utilization_Field->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	utilization_Field->Value = atoi(BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Utilization_Alpha_Field(sUtilization_Field_Alpha_Information* utilization_Alpha_Field)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = utilization_Alpha_Field->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_Utilization_Alpha_Field);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(utilization_Alpha_Field->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	utilization_Alpha_Field->Value = atoi(BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Set_Utilization_Alpha_Field(sUtilization_Field_Alpha_Information* utilization_Alpha_Field)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = utilization_Alpha_Field->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_Utilization_Alpha_Field);

	sprintf(BBTDataModelOneParameter.AttributeValue, "%d", utilization_Alpha_Field->Value);

	if ((GHN_LIB_STAT = Ghn_Set_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(utilization_Alpha_Field->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	utilization_Alpha_Field->bNeedApplyParameterSetting = BBTDataModelOneParameter.bNeedApplyParameterSetting;

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_T0_Timer(sT0_Timer_Information* t0_Timer)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = t0_Timer->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_DeviceInfo);
	strcpy(BBTDataModelOneParameter.AttributeName, X_00C5D9_T0_Time_sec);

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(t0_Timer->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	t0_Timer->T0_Timer = atoi(BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_GHN_NN_Number_Of_Interference(sGHN_NN_Number_Of_Interference* GHN_NN_NumInterference)
{
	sBBT_Data_Model_One_Parameter_Information	BBTDataModelOneParameter;
	eGHN_LIB_STAT								GHN_LIB_STAT;

	LOG_INFO("Started...");

	BBTDataModelOneParameter.Connection = GHN_NN_NumInterference->Connection;

	strcpy(BBTDataModelOneParameter.AttributePath, Node_Interface_Debug_GHN_NN);
	strcpy(BBTDataModelOneParameter.AttributeName, "InterferenceNumberOfEntries");

	if ((GHN_LIB_STAT = Ghn_Get_BBT_Data_Model_One_Parameter(&BBTDataModelOneParameter)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(GHN_NN_NumInterference->ErrorDescription, BBTDataModelOneParameter.ErrorDescription);
		return GHN_LIB_STAT;
	}

	GHN_NN_NumInterference->Number_Of_Interference = atoi(BBTDataModelOneParameter.AttributeValue);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Restore_Factory_Default(sRestore_Factory_Default_Information* RestoreFactoryDefault)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	Layer2Connection			layer2Connection;

	LOG_INFO("Started...");

	if ((GHN_LIB_STAT = Open_Layer2_Connection(	&RestoreFactoryDefault->Connection,
												ETHER_TYPE_GHN,
												&layer2Connection,
												RestoreFactoryDefault->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		return GHN_LIB_STAT;
	}

	if (RestoreFactoryDefaultCommandFunction(&layer2Connection) != CG_STAT_SUCCESS)
	{
		LOG_INFO("Call RestoreFactoryDefaultCommandFunction() Failed");
		Close_Layer2_Connection(&layer2Connection);
		return eGHN_LIB_STAT_FAILURE;
	}

	Close_Layer2_Connection(&layer2Connection);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Pair_Device(sPair_Device_Information* PairDevice)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	Layer2Connection			layer2Connection;

	LOG_INFO("Started...");

	if ((GHN_LIB_STAT = Open_Layer2_Connection(	&PairDevice->Connection,
												ETHER_TYPE_GHN,
												&layer2Connection,
												PairDevice->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		return GHN_LIB_STAT;
	}

	if (PairDeviceCommandFunction(&layer2Connection) != CG_STAT_SUCCESS)
	{
		LOG_INFO("Call PairDeviceCommandFunction() Failed");
		Close_Layer2_Connection(&layer2Connection);
		return eGHN_LIB_STAT_FAILURE;
	}

	Close_Layer2_Connection(&layer2Connection);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Unpair_Device(sUnpair_Device_Information* UnpairDevice)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	Layer2Connection			layer2Connection;

	LOG_INFO("Started...");

	if ((GHN_LIB_STAT = Open_Layer2_Connection(	&UnpairDevice->Connection,
												ETHER_TYPE_GHN,
												&layer2Connection,
												UnpairDevice->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		return GHN_LIB_STAT;
	}

	if (UnpairDeviceCommandFunction(&layer2Connection) != CG_STAT_SUCCESS)
	{
		LOG_INFO("Call UnpairDeviceCommandFunction() Failed");
		Close_Layer2_Connection(&layer2Connection);
		return eGHN_LIB_STAT_FAILURE;
	}

	Close_Layer2_Connection(&layer2Connection);

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Link_Statistics(sLink_Statistics_Information* linkStatistics)
{
	sGet_stations_Information		getstation;
	eGHN_LIB_STAT					GHN_LIB_STAT;
	char							DeviceIP[HTTP_LIB_IP_ADDRESS_LEN];
	char							SelectedNetworkCardIP[HTTP_LIB_IP_ADDRESS_LEN]; 

	sGet_Data_Mode_lnformation*		getDataModel;

	XMLParserHandler				XMLHandler;
	
	char							Value[XML_PARSER_MAX_STRING];

	sLink_Statistics_Test			linkStatistics_Test;
	sLink_Statistics_Result			linkStatistics_Result;

	char							ReceiverDeviceIP[GHN_LIB_IP_ADDRESS_LEN];

	LOG_INFO("Started...");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// We have to get the IP-Address of the device from "AdapterIP" and "DeviceMAC" before we can continue
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (linkStatistics->Connection.bHasAdapterIP == TRUE)
	{
		memset(&getstation, 0x00, sizeof(sGet_stations_Information));

		// Duplicate the "sConnection_Information" structure
		memcpy(&getstation.Connection, &linkStatistics->Connection, sizeof(sConnection_Information));

		if ((GHN_LIB_STAT = Ghn_Get_Stations(&getstation)) != eGHN_LIB_STAT_SUCCESS)
		{
			strcpy(linkStatistics->ErrorDescription,"Failed to get the stations list");
			return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
		}

		if ((GHN_LIB_STAT = Get_IP_Address_From_Station_List(&getstation, linkStatistics->Connection.DeviceMAC, DeviceIP)) != eGHN_LIB_STAT_SUCCESS)
		{
			strcpy(linkStatistics->ErrorDescription,"Failed to get the IP Address of the device");
			return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
		}

		strcpy(SelectedNetworkCardIP, getstation.Connection.SelectedNetworkCardIP);
	}
	else
	{
		strcpy(DeviceIP, linkStatistics->Connection.DeviceIP);
		strcpy(SelectedNetworkCardIP, linkStatistics->Connection.SelectedNetworkCardIP);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "getDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	getDataModel = (sGet_Data_Mode_lnformation*)malloc(sizeof(sGet_Data_Mode_lnformation));
	if (getDataModel == NULL)
	{
		strcpy(linkStatistics->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(getDataModel,0x00,sizeof(sGet_Data_Mode_lnformation));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get Station-List From Local-Device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	LOG_INFO("Get Station-List From Local-Device");
	memset(&getstation,0x00,sizeof(sGet_stations_Information));
	getstation.Connection.bHasDeviceIP = TRUE;
	strcpy(getstation.Connection.DeviceIP, DeviceIP);
	strcpy(getstation.Connection.SelectedNetworkCardIP, SelectedNetworkCardIP);

	getstation.bQueryOnlyLocalDevice = FALSE;

	if ((GHN_LIB_STAT = Ghn_Get_Stations(&getstation)) != eGHN_LIB_STAT_SUCCESS)
	{
		strcpy(linkStatistics->ErrorDescription,getstation.ErrorDescription);
		LOG_INFO("Call Get_Stations() Failed");
		free(getDataModel);
		return GHN_LIB_STAT;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get the IP address of the receiver device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if ((GHN_LIB_STAT = Get_IP_Address_From_Station_List(&getstation, linkStatistics->ReceiverDeviceMAC, ReceiverDeviceIP)) != eGHN_LIB_STAT_SUCCESS)
	{
		sprintf(linkStatistics->ErrorDescription,"Failed to get the IP Address of the receiver device");
		LOG_INFO(linkStatistics->ErrorDescription);
		free(getDataModel);
		return GHN_LIB_STAT;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Prepare the Link_Statistics-Test Setup
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	memset(&linkStatistics_Test,0x00,sizeof(sLink_Statistics_Test));
	str_to_mac(linkStatistics->TransmitterDeviceMAC,&linkStatistics_Test.StaTxMAC);
	str_to_mac(linkStatistics->ReceiverDeviceMAC,&linkStatistics_Test.StaRxMAC);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// fprintf(stdout,"\rgetting counters sample #%d...   ",SampleIteration);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get Link_Statistics results from the Receiver Device
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	LOG_INFO("Get Link_Statistics results from the Receiver Device strIP=%s",ReceiverDeviceIP);

	// Create the structure for GetDataModel()
	strcpy(getDataModel->DeviceIP, ReceiverDeviceIP);
	getDataModel->bHasNetworkCardIP = strlen(SelectedNetworkCardIP)>0;
	strcpy(getDataModel->NetworkCardIP, SelectedNetworkCardIP);
	getDataModel->IncludeBranch_Size = 2;
	getDataModel->ExcludeBranch_Size = 5;
		
	strcpy(getDataModel->IncludeBranch_Array[0].Name, Node_DeviceInfo);
	strcpy(getDataModel->IncludeBranch_Array[1].Name, Node_Interface);

	strcpy(getDataModel->ExcludeBranch_Array[0].Name, Nodes_Interface_PeriodicStatistics);
	strcpy(getDataModel->ExcludeBranch_Array[1].Name, Nodes_Interface_PhyDiagConfig);
	strcpy(getDataModel->ExcludeBranch_Array[2].Name, Nodes_Interface_Alarms);
	strcpy(getDataModel->ExcludeBranch_Array[3].Name, Nodes_Interface_Debug);
	strcpy(getDataModel->ExcludeBranch_Array[4].Name, Nodes_BitLoadingTable);

	if (Get_Data_Model(getDataModel) == FALSE)
	{
		// Error getting the XML file from the device
		sprintf(linkStatistics->ErrorDescription,"Failed to run Get_Data_Model(). %s",getDataModel->ErrorDescription);
		LOG_INFO(linkStatistics->ErrorDescription);
		free(getDataModel);
		return eGHN_LIB_STAT_HTTP_REQUEST_FAILED;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	if (XMLParser_OpenXMLBuffer((char*)getDataModel->DataModel_Buffer, &XMLHandler) != eXMLPARSER_STAT_SUCCESS)
	{
		sprintf(linkStatistics->ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
		LOG_INFO(linkStatistics->ErrorDescription);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Get NodeTypeConfiguration
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if ((XMLParser_GetNodeAttributeByNodePath(XMLHandler,Node_Interface,"NodeTypeConfiguration",&Value[0]) == eXMLPARSER_STAT_SUCCESS) &&
		(strcmp(Value,"SISO")==0))
	{
		linkStatistics_Test.NodeTypeConfiguration = eLink_Statistics_NodeTypeConfiguration_SISO;
	}
	else
	{
		linkStatistics_Test.NodeTypeConfiguration = eLink_Statistics_NodeTypeConfiguration_MIMO;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	GHN_LIB_STAT = Link_Statistics_ReadCEParameters(XMLHandler,&linkStatistics_Test, NULL);

	XMLParser_CloseXML(&XMLHandler);

	if (GHN_LIB_STAT != eGHN_LIB_STAT_SUCCESS)
	{
		sprintf(linkStatistics->ErrorDescription,"Failed to read Channel Estimation Parameters");
		LOG_INFO(linkStatistics->ErrorDescription);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILURE;
	}

	memset(&linkStatistics_Result,0x00,sizeof(sLink_Statistics_Result));

	if ((GHN_LIB_STAT = Link_Statistics_CalcCEParameters(&linkStatistics_Test,&linkStatistics_Result)) != eGHN_LIB_STAT_SUCCESS)
	{
		sprintf(linkStatistics->ErrorDescription,"Failed to calculate Channel Estimation Parameters");
		LOG_INFO(linkStatistics->ErrorDescription);
		free(getDataModel);
		return GHN_LIB_STAT;
	}

	linkStatistics->PHY = linkStatistics_Result.PHY;

	linkStatistics->SNR = ((double)(linkStatistics_Result.SNR[0] + linkStatistics_Result.SNR[1] + linkStatistics_Result.SNR[2] +
 									linkStatistics_Result.SNR[3] + linkStatistics_Result.SNR[4] + linkStatistics_Result.SNR[5]))/linkStatistics_Result.NumberOfInterval;

	linkStatistics->RXPower0 = linkStatistics_Result.RXPower0;
	linkStatistics->RXPower1 = linkStatistics_Result.RXPower1;

	linkStatistics->BytesReceived = linkStatistics_Result.BytesReceived;

	free(getDataModel);
	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Apply_Parameters_Setting(sApply_Parameters_Setting_Information* applyParametersSetting)
{
	sReset_Information				reset;
	eGHN_LIB_STAT					GHN_LIB_STAT;

	LOG_INFO("Started...");

	memset(&reset, 0x00, sizeof(sReset_Information));
	reset.Connection = applyParametersSetting->Connection;

	reset.ResetMode = eReset_Mode_Firmware;
	reset.HardwareReset = FALSE;

	if ((GHN_LIB_STAT = Ghn_Reset(&reset)) != eGHN_LIB_STAT_SUCCESS)
	{
		// Error reseting the device
		sprintf(applyParametersSetting->ErrorDescription,"Failed to apply parameters setting. %s",reset.ErrorDescription);
		LOG_INFO(applyParametersSetting->ErrorDescription);
		return GHN_LIB_STAT;
	}

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Local_Hosts_MAC_Addresses(sLocal_Hosts_MAC_Addresses_Information* LocalHosts)
{
	sGet_stations_Information		getstation;
	eGHN_LIB_STAT					GHN_LIB_STAT;
	char							DeviceIP[HTTP_LIB_IP_ADDRESS_LEN];
	char							SelectedNetworkCardIP[HTTP_LIB_IP_ADDRESS_LEN]; 

	sGet_Data_Mode_lnformation*		getDataModel;

	eXMLPARSER_STAT					status;
	XMLParserHandler				Handler;
	char							Value[XML_PARSER_MAX_STRING];
	char*							ptr;

	LOG_INFO("Started...");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// We have to get the IP-Address of the device from "AdapterIP" and "DeviceMAC" before we can continue
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (LocalHosts->Connection.bHasAdapterIP == TRUE)
	{
		memset(&getstation, 0x00, sizeof(sGet_stations_Information));

		// Duplicate the "sConnection_Information" structure
		memcpy(&getstation.Connection, &LocalHosts->Connection, sizeof(sConnection_Information));

		if ((GHN_LIB_STAT = Ghn_Get_Stations(&getstation)) != eGHN_LIB_STAT_SUCCESS)
		{
			strcpy(LocalHosts->ErrorDescription,"Failed to get the stations list");
			return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
		}

		if ((GHN_LIB_STAT = Get_IP_Address_From_Station_List(&getstation, LocalHosts->Connection.DeviceMAC, DeviceIP)) != eGHN_LIB_STAT_SUCCESS)
		{
			strcpy(LocalHosts->ErrorDescription,"Failed to get the IP Address of the device");
			return eGHN_LIB_STAT_DEVICE_NOT_FOUND;
		}

		strcpy(SelectedNetworkCardIP, getstation.Connection.SelectedNetworkCardIP);
	}
	else
	{
		strcpy(DeviceIP, LocalHosts->Connection.DeviceIP);
		strcpy(SelectedNetworkCardIP, LocalHosts->Connection.SelectedNetworkCardIP);
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "getDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	getDataModel = (sGet_Data_Mode_lnformation*)malloc(sizeof(sGet_Data_Mode_lnformation));
	if (getDataModel == NULL)
	{
		strcpy(LocalHosts->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(getDataModel,0x00,sizeof(sGet_Data_Mode_lnformation));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// Create the structure for GetDataModel()
	strcpy(getDataModel->DeviceIP, DeviceIP);
	
	getDataModel->bHasNetworkCardIP = strlen(SelectedNetworkCardIP)>0;
	strcpy(getDataModel->NetworkCardIP, SelectedNetworkCardIP);
	getDataModel->IncludeBranch_Size = 1;
	getDataModel->ExcludeBranch_Size = 0;
	strcpy(getDataModel->IncludeBranch_Array[0].Name, "Device.Ghn.Interface.*.BridgedClientMacAddresses");

	if (Get_Data_Model(getDataModel) == FALSE)
	{
		// Error getting the XML file from the device
		sprintf(LocalHosts->ErrorDescription,"Failed to run Get_Data_Model(). %s",getDataModel->ErrorDescription);
		LOG_INFO(LocalHosts->ErrorDescription);
		free(getDataModel);
		return eGHN_LIB_STAT_HTTP_REQUEST_FAILED;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Parsing the BBT.xml file
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	LOG_INFO("Parsing the BBT Data-Model");

	if ((status = XMLParser_OpenXMLBuffer((char*)getDataModel->DataModel_Buffer,&Handler)) != eXMLPARSER_STAT_SUCCESS)
	{
		sprintf(LocalHosts->ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
		LOG_INFO(LocalHosts->ErrorDescription);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}

	// Read the BridgedClientMacAddresses
	if (XMLParser_GetNodeAttributeByNodePath(Handler,Node_Interface,"BridgedClientMacAddresses",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
	{
		sprintf(LocalHosts->ErrorDescription,"Failed to read the BridgedClientMacAddresses field");
		LOG_INFO(LocalHosts->ErrorDescription);
		XMLParser_CloseXML(&Handler);
		free(getDataModel);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}

	LocalHosts->Size = 0;

	ptr = strtok(Value,",");

	while (ptr != NULL)
	{
		strcpy(LocalHosts->sMACAddressArray[LocalHosts->Size].DeviceMAC, ptr);
		LocalHosts->Size++;

		ptr = strtok(NULL, ",");
	}

	free(getDataModel);
	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Ghn_Get_Chip_Type_Information(sGet_Chip_Type_Information* getChipTypeInformation)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	Layer2Connection			layer2Connection;
	bool						bHas_layer2Connection = FALSE;

	cg_stat_t					cg_stat;

	LOG_INFO("Started...");

	fflush(stdout);

	if ((GHN_LIB_STAT = Open_Layer2_Connection(	&getChipTypeInformation->Connection,
												ETHER_TYPE_GHN,
												&layer2Connection,
												getChipTypeInformation->ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		GHN_LIB_STAT = eGHN_LIB_STAT_FAILURE;
		goto exit;
	}

	bHas_layer2Connection = TRUE;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Read Chip-Type 
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (GetChipType(&layer2Connection, getChipTypeInformation->Device_ChipType) == FALSE)
	{
		cg_stat = CG_STAT_FAILURE;
		sprintf(getChipTypeInformation->ErrorDescription,"Failed to read the Chip-Type from the device (stat=%lu)",cg_stat);
		LOG_INFO(getChipTypeInformation->ErrorDescription);
		GHN_LIB_STAT = ConvertReturnStatus(cg_stat);
		goto exit;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	GHN_LIB_STAT = eGHN_LIB_STAT_SUCCESS;

exit:

	if (bHas_layer2Connection == TRUE)
	{
		Close_Layer2_Connection(&layer2Connection);
	}

	return GHN_LIB_STAT;
}

eGHN_LIB_STAT Ghn_Parse_BPL_Topology_Information(	char*				DataModel_Buffer,
													char*				NodePrefix,
													sURT_Info*			URT,
													sBRT_Info*			BRT,
													char*				ErrorDescription)
{
	eGHN_LIB_STAT		GHN_LIB_STAT = eGHN_LIB_STAT_SUCCESS;
	eXMLPARSER_STAT		status;
	XMLParserHandler	XMLHandler;
	char				Value[XML_PARSER_MAX_STRING];
	
	char				NodePath[XML_PARSER_MAX_STRING];

	int					URT_Num_devices;
	char				URT_Device_did_list[XML_PARSER_MAX_STRING];
	char				URT_table[XML_PARSER_MAX_STRING];

	int					BRT_Num_devices;
	char				BRT_Device_did_list[XML_PARSER_MAX_STRING];
	char				BRT_Root_Array[XML_PARSER_MAX_STRING];
	char				BRT_Branch_Array[XML_PARSER_MAX_STRING];

	int					y,x;
	int					i;
	char*				ptr;


	URT->Size = 0;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Parsing the XML file
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	LOG_INFO("Parsing the BBT Data-Model");

	if ((status = XMLParser_OpenXMLBuffer(DataModel_Buffer,&XMLHandler)) != eXMLPARSER_STAT_SUCCESS)
	{
		sprintf(ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
		LOG_INFO(ErrorDescription);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Read URL Information
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface_X_00C5D9_BPL_URT);
	if (XMLParser_GetNodeAttributeByNodePath(XMLHandler,NodePath,"Num_devices",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_CloseXML(&XMLHandler);
		return eGHN_LIB_STAT_NOT_SUPPORTED;
	}

	URT_Num_devices = atoi(Value);

	if (XMLParser_GetNodeAttributeByNodePath(XMLHandler,NodePath,"Device_did_list",&URT_Device_did_list[0]) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_CloseXML(&XMLHandler);
		return eGHN_LIB_STAT_NOT_SUPPORTED;
	}

	if (XMLParser_GetNodeAttributeByNodePath(XMLHandler,NodePath,"URT_table",&URT_table[0]) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_CloseXML(&XMLHandler);
		return eGHN_LIB_STAT_NOT_SUPPORTED;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Read BRT Information
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface_X_00C5D9_BPL_BRT);
	if (XMLParser_GetNodeAttributeByNodePath(XMLHandler,NodePath,"Num_devices",&Value[0]) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_CloseXML(&XMLHandler);
		return eGHN_LIB_STAT_NOT_SUPPORTED;
	}

	BRT_Num_devices = atoi(Value);

	if (XMLParser_GetNodeAttributeByNodePath(XMLHandler,NodePath,"Device_did_list",&BRT_Device_did_list[0]) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_CloseXML(&XMLHandler);
		return eGHN_LIB_STAT_NOT_SUPPORTED;
	}

	if (XMLParser_GetNodeAttributeByNodePath(XMLHandler,NodePath,"Root_path_table",&BRT_Root_Array[0]) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_CloseXML(&XMLHandler);
		return eGHN_LIB_STAT_NOT_SUPPORTED;
	}
	if (XMLParser_GetNodeAttributeByNodePath(XMLHandler,NodePath,"Branch_path_table",&BRT_Branch_Array[0]) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_CloseXML(&XMLHandler);
		return eGHN_LIB_STAT_NOT_SUPPORTED;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


	if ((status = XMLParser_CloseXML(&XMLHandler)) != eXMLPARSER_STAT_SUCCESS)
	{
		LOG_INFO("Call XMLParser_CloseXML() Failed");
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	/*
	printf("\n");
	printf("URT_Num_devices=%d\n", URT_Num_devices);
	printf("URT_Device_did_list=%s\n", URT_Device_did_list);
	printf("URT_table=%s\n", URT_table);
	
	printf("\n");
	printf("BRT_Num_devices=%d\n", BRT_Num_devices);
	printf("BRT_Device_did_list=%s\n", BRT_Device_did_list);
	printf("BRT_Root_Array=%s\n", BRT_Root_Array);
	printf("BRT_Branch_Array=%s\n", BRT_Branch_Array);
	*/

	URT->Size = URT_Num_devices;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Parse URT Device_did_list
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	ptr = strtok(URT_Device_did_list,",");

	i = 1;

	while (ptr != NULL)
	{
		URT->Device_did_Mapping[i-1] = atoi(ptr);
		i++;
		
		ptr = strtok(NULL, ",");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Parse URT Table
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	ptr = strtok(URT_table,",");

	y = 1;
	x = 1;

	while (ptr != NULL)
	{
		URT->Array[y-1][x-1] = atoi(ptr);
		
		if (x < URT_Num_devices)
		{
			x++;
		}
		else
		{
			y++;
			x=1;

		}
		ptr = strtok(NULL, ",");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	BRT->Size = BRT_Num_devices;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Parse BRT Device_did_list
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	ptr = strtok(BRT_Device_did_list,",");

	i = 1;

	while (ptr != NULL)
	{
		BRT->Device_did_Mapping[i-1] = atoi(ptr);
		i++;

		ptr = strtok(NULL, ",");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Parse BRT Root-Array Table
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	ptr = strtok(BRT_Root_Array,",");

	y = 1;
	x = 1;

	while (ptr != NULL)
	{
		BRT->Root_Array[y-1][x-1] = atoi(ptr);

		if (x < URT_Num_devices)
		{
			x++;
		}
		else
		{
			y++;
			x=1;

		}
		ptr = strtok(NULL, ",");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Parse BRT Branch-Array Table
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	ptr = strtok(BRT_Branch_Array,",");

	y = 1;
	x = 1;

	while (ptr != NULL)
	{
		BRT->Branch_Array[y-1][x-1] = atoi(ptr);

		if (x < URT_Num_devices)
		{
			x++;
		}
		else
		{
			y++;
			x=1;

		}
		ptr = strtok(NULL, ",");
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	return GHN_LIB_STAT;
}

eGHN_LIB_STAT Ghn_Parse_BPL_Topology_Mapping_Information(	char*				DataModel_Buffer,
															char*				NodePrefix,
															sBPL_Mapping_Info*	Mapping,
															char*				ErrorDescription)
{
	eGHN_LIB_STAT		GHN_LIB_STAT = eGHN_LIB_STAT_SUCCESS;
	eXMLPARSER_STAT		status;
	XMLParserHandler	XMLHandler;
	XMLParserSubTree	SubTree;

	char				Value[XML_PARSER_MAX_STRING];
	
	char				NodePath[XML_PARSER_MAX_STRING];

	UINT8 GhnDeviceID = 0;

	memset(Mapping, 0x00, sizeof(sBPL_Mapping_Info));
	Mapping->Size = 0;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Parsing the XML file
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	LOG_INFO("Parsing the BBT Data-Model");

	if ((status = XMLParser_OpenXMLBuffer(DataModel_Buffer,&XMLHandler)) != eXMLPARSER_STAT_SUCCESS)
	{
		sprintf(ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
		LOG_INFO(ErrorDescription);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Read Mapping Information
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	strcpy(NodePath,""); if (NodePrefix != NULL) strcat(NodePath, NodePrefix); strcat(NodePath, Node_Interface);
	if (XMLParser_CreateSubTree(XMLHandler, NodePath, &SubTree) != eXMLPARSER_STAT_SUCCESS)
	{
		XMLParser_CloseXML(&XMLHandler);
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}

	GhnDeviceID = 0;
	GhnDeviceID = XMLParser_GetIntValue(XMLHandler, SubTree, "GhnDeviceID");

	if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree,"GhnMACAddress",Value) == eXMLPARSER_STAT_SUCCESS)
	{
		strcpy(Mapping->Array[Mapping->Size].GhnDeviceMAC, Value);
		Mapping->Array[Mapping->Size].GhnDeviceID = GhnDeviceID;
		Mapping->Size++;
		Mapping->DM_Index = 1;
	}

	// All "AssociatedDevice" under "Device.Ghn.Interface"
	while ((status = XMLParser_FindNode(XMLHandler,SubTree,"AssociatedDevice")) == eXMLPARSER_STAT_SUCCESS)
	{
		GhnDeviceID = 0;
		GhnDeviceID = XMLParser_GetIntValue(XMLHandler, SubTree, "GhnDeviceID");

		if (XMLParser_GetNodeAttributeByName(XMLHandler,SubTree,"GhnMACAddress",Value) == eXMLPARSER_STAT_SUCCESS)
		{
			strcpy(Mapping->Array[Mapping->Size].GhnDeviceMAC, Value);
			Mapping->Array[Mapping->Size].GhnDeviceID = GhnDeviceID;
			Mapping->Size++;
		}
	}

	if ((status = XMLParser_CloseXML(&XMLHandler)) != eXMLPARSER_STAT_SUCCESS)
	{
		LOG_INFO("Call XMLParser_CloseXML() Failed");
		return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		
	return GHN_LIB_STAT;
}

eGHN_LIB_STAT Ghn_Get_BPL_Topology_Information(sBPL_Topology_Information* BPL_Info)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	sDevice_Information			devinf;
	UINT32						i,j;

	char						DM_DeviceIP[HTTP_LIB_IP_ADDRESS_LEN];

	sGet_Data_Mode_lnformation*	getDataModel;

	LOG_INFO("Started...");

	// Init
	memset(&BPL_Info->URT, 0x00, sizeof(BPL_Info->URT));


	if (BPL_Info->getstation.Size == 0)
	{
		strcpy(BPL_Info->ErrorDescription,"No stations found.");
		return eGHN_LIB_STAT_FAILURE;
	}

	for (i=1 ; i <= BPL_Info->getstation.Size ; i++)
	{
		memset(&devinf, 0x00, sizeof(sDevice_Information));

		devinf.Connection.bHasDeviceIP = TRUE;
		strcpy(devinf.Connection.DeviceIP,BPL_Info->getstation.sStationArray[i-1].DeviceIP);
		strcpy(devinf.Connection.SelectedNetworkCardIP, BPL_Info->getstation.Connection.SelectedNetworkCardIP);

		devinf.bAdvanced = TRUE;

		if ((GHN_LIB_STAT = Ghn_Get_Device_Information(&devinf)) == eGHN_LIB_STAT_SUCCESS)
		{
			for (j=1; j <= devinf.Size; j++)
			{
				if (strcmp(devinf.AttributeArray[j-1].Name, "GhnMACAddress") == 0)
				{
					strcpy(BPL_Info->Mapping.Array[i-1].GhnDeviceMAC, devinf.AttributeArray[j-1].Value);
				}

				if (strcmp(devinf.AttributeArray[j-1].Name, "GhnDeviceID") == 0)
				{
					BPL_Info->Mapping.Array[i-1].GhnDeviceID = atoi(devinf.AttributeArray[j-1].Value);
				}

				if (strcmp(devinf.AttributeArray[j-1].Name, "Device Name") == 0)
				{
					strcpy(BPL_Info->Mapping.Array[i-1].DeviceName, devinf.AttributeArray[j-1].Value);
				}
			
				if (strcmp(devinf.AttributeArray[j-1].Name,"Node Type") == 0)
				{
					if (strcmp(devinf.AttributeArray[j-1].Value, "DM") == 0)
					{
						strcpy(DM_DeviceIP, BPL_Info->getstation.sStationArray[i-1].DeviceIP);
						BPL_Info->Mapping.DM_Index = i;
					}
				}
			}
		}
		else
		{
			// Use the information from the "BPL_Info->getstation.sStationArray"
			strcpy(BPL_Info->Mapping.Array[i-1].GhnDeviceMAC, BPL_Info->getstation.sStationArray[i-1].DeviceMAC);
			BPL_Info->Mapping.Array[i-1].GhnDeviceID = BPL_Info->getstation.sStationArray[i-1].GhnDeviceID;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	BPL_Info->Mapping.Size = BPL_Info->getstation.Size;

	// Check that we can reach the DM device
	if (strcmp(DM_DeviceIP, "") == 0)
	{
		sprintf(BPL_Info->ErrorDescription,"Failed to read data from the DM device");
		return eGHN_LIB_STAT_FAILURE;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "getDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	getDataModel = (sGet_Data_Mode_lnformation*)malloc(sizeof(sGet_Data_Mode_lnformation));
	if (getDataModel == NULL)
	{
		strcpy(BPL_Info->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(getDataModel,0x00,sizeof(sGet_Data_Mode_lnformation));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// Create the structure for GetDataModel()
	strcpy(getDataModel->DeviceIP,DM_DeviceIP);
	getDataModel->bHasNetworkCardIP = strlen(BPL_Info->getstation.Connection.SelectedNetworkCardIP)>0;
	strcpy(getDataModel->NetworkCardIP, BPL_Info->getstation.Connection.SelectedNetworkCardIP);
	
	getDataModel->IncludeBranch_Size = 1;
	getDataModel->ExcludeBranch_Size = 0;
	strcpy(getDataModel->IncludeBranch_Array[0].Name, Nodes_Interface_X_00C5D9_BPL);

	if (Get_Data_Model(getDataModel) == FALSE)
	{
		// Error getting the XML file from the device
		sprintf(BPL_Info->ErrorDescription,"Failed to run Get_Data_Model(). %s",getDataModel->ErrorDescription);
		LOG_INFO(BPL_Info->ErrorDescription);
		free(getDataModel);
		return eGHN_LIB_STAT_HTTP_REQUEST_FAILED;
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	GHN_LIB_STAT = Ghn_Parse_BPL_Topology_Information(	(char*)getDataModel->DataModel_Buffer,
														NULL,
														&BPL_Info->URT,
														&BPL_Info->BRT,
														BPL_Info->ErrorDescription);

	free(getDataModel);


	// Get the Mapping Information

	return GHN_LIB_STAT;
}

eGHN_LIB_STAT Ghn_Get_BPL_Topology_Information_From_XML_File(sBPL_Topology_XML_File_Information* BPL_Info)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	int							FileSize;
	char*						DataModel_Buffer;
	FILE*						f;

	char						NodePath[XML_PARSER_MAX_STRING];

	LOG_INFO("Started...");

	FileSize = file_get_size(BPL_Info->FileName);

	if (FileSize == 0)
	{
		sprintf(BPL_Info->ErrorDescription,"Failed to read from file");
		LOG_INFO(BPL_Info->ErrorDescription);
		return eGHN_LIB_STAT_FAILED_READ_FILE;
	}

	if ((DataModel_Buffer = (char*)malloc(FileSize+1)) == NULL)
	{
		sprintf(BPL_Info->ErrorDescription,"Failed to allocate memory");
		LOG_INFO(BPL_Info->ErrorDescription);
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;
	}

	memset(DataModel_Buffer, 0x00, FileSize+1);

	if ((f = fopen(BPL_Info->FileName,"rb")) == NULL)
	{
		sprintf(BPL_Info->ErrorDescription,"Failed to read from file");
		LOG_INFO(BPL_Info->ErrorDescription);
		free(DataModel_Buffer);
		return eGHN_LIB_STAT_FAILED_READ_FILE;
	}

	fread(DataModel_Buffer, 0x01, FileSize,f);

	strcpy(NodePath,""); if (BPL_Info->bIsRM_XML_File == TRUE) strcat(NodePath, "q0:PeriodicCounterInfo.");

	GHN_LIB_STAT = Ghn_Parse_BPL_Topology_Mapping_Information(	(char*)DataModel_Buffer,
																NodePath,

																&BPL_Info->Mapping,
																BPL_Info->ErrorDescription);

	GHN_LIB_STAT = Ghn_Parse_BPL_Topology_Information(	(char*)DataModel_Buffer,
														NodePath,

														&BPL_Info->URT,
														&BPL_Info->BRT,
														BPL_Info->ErrorDescription);



	fclose(f);
	free(DataModel_Buffer);
	return GHN_LIB_STAT;
}

eGHN_LIB_STAT Ghn_Get_PHY_Rate_Table_Information(sGet_PHY_Rate_Table_Information* get_PHY_Rate)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	UINT32						i,j;

	sGet_Data_Mode_lnformation*	getDataModel;

	eXMLPARSER_STAT					status;
	XMLParserHandler				Handler;
	XMLParserSubTree				SubTree;
	char							Value[XML_PARSER_MAX_STRING];

	char							TxDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];
	int								RxPhyRate;
	//ULONG64							BytesReceived;

	LOG_INFO("Started...");

	// Init
	memset(&get_PHY_Rate->RX_PhyRateTable, 0x00, sizeof(get_PHY_Rate->RX_PhyRateTable));

	if (get_PHY_Rate->getstation.Size == 0)
	{
		strcpy(get_PHY_Rate->ErrorDescription,"No stations found.");
		return eGHN_LIB_STAT_FAILURE;
	}

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "getDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	getDataModel = (sGet_Data_Mode_lnformation*)malloc(sizeof(sGet_Data_Mode_lnformation));
	if (getDataModel == NULL)
	{
		strcpy(get_PHY_Rate->ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	for (i=1 ; i <= get_PHY_Rate->getstation.Size ; i++)
	{
		memset(getDataModel,0x00,sizeof(sGet_Data_Mode_lnformation));

		// Create the structure for GetDataModel()
		strcpy(getDataModel->DeviceIP, get_PHY_Rate->getstation.sStationArray[i-1].DeviceIP);
		getDataModel->bHasNetworkCardIP = strlen(get_PHY_Rate->getstation.Connection.SelectedNetworkCardIP)>0;
		strcpy(getDataModel->NetworkCardIP, get_PHY_Rate->getstation.Connection.SelectedNetworkCardIP);
	
		getDataModel->IncludeBranch_Size = 2;
		getDataModel->ExcludeBranch_Size = 2;
		strcpy(getDataModel->IncludeBranch_Array[0].Name, Node_DeviceInfo);
		strcpy(getDataModel->IncludeBranch_Array[1].Name, Nodes_Interface_AssociatedDevice);
		strcpy(getDataModel->ExcludeBranch_Array[0].Name, Nodes_IntervalCEParameters);
		strcpy(getDataModel->ExcludeBranch_Array[1].Name, Nodes_BitLoadingTable);

		if (Get_Data_Model(getDataModel) == FALSE)
		{
			// Error getting the XML file from the device
			sprintf(get_PHY_Rate->ErrorDescription,"Failed to run Get_Data_Model(). %s",getDataModel->ErrorDescription);
			LOG_INFO(get_PHY_Rate->ErrorDescription);
			free(getDataModel);
			return eGHN_LIB_STAT_HTTP_REQUEST_FAILED;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Parsing the BBT.xml file
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		LOG_INFO("Parsing the BBT Data-Model");

		if ((status = XMLParser_OpenXMLBuffer((char*)getDataModel->DataModel_Buffer,&Handler)) != eXMLPARSER_STAT_SUCCESS)
		{
			sprintf(get_PHY_Rate->ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
			LOG_INFO(get_PHY_Rate->ErrorDescription);
			free(getDataModel);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}

		if (XMLParser_GetNodeAttributeByNodePath(Handler,Node_DeviceInfo,"X_00C5D9_DeviceName",&Value[0]) == eXMLPARSER_STAT_SUCCESS)
		{
			strcpy(get_PHY_Rate->Array[i-1].DeviceName, Value);
		}

		if (XMLParser_CreateSubTree(Handler, Node_Interface, &SubTree) == eXMLPARSER_STAT_SUCCESS)
		{
			// All "AssociatedDevice" under "Device.Ghn.Interface"
			while ((status = XMLParser_FindNode(Handler,SubTree,"AssociatedDevice")) == eXMLPARSER_STAT_SUCCESS)
			{
				if (XMLParser_GetNodeAttributeByName(Handler,SubTree,"GhnMACAddress",Value) != eXMLPARSER_STAT_SUCCESS)
				{
					continue;
				}

				strcpy(TxDeviceMAC, Value);

				if (XMLParser_GetNodeAttributeByName(Handler,SubTree,"RxPhyRate",Value) != eXMLPARSER_STAT_SUCCESS)
				{
					continue;
				}

				RxPhyRate = atoi(Value);

				if (XMLParser_GetNodeAttributeByName(Handler,SubTree,"X_00C5D9_BytesReceived",Value) != eXMLPARSER_STAT_SUCCESS)
				{
					continue;
				}
				
				//BytesReceived = strtoll(Value, NULL, 10);

				// Check if there was some traffic on that link
				//if (BytesReceived < 1000000)
				//{
				//	continue;
				//}
				
				//printf("%s -> %s : %d Mbps , %d\n",		TxDeviceMAC,
				//										get_PHY_Rate->getstation.sStationArray[i-1].DeviceMAC,
				//										RxPhyRate,
				//										BytesReceived);

				// Search for the Index of the "TxDeviceMAC" device
				for (j=1; j<= get_PHY_Rate->getstation.Size; j++)
				{
					if (OS_STRICMP(TxDeviceMAC, get_PHY_Rate->getstation.sStationArray[j-1].DeviceMAC) == 0)
					{
						// Found
						get_PHY_Rate->RX_PhyRateTable[j-1][i-1] = RxPhyRate;
						break;
					}
				}
			}

			XMLParser_FreeSubTree(Handler,&SubTree);
		}

		if ((status = XMLParser_CloseXML(&Handler)) != eXMLPARSER_STAT_SUCCESS)
		{
			LOG_INFO("Call XMLParser_CloseXML() Failed");
			free(getDataModel);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	free(getDataModel);

	GHN_LIB_STAT = eGHN_LIB_STAT_SUCCESS;

	return GHN_LIB_STAT;
}

int Netinf_Entire_Network_Progress_Inication;

int Ghn_Get_Netinf_Entire_Network_Progress_Inication()
{
	return Netinf_Entire_Network_Progress_Inication;
}

eGHN_LIB_STAT Ghn_Run_Netinf_Entire_Network(sNetinf_Entire_Network_Information* netinf_Entire)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;

	UINT32						i,j;

	sSet_Data_Mode_lnformation		setDataModel;
	sPhy_Diag_Config_Information	PhyDiagConfig;

	char							Run_Table[GHN_LIB_MAX_GETSTATIONS][GHN_LIB_MAX_GETSTATIONS];	// '-' Not Run yet
																									// 'p' Pending
																									// '+' Already Run
	
	char							Run_Devices[GHN_LIB_MAX_GETSTATIONS];							// '-' TX device is available
																									// '+' TX device is busy
	
	int								Progress_Inication_Percentage;

	bool							bFinish;

	LOG_INFO("Started...");

	// Init
	Netinf_Entire_Network_Progress_Inication = -1;

	if (netinf_Entire->getstation.Size == 0)
	{
		strcpy(netinf_Entire->ErrorDescription,"No stations found.");
		return eGHN_LIB_STAT_FAILURE;
	}

	if (netinf_Entire->getstation.Size < 2)
	{
		strcpy(netinf_Entire->ErrorDescription,"Found only one G.hn Device. Need at least 2 devices or more");
		return eGHN_LIB_STAT_FAILURE;
	}

	for (i=1 ; i <= netinf_Entire->getstation.Size ; i++)
	{
		for (j=1 ; j <= netinf_Entire->getstation.Size ; j++)
		{
			if (i==j)
			{
				Run_Table[i-1][j-1] = 'x';
			}
			else
			{
				Run_Table[i-1][j-1] = '-';
			}
		}
	}
	
	for (i=1 ; i <= netinf_Entire->getstation.Size ; i++)
	{
		Run_Devices[i-1] = '-';
	}

	Netinf_Entire_Network_Progress_Inication = 1; // Starting
	Progress_Inication_Percentage = 100 / (netinf_Entire->getstation.Size * (netinf_Entire->getstation.Size-1));

	bFinish = FALSE;

	while (bFinish != TRUE)
	{
		bFinish = TRUE;

		// Search for available link

		for (i=1 ; i <= netinf_Entire->getstation.Size ; i++)
		{
			for (j=1 ; j <= netinf_Entire->getstation.Size ; j++)
			{
				if (Run_Table[i-1][j-1] == '-')
				{
					bFinish = FALSE;

					// Check if the Transmitter device is available
					if (Run_Devices[i-1] == '-')
					{
						Run_Table[i-1][j-1] = 'p';
						Run_Devices[i-1] = '+';

						// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
						// Start the Netinf
						// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
						strcpy(setDataModel.DeviceIP, netinf_Entire->getstation.sStationArray[i-1].DeviceIP);
						setDataModel.bHasNetworkCardIP = strlen(netinf_Entire->getstation.Connection.SelectedNetworkCardIP)>0;
						strcpy(setDataModel.NetworkCardIP, netinf_Entire->getstation.Connection.SelectedNetworkCardIP);

						setDataModel.SetParameter_Size = 4;

						strcpy(setDataModel.SetParameter_Array[0].Name, Node_PhyDiagReceiverMAC);
						strcpy(setDataModel.SetParameter_Array[0].Value, netinf_Entire->getstation.sStationArray[j-1].DeviceMAC);

						strcpy(setDataModel.SetParameter_Array[1].Name, Node_PhyDiagTrafficBurstSize);
						sprintf(setDataModel.SetParameter_Array[1].Value, "%d", 25);

						strcpy(setDataModel.SetParameter_Array[2].Name, Node_PhyDiagTrafficTimeOut);
						sprintf(setDataModel.SetParameter_Array[2].Value, "%d", netinf_Entire->TestDuration);

						strcpy(setDataModel.SetParameter_Array[3].Name, Node_StartPhyDiagTest);
						strcpy(setDataModel.SetParameter_Array[3].Value, "true");

						if (Set_Data_Model(&setDataModel) == FALSE)
						{
							sprintf(netinf_Entire->ErrorDescription,"Failed to start netinf %s",setDataModel.ErrorDescription);
							LOG_INFO(netinf_Entire->ErrorDescription);
							return eGHN_LIB_STAT_FAILED_START_NETINF;
						}
						// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

						printf("Run Netinf (%s) -> (%s)\n",
										netinf_Entire->getstation.sStationArray[i-1].DeviceMAC,
										netinf_Entire->getstation.sStationArray[j-1].DeviceMAC);
					}
				}
				else if (Run_Table[i-1][j-1] == 'p')
				{
					// Check if netinf was finish

					strcpy(PhyDiagConfig.DeviceIP, netinf_Entire->getstation.sStationArray[i-1].DeviceIP);
					PhyDiagConfig.bHasNetworkCardIP = strlen(netinf_Entire->getstation.Connection.SelectedNetworkCardIP)>0;
					strcpy(PhyDiagConfig.NetworkCardIP, netinf_Entire->getstation.Connection.SelectedNetworkCardIP);
					
					if ((GHN_LIB_STAT = Ghn_Phy_Diag_Config_Information(&PhyDiagConfig)) == eGHN_LIB_STAT_SUCCESS)
					{
						if (PhyDiagConfig.StartPhyDiagTest == TRUE)
						{
							// Netinf is still running
							bFinish = FALSE;
						}
						else
						{
							// Netinf finish
							Run_Table[i-1][j-1] = '+';
							Run_Devices[i-1] = '-';

							Netinf_Entire_Network_Progress_Inication += Progress_Inication_Percentage;
						}
					}
				}


			} // j++
		} // i++;

		if (bFinish == FALSE)
		{
			OS_Sleep(250);
		}

	} // while (bFinish != TRUE)

	GHN_LIB_STAT = eGHN_LIB_STAT_SUCCESS;

	return GHN_LIB_STAT;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
