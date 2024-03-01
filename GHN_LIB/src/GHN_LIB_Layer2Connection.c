#include "GHN_LIB_typedefs.h"
#include "GHN_LIB_int.h"
#include "GHN_LIB_int_consts.h"
#include "GHN_LIB_Layer2Connection.h"
#include "console.h"
#include "common.h"

#include "XMLParser.h"
#include "HTTP_LIB_ext.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

eGHN_LIB_STAT Ghn_Get_Network_Interface_By_Device_IP(ip_address_t xi_DeviceIP,ip_address_t* xo_NetworkCard);

/***************************************************************************************************
* Prepare_Layer2_Connection()                                                                      *
*                                                                                                  *
* + Check user input                                                                               *
* + Get the NetworkCard IP                                                                         *
* + Get the Local Device MAC Address                                                               *
***************************************************************************************************/
eGHN_LIB_STAT Prepare_Layer2_Connection(	sConnection_Information*		xi_Connection,
											ip_address_t*					xo_NetworkCardIP,
											char*							xo_LocalDeviceMAC,
											char*							xo_ErrorDescription)
{
	sGet_Data_Mode_lnformation*	getDataModel;

	eXMLPARSER_STAT		status;
	XMLParserHandler	Handler;
	XMLParserSubTree	SubTree;
	mac_address_t		mac;
	macStruct			LocalDeviceMac;
	char				Value[XML_PARSER_MAX_STRING];

	ip_address_t		DeviceIP=0;
	ip_address_t		NetworkCardIP=0;
	char				strDeviceIP[256];

	LOG_INFO("Started...");

	strcpy(xo_ErrorDescription,"");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Malloc memory for the "getDataModel"
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	getDataModel = (sGet_Data_Mode_lnformation*)malloc(sizeof(sGet_Data_Mode_lnformation));
	if (getDataModel == NULL)
	{
		strcpy(xo_ErrorDescription,"Failed to malloc memory");
		return eGHN_LIB_STAT_FAILED_MEMORY_ALLOCATION;

	}
	memset(getDataModel,0x00,sizeof(sGet_Data_Mode_lnformation));
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// The user didn't specify "bHasAdapterIP" or "bHasDeviceIP"
	if ((xi_Connection->bHasAdapterIP	== FALSE) &&
		(xi_Connection->bHasDeviceIP	== FALSE))
	{
		strcpy(xo_ErrorDescription,"The user didn't specify \"bHasAdapterIP\" or \"bHasDeviceIP\"");
		free(getDataModel);
		return eGHN_LIB_STAT_INVALID_PARAMETER;
	}

	// The user specify both "bHasAdapterIP" and "bHasDeviceIP"
	if ((xi_Connection->bHasAdapterIP	== TRUE) &&
		(xi_Connection->bHasDeviceIP	== TRUE))
	{
		strcpy(xo_ErrorDescription,"The user specify both \"bHasAdapterIP\" and \"bHasDeviceIP\"");
		free(getDataModel);
		return eGHN_LIB_STAT_INVALID_PARAMETER;
	}

	if (xi_Connection->bHasAdapterIP == TRUE)
	{
		// The user specify the IP address of the network-card and the MAC address
		NetworkCardIP = str_to_ip(xi_Connection->AdapterIP);
		ip_to_str(NetworkCardIP,xi_Connection->SelectedNetworkCardIP);

		str_to_mac(xi_Connection->DeviceMAC,&mac);
		memcpy(LocalDeviceMac.macAddress,mac,HMAC_LEN);
	}

	if (xi_Connection->bHasDeviceIP == TRUE)
	{
		// The user specify the IP address of the device 
		DeviceIP = str_to_ip(xi_Connection->DeviceIP);

		// Check if the Local device has a valid IP address
		if (DeviceIP == 0x00)
		{
			// No valid IP address
			sprintf(xo_ErrorDescription,"The IP 0.0.0.0 is not valid");
			LOG_INFO(xo_ErrorDescription);
			free(getDataModel);
			return eGHN_LIB_STAT_IP_ADDRESS_INVALID;
		}

		if (strlen(xi_Connection->SelectedNetworkCardIP)!=0)
		{
			// The user specify the IP address of the network-card
			NetworkCardIP = str_to_ip(xi_Connection->SelectedNetworkCardIP);
		}
		else
		{
			if (Ghn_Get_Network_Interface_By_Device_IP(DeviceIP,&NetworkCardIP) != eGHN_LIB_STAT_SUCCESS)
			{
				sprintf(xo_ErrorDescription,"The device with IP=%s is not in the Subnet-Mask of any network interface",
					xi_Connection->DeviceIP);
				LOG_INFO(xo_ErrorDescription);
				free(getDataModel);
				return eGHN_LIB_STAT_SUBNET_MASK_MISMATCH;
			}

			ip_to_str(NetworkCardIP,xi_Connection->SelectedNetworkCardIP);
		}

		ip_to_str(DeviceIP,strDeviceIP);

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Get Station-List From LocalDevice
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		LOG_INFO("Call GetDataModel() strIP=%s",strDeviceIP);

		// Create the structure for GetDataModel()
		strcpy(getDataModel->DeviceIP,strDeviceIP);
		getDataModel->bHasNetworkCardIP = TRUE;
		strcpy(getDataModel->NetworkCardIP, xi_Connection->SelectedNetworkCardIP);
		getDataModel->IncludeBranch_Size = 1;
		getDataModel->ExcludeBranch_Size = 0;
		strcpy(getDataModel->IncludeBranch_Array[0].Name,Nodes_Interface_GhnMACAddress);

		if (Get_Data_Model(getDataModel) == FALSE)
		{
			// Error getting the XML file from the device
			sprintf(xo_ErrorDescription,"Failed to run Get_Data_Model(). %s",getDataModel->ErrorDescription);
			LOG_INFO(xo_ErrorDescription);
			free(getDataModel);
			return eGHN_LIB_STAT_HTTP_REQUEST_FAILED;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		// Parsing the BBT Data-Model
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
		LOG_INFO("Parsing the BBT Data-Model");

		if ((status = XMLParser_OpenXMLBuffer((char*)getDataModel->DataModel_Buffer, &Handler)) != eXMLPARSER_STAT_SUCCESS)
		{
			LOG_INFO("Call XMLParser_OpenXMLBuffer() Failed");

			sprintf(xo_ErrorDescription,"Call XMLParser_OpenXMLBuffer() Failed");
			LOG_INFO(xo_ErrorDescription);
			free(getDataModel);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}

		if (XMLParser_CreateSubTree(Handler, Node_Interface, &SubTree) == eXMLPARSER_STAT_SUCCESS)
		{
			// Get MAC of Local-Device
			if (XMLParser_GetNodeAttributeByName(Handler,SubTree,"GhnMACAddress",Value) == eXMLPARSER_STAT_SUCCESS)
			{
				str_to_mac(Value,&mac);
				memcpy(LocalDeviceMac.macAddress,mac,HMAC_LEN);
			}

			XMLParser_FreeSubTree(Handler,&SubTree);
		}
		else
		{
			XMLParser_CloseXML(&Handler);
			free(getDataModel);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}

		if ((status = XMLParser_CloseXML(&Handler)) != eXMLPARSER_STAT_SUCCESS)
		{
			LOG_INFO("Call XMLParser_CloseXML() Failed");
			free(getDataModel);
			return eGHN_LIB_STAT_FAILED_TO_PARSE_XML;
		}
		// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	}

	sprintf(xo_LocalDeviceMAC,MAC_ADDR_FMT,MAC_ADDR(LocalDeviceMac.macAddress));
	*xo_NetworkCardIP = NetworkCardIP;

	free(getDataModel);
	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Open_Layer2_Connection(	sConnection_Information*		xi_Connection,
										UINT16							xi_etype,
										Layer2Connection*				xo_layer2Connection,
										char*							xo_ErrorDescription)
{
	eGHN_LIB_STAT				GHN_LIB_STAT;
	char						LocalDeviceMAC[GHN_LIB_MAC_ADDRESS_LEN];

	cg_stat_t					cg_stat;
	ip_address_t				NetworkCardIP;
	mac_address_t				mac;
	macStruct					mac2;

	LOG_INFO("Started...");

	if ((GHN_LIB_STAT = Prepare_Layer2_Connection(xi_Connection,
													&NetworkCardIP,
													LocalDeviceMAC,
													xo_ErrorDescription)) != eGHN_LIB_STAT_SUCCESS)
	{
		return GHN_LIB_STAT;
	}

	memset(xo_layer2Connection, 0x00, sizeof(Layer2Connection));

	// initialize random generator
	OS_Sleep(1);
#ifdef _WIN32
	srand((unsigned int)(time(NULL)*console_get_msectime()*GetCurrentProcessId()));
#elif __linux__
	srand((unsigned int)(time(NULL)*console_get_msectime()*getpid()));
#endif


	// generate random numbers
	xo_layer2Connection->m_transIdCounter = rand();			// (#define RAND_MAX 0x7fff)

	if ((cg_stat=CM_connect(NetworkCardIP,
							NULL,
							xi_etype,
							&xo_layer2Connection->m_eth_handle_t,
							xi_Connection->bSnifferMode,
							xi_Connection->bAllow_Incoming_Broadcast_Packets)) != CG_STAT_SUCCESS)
	{
		sprintf(xo_ErrorDescription,"failed to connect device (stat=%lu)",cg_stat);
		LOG_INFO(xo_ErrorDescription);
		return ConvertReturnStatus(cg_stat);
	}

	str_to_mac(LocalDeviceMAC,&mac);
	memcpy(mac2.macAddress,mac,HMAC_LEN);
	memcpy(&xo_layer2Connection->m_MAC_Address,&mac2,sizeof(macStruct));

	return eGHN_LIB_STAT_SUCCESS;
}

eGHN_LIB_STAT Close_Layer2_Connection(Layer2Connection* layer2Connection)
{
	CM_disconnect(layer2Connection->m_eth_handle_t);
	layer2Connection->m_transIdCounter = 0x00;
	memset(&layer2Connection->m_MAC_Address,0x00,sizeof(macStruct));

	return eGHN_LIB_STAT_SUCCESS;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
