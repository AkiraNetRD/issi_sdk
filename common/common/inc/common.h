
#ifndef _COMMON_H
#define _COMMON_H

#include "defs.h"

#include <stdio.h>

#ifdef __linux__
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>
#endif 

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************************
                          MACRO DEFINITIONS
********************************************************************************/
#define MAC_ADDR_FMT            "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_ADDR(p)             (p)[0],(p)[1],(p)[2],(p)[3],(p)[4],(p)[5]

#define MAC_ADDR_NO_COLON		"%02x%02x%02x%02x%02x%02x"

#define MAX_FILE_LINE_LEN       4096
#define EMPTY_STR               ""
#define SINGLE                  1

#define HMAC_LEN			6

typedef struct
{
	UINT8 macAddress[HMAC_LEN];
} macStruct;

/*
 *	MAC Address
 */
typedef UINT8       mac_address_t[6];

/*
 *	IPv4 Address
 */
typedef UINT32      ip_address_t;

/********************************************************************************
                            HELPER ROUTINES
********************************************************************************/
char*           read_string(char *str,int size);
unsigned long   read_int();
ip_address_t    str_to_ip(char *xi_ip);
void			ip_to_str(ip_address_t xi_ip, char *xo_ip);
bool			is_valid_str_ip(char *xi_ip);
bool			is_valid_str_mac(char *xi_mac);
void            str_to_mac(char *xi_mac, mac_address_t *xo_mac);
void			version_info();
bool			file_exists(const char * filename);
bool			folder_exists(const char * folder);
long			file_get_size(const char* filename);
bool			file_get_Modified_Time(const char * FileName, time_t* mtime);
bool			CopyBinaryFile(char* InputFile,char* OuputFile);
bool			isStrTerminal(char* str,int MaxLength);
unsigned char	CHAR2HEX(char c);
void			ConvertHexString_to_Buffer(char* StrBuffer,UINT32* BufferLength,char* Buffer);
void			ConvertHexString_to_Buffer_Little_Endian(char* StrBuffer,UINT32* BufferLength,char* Buffer);
void			Printf_Highlight(char* Message);

void			PrintProgressIndication(int c, int C, bool FirstTime, bool LastTime);
void			UpdateProgressIndication(macStruct MAC_Address,int Progress);
void			DeleteProgressIndication(macStruct MAC_Address);
int				GetProgressIndication(macStruct MAC_Address);

int				system_silent(char* Command);

bool			Convert_String_To_Array(char*	StrBuffer,
										int		MaxLength,
										UINT8*	Array);

void			mkdir_recursive(char *path);
void			mkdir_recursive_BackSlash(char *path);

char*			strptime(const char *buf, const char *fmt, struct tm *tm);

// Support Windows 8
void			Check_Write_Permission_And_Update_Output_Folder(char* Folder);

size_t			Trim_Whitespaces(char *out, size_t len, const char *str);
void			str_replace_inline(char *str, char *orig, char *rep);

#ifdef __linux__
int     set_term(struct termios *xo_term);
void    unset_term(struct termios *xi_term);
int		get_term(struct termios *xo_term);
int		check_term();
int     _kbhit();
char    _getch();
void	_clear_kb(void);
#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _COMMON_H */
