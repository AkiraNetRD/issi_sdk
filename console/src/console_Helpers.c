
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "common.h"
#include "console_Helpers.h"
#include "console_CommandFunc.h"

#include "console_PacketHandling.h"
#include "console_Logger.h"

#ifdef __linux__
#include <sys/time.h>
#endif


unsigned long console_read_int()
{
	char buf[64];
	char *endptr;
	unsigned long val;

	memset(buf,'\0',sizeof(buf));
	fgets(buf,sizeof(buf),stdin);
	errno = 0;
	val = strtoul(buf,&endptr,0);
	if ((errno != 0) || (!isspace(*endptr) && ((*endptr) != '\0')))
		return (unsigned long)-1;

	return val;
}

long console_get_msectime()
{
	long mtime = 0;

#ifdef _WIN32
	mtime = clock(); // The elapsed wall-clock time since the start of the process
#endif

#ifdef __linux__
	struct timeval t_current_time;
	long  seconds, useconds;     

	gettimeofday(&t_current_time, NULL);

	seconds  = t_current_time.tv_sec;
	useconds = t_current_time.tv_usec;
	mtime = seconds  *1000 + useconds/1000;
#endif

	return mtime;
}

void LOG_INFO_Highlight(char* Message)
{
	enum Colors { blue=1, green, cyan, red, purple, yellow, grey, dgrey, hblue, hgreen, hcyan, hred, hpurple, hyellow, hwhite };

	#ifdef _WIN32
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(handle,hwhite);
	#endif
	printf("%s",Message);

	#ifdef _WIN32
	SetConsoleTextAttribute(handle,grey);
	#endif
}


