
#ifndef HTTP_LIB_ext_h__
#define HTTP_LIB_ext_h__

#include "HTTP_LIB_typedef.h"

#ifdef _WIN32
#define DllExport   __declspec( dllexport ) 
#else
#define DllExport
#endif

#ifdef __cplusplus
extern "C" {
#endif

DllExport bool Get_Data_Model(sGet_Data_Mode_lnformation* getDataModel);
DllExport bool Set_Data_Model(sSet_Data_Mode_lnformation* setDataModel);

#ifdef __cplusplus
}
#endif


#endif // HTTP_LIB_ext_h__
