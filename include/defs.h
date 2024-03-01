#ifndef defs_h__
#define defs_h__

/********************************************************************************
                            BUILD DEFINITIONS
********************************************************************************/

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Data Types
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// typedef signed char             SINT1;
// typedef unsigned char           UINT1;
// 
// typedef signed short int        SINT2;
// typedef unsigned short int      UINT2;


typedef signed char             INT8;
typedef signed char             SINT8;

typedef signed short int        INT16;
typedef signed short int        SINT16;

typedef signed int              INT32;
typedef signed int              SINT32;

typedef float                   SFLOAT;
typedef signed long long        SLONG64;
typedef signed long int         SLONG32;
typedef double                  SDOUBLE;

typedef unsigned char           UINT8;
typedef unsigned short int      UINT16;
typedef unsigned int            UINT32;
typedef unsigned long long      ULONG64;

typedef long long int           INT64;

typedef unsigned char			BYTE;

#ifndef __cplusplus
typedef	char					bool;
//typedef unsigned char           byte;
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#if defined(_MSC_VER)
#define strtoll _strtoi64
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

/********************************************************************************
                        ENDIANESS DEFINITIONS
********************************************************************************/
#define swap16(n)                       ((UINT16)((((n)<<8)&0xFF00)|(((n)>>8)&0x00FF)))
#define swap32(n)                       (((swap16((n)&0xFFFF)<<16)&0xFFFF0000)| \
                                        (swap16(((n)>>16)&0xFFFF)&0x0000FFFF))

/*
 *	Following is a set of Host TO Little Endian 
 *  macros to support Big/Little endian machines
 *  If the target machine is big endian, please set
 *  BIG_ENDIAN_MACHINE symbol to 1
 */
#ifdef BIG_ENDIAN_MACHINE   
#define htoles(n)                       swap16(n)   /* convert host to little endian short */
#define htolel(n)                       swap32(n)   /* convert host to little endian long */
#define letohs(n)                       swap16(n)   /* convert little endian to host short */
#define letohl(n)                       swap32(n)   /* convert little endian to host long */
#define htobes(n)                       (n)                        
#define htobel(n)                       (n)
#define betohs(n)                       (n)
#define betohl(n)                       (n)
#else
#define htoles(n)                       (n)                        
#define htolel(n)                       (n)
#define letohs(n)                       (n)
#define letohl(n)                       (n)
#define htobes(n)                       swap16(n)   /* convert host to big endian short */
#define htobel(n)                       swap32(n)   /* convert host to big endian long */
#define betohs(n)                       swap16(n)   /* convert big endian to host short */
#define betohl(n)                       swap32(n)   /* convert big endian to host long */
#endif /* BIG_ENDIAN_MACHINE */


#ifdef __GNUC__
#define PACKED( structure_to_pack ) structure_to_pack __attribute__((__packed__))
#else
#define PACKED( structure_to_pack ) __pragma( pack(push, 1) ) structure_to_pack __pragma( pack(pop) )
#endif

#endif // defs_h__
