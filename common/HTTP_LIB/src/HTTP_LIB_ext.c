// This is the main DLL file.

#include "stdio.h"
#include "stdlib.h"

#include <string.h>

#include "HTTP_LIB_ext.h"
#include "HTTP_LIB_typedef.h"

#ifdef _WIN32
#define HTTP_LIB_MAX_PATH MAX_PATH
#elif __linux__
#include <linux/limits.h>
#define HTTP_LIB_MAX_PATH PATH_MAX
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CURL_STATICLIB
#include <curl/curl.h>

typedef struct
{
	char*	Buffer;
	int		BuffeSize;
	int		Size;
} sHTTP_LIB_Buffer;

// Your callback function should return the number of bytes it "took care of".
static size_t my_fwriteBuffer(void *buffer, size_t size, size_t nmemb, void *stream)
{
	//FILE* fout = (FILE*)stream;
	sHTTP_LIB_Buffer* HTTP_LIB_Buffer = (sHTTP_LIB_Buffer*)stream;

	//return fwrite(buffer, size, nmemb, fout);

	if (((int)(HTTP_LIB_Buffer->Size + size*nmemb + 1)) < HTTP_LIB_Buffer->BuffeSize)
	{
		memcpy(&HTTP_LIB_Buffer->Buffer[HTTP_LIB_Buffer->Size],buffer,size*nmemb);
		HTTP_LIB_Buffer->Size += size*nmemb;

		HTTP_LIB_Buffer->Buffer[HTTP_LIB_Buffer->Size+1]='\0';
	}
	else
	{
		// Ignore the suffix of the response in case the response is larger than "HTTP_LIB_MAX_DATA_MODEL_BUFFER_SIZE" bytes
	}


	return size*nmemb;
}

/***************************************************************************************************
* Get_Data_Model()                                                                                 *
*                                                                                                  *
* Summary:                                                                                         *
*        Send HTTP request to the device and return the result (Buffer).                           *
*                                                                                                  *
* Return:                                                                                          *
*        TRUE/FALSE on success/failure                                                             *
***************************************************************************************************/
bool Get_Data_Model(sGet_Data_Mode_lnformation* getDataModel)
{
	CURL*				curl_handle;
	CURLcode			res;
	sHTTP_LIB_Buffer	HTTP_LIB_Buffer;

	char				strURL[HTTP_LIB_MAX_PATH];

	char				data[HTTP_LIB_MAX_BRANCH_NAME_SIZE * HTTP_LIB_MAX_BRANCHS] = "";
	char				TempBranch[HTTP_LIB_MAX_BRANCH_NAME_SIZE];
	UINT32				i;


	sprintf(strURL,"%s/getLiveData",getDataModel->DeviceIP);

	strcat(data,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	strcat(data,"<getLiveData xmlns=\"http://www.coppergate.com/AP/PerfDataReq\" operName=\"retrieve\" userid=\"00:13:92:AA:BB:CC\" password=\"perfdata\">");
	
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Build the Include-Branch List
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	strcat(data,"<includeBranchOrInstanceList>");
	if (getDataModel->IncludeBranch_Size == 0)
	{
		strcat(data,"<branch branchName=\"Device.\"/>");
	}
	else
	{
		for(i=0;i<getDataModel->IncludeBranch_Size;i++)
		{
			sprintf(TempBranch,"<branch branchName=\"%s\"/>",getDataModel->IncludeBranch_Array[i].Name);
			strcat(data,TempBranch);
		}
	}
	strcat(data,"</includeBranchOrInstanceList>");
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Build the Exclude-Branch List
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	strcat(data,"<excludeBranchOrInstanceList>");
	if (getDataModel->ExcludeBranch_Size >0)
	{
		for(i=0;i<getDataModel->ExcludeBranch_Size;i++)
		{
			sprintf(TempBranch,"<branch branchName=\"%s\"/>",getDataModel->ExcludeBranch_Array[i].Name);
			strcat(data,TempBranch);
		}
	}
	strcat(data,"</excludeBranchOrInstanceList>");
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	strcat(data,"</getLiveData>");

	curl_handle = curl_easy_init();

	if (curl_handle == NULL)
	{
		return FALSE;
	}

	HTTP_LIB_Buffer.Buffer = getDataModel->DataModel_Buffer;
	HTTP_LIB_Buffer.BuffeSize = sizeof(getDataModel->DataModel_Buffer);
	HTTP_LIB_Buffer.Size = 0;

	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);

	/* Define our callback to get called when there's data to be written */ 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_fwriteBuffer);

	/* Set a pointer to our struct to pass to the callback */ 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &HTTP_LIB_Buffer);

	/* Switch on full protocol/debug output */ 
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);

	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, getDataModel->ErrorDescription);

	curl_easy_setopt(curl_handle, CURLOPT_URL, strURL);

	/* It should contain the maximum time in seconds that you allow the connection phase to the server to take */
	curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT , HTTP_LIB_MAX_CONNECTTIMEOUT);

	/* Abort if the average transfer speed during the last 60 seconds is slower than 20 bytes/sec */
	/* In other words, if we get less than 1200 bytes (a packet) for the last minute */
	curl_easy_setopt(curl_handle, CURLOPT_LOW_SPEED_TIME, 60L);
	curl_easy_setopt(curl_handle, CURLOPT_LOW_SPEED_LIMIT, 20L);

	if (getDataModel->bHasNetworkCardIP)
	{
		/* This sets the interface name to use as outgoing network interface */
		curl_easy_setopt(curl_handle, CURLOPT_INTERFACE , getDataModel->NetworkCardIP);
	}

	res = curl_easy_perform(curl_handle);

	if (res == CURLE_COULDNT_CONNECT)
	{
		/* This clears the interface name - Fallback */
		curl_easy_setopt(curl_handle, CURLOPT_INTERFACE , NULL);

		res = curl_easy_perform(curl_handle);
	}

	/* always cleanup */
	curl_easy_cleanup(curl_handle);

	if (res != CURLE_OK)
	{
		return FALSE;
	}

	getDataModel->DataModel_Size = HTTP_LIB_Buffer.Size;

	if (0)
	{
		FILE* f;

		if ((f = fopen("getDataModel.xml","wb")) != NULL)
		{
			fwrite(getDataModel->DataModel_Buffer,1,getDataModel->DataModel_Size,f);
			fclose(f);
		}
	}

	return TRUE;
}

