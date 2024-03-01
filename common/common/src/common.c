
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <sys/stat.h>


#ifdef _WIN32
#include <windows.h>
#include "direct.h"
#endif

#include <errno.h>
#include "common.h"
#include "sigmadesigns.ver"
#ifdef __linux__
static int s_peek = -1;
#endif

#ifdef __linux__
#include <netinet/in.h>
#endif

// Support Windows 8
#ifdef _WIN32
#include <shlobj.h>				// SHGetFolderPath()
#include "Shlwapi.h"			// PathIsRelative
#endif

#ifdef _WIN32
#define COMMON_MAX_PATH MAX_PATH
#define OS_MKDIR(x) _mkdir(x)
#define COMMON_GETCWD(buf,size) _getcwd(buf,size)
#define OS_STRTOK_THREAD_SAFE(x,y) strtok(x,y)
#elif __linux__
#include <linux/limits.h>
#define COMMON_MAX_PATH PATH_MAX
#include <linux/limits.h>
#define OS_MKDIR(x) mkdir(x,0777)
#define OS_STRTOK_THREAD_SAFE(x,y) strtok_r(x,y,&saveptr);
#endif


/********************************************************************************
                                GLOBALS
********************************************************************************/


/********************************************************************************
                          MACRO DEFINITIONS
********************************************************************************/


#define fexit(s)   {status=s;goto exit;}
/********************************************************************************
                    LINUX SPECIFIC IMPLEMENTATION
********************************************************************************/
#ifdef __linux__
/* 
   Linux/Unix Note:
   -----------------

   This is the xNIX way of doing kbhit - note that this code might not be 
   compatible on all xNIX flavours, in any case this is just 
   the user interface implementation of the stats tool and has
   nothing to do with the statistics counters.
*/
int set_term(struct termios *xo_term)
{
    struct termios term;
    /* get term attributes */
    if (tcgetattr(0,xo_term)) {
        fprintf(stderr,"(!) failed to get TERM attr (stat=%d)\n",errno);
        return 0;
    }
    memcpy(&term,xo_term,sizeof(struct termios));
    
    /* modify term */
    term.c_lflag &= ~(ICANON|ECHO|ISIG);
    term.c_cc[VMIN]  = 0;
    term.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &term)) {
        fprintf(stderr,"(!) failed to set TERM attr (stat=%d)\n",errno); 
        return 0;
    }
    return 1;
}

void unset_term(struct termios *xi_term)
{
    if (tcsetattr(0,TCSANOW, xi_term)) {
        fprintf(stderr,"(!) failed to reset TERM (stat=%d)\n",errno); 
    }
}

int get_term(struct termios *xo_term)
{
	/* get term attributes */
	if (tcgetattr(0,xo_term)) {
		fprintf(stderr,"(!) failed to get TERM attr (stat=%d)\n",errno);
		return 0;
	}

	return 1;
}

int check_term()
{
	struct termios term;

	/* get term attributes */
	if (tcgetattr(0,&term))
	{
		return 0;
	}

	// OK
	return 1;
}


int  _kbhit()
{
    char ch;
    int n;
    /* return if peek queue is not empty */
    if(s_peek != -1) return 1;
    n = read(0,&ch,1);
    if(n == 1) {
        s_peek = ch;
        return 1;
    }
    return 0;
}

char _getch()
{
    char ch;
    
    if(s_peek != -1) {
        ch = s_peek;
        s_peek = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}


void _clear_kb(void)
{
	char junk[255];
	fgets (junk,255,stdin);
}

#endif /* __linux__ */


/********************************************************************************
                            IMPLEMENTATION
********************************************************************************/
void version_info()
{
	fprintf(stdout,"SDK %s.%s.%s %s %s\n", CG_MAIN_VERSION_SZ, CG_MINOR_VERSION_SZ, CG_SUB_VERSION_SZ, __TIME__, __DATE__);
}


unsigned long read_int()
{
   char buf[64];
   char *endptr;
   unsigned long val;
   
   memset(buf,'\0',sizeof(buf));
   fgets(buf,sizeof(buf),stdin);
   errno = 0;
   val = strtoul(buf,&endptr,0);
   if (errno != 0)
       return (unsigned long)-1;
   
   return val;
}

char * read_string(char *str,int size)
{
   char *endptr;
   
   memset(str,'\0',size);
   endptr = fgets(str,size,stdin);
   if ((endptr == NULL) && (ferror(stdin)))
       return NULL;

   endptr = strstr(str,"\n");
   *endptr = '\0';

   return str;
}

ip_address_t str_to_ip(char *xi_ip)
{
	char* buffer;
	char *dig;

    BYTE ip[4];
    int  idx;

#ifdef __linux__
	char *saveptr;
#endif

	/* First copy xi_ip to temp buffer */
	buffer = (char*)malloc(strlen(xi_ip)+1);
	strcpy(buffer,xi_ip);

    dig = OS_STRTOK_THREAD_SAFE(buffer,".");
    for (idx=0;idx<4;idx++) {
        if (dig) {
            ip[idx] = (unsigned char)strtoul(dig,NULL,10);
            dig = OS_STRTOK_THREAD_SAFE(NULL,".");
        }
    }
    
	free(buffer);
	
    return (*((ip_address_t*)ip));
}

void ip_to_str(ip_address_t xi_ip, char *xo_ip)
{
	xi_ip = htolel(xi_ip);

	sprintf(xo_ip,"%d.%d.%d.%d",
								(xi_ip >> 0)  & 0xFF,
								(xi_ip >> 8)  & 0xFF,
								(xi_ip >> 16) & 0xFF,
								(xi_ip >> 24) & 0xFF);
}

bool is_hexdigit(char c)
{
	if ( (c>='0') && (c<='9') )
		return TRUE;
	if ( (c>='a') && (c<='f') )
		return TRUE;
	if ( (c>='A') && (c<='F') )
		return TRUE;

	return FALSE;
}

bool is_valid_str_ip(char *xi_ip)
{
	int  i;
	int	a,b,c,d;

	i = sscanf(xi_ip,"%d.%d.%d.%d",&a,&b,&c,&d);

	// Check number of args
	if (i != 4)
	{
		return FALSE;
	}

	// Check range
	if ((a<0) || (a>255) || (b<0) || (b>255) || (c<0) || (c>255) || (d<0) || (d>255))
	{
		return FALSE;
	}

	return TRUE;
}

bool is_valid_str_mac(char *xi_mac)
{
    int  i;

	// Check length
	if (strlen(xi_mac) != 17)
	{
		return FALSE;
	}

	for (i=0;i<17;i++)
	{
		if ((i+1) % 3 == 0)
		{
			// Check Delimiter
			if ((xi_mac[i] != ':') && (xi_mac[i] != '-')) return FALSE;
		}
		else
		{
			// Check HexDigit
			if (!is_hexdigit(xi_mac[i])) return FALSE;
		}
	}

	return TRUE;
}

void str_to_mac(char *xi_mac, mac_address_t *xo_mac)
{
	char* buffer;
	char *dig;
	int  idx;

#ifdef __linux__
	char *saveptr;
#endif

	/* First copy xi_mac to temp buffer */
	buffer = (char*)malloc(strlen(xi_mac)+1);
	strcpy(buffer,xi_mac);

	dig = OS_STRTOK_THREAD_SAFE(buffer,":-");
	for (idx=0;idx<6;idx++) {
		if (dig) {
			(xo_mac[0])[idx] = (unsigned char)strtoul(dig,NULL,16);
			dig = OS_STRTOK_THREAD_SAFE(NULL,":-");
		}
	}

	free(buffer);
}

bool file_exists(const char * filename)
{
	FILE* file;
	if ((file = fopen(filename, "r")) != NULL)
	{
		fclose(file);
		return TRUE;
	}
	return FALSE;
}

bool folder_exists(const char * folder)
{
	struct stat St;
	bool	RetVal;

	RetVal = (stat(folder, &St) == 0);

	return RetVal;
}

long file_get_size(const char* filename)
{
	FILE* file;
	long size;

	if ((file = fopen(filename, "r")) != NULL)
	{
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fclose(file);
		return size;
	}

	return 0;
}

bool file_get_Modified_Time(const char * FileName, time_t* mtime)
{
	struct stat St;
	bool	RetVal;

	RetVal = (stat(FileName, &St) == 0);

	*mtime = St.st_mtime;

	return RetVal;
}

bool CopyBinaryFile(char* InputFile,char* OuputFile)
{
	FILE *f_in;
	FILE *f_out;
	char buffer[1024];
	int bytesread;

	if ((f_in = fopen(InputFile, "rb")) == NULL)
	{
		return FALSE;
	}

	if ((f_out = fopen(OuputFile, "wb")) == NULL)
	{
		fclose(f_in);
		return FALSE;
	}

	/* copy file */
	while((bytesread=(int)fread(buffer, 1, sizeof(buffer), f_in)) >0)
		fwrite(buffer, 1, bytesread, f_out);

	fclose(f_in);
	fclose(f_out);

	return TRUE;
}

bool isStrTerminal(char* str,int MaxLength)
{
	int i;

	for (i=0 ; i<MaxLength; i++)
	{
		if (str[i] == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

unsigned char CHAR2HEX(char c) {	if (c>='0' && c<='9') return c-'0'; 
									if (c>='A' && c<='F') return c-'A'+10; 
									if (c>='a' && c<='f') return c-'a'+10; 
									return c=0; } 

// Attention: Treat "StrBuffer"
void ConvertHexString_to_Buffer(char* StrBuffer,UINT32* BufferLength,char* Buffer)
{
	int		StrIndex = 0;
	int		StrLength = 0;
	int		BufferIndex = 0;
	UINT32	i;

	if ((StrBuffer[0]=='0') && ((StrBuffer[1]=='x') || (StrBuffer[1] == 'X')))
	{
		StrIndex+=2; // Skip "0x"/"0X" Prefix
	}

	StrLength = (int)strlen(&StrBuffer[StrIndex]);

	*BufferLength = StrLength / 2;

	if ((StrLength % 2) != 0)
	{
		*BufferLength = *BufferLength +1;
	}

	for (i=1; i <=*BufferLength ; i++)
	{
		if ((i==1) && ((StrLength % 2) != 0))
		{
			// Take only one Nibble
			Buffer[BufferIndex] = CHAR2HEX(StrBuffer[StrIndex]);
			StrIndex+=1;
		}
		else
		{
			// Take one Byte
			Buffer[BufferIndex] = (CHAR2HEX(StrBuffer[StrIndex]) << 4) + CHAR2HEX(StrBuffer[StrIndex+1]);
			StrIndex+=2;
		}

		BufferIndex++;
	}
}

// Attention: Treat "StrBuffer" as Little Endian
void ConvertHexString_to_Buffer_Little_Endian(char* StrBuffer,UINT32* BufferLength,char* Buffer)
{
	int		StrIndex = 0;
	int		StrLength = 0;
	int		BufferIndex = 0;
	UINT32	i;

	if ((StrBuffer[0]=='0') && ((StrBuffer[1]=='x') || (StrBuffer[1] == 'X')))
	{
		StrIndex+=2; // Skip "0x"/"0X" Prefix
	}

	StrLength = (int)strlen(&StrBuffer[StrIndex]);

	*BufferLength = StrLength / 2;

	if ((StrLength % 2) != 0)
	{
		*BufferLength = *BufferLength +1;
	}

	for (i=1; i <=*BufferLength ; i++)
	{
		if ((i==1) && ((StrLength % 2) != 0))
		{
			// Take only one Nibble
			Buffer[*BufferLength-1 - BufferIndex] = CHAR2HEX(StrBuffer[StrIndex]);
			StrIndex+=1;
		}
		else
		{
			// Take one Byte
			Buffer[*BufferLength-1 - BufferIndex] = (CHAR2HEX(StrBuffer[StrIndex]) << 4) + CHAR2HEX(StrBuffer[StrIndex+1]);
			StrIndex+=2;
		}

		BufferIndex++;
	}
}

void Printf_Highlight(char* Message)
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

void PrintProgressIndication(int c, int C, bool FirstTime, bool LastTime)
{
	int i;

	if (!FirstTime)
	{
		for (i=1;i<=(C+2);i++)
		{
			printf("\b");
		}
	}

	if (LastTime)
	{
		for (i=1;i<=(C+2);i++)
		{
			printf(" ");
		}
		for (i=1;i<=(C+2);i++)
		{
			printf("\b");
		}
		return;
	}

	Printf_Highlight("[");
	for (i=1;i<=C;i++)
	{
		if (i<=c)
		{
			Printf_Highlight("o");
		}
		else
		{
			Printf_Highlight(".");
		}

	}
	Printf_Highlight("]");
}

void UpdateProgressIndication(macStruct MAC_Address,int Progress)
{
	FILE*	f;
	char	FileName[COMMON_MAX_PATH];

	sprintf(FileName,"ProgressIndication_"MAC_ADDR_NO_COLON".tmp",MAC_ADDR(MAC_Address.macAddress));

	f = fopen(FileName,"w");

	if (f != NULL)
	{
		fprintf(f,"%d",Progress);

		fclose(f);
	}
}
void DeleteProgressIndication(macStruct MAC_Address)
{
	char	FileName[COMMON_MAX_PATH];

	sprintf(FileName,"ProgressIndication_"MAC_ADDR_NO_COLON".tmp",MAC_ADDR(MAC_Address.macAddress));

	remove(FileName);
}

int GetProgressIndication(macStruct MAC_Address)
{
	FILE*	f;
	char	FileName[COMMON_MAX_PATH];
	int		Progress = 0;

	sprintf(FileName,"ProgressIndication_"MAC_ADDR_NO_COLON".tmp",MAC_ADDR(MAC_Address.macAddress));

	f = fopen(FileName,"r");

	if (f != NULL)
	{
		fscanf(f,"%d",&Progress);

		fclose(f);
	}

	return Progress;
}

#ifdef _WIN32
int system_silent(char* Command)
{
	STARTUPINFO siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	FILE*				fp;

	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);

	// Hide the output window
	siStartupInfo.dwFlags = STARTF_USESHOWWINDOW;	// Enable windows mode control
	siStartupInfo.wShowWindow = SW_HIDE;			// Start terminal as minimized

	// Use a batch file to support redirection of output (i.e "> output.txt" in the Command)
	if ((fp=fopen("__tmp.bat","w")) == NULL)
	{
		return -1;
	}

	fprintf(fp,"%s\n",Command);
	fclose(fp);

	// Create the process
	if (!CreateProcess(NULL, "__tmp.bat", NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &siStartupInfo, &piProcessInfo))
	{
		DeleteFile("__tmp.bat");
		return -1;
	}

	// Wait for the thread to stop
	WaitForSingleObject(piProcessInfo.hProcess, INFINITE);

	DeleteFile("__tmp.bat");

	CloseHandle(piProcessInfo.hProcess);
	CloseHandle(piProcessInfo.hThread);

	return 0;
}
#endif

#ifdef __linux__
int system_silent(char* Command)
{
	system(Command);

	return 0;
}
#endif

// ATTEN: StrBuffer should contains even number of HexDigits
bool Convert_String_To_Array(	char*	StrBuffer,
								int		MaxLength,
								UINT8*	Array)
{
	int StrLength;
	int StrIndex = 0;
	int BufferIndex = 0;

	StrLength = (int)strlen(StrBuffer);

	if ((StrBuffer[0]=='0') && ((StrBuffer[1]=='x') || (StrBuffer[1] == 'X')))
	{
		StrIndex+=2; // Skip "0x"/"0X" Prefix
	}

	while (StrIndex < (StrLength-1))
	{
		UINT8 High_Nibble;
		UINT8 Low_Nibble;

		while ((StrBuffer[StrIndex] == ':') || (StrBuffer[StrIndex] == '-'))
		{
			StrIndex++;
		}

		High_Nibble = CHAR2HEX(StrBuffer[StrIndex++]);

		while ((StrBuffer[StrIndex] == ':') || (StrBuffer[StrIndex] == '-'))
		{
			StrIndex++;
		}

		Low_Nibble = CHAR2HEX(StrBuffer[StrIndex++]);

		if (BufferIndex >= MaxLength)
		{
			return FALSE;
		}

		Array[BufferIndex++] = (High_Nibble << 4) + Low_Nibble;
	}

	return TRUE;
}

void mkdir_recursive(char *path)
{
	char*		ptrPath;			// Where to search for the next slash
	char*		ptrSlash;			// Found some slash

	ptrPath = path;

	while ((ptrSlash = strchr(ptrPath, '/')) != NULL)
	{
		*ptrSlash = '\0';
		OS_MKDIR(path);
		*ptrSlash = '/';
		ptrSlash++;
		ptrPath = ptrSlash;
	}

	OS_MKDIR(path);
}

void mkdir_recursive_BackSlash(char *path)
{
	char*		ptrPath;			// Where to search for the next slash
	char*		ptrSlash;			// Found some Backslash

	ptrPath = path;

	while ((ptrSlash = strchr(ptrPath, '\\')) != NULL)
	{
		*ptrSlash = '\0';
		OS_MKDIR(path);
		*ptrSlash = '\\';
		ptrSlash++;
		ptrPath = ptrSlash;
	}

	OS_MKDIR(path);
}

#ifdef _WIN32

// Support Windows 8
BOOL Check_If_Directory_Has_Write_Permission(char* Folder)
{
	char		FileName[COMMON_MAX_PATH];
	FILE*		f;

	sprintf(FileName, "%s\\SigmaDesigns.tmp", Folder);

	if ((f = fopen(FileName, "w")) == NULL)
	{
		return FALSE;
	}

	fclose(f);
	remove(FileName);

	return TRUE;
}

// Support Windows 8
void Check_Write_Permission_And_Update_Output_Folder(char* Folder)
{
	char		my_documents[COMMON_MAX_PATH];
	char		OutputFolder[COMMON_MAX_PATH];
	char		AbsolutePath[COMMON_MAX_PATH];
	HRESULT		result;

	// Prepare the Absolute-Path
	_fullpath(AbsolutePath, Folder, COMMON_MAX_PATH);
	
	// Create the Absolute-Path on disk
	mkdir_recursive_BackSlash(AbsolutePath);

	if (Check_If_Directory_Has_Write_Permission(Folder) == TRUE)
	{
		// Convert to the Absolute-Path
		strcpy(Folder, AbsolutePath);
		return;
	}

	// Check if "FileName" path is Absolute
	if (PathIsRelative(Folder) == FALSE)
	{
		// Path is Absolute
		// Nothing that we can do
		return;
	}

	// As a last resort, update the "FileName" to "MY_DOCUMENTS"
	if ((result = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, my_documents)) == S_OK)
	{
		strcpy(OutputFolder, my_documents);

		strcat(OutputFolder, "\\");
		strcat(OutputFolder, "Sigma Designs");
		OS_MKDIR(OutputFolder);

		strcat(OutputFolder, "\\");
		strcat(OutputFolder, "G.hn");
		OS_MKDIR(OutputFolder);

		strcat(OutputFolder, "\\");
		strcat(OutputFolder, Folder);

		// Convert to the Absolute-Path
		_fullpath(AbsolutePath, OutputFolder, COMMON_MAX_PATH);
		strcpy(Folder, AbsolutePath);
		return;
	}
}

#elif __linux__

void Check_Write_Permission_And_Update_Output_Folder(char* Folder)
{
}

#endif


// Stores the trimmed input string into the given output buffer, which must be
// large enough to store the result.  If it is too small, the output is truncated.
size_t Trim_Whitespaces(char *out, size_t len, const char *str)
{
	const char *end;
	size_t out_size;

	if(len == 0)
		return 0;

	// Trim leading space
	while(isspace(*str)) str++;

	if(*str == 0)  // All spaces?
	{
		*out = 0;
		return 1;
	}

	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) end--;
	end++;

	// Set output size to minimum of trimmed string length and buffer size minus 1
	out_size = (end - str) < len-1 ? (end - str) : len-1;

	// Copy trimmed string and add null terminator
	memcpy(out, str, out_size);
	out[out_size] = 0;

	return out_size;
}

void str_replace_inline(char *str, char *orig, char *rep)
{
	static char buffer[4096];
	char *p;

	while ((p = strstr(str, orig)) != NULL)
	{
		strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
		buffer[p-str] = '\0';

		sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

		strcpy(str, buffer);
	}
}