/***************************************************************************************************
* Firmware_Reset_Device()                                                                          *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
bool Firmware_Reset_Device(Layer2Connection* layer2Connection, int NewState)
{
	cg_stat_t			status;

	if (NewState == 0)
	{
		LOG_INFO("Reset to Firmware mode");
	}
	else if (NewState == 1)
	{
		LOG_INFO("Reset to EtherBoot mode");
	}
	else
	{
		return FALSE;
	}

	fflush(stdout);
	if ((status = ResetCommandFunction(layer2Connection, NewState)) != CG_STAT_SUCCESS)
	{
		LOG_INFO("FAILED");
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************************************
* Hardware_Reset_Device()                                                                          *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
bool Hardware_Reset_Device(Layer2Connection* layer2Connection, int NewState)
{
	cg_stat_t			status;

	if (NewState == 0)
	{
		LOG_INFO("Reset to Firmware mode");
	}
	else if (NewState == 1)
	{
		LOG_INFO("Reset to EtherBoot mode");
	}
	else
	{
		return FALSE;
	}

	fflush(stdout);
	if ((status = SetMemoryCommandFunctionUINT32(layer2Connection, 0x706C0208,NewState)) != CG_STAT_SUCCESS)
	{
		LOG_INFO("FAILED");
		return FALSE;
	}
	SetMemoryCommandFunctionUINT32NoReply(layer2Connection, 0x61030014,1);

	return TRUE;
}

/***************************************************************************************************
* Debug_Info()                                                                                     *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
bool Console_Debug_Info(Layer2Connection* layer2Connection, char* Message, char* Response, UINT32 MaxResponseTimeOut)
{
	cg_stat_t			status;

	if (Message != NULL)
	{
		LOG_INFO("Message(%s)",Message);

		fflush(stdout);
		if ((status = SetMemoryCommandFunctionString(layer2Connection, VIRTUAL_ADDRESS_UART_COMMAND,Message)) != CG_STAT_SUCCESS)
		{
			LOG_INFO("FAILED");
			return FALSE;
		}
		
		OS_Sleep(250);
	}

	if (Response != NULL)
	{
		if ((status = GetMemoryCommandFunctionString(layer2Connection, VIRTUAL_ADDRESS_UART,Response,MaxResponseTimeOut)) != CG_STAT_SUCCESS)
		{
			strcpy(Response,"");
			LOG_INFO("FAILED");
			return FALSE;
		}
	}

	return TRUE;
}

/***************************************************************************************************
* Debug_Info_Post_Mortem_Analysis()                                                                *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
bool Console_Debug_Info_Post_Mortem_Analysis(	Layer2Connection* layer2Connection,
												char* Response,
												char* SLOG_Response,
												int* SLOG_Response_Size,
												bool bIsEtherBoot)
{
	cg_stat_t			status;
	PBLOCK_s			PBlock;
	char				Buffer[4000];
	int					i;

	UINT32				SLOG_BUFFER_Size = 0;
	UINT32				SLOG_BUFFER_Address = 0;

	strcpy(Response,"");

	sprintf(Buffer,"=========================================================\n"); strcat(Response,Buffer);
	sprintf(Buffer,"Reading PBLOCK...\n"); strcat(Response,Buffer);

	if (bIsEtherBoot)
	{
		status = GetMemoryCommandFunction(layer2Connection, ABSOLUTE_ADDRESS_PBLOCK,sizeof(PBlock),(UINT8*)&PBlock);
	}
	else
	{
		status = GetMemoryCommandFunction(layer2Connection, VIRTUAL_ADDRESS_PBLOCK,sizeof(PBlock),(UINT8*)&PBlock);
	}

	if (status != CG_STAT_SUCCESS)
	{
		sprintf(Buffer,"Failed to read PBLOCK\n"); strcat(Response,Buffer);
		sprintf(Buffer,"=========================================================\n"); strcat(Response,Buffer);
		LOG_INFO("FAILED");
		return FALSE;
	}

	if (PBlock.magic_num != PBLOCK_MAGIC_NUMBER)
	{
		sprintf(Buffer,"Unknown magic number(0x%X)\n",PBlock.magic_num); strcat(Response,Buffer);
		sprintf(Buffer,"=========================================================\n"); strcat(Response,Buffer);
		return TRUE;
	}

	sprintf(Buffer,"Found PBLOCK version(%d)\n",PBlock.version); strcat(Response,Buffer);
	
	if (PBlock.version > PBLOCK_VERSION)
	{
		sprintf(Buffer,"Unknown PBLOCK Version number\n"); strcat(Response,Buffer);
		return TRUE;
	}

	sprintf(Buffer,"reset Counter (%d)\n",PBlock.reset_counter); strcat(Response,Buffer);
	sprintf(Buffer,"reset Index (%d)\n",PBlock.reset_index); strcat(Response,Buffer);

	sprintf(Buffer,"Legend: reset-cause 1=assert 2=watchdog, showing the list from the oldest to the newest.\n"); strcat(Response,Buffer);


	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Read Assertion information
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	for (i=0; i< PBLOCK_MAX_RESET_ENTRIES; i++)
	{
		int index = (PBlock.reset_index + i)  % PBLOCK_MAX_RESET_ENTRIES;

		if (PBlock.reset_entries[index].reset_cause != 0)
		{
			sprintf(Buffer,"reset: index(%d) cause(%d) system_up_time(%d sec) file(%s) line(%d)\n",
								index,
								PBlock.reset_entries[index].reset_cause,
								PBlock.reset_entries[index].system_up_time_os_tick/100,
								PBlock.reset_entries[index].detail.assert.file,
								PBlock.reset_entries[index].detail.assert.line);
			strcat(Response,Buffer);
		}
	}
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	sprintf(Buffer,"continues reset accumulator (%d)\n",PBlock.continues_reset_accumulator); strcat(Response,Buffer);
	sprintf(Buffer,"Total Init (%d)\n",PBlock.total_inits); strcat(Response,Buffer);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// EBL AVL (Active/Valid/Launched)
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	sprintf(Buffer,"\n"); strcat(Response,Buffer);

	sprintf(Buffer,"EBL Version=%d\n", PBlock.ebl.version); strcat(Response,Buffer);

	sprintf(Buffer,"EBL FW1: valid=%d active=%d", PBlock.ebl.FW1_valid, PBlock.ebl.FW1_active); strcat(Response,Buffer);
	if (PBlock.ebl.launched == 0)
	{
		sprintf(Buffer," (launched)"); strcat(Response,Buffer);
	}
	sprintf(Buffer,"\n"); strcat(Response,Buffer);
	sprintf(Buffer,"EBL FW2: valid=%d active=%d", PBlock.ebl.FW2_valid, PBlock.ebl.FW2_active); strcat(Response,Buffer);
	if (PBlock.ebl.launched == 1)
	{
		sprintf(Buffer," (launched)"); strcat(Response,Buffer);
	}
	sprintf(Buffer,"\n"); strcat(Response,Buffer); strcat(Response,Buffer);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	sprintf(Buffer,"=========================================================\n"); strcat(Response,Buffer);
	strcat(Response,"UART Buffer:\n"); strcat(Response,Buffer);

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Read UART Buffer
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	if (bIsEtherBoot)
	{
		status = GetMemoryCommandFunction(layer2Connection, ABSOLUTE_ADDRESS_PBLOCK_UART_BUF,4000,(UINT8*)Buffer);
	}
	else
	{
		status = GetMemoryCommandFunction(layer2Connection, VIRTUAL_ADDRESS_PBLOCK_UART_BUF,4000,(UINT8*)Buffer);
	}
	
	if (status != CG_STAT_SUCCESS)
	{
		sprintf(Buffer,"Failed to read UART buffer\n"); strcat(Response,Buffer);
		sprintf(Buffer,"=========================================================\n"); strcat(Response,Buffer);
		LOG_INFO("FAILED");
		return FALSE;
	}
	strcat(Response,Buffer);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	sprintf(Buffer,"\n"); strcat(Response,Buffer);
	sprintf(Buffer,"=========================================================\n"); strcat(Response,Buffer);

	if (bIsEtherBoot)
	{
		GetMemoryCommandFunctionUINT32(layer2Connection, ABSOLUTE_ADDRESS_PBLOCK_SLOG_BUF_SIZE, &SLOG_BUFFER_Size);

		if (SLOG_BUFFER_Size == 0x00000000)
		{
			SLOG_BUFFER_Size = 65535; // 64Kb
		}

		SLOG_BUFFER_Address = ABSOLUTE_ADDRESS_PBLOCK_SLOG_BUF;
	}
	else // FW-Operational
	{
		if (PBlock.version == 1)
		{
			// Old FW
			SLOG_BUFFER_Size = 65535; // 64Kb
		}
		else // if (PBlock.version >= 2)
		{
			if (GetMemoryCommandFunctionUINT32(layer2Connection, VIRTUAL_ADDRESS_SLOG_BUF_SIZE, &SLOG_BUFFER_Size) != CG_STAT_SUCCESS)
			{
				sprintf(Buffer,"Failed to read SLOG BUFFER SIZE\n"); strcat(Response,Buffer);
				LOG_INFO("FAILED");
				return FALSE;
			}
		}

		SLOG_BUFFER_Address = VIRTUAL_ADDRESS_PBLOCK_SLOG_BUF;
	}

	if (SLOG_BUFFER_Size > 0)
	{
		status = GetMemoryCommandFunction(layer2Connection, SLOG_BUFFER_Address , SLOG_BUFFER_Size, (UINT8*)SLOG_Response);

		if (status != CG_STAT_SUCCESS)
		{
			LOG_INFO("FAILED");
			return FALSE;
		}
	}

	*SLOG_Response_Size = SLOG_BUFFER_Size;

	return TRUE;
}


/***************************************************************************************************
* GetEBLVersion()                                                                                  *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
bool GetEBLVersion(Layer2Connection* layer2Connection, UINT16* EBLVersion)
{
	cg_stat_t			status;
	PBLOCK_s			PBlock;

	if ((status = GetMemoryCommandFunction(layer2Connection, VIRTUAL_ADDRESS_PBLOCK,sizeof(PBlock),(UINT8*)&PBlock)) != CG_STAT_SUCCESS)
	{
		LOG_INFO("FAILED");
		return FALSE;
	}

	// Support Big/Little endian machines
	PBlock.magic_num = letohl(PBlock.magic_num);
	PBlock.version = letohs(PBlock.version);
	PBlock.ebl.version = letohs(PBlock.ebl.version);

	if (PBlock.magic_num != PBLOCK_MAGIC_NUMBER)
	{
		LOG_INFO("FAILED");
		return FALSE;
	}

	if (PBlock.version > PBLOCK_VERSION)
	{
		LOG_INFO("FAILED");
		return TRUE;
	}

	*EBLVersion = PBlock.ebl.version;

	return TRUE;
}

/***************************************************************************************************
* GetChipType()                                                                                  *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
// CHIP bonding map 
//Product   val.	AFE core		Bond[4:0]	Bond[5]
// Golan3:
// CG5311	 2   Coax + MIMO	    00010	
// CG5315   38 	    Coax	    	00110   	1
// CG5321    6	    MIMO		    00110	    0
// CG5325    5	    MIMO		    00100	
// CG5331   22	    SISO		    10110	
// CG5335	20       SISO		    10100	
// Golan 2:
// CG5210    2                      00010
// CG5220   10                      01010
// CG5230   14                      01110

bool GetChipType(Layer2Connection* layer2Connection, char* Device_ChipType)
{
	cg_stat_t			status;
	UINT8				chip_Type;
	UINT32				chip_version;
	UINT32				rom_version;
		
	if ((status = GetMemoryCommandFunctionUINT32(layer2Connection, VERSION_REG, &chip_version)) != CG_STAT_SUCCESS)
	{
		LOG_INFO("Failed to get Chip Type (version)");
		return FALSE;
	}

	if (chip_version == 0x00004152)
	{
		// Golan1

		strcpy(Device_ChipType, "CG5110");
		return TRUE;
	}
	
	if (chip_version == 0x00005210)
	{
		// Golan2/Golan3

		if ((status = GetMemoryCommandFunction(layer2Connection, TOP_RC_BONDING_OPTION_REG,sizeof(chip_Type),(UINT8*)&chip_Type)) != CG_STAT_SUCCESS)
		{
			LOG_INFO("Failed to get Chip Type (bonding)");
			return FALSE;
		}

		chip_Type =chip_Type & 0x3E; // mask  use only bit 2-5 ;Bond0 =don’t care (monitor bus) ; 6-11 bits not used


		if ((status = GetMemoryCommandFunctionUINT32(layer2Connection, ROM_REG, &rom_version)) != CG_STAT_SUCCESS)
		{
			LOG_INFO("Failed to get Chip Type (ROM)");
			return FALSE;
		}
			
		if (rom_version == 0x01000006) 
		{
			// Golan2

			switch (chip_Type)
			{
				case 2:
				{
					strcpy(Device_ChipType,"CG5210"); 
				} break;

				case 10:
				{
					strcpy(Device_ChipType,"CG5220");
				} break;

				case 14:
				{
					strcpy(Device_ChipType,"CG5230");
				} break;
				default:
					LOG_INFO("Failed to determine Chip Type");
					return FALSE;
					break;
			}

			return TRUE;
		}

		if (rom_version == 0x81000011)
		{
			// Golan 3

			switch (chip_Type)
			{
				case 2:
				case 34:
				case 3:  //debug
				case 35:  //debug


				{
					strcpy(Device_ChipType,"CG5311");
				} break;
					
				case 38:
				case 39: //debug

				{
					// COAX100
					strcpy(Device_ChipType,"CG5315");
				} break;

				case 6:
				case 7:  // debug
				{
					strcpy(Device_ChipType,"CG5321");
				} break;

				case 4:
				case 36:
				case 5: //debug
				case 37: //debug
				{
					strcpy(Device_ChipType,"CG5325");
				} break;

				case 22:
				case 54:
				case 23: //debug
				case 55: //debug
				{
					strcpy(Device_ChipType,"CG5331");
				} break;

				case 20:
				case 52:
				case 21: //debug
				case 53: //debug
				{
					strcpy(Device_ChipType,"CG5335");
				} break;

				default:
					LOG_INFO("Failed to determine Chip Type");
					return FALSE;
					break;
			}

			return TRUE;
		}
	}

	return FALSE;
}

unsigned long Get_FUUF_First_Address(char* FileName, UINT32* Address)
{
	FILE*					file;
	UINT8*					Buffer = NULL;

	BIN_image_loader_t*		image;
	long					FileSize;

	LOG_INFO("FileName(%s)",FileName);

	FileSize = file_get_size(FileName);

	Buffer = (UINT8*)malloc(FileSize);
	if (Buffer == NULL)
	{
		printf("Failed to malloc");
		return CG_STAT_INSUFFICIENT_MEMORY;
	}

	image = (BIN_image_loader_t*)Buffer;

	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	// Open the file
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	file = fopen(FileName,"rb");
	if (file == NULL)
	{
		printf("Failed to open the file \"%s\"\n", FileName);
		free(Buffer);
		return CG_STAT_FILE_NOT_FOUND;
	}
	fread(Buffer,1,FileSize,file);
	fclose(file);
	// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

	*Address = image->image_sections[0].address;

	free(Buffer);

	return CG_STAT_SUCCESS;
}