/***************************************************************************************************
* Set_Data_Model()                                                                                 *
*                                                                                                  *
* Summary:                                                                                         *
*        Send HTTP request to the set parameter in the DataModel and return the result (Buffer).   *
*                                                                                                  *
* Return:                                                                                          *
*        TRUE/FALSE on success/failure                                                             *
***************************************************************************************************/
bool Set_Data_Model(sSet_Data_Mode_lnformation* setDataModel)
{
	CURL*				curl_handle;
	CURLcode			res;
	sHTTP_LIB_Buffer	HTTP_LIB_Buffer;

	char				strURL[HTTP_LIB_MAX_PATH];

	char				data[HTTP_LIB_MAX_BRANCH_NAME_SIZE * HTTP_LIB_MAX_BRANCHS] = "";
	char				TempBranch[HTTP_LIB_MAX_BRANCH_NAME_SIZE];
	UINT32				i;


	if (setDataModel->SetParameter_Size == 0)
	{
		return FALSE;
	}

	sprintf(strURL,"%s/setData",setDataModel->DeviceIP);

	strcat(data,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

	strcat(data,"<setData xmlns=\"http://www.coppergate.com/AP/PerfDataReq\"\"operName=\"retrieve\" userid=\"00:13:92:AA:BB:CC\" password=\"perfdata\">\n");

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Build the Set-Branch List
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	strcat(data,"<setParameterList>\n");

	for(i=0;i<setDataModel->SetParameter_Size;i++)
	{
		sprintf(TempBranch,"<Parameters Name=\"%s\" Value=\"%s\"/>",
							setDataModel->SetParameter_Array[i].Name,
							setDataModel->SetParameter_Array[i].Value);
		strcat(data,TempBranch);
	}

	strcat(data,"</setParameterList>");
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	strcat(data,"</setData>");

	curl_handle = curl_easy_init();

	if (curl_handle == NULL)
	{
		return FALSE;
	}

	HTTP_LIB_Buffer.Buffer = setDataModel->DataModel_Buffer;
	HTTP_LIB_Buffer.BuffeSize = sizeof(setDataModel->DataModel_Buffer);
	HTTP_LIB_Buffer.Size = 0;

	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data);

	/* Define our callback to get called when there's data to be written */ 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_fwriteBuffer);

	/* Set a pointer to our struct to pass to the callback */ 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &HTTP_LIB_Buffer);

	/* Switch on full protocol/debug output */ 
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);

	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, setDataModel->ErrorDescription);

	curl_easy_setopt(curl_handle, CURLOPT_URL, strURL);

	/* It should contain the maximum time in seconds that you allow the connection phase to the server to take */
	curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT , HTTP_LIB_MAX_CONNECTTIMEOUT);

	/* Abort if the average transfer speed during the last 60 seconds is slower than 20 bytes/sec */
	curl_easy_setopt(curl_handle, CURLOPT_LOW_SPEED_TIME, 60L);
	curl_easy_setopt(curl_handle, CURLOPT_LOW_SPEED_LIMIT, 20L);

	if (setDataModel->bHasNetworkCardIP)
	{
		/* This sets the interface name to use as outgoing network interface */
		curl_easy_setopt(curl_handle, CURLOPT_INTERFACE , setDataModel->NetworkCardIP);
	}

	res = curl_easy_perform(curl_handle);

	/* always cleanup */
	curl_easy_cleanup(curl_handle);

	if (res != CURLE_OK)
	{
		return FALSE;
	}

	setDataModel->DataModel_Size = HTTP_LIB_Buffer.Size;

	if (0)
	{
		FILE* f;

		if ((f = fopen("setDataModel.xml","wb")) != NULL)
		{
			fwrite(setDataModel->DataModel_Buffer,1,setDataModel->DataModel_Size,f);
			fclose(f);
		}
	}

	return TRUE;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */


